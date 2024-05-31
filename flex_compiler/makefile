.DEFAULT_GOAL := all

VERSION=default

ifeq ($(VERSION), default)
	DEFAULT_TARGETS := help
else
	DEFAULT_TARGETS := test$(VERSION)
endif

help: ## show this help
	@awk 'BEGIN {FS=":.*?##" } /^[a-zA-Z_-]+:.*?## / {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}' $(MAKEFILE_LIST)

all: $(DEFAULT_TARGETS) ## Example : make VERSION=Double. Replace 'Double' by any of the test files in the pascal_test directory

clean: ## clean all compiled files
		echo "Version :$(VERSION)"
		rm *.o *.s
		rm tokeniser.cpp
		rm test
		rm compiler
tokeniser.cpp:	tokeniser.l ## generate the tokeniser.cpp file
		flex++ -d -otokeniser.cpp tokeniser.l
tokeniser.o:	tokeniser.cpp ## compile the tokeniser.cpp file
		g++ -c tokeniser.cpp
compiler:	compiler.cpp tokeniser.o ## compile the compiler.cpp file
		g++ -ggdb -o compiler compiler.cpp tokeniser.o
test$(VERSION): compiler pascal_test/test$(VERSION).p ## compile the test file
		./compiler < pascal_test/test$(VERSION).p > test.s
		gcc -ggdb -no-pie -fno-pie test.s -o test
prog:		compiler prog.p ## compile the prog file
		./compiler <prog.p >prog.s
		gcc -ggdb -no-pie -fno-pie prog.s -o prog


