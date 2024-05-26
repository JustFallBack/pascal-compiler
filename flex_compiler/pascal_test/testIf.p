VAR     a,b,c : INTEGER.

a:=3;
c:=5;
b:=0;

IF a==1 THEN
    b:=b+1
ELSE 
    IF a>2 THEN BEGIN
        b:=b*2;
        c:=c+1
    END
    ELSE
        b:=b-1;

IF a==3 THEN
    b:=b+1;

DISPLAY b;
DISPLAY c.