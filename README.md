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
- minunit: minimal unit testing header only library;
- mpc: a parser combinator library for C.

### Building & running

```bash
git clone https://github.com/hipparcos/aalisp.git
cd aalisp
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

    - [ ] Rewrite the parser myself.
    - [ ] Support bigdouble.
    - [ ] Support rational numbers.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
