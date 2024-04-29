[INTEGER a, INTEGER b, INTEGER i, INTEGER c]

a:=3;
FOR i:=1 TO 3 DO 
    BEGIN 
        IF a==1 THEN
            b:=b+1
        ELSE 
        BEGIN
            b:=b*2;
            c:=c+1
        END
    END.