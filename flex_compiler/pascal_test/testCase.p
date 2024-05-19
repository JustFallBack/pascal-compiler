VAR     a,b,c : INTEGER;
        d     : DOUBLE;
        e     : CHAR.

BEGIN
    a:=0;
    b:=0;
    c:=0;
    d:=0.0;
    e:='a'
END;

DISPLAY a;
DISPLAY e;

CASE a OF
1    : a := 100;
2..5 : a := 100;
6,7  : a := 50
ELSE
    a := 8
END;

DISPLAY a
.
