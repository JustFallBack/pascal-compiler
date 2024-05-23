# Pascal compiler

A simple Pascal compiler.<br>
From : Pascal-like imperative LL(k) langage<br>
To : 64 bit 80x86 assembly langage (AT&T)

This project has been done for academical purposes.<br>
This compiler is very simple, some pascal features are not handled<br>
The set of instructions used for the assembly code work with 64 bit 80x86 assembly langage.

This project was developped on standard Ubuntu distribution.

## Build the compiler :
You can test different files that are in `pascal_test` folder.<br>
For exemple :

> make VERSION=All

will compile the program using `testAll.p` file.

**You can have a look at the produced assembly code in `test.s`.**

**Download the repository :**

> git clone git@github.com:JustFallBack/Compilateur.git

## Grammar

```md
-  Statement := AssignementStatement | IfStatement | WhileStatement | ForStatement | BlockStatement | DisplayStatement | CaseStatement
-  IfStatement := "IF" Expression "THEN" Statement [ "ELSE" Statement ]
-  WhileStatement := "WHILE" Expression "DO" Statement
-  ForStatement := "FOR" AssignementStatement ( "TO" | "DOWNTO" ) Expression "DO" Statement
-  BlockStatement := "BEGIN" Statement { ";" Statement } "END"
-  DisplayStatement := "DISPLAY" Expression
-  CaseStatement := "CASE" Expression "OF" CaseListElement {";" CaseListElement} ["ELSE" Statement] "END"
-  CaseListElement := CaseLabel ":" Statement
-  CaseLabel := Factor { "," Factor }
```
<br>

```md
-  Program := [VarDeclarationPart] StatementPart
-  VarDeclarationPart := "VAR" VarDeclaration {";" VarDeclaration} "."
-  VarDeclaration := Identifer {"," Identifier} ":" Type
-  StatementPart := Statement {";" Statement} "."
-  Statement := AssignementStatement
-  AssignementStatement := Letter "=" Expression
```

<br>

```md
-  Expression := SimpleExpression [RelationalOperator SimpleExpression]
-  SimpleExpression := Term {AdditiveOperator Term}
-  Term := Factor {MultiplicativeOperator Factor}
-  Factor := "(" Expression ")" | Number | Identifier | CharConst
-  Identifier := Letter{Letter|Digit}
-  Number := {digit}+(\.{digit}+)?
-  CharConst := "'" Letter "'"
```

<br>

```md
-  AdditiveOperator := "+" | "-" | "||"
-  MultiplicativeOperator := "*" | "/" | "%" | "&&"
-  RelationalOperator := "==" | "!=" | "<" | ">" | "<=" | ">="  
-  Digit := "0"|"1"|"2"|"3"|"4"|"5"|"6"|"7"|"8"|"9"
-  Letter := "a"|...|"z"
-  Type := "INTEGER" | "BOOLEAN" | "DOUBLE" | "CHAR"
```

## Type handled

- INTEGER
- BOOLEAN
- CHAR
- DOUBLE

**Negative `INTEGER` and `DOUBLE` are not supported.**<br>
**It is not possible to directly assign a `BOOLEAN` value to a `BOOLEAN`.**

## Debug the executable with ddd :

> ddd ./test

## Debug the executable manually with gdb 

> gdb ./test

Create a break point where gdb stops at specified label (for exemple, `break main` makes gdb stops at the start of the assembly code).<br>
You can make break point at a specific line (`break 26` to make gdb stops at 26th line) :
>(gdb) break _label_<br>
>(gdb) break _line_

Clear specified break point :
>(gdb) clear _label_<br>
>(gdb) clear _line_

Run the debug :
>(gdb) run

Next step :
>(gdb) step<br>
>(gdb) s

Continue to next break point :
>(gdb) continue<br>
>(gdb) c


## Information on CaseStatement :

Here are a few example of what *CaseStatement* can handle in this version.<br>

>WARNING<br>

This version does not handle repetition.<br>
For example, the following code won't alert that there is the possibilty of `e` being `1` multiples time.<br>
It will execute the statement associated to the first occurence (here the first line) and will skip every other statements in the *CaseStatement* (like if there was a **break** instruction after each *Statement*).<br>

```pascal
VAR e : INTEGER.

CASE e OF
1       : Statement;
1,2,3   : Statement;  
1..5    : Statement  
END.
```

**CaseStatement with INTEGER :**

```pascal
VAR e,a : INTEGER.

a:=86;

CASE e OF
1       : Statement;  // if e is 1
6,7,12  : Statement;  // if e is either 6,7 or 12
a,999   : Statement   // if e is either equal to a (here, 86) or 999
ELSE
    Statement         // if e is not equal to any of the specified value
END.
```

**CaseStatement with DOUBLE :**

```pascal
VAR     a,d : DOUBLE;

a := 15.0/2.0;
d := 4.2*2.0;

CASE a OF
1.0      : Statement;  // if a is 1.0
2.0,7.54 : Statement;  // if a is either 2.0 or 7.54
d, 8.0   : Statement   // if a is either equal to d or 8.0
ELSE
    Statement          // if a is not equal to any of the specified value
END.
```

**CaseStatement with CHAR :**

>WARNING<br>

There must be a whitespace between the `,` (COMMA) and the `'` (single quote).<br>
It is due to the way *lexer* handles **tokens**.<br>

```pascal
VAR e,d : CHAR.

d := '$';

CASE e OF
'a'         : Statement;  // if e is 'a'
'b', 'c'    : Statement;  // if e is either 'b' or 'c'
d, 'd'      : Statement   // if e is either equal to d or is 'd'
ELSE
    Statement             // if e is not equal to any of the specified value
END.
```