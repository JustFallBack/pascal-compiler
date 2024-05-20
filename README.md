# CERIcompiler

A simple compiler.
From : Pascal-like imperative LL(k) langage
To : 64 bit 80x86 assembly langage (AT&T)

**Download the repository :**

> git clone git@github.com:JustFallBack/Compilateur.git

## Build the compiler and test it :
You can test different file that are in `pascal_test` folder.<br>
For exemple :

> make VERSION=All

will compile the program using `testAll.p` file.

**Have a look at the output :**

> gedit test.s

**Debug the executable with ddd:**

> ddd ./test

**Debug the executable manually with gdb:**

> gdb ./test

## Debug the executable with gdb:
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

## Type handled

- INTEGER
- BOOLEAN
- CHAR
- DOUBLE

**Note : DISPLAY is not supported when using FOR statements.**<br>
**CASE statement is not fully implemented yet.**

## Grammar

-  Statement := AssignementStatement | IfStatement | WhileStatement | ForStatement | BlockStatement | DisplayStatement | CaseStatement
-  IfStatement := "IF" Expression "THEN" Statement [ "ELSE" Statement ]
-  WhileStatement := "WHILE" Expression "DO" Statement
-  ForStatement := "FOR" AssignementStatement ( "TO" | "DOWNTO" ) Expression "DO" Statement
-  BlockStatement := "BEGIN" Statement { ";" Statement } "END"
-  DisplayStatement := "DISPLAY" Expression
-  CaseStatement := "CASE" Expression "OF" CaseListElement {";" CaseListElement} ["ELSE" Statement] "END"
-  CaseListElement := CaseLabel ":" Statement
-  CaseLabel := CharConst {"," CharConst} | {Digit}+ ".." {Digit}+ | Number {"," Number} | Identifier

<br>

-  Program := [VarDeclarationPart] StatementPart
-  VarDeclarationPart := "VAR" VarDeclaration {";" VarDeclaration} "."
-  VarDeclaration := Identifer {"," Identifier} ":" Type
-  StatementPart := Statement {";" Statement} "."
-  Statement := AssignementStatement
-  AssignementStatement := Letter "=" Expression

<br>

-  Expression := SimpleExpression [RelationalOperator SimpleExpression]
-  SimpleExpression := Term {AdditiveOperator Term}
-  Term := Factor {MultiplicativeOperator Factor}
-  Factor := "(" Expression ")" | Number | Identifier | CharConst
-  Identifier := Letter{Letter|Digit}
-  Number := {digit}+(\.{digit}+)?
-  CharConst := "'" Letter "'"

<br>

-  AdditiveOperator := "+" | "-" | "||"
-  MultiplicativeOperator := "*" | "/" | "%" | "&&"
-  RelationalOperator := "==" | "!=" | "<" | ">" | "<=" | ">="  
-  Digit := "0"|"1"|"2"|"3"|"4"|"5"|"6"|"7"|"8"|"9"
-  Letter := "a"|...|"z"
-  Type := "INTEGER" | "BOOLEAN" | "DOUBLE" | "CHAR"

## Information on CaseStatement :

Here are a few example of what CaseStatement can handle in this version.<br>

>WARNING<br>

This version does not handle repetition.<br>
For example, the following code won't alert that there is the possibilty of e being 1 multiples time.<br>
It will execute the statement associated to the first occurence (here the first line) and will skip every other statements in the CaseStatement (like if there was a break instruction after each Statement).<br>

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
2..5    : Statement;  // if e is in the range of 2 to 5
6,7,12  : Statement;  // if e is either 6,7 or 12
a       : Statement   // if e is equal to a (here, 86)
ELSE
    Statement
END.
```

**CaseStatement with CHAR :**

>WARNING<br>

There must be a whitespace between the `,` (COMMA) and the `'` (single quote).<br>
It is due to the way *lexer* handles **tokens**.<br>

```pascal
VAR e : CHAR.

CASE e OF
'a'         : Statement;
'b', 'c'    : Statement;
'd'         : Statement
ELSE
    Statement
END.
```

**CaseStatement with BOOL :**

**CaseStatement with DOUBLE :**