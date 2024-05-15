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

## Grammar

-  Statement := AssignementStatement | IfStatement | WhileStatement | ForStatement | BlockStatement | DisplayStatement
-  IfStatement := "IF" Expression "THEN" Statement [ "ELSE" Statement ]
-  WhileStatement := "WHILE" Expression "DO" Statement
-  ForStatement := "FOR" AssignementStatement "To" Expression "DO" Statement
-  BlockStatement := "BEGIN" Statement { ";" Statement } "END"
-  DisplayStatement := "DISPLAY" Expression

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
-  Number := Digit{Digit}
-  CharConst := "'" Letter "'"

<br>

-  AdditiveOperator := "+" | "-" | "||"
-  MultiplicativeOperator := "*" | "/" | "%" | "&&"
-  RelationalOperator := "==" | "!=" | "<" | ">" | "<=" | ">="  
-  Digit := "0"|"1"|"2"|"3"|"4"|"5"|"6"|"7"|"8"|"9"
-  Letter := "a"|...|"z"
-  Type := "INTEGER" | "BOOLEAN" | "DOUBLE" | "CHAR"
