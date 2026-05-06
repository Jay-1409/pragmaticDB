all:
	g++ -std=c++20 src/*.cpp main.cpp -Iinclude -o test

run: all
	./test

clean:
	rm -f test