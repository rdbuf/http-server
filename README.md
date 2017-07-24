# http-server
It is a simple lib for writing http services. Uses asio, pegtl, fmtlib, range-v3.

## Building the demo
```bash
$ git clone --recursive -j5 git@github.com:rdbuf/http-server.git
$ cd http-server
$ mkdir build
$ cd build
$ cmake ..
$ make && test/example-server &
$ chromium http://localhost:1970
```

---------------

```
TODO
[x] POST payload support
[x] distribution as a shared library
[ ] multithreading
[ ] performance
[ ] .vscode
[ ] keep-alive support
[ ] asio exception handling
[ ] SSL
```
