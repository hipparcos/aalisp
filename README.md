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
> (fun {sq} {x} {* x x}) (sq 9)
81
> (* 4 21)(/ . 2) ; `.` is equal to the last computed value.
42
```

## TODO

- [x] Rewrite the parser myself (see llexer & lparser);
- [x] Implement S-Expressions (chapter 9);
- [x] Implement Q-Expressions (chapter 10);
- [x] Implement variables (chapter 11);
- [x] Add tests for lenv;
- [ ] Add tests for lambda builtin;
- [ ] Implement partial function application;
- [ ] Implement functions (chapter 12);
- [ ] Improve error reporting;
- [ ] Implement conditionals (chapter 13);
- [ ] Implement strings (chapter 14);
- [ ] Implemnt elem, take, map, filter, fold, reverse;
- [ ] Implement standard library (chapter 15);
- [x] Add comments support;
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
