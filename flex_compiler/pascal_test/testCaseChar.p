VAR     a,b,c : INTEGER;
        d     : DOUBLE;
        e     : CHAR.

BEGIN
    a:=9;
    b:=0;
    c:=0;
    d:=0.0;
    e:='a'
END;

DISPLAY a;
DISPLAY e;

CASE e OF
'b', 'a' : a:= 100;
'a' : a:= 50
ELSE
    a := 8
END;


DISPLAY a.
