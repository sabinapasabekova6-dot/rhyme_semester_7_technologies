CXX       := g++
CXXFLAGS  := -std=c++20 -Wall -Wextra -Werror -Wpedantic -g -I.
GTEST     := -lgtest -lgtest_main -pthread

LIB_SRC   := IdentVerbs.cpp
TEST_SRC  := tests/test_identverbs.cpp
TEST_BIN  := test_identverbs

.PHONY: all test clean

all: $(TEST_BIN)

$(TEST_BIN): $(LIB_SRC) $(TEST_SRC)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(GTEST)

test: $(TEST_BIN)
	./$(TEST_BIN)

clean:
	rm -f $(TEST_BIN)