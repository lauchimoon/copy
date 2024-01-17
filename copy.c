// Copyright 2024 Laurentino Luna <laurentino.luna06@gmail.com>

// copy.c: barebones copy of coreutils 'cp'
//
// MIT License
//
// Copyright (c) 2024 Laurentino Luna
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <libgen.h>

int get_mode(const char *file)
{
    struct stat sb;
    if (stat(file, &sb) < 0) {
        return -1;
    }

    return sb.st_mode;
}

// https://unix.stackexchange.com/a/74843
void mkdir_recursive(const char *path)
{
    char *subpath, *fullpath;

    // Duplicate string
    fullpath = strdup(path);
    subpath = dirname(fullpath);
    if (strlen(subpath) > 1) {
        mkdir_recursive(subpath);
    }

    mkdir(path, 0755);
    free(fullpath);
}

int copy_file_contents(const char *src, const char *dst)
{
    if (strcmp(src, dst) == 0) {
        return 1;
    }

    char c;
    FILE *f1 = fopen(src, "r");
    if (f1 == NULL) {
        return 2;
    }
    FILE *f2 = fopen(dst, "w");

    while ((c = fgetc(f1)) != EOF) {
        fprintf(f2, "%c", c);
    }

    fclose(f2);
    fclose(f1);

    return 0;
}

// https://stackoverflow.com/a/8438663
int copy_dirs(const char *root, const char *dst)
{
    DIR *d = opendir(root);
    struct dirent *dir;

    if (!d) {
        return 1;
    }

    if (strcmp(root, dst) == 0) {
        return 2;
    }

    while ((dir = readdir(d)) != NULL) {
        char path_root[1024];
        char path_dst[1024];
        if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0) {
            continue;
        }
        snprintf(path_root, sizeof(path_root), "%s/%s", root, dir->d_name);
        snprintf(path_dst, sizeof(path_dst), "%s/%s", dst, dir->d_name);

        int mode = get_mode(path_root);
        if (S_ISREG(mode)) {
            copy_file_contents(path_root, path_dst);
        } else if (S_ISDIR(mode)) {
            mkdir_recursive(path_dst);
            copy_dirs(path_root, path_dst);
        }
    }

    closedir(d);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc < 3) {
        printf("usage: copy <src-operand> <dst-operand>\n");
        printf("  where operand can be either a file or a directory\n");
        return 1;
    }

    const char *src = argv[1];
    const char *dst = argv[2];

    int src_ret = get_mode(src);
    if (src_ret < 0) {
        printf("copy: operand '%s' does not exist.\n", src);
        return 1;
    }
    int dst_ret = get_mode(dst);

    // Operand dst does not exist so create it
    if (dst_ret < 0) {
        if (S_ISREG(src_ret)) {
            FILE *tmp = fopen(dst, "w");
            fclose(tmp);
        } else if (S_ISDIR(src_ret)) {
            int ret = mkdir(dst, 0755);
            if (ret < 0) {
                printf("copy: failed to make directory '%s'\n", dst);
                return 1;
            }
        }
    }

    // Update the file mode, as it now exists
    dst_ret = get_mode(dst);

    // What if ret1 and ret2 have different modes?
    if (src_ret != dst_ret) {
        printf("copy: '%s' and '%s' have different types.\n", src, dst);
        return 1;
    }

    if (S_ISREG(src_ret)) {
        // If src is a file, copy file contents to dst file
        int ret = copy_file_contents(src, dst);
        if (ret == 1) {
            printf("copy: files '%s' and '%s' are the same.\n", src, dst);
        }
        if (ret == 2) {
            printf("copy: file '%s' does not exist.\n", src);
        }

        return ret;
    } else if (S_ISDIR(src_ret)) {
        // If src is a directory, copy all files into dst dir recursively
        int ret = copy_dirs(src, dst);
        if (ret == 1) {
            printf("copy: directory '%s' does not exist.\n", src);
        }
        if (ret == 2) {
            printf("copy: directories '%s' and '%s' are the same.\n", src, dst);
        }

        return ret;
    }

    return 0;
}
