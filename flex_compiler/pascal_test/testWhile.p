VAR     i,j,b : INTEGER.

b:=3;

WHILE i <= 3 DO
BEGIN
    j := 1;
    WHILE j <= 4 DO
    BEGIN
        b := b + i * j;
        j := j + 1
    END;
    i := i + 1
END.