# FreeBSD's SH

This is a Linux-port of FreeBSD's POSIX-compatible
Ash-shell (“Almquist-Shell”).

# Why not use Dash?

Dash (“Debian's Almquist Shell”) is intended for non-interactive
use, while FreeBSD's `sh` includes enhancements making it usable
in interactive mode.

# Build

You'll need `libedit` and `libbsd` as dependencies.

From the top-level source directoy do
```
mkdir build
cd build
cmake /..
make
```

The resulting binary will be `src/sh`. Currently, there is no
code for installation yet, so you'll have to do it manually.
