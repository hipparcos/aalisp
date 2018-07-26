# aalisp

A dialect of Lisp written in C (version C11).
WIP...

## Getting started

### Prerequisites

Building:

- GNU Compiler Collection (gcc);
- GNU Make (make);
- GNU C Library (glibc/getopt): CLI flags parsing.

Testing:

- Valgrind (valgrind): memory leaks detection;

### Dependancies

Included under vendor/:

- mini-gmp: bignum arithmetic;
- [mortie/snow](https://github.com/mortie/snow): unit testing.

### Building & running

```bash
git clone https://github.com/hipparcos/aalisp.git
cd aalisp
git submodule init
git submodule update
make
./aalisp
```

### Running the tests

```bash
make test
```

## Features

For now, it's only a [normal Polish notation](https://en.wikipedia.org/wiki/Polish_notation) interpreter.

## TODO

    - [x] Rewrite the parser myself (see llexer & lparser).
    - [ ] Support bigdouble.
    - [ ] Support rational numbers.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
