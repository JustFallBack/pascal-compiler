VAR     a,d : DOUBLE;
        c   : INTEGER.

a := 15.0/2.0;
d := 4.2*2.0;
c := 1;

DISPLAY a;
DISPLAY d;

d := 7.5;

CASE a OF
1.0      : IF d==8.4 THEN c := 3;
2.0,7.54  : c := 5;
d, 8.0   : c := 9002
ELSE
    c := 80
END;

DISPLAY c.
