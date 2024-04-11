# CERIcompiler

A simple compiler.
From : Pascal-like imperative LL(k) langage
To : 64 bit 80x86 assembly langage (AT&T)

## This version can handle :

- Number := Digit{Digit}
- Letter := "a"|...|"z"
- AdditiveOperator := "+" | "-" | "||"
- MultiplicativeOperator := "*" | "/" | "%" | "&&"
- RelationalOperator := "==" | "!=" | "<" | ">" | "<=" | ">="  

- Expression := ArithmeticExpression [RelationalOperator ArithmeticExpression]
- ArithmeticExpression := Term {AdditiveOperator Term}
- Term := Factor {MultiplicativeOperator Factor}
- Factor := Number | Letter | "(" Expression ")"| "!" Factor

## How to use 

**Build the compiler with debug symbol :**

> bash ./compil.sh

**Debug the executable :**

Run gdb on console :
> gdb ./test

**gdb usage**

Create a break point where gdb stops at specified label (for exemple, `break main` makes gdb stops at the start of the assembly code).<br>
You can make break point at a specific line (`break 26` to make gdb stops at 26th line) :
>(gdb) break label<br>
>(gdb) break line

Clear specified break point :
>(gdb) break label<br>
>(gdb) break line

Run the debug :
>(gdb) run

Next step :
>(gdb) step<br>
>(gdb) s

Continue to next break point :
>(gdb) continue<br>
>(gdb) c


## Not up to date (don't read)

**Download the repository :**

> git clone git@framagit.org:jourlin/cericompiler.git

**Build the compiler with debug symbols :**

> g++ -ggdb compilateur.cpp -o compilateur

**Compile the test program :**

> cat test.p | ./compilateur > test.s

**Have a look at the output :**

> gedit test.s

**Produce the executable (with debug info) :**

> gcc -ggdb test.s -o test :

**Debug the executable :**

> ddd ./test

**Commit the new version :**

> git commit -a -m "What's new..."

**Send to your framagit :**

> git push -u origin master

**Get from your framagit :**

> git pull -u origin master

**gdb usage**

Run gdb on console.
> gdb ./test

Create a break point where gdb stops (for exemple, break main makes gdb stops at the start of the assembly code). You can make break at a specific line (break 26 for line number 26) :
> break (break_tag)

Clear an existing break point :
> clear (break_tag)

Run the gdb
> run

Go to next break point :
> continue
> c

Next step :
> step
> s