VAR     a,b,c : INTEGER;
        d     : DOUBLE;
        e     : CHAR;
        di    : BOOLEAN.

BEGIN
    a:=3;
    b:=0;
    c:=0;
    d:=0.0;
    e:='a'
END;

DISPLAY a;
DISPLAY e;

CASE a OF
1,2  : a := 100;
6    : a := 50;
a    : a := 4
ELSE
    a := 8
END;


DISPLAY a.
