VERSION=1
all:	test$(VERSION)
clean:
		echo "Version :$(VERSION)"
		rm *.o *.s
		rm tokeniser.cpp
tokeniser.cpp:	tokeniser.l
		flex++ -d -otokeniser.cpp tokeniser.l
tokeniser.o:	tokeniser.cpp
		g++ -c tokeniser.cpp
compilateur:	compilateur.cpp tokeniser.o
		g++ -ggdb -o compilateur compilateur.cpp tokeniser.o
test$(VERSION): compilateur pascal_test/test$(VERSION).p
		./compilateur < pascal_test/test$(VERSION).p > test.s
		gcc -ggdb -no-pie -fno-pie test.s -o test
prog:		compilateur prog.p
		./compilateur <prog.p >prog.s
		gcc -ggdb -no-pie -fno-pie prog.s -o prog


