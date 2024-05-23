VAR     a,b,c,d : INTEGER;
        e,f,g,h : DOUBLE;
        hello   : CHAR;
        bool    : BOOLEAN.

a:=0;
b:=12;
c:=b/5+15;
d:=(a+c)%2;

a:=(a+b*(c-b));
hello:='$';
DISPLAY a;
DISPLAY hello;


hello:='a';
bool:=hello=='a';
DISPLAY bool;

e:=50.785;
f:=e/24.0+3.5;
DISPLAY f
.
