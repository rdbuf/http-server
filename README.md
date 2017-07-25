# http-server
It is a simple lib for writing http services. Uses asio, pegtl, fmtlib, range-v3.

# Building the demo
```bash
$ git clone git@github.com:rdbuf/http-server.git
$ cd http-server
$ git submodule update
$ mkdir build
$ cd build
$ cmake ..
$ make && demo/example-server &
$ chromium http://localhost:1970
```

# TODO
[ ] POST payload support
[ ] keep-alive support
[ ] distribution as a shared library
[ ] asio exception handling
[ ] SSL
