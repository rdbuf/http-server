# libhttpservice
It is a simple lib for writing http services. It uses asio, pegtl, fmtlib, range-v3.

## Build example
```bash
$ git clone --recursive -j5 git@github.com:rdbuf/http-server.git
$ cd http-server
$ mkdir build
$ cd build
$ cmake ..
$ make && example/example-server &
$ xdg-open http://localhost:1970
```

---------------

```
TODO
[x] POST payload support
[x] distribution as a shared library
[x] multithreading
[ ] profile
[ ] fix that segfault on ~6160 request when running single-threaded
[ ] exceptions handling
[ ] keep-alive support
[ ] SSL
```
