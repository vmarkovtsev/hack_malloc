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
LD_PRELOAD=/path/to/liboverridemalloc.so ./program
```
Mac OS X:
```
DYLD_INSERT_LIBRARIES=/path/to/liboverridemalloc.dylib DYLD_FORCE_FLAT_NAMESPACE=1 ./program
```
Note for Mac OS X: due to [SIP](https://en.wikipedia.org/wiki/System_Integrity_Protection), programs at /usr/bin, e.g., ls, are not going to be affected.
