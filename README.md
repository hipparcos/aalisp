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

Comments:
```lisp
> ; I'm a comment up to the end of line.
nil
```
Arithmetic:
```lisp
> + 1 2 3 4
10
> / (! 21) (! 20)
21
> ^ 2 16
65536
> - 618.5 572.5 4
42
```
Control flow:
```lisp
> if (> 42 0) {+ 21 21} {0}
42
```
List operations:
```lisp
> list 1 2 3 4
{1 2 3 4}
> join {1 2} {3 4}
{1 2 3 4}
> head (list 1 2 3)
1
> eval (head {(+ 1 1) (+ 2 2)})
2
> map (\ {x} {* x x}) {1 2 3 4}
{1 4 9 16}
```
String manipulations:
```lisp
> join "join" " also operates " "on strings"
"join also operates on strings"
```
Functions definition:
```lisp
> (\ {x} {* x x}) 9
81
```
Environment operations:
```lisp
> (def {x y} 1 2)(+ x y)
3
> (def {sq} (\ {x} {* x x})) (sq 9)
81
> (fun {sq x} {* x x}) (sq 9)
81
> (* 4 21)(/ . 2) ; `.` is equal to the last computed value.
42
```

## TODO

- [ ] Implement min, max as builtins;
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
