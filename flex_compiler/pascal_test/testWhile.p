VAR     i,j,b : INTEGER.

b:=3;

(* By default, INTEGER that are declared but not initiliased are equal to zero *)

WHILE i <= 3 DO
BEGIN
    j := 1;
    WHILE j <= 4 DO
    BEGIN
        b := b + i * j;
        j := j + 1
    END;
    i := i + 1
END;

DISPLAY i;
DISPLAY j;
DISPLAY b.