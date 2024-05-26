VAR
  a, b, c, d, i, j, k : INTEGER;
  x, y : DOUBLE;
  flag : BOOLEAN;
  ch : CHAR.

BEGIN
  a := 3;
  b := 0;
  c := 0;
  d := 0;
  x := 0.0;
  y := 1.5;
  flag := TRUE;
  ch := 'A';

  FOR j := 1 TO 10 DO
  BEGIN
    IF j % 2 == 0 THEN
    BEGIN
      FOR k := 1 TO j DO
      BEGIN
        IF a == 1 THEN
          b := b + 1
        ELSE
        BEGIN
          b := b + 2;
          c := c + 1;
          x := x + y;  
          IF ch == 'A' THEN
            ch := 'B'
          ELSE
            ch := 'A'
        END
      END
    END
    ELSE
    BEGIN
      i := j;
      WHILE i > 0 DO
      BEGIN
        IF flag THEN
        BEGIN
          b := b + 1;
          flag := FALSE
        END
        ELSE
        BEGIN
          b := b + 2;
          c := c + 1;
          y := y * 1.1;
          flag := TRUE
        END;
        i := i - 1
      END
    END;
    FOR i := 1 TO 5 DO
    BEGIN
      IF i % 2 == 1 THEN
      BEGIN
        flag := TRUE;
        IF flag THEN
          d := d + i
        ELSE
          d := d - i
      END;
      FOR a := 1 TO 3 DO
      BEGIN
        ch := 'A';
        IF ch == 'A' THEN
          b := b + 1
        ELSE IF ch == 'B' THEN
          c := c + 1
        ELSE
          d := d + 1
      END
    END
  END;

  DISPLAY a;
  DISPLAY b;
  DISPLAY c;
  DISPLAY d;
  DISPLAY x;
  DISPLAY y;
  DISPLAY flag;
  DISPLAY ch
END.
