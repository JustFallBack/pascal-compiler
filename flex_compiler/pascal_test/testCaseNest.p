VAR         a,b : INTEGER;
            flag1, flag2: BOOLEAN.

BEGIN
    a:=55;
    b:=69;
    flag1 := FALSE; flag2 := FALSE
END;

CASE a OF
    1,2,3 : flag1 := TRUE;
    4     : CASE b OF 
                1,2,3 : flag1 := TRUE;
                b     : flag2 := TRUE
            ELSE a := 3
            END;
    55    : a := 2
END;

DISPLAY a;
DISPLAY b;
DISPLAY flag1;
DISPLAY flag2.
     
