CXX = g++
CXXFLAGS = -std=c++20 -Iinclude

APP_BIN = app
TEST_BIN = test_runner

app:
	$(CXX) $(CXXFLAGS) src/manager/*.cpp src/factory/*.cpp src/type/*.cpp src/catalog/*.cpp src/utils/*.cpp main.cpp -o $(APP_BIN)

test-build:
	$(CXX) $(CXXFLAGS) src/manager/*.cpp src/factory/*.cpp src/type/*.cpp src/catalog/*.cpp src/utils/*.cpp tests/*.cpp -o $(TEST_BIN)

run: app
	./$(APP_BIN)

test: test-build
	./$(TEST_BIN)

all: app test-build

clean:
	rm -f $(APP_BIN) $(TEST_BIN)