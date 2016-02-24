# hack_malloc
malloc() / free() PoC implementation

### How to build
Linux:
```
make -f Makefile.linux
```
Mac OS X:
```
make -f cp Makefile.darwin
```

### How to run
```
./test
```

### How to inject
Linux:
```
LD_PRELOAD=/path/to/libhackmalloc.so ./program
```
Mac OS X:
```
DYLD_INSERT_LIBRARIES=/path/to/libhackmalloc.so ./program
```
