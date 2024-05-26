VAR     n : BOOLEAN;
        b : INTEGER.

n:=FALSE;

CASE n OF
FALSE : b:=2;
TRUE  : b := 1
ELSE (* should not happen *)
    b := 3
END;

DISPLAY n;
DISPLAY b.
