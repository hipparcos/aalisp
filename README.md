# dialecte

dialecte \dja.lɛkt\ is a dialect of Lisp written in C (version C11).
WIP...

## Getting started

### Prerequisites

Building:
- GNU Compiler Collection (gcc);
- GNU Make (make);
- GNU C Library (glibc/getopt): CLI flags parsing;
- GNU Readline (readline-x.x): CLI interface.

Testing:
- Valgrind (valgrind): memory leaks detection;

### Dependancies

Included under vendor/:
- mini-gmp: bignum arithmetic;
- [mortie/snow](https://github.com/mortie/snow): unit testing.

### Building & running

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
> (fun {max xs} { fold (\ {x y} {if (> x y) {x} {y}}) (head xs) xs })
{max}
> max $ list 4 5 1 9 8 3 0
9
```

## TODO

- [ ] Add debug functions (print-env, print-func, ...);
- [ ] Delete unused builtins;
- [ ] Define aliases in stdlib;
- [ ] Implement standard library (chapter 15);
- [ ] Add tests;
- [ ] Improve error reporting;
- [ ] Implement pool allocation for lval;
- [ ] Reduce the number of lval copy;
- [ ] Add cast functions;
- [ ] Add string functions (+ character type?);
- [ ] Support UTF-8;
- [ ] Implement user defined types;
- [ ] Implement list literal;
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
