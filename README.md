# dialecte

*dialecte* \dja.lɛkt\ is a dialect of Lisp written in C (revision C11).
Its goals are simplicity and correctness.

*dialecte* is a work in progress.
It started when I read [Build Your Own Lisp](http://www.buildyourownlisp.com/).
It's an excuse to develop in C, my favourite language so far.

## Getting started

### Prerequisites

Building:
- GNU Compiler Collection (gcc);
- GNU Make (make);
- GNU C Library (glibc/getopt): CLI flags parsing;
- GNU Readline (readline-x.x): CLI interface.

Testing:
- Valgrind (valgrind): memory leaks detection.

### Dependancies

Included under vendor/:
- mini-gmp: bignum arithmetic;
- [mortie/snow](https://github.com/mortie/snow): unit testing.

### Building & running

*dialecte* has only been compiled on Arch Linux so far.

```bash
git clone https://github.com/hipparcos/dialecte.git
cd dialecte
git submodule init
git submodule update
make
./dialecte
```

### Running the tests

```bash
make test
```

Some tests use fmemopen which is not part of C11 standard.

You can disable Valgrind when running tests:
```bash
make test DO_MEMCHECK=false
```

## Features

See [DOC.md](https://github.com/hipparcos/dialecte/blob/master/DOC.md).

```lisp
> fun {max xs} {fold (\ {x y} {if (> x y) {x} {y}}) (head xs) xs}
{max}
> max $ list 6 7 4 2 1 9 8 0
9
```

## TODO

- [ ] Implement pool allocation for lval;
- [ ] Reduce the number of call to lval_copy;
- [ ] Add debug functions (debug, next, step);
- [ ] Implement standard library (chapter 15);
- [ ] Add more unit tests (lval, builtins);
- [ ] Improve error reporting;
- [ ] Add cast functions;
- [ ] Add string functions (+ character type?);
- [ ] Support UTF-8;
- [ ] Implement user defined types;
- [ ] Implement OS interaction;
- [ ] Implement variables hashtable;
- [ ] Implement garbage collection;
- [ ] Implement tail call optimisation;
- [ ] Implement lexical scoping;
- [ ] Implement static typing;
- [ ] Support bigdouble;
- [ ] Support rational numbers;
- [ ] Support complex numbers;
- [ ] Support matrices.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
