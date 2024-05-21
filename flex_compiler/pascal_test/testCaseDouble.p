VAR     a,d : DOUBLE;
        c   : INTEGER.

a := 2.0/2.0;
d := 4.2*2.0;
c := 1;
DISPLAY a;
DISPLAY d;

CASE a OF
1.0     : IF d==8.4 THEN c := 3;
5.0     : c := 4;
2.0     : c := 5
ELSE
    c := 80
END;

DISPLAY c.
