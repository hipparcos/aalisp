# dialecte documentation

*dialecte* \dja.lɛkt\ is a dialect of Lisp.

## Syntax

### Comments

Comments start with a semicolon `;` (outside of a string) and extend up to the
end of the line.
Comments are ignored by the interpreter.
```lisp
; I'm a comment.
```

### Atoms

Atoms are the basic building blocks of lisp.
They can be of type number, double, symbol or string.
```lisp
42   ; I'm a number.
42.0 ; I'm a double.
"42" ; I'm a string.
sym  ; I'm a symbol.
```

### Lists

Lists are infinite lists of atoms or lists.
They can be of type S-Expression or Q-Expression.

#### Q-Expression

Q-Expressions are lists enclosed in braces `{ }`.
The content of a Q-Expression is not evaluated by the interpreter.
```lisp
{1 2 3 4} ; This is a list.
```

#### S-Expression

S-Expressions are lists enclosed in parentheses `( )`.
The content of an S-Expression is evaluated by the interpreter.
The value of an S-Expression is the value of its last child after evaluation.
```lisp
(+ 1 2 3 4) ; This S-Expression evaluates to 10.
```

### Expressions

Expressions are executable lists.
They are enclosed in S-Expressions.
Expressions start with a symbol followed by operands.
This symbol must be bound to a function, either a built-in function or a
user-defined function.
Operands are atoms or lists.

A program is a list of S-Expressions containing expressions.
The value of a program is the value of the last S-Expression.

```lisp
(def {x} 10)
(* x x)
```


## Types

### Booleans

Booleans are either true or false.

### Numbers

A number can be any integer from **Z**.
*dialecte* can do arbitrary-precision arithmetic (bignum).

### Doubles

Double numbers are represented as IEEE 754 double-precision binary floating-point numbers (binary64).
Thus, rounding errors can occur and precision is limited.

### Strings

Strings start and end with a double quotes `"`.
Any included double quotes must be escaped using a backslash `\"`.

### Symbols

Symbols start with a letter or a sign.
They can contain lower and upper cases letters, signs and numbers.
Valid signs are `+-*/\%^?!&|:,._#~<>=$§£¤µ`.
They are case sensitive.
Symbols are used to represents an underlying value of any type.

### Functions

Functions can be built-in functions or user-defined functions.
They take operands and give a result.
They are usually bound to a symbol but can also be anonymous (see lambda).

### Q-Expression

Q-Expressions are lists enclosed in braces `{ }`.

### S-Expression

S-Expressions are lists enclosed in parentheses `( )`.


## Built-in symbols

- `true`;
- `false`;
- `.`.


## Built-in functions

To view the type of a function in the interpreter, type `::` followed by the
function name.
Example for `+`:
```lisp
> :: + ; 0 arguments bound for a minimum of one and an undefined maximum.
func@+(0/1-∞)
```

### Arithmetic operators

Arithmetic operators are used to do operations on numbers.
Operands are casted to the appropriate type in this order:
number -> bignum -> double

- `+` addition;
- `-` substraction;
- `*` multiplication;
- `/` division;
- `%` integer division;
- `^` exponentiation;
- `!` factorial.

### Boolean operators

Boolean operators returns boolean.

- `==` equality test;
- `!=` inequality test;
- `>` greater than;
- `>=` greater than or equal;
- `<` lower than;
- `<=` lower than or equal;
- `and` logical conjunction;
- `or` logical disjunction;
- `not` logical negation.

### List functions

Strings are considered as lists.

- `head` returns the first element of a list;
- `tail`;
- `init`;
- `last`;
- `index` or `!!`;
- `elem`;
- `take`;
- `drop`;
- `cons`;
- `len`;
- `join` or `++`;
- `list`;
- `seq` creates a list of integers:
    > seq 1 5 2
    {1 3 5}
- `eval`;
- `map`;
- `filter`;
- `fold`;
- `reverse`;
- `all`;
- `any`;
- `zip`;
- `sort` sorts a list.

### Control flow functions

- `if`;
- `loop`.

### Error functions

- `error`.

### Environment functions

- `def`;
- `put` or `=`;
- `fun`;
- `load`.

### Function manipulation functions

- `lambda` or `\ `;
- `pack`;
- `unpack`;
- `::`.

### IO functions

- `print`.

### Debug functions

- `debug-env`;
- `debug-fun`;
- `debug-val`.
