edx
===
Edx is a small Wordstar-like text editor for X11.

Building
--------
```sh
$ ./configure
$ make
$ sudo make install
```

X11 flags will be detected using
[`pkg-config`](https://www.freedesktop.org/wiki/Software/pkg-config/).

If you don't have `pkg-config`, X11 flags can be manually set using
the `--cflags` and `--ldflags` options to `configure`.
