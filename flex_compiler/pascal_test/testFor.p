VAR     voila,super : INTEGER;
        c12e6aef6,i,j : INTEGER;
        d : DOUBLE.

voila:=3;
c12e6aef6:=5;
super:=0;

FOR i := 3 TO 8 DO
BEGIN
    super := super + i
END;

DISPLAY super;
DISPLAY i;

FOR i := 7 DOWNTO 3 DO
    super := super - i/2;

DISPLAY super;

FOR i := 0 TO 10 DO
    BEGIN
    FOR j := 10 DOWNTO 0 DO
        voila := i + j
    END;

DISPLAY i;
DISPLAY j.
