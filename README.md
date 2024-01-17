# copy
Barebones copy of coreutils 'cp'

## Compiling
Use `build.sh`:
```sh
$ chmod +x ./build.sh
$ ./build.sh
```

## Usage
Copy a file into another:
```sh
$ ./copy src-file dst-file
```

Copy a directory into another:
```sh
$ ./copy src-dir dst-dir
```
You may notice no `-r` is needed. This is a design choice so that the user can copy directories without any extra options.
