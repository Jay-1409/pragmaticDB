CXX = g++
CXXFLAGS = -std=c++20 -Iinclude

APP_BIN  = app
TEST_BIN = test_runner
BENCH_BIN = bench

app:
	$(CXX) $(CXXFLAGS) src/manager/*.cpp src/factory/*.cpp src/type/*.cpp src/catalog/*.cpp src/utils/*.cpp src/query/*.cpp src/network/*.cpp main.cpp -o $(APP_BIN)

test-build:
	$(CXX) $(CXXFLAGS) src/manager/*.cpp src/factory/*.cpp src/type/*.cpp src/catalog/*.cpp src/utils/*.cpp src/query/*.cpp src/network/*.cpp tests/*.cpp -o $(TEST_BIN)

run: app
	./$(APP_BIN)

test: test-build
	./$(TEST_BIN)

all: app test-build

bench:
	$(CXX) $(CXXFLAGS) tools/bench.cpp -o $(BENCH_BIN)

clean:
	rm -f $(APP_BIN) $(TEST_BIN) $(BENCH_BIN)