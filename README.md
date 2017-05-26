# hack_malloc
Fun with `malloc()` and `free()`. Tested on Linux and macOS.

* `hack_malloc.c` - super inefficient but working implementation.
* `override_malloc.c` - demonstration how to intercept `malloc()` and `free()`.
* `trace_malloc.c` - advanced heap interceptor which writes the log to disk.
* `model.py` - [Keras](https://keras.io/) RNN model which predicts `malloc()` and `free()` taking the log from `libtracemalloc.so` as the input.

Статьи на русском: [1](https://habrahabr.ru/company/mailru/blog/281497/), [2](https://habrahabr.ru/company/mailru/blog/329518/).

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

python3 model.py --input log.txt.gz --epochs 1 --type GRU --validation 0.15
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

### License
MIT license.
