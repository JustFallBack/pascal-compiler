VAR a,b,c,d,i,j,k : INTEGER.

BEGIN
  a := 3;
  b := 0;
  c := 0;
  d := 0;

  FOR j := 1 TO 100 DO
  BEGIN
    IF j % 2== 0 THEN
    BEGIN
        FOR k := 1 TO j DO
      BEGIN
        IF a== 1 THEN
          b := b + 1
        ELSE
        BEGIN
          b := b + 2;
          c := c + 1
        END
      END
    END
    ELSE
    BEGIN
      i := j;
      WHILE i > 0 DO
      BEGIN
        IF a== 1 THEN
          b := b + 1
        ELSE
        BEGIN
          b := b + 2;
          c := c + 1
        END;
        i := i - 1
      END
    END
  END
END.