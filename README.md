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

You can disable Valgrind when running tests:
```bash
make test DO_MEMCHECK=false
```

## Features

For now, it's only a [normal Polish notation](https://en.wikipedia.org/wiki/Polish_notation) interpreter.

## TODO

- [x] Rewrite the parser myself (see llexer & lparser);
- [x] Implement S-Expressions (chapter 9);
- [ ] Implement Q-Expressions (chapter 10);
- [ ] Implement variables (chapter 11);
- [ ] Implement functions (chapter 12);
- [ ] Implement conditionals (chapter 13);
- [ ] Implement strings (chapter 14);
- [ ] Implement standard library (chapter 15);
- [ ] Implement user defined types;
- [ ] Implement list literal;
- [ ] Implement OS interaction;
- [ ] Implement pool allocation for lval;
- [ ] Implement variables hashtable;
- [ ] Implement garbage collection;
- [ ] Implement tail call optimisation;
- [ ] Implement lexical scoping;
- [ ] Implement static typing;
- [ ] Support bigdouble;
- [ ] Support rational numbers.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
