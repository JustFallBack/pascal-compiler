VAR     a,b,c : INTEGER;
        d     : DOUBLE;
        e,f   : CHAR.

BEGIN
    a:=9;
    b:=0;
    c:=0;
    d:=0.0;
    e:='a';
    f:='5'
END;

DISPLAY a;
DISPLAY e;

CASE e OF
'b', 'a' : a:= 100;
'a' : a:= 50;
f   : a:= 1
ELSE
    a := 8
END;


DISPLAY a.
