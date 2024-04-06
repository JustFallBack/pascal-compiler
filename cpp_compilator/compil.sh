g++ -ggdb compilateur.cpp -o compilateur
cat test.p | ./compilateur > test.s
gcc -ggdb test.s -o test