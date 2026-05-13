CXX = g++
CXXFLAGS = -std=c++20 -Iinclude -Isrc/ds

APP_BIN = app
TEST_BIN = test_runner

app:
	$(CXX) $(CXXFLAGS) src/*.cpp main.cpp -o $(APP_BIN)

test-build:
	$(CXX) $(CXXFLAGS) src/*.cpp tests/*.cpp -o $(TEST_BIN)

run: app
	./$(APP_BIN)

test: test-build
	./$(TEST_BIN)

all: app test-build

clean:
	rm -f $(APP_BIN) $(TEST_BIN)