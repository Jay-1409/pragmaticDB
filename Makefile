all:
	g++ -std=c++20 src/*.cpp tests/*.cpp main.cpp -Iinclude -Isrc/ds -o test

run: all
	./test

clean:
	rm -f test