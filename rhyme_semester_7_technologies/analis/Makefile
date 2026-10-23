CXX       := g++
CXXFLAGS  := -std=c++20 -Wall -Wextra -Werror -Wpedantic -g -I.
GTEST     := -lgtest -lgtest_main -pthread

.PHONY: all test test_rhymes test_identverbs clean

all: test_rhymes test_identverbs

test_rhymes: RhymesAnalysis.cpp tests/test_rhymes.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(GTEST)
	./test_rhymes

test_identverbs: IdentVerbs.cpp tests/test_identverbs.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(GTEST)
	./test_identverbs

# запуск всех тестов подряд
test: test_rhymes test_identverbs
	@echo "===== RhymesAnalysis ====="
	./test_rhymes
	@echo
	@echo "===== IdentVerbs ====="
	./test_identverbs

clean:
	rm -f test_rhymes test_identverbs