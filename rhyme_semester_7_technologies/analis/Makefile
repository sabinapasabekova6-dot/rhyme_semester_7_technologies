CXX       := g++
CXXFLAGS  := -std=c++20 -Wall -Wextra -Werror -Wpedantic -g -I.
GTEST     := -lgtest -lgtest_main -pthread

.PHONY: all test test_rhymes test_identverbs clean test_diagnostic test_rhymes_complex test_rhymes_precision

all: test_rhymes test_identverbs test_rhymes_complex test_rhymes_precision test_diagnostic

test_rhymes: RhymesAnalysis.cpp tests/test_rhymes.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(GTEST)
	./test_rhymes

test_rhymes_precision: RhymesAnalysis.cpp tests/test_rhymes_precision.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(GTEST)
	./test_rhymes_precision

test_rhymes_complex: RhymesAnalysis.cpp tests/test_rhymes_complex.cpp MorphRules.cpp IdentVerbs.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(GTEST)
	./test_rhymes_complex

test_identverbs: IdentVerbs.cpp MorphRules.cpp tests/test_identverbs.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(GTEST)
	./test_identverbs

test_diagnostic: IdentVerbs.cpp MorphRules.cpp tests/test_diagnostic.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(GTEST)
	./test_diagnostic

# запуск всех тестов подряд
test: test_rhymes test_identverbs
	@echo "===== RhymesAnalysis ====="
	./test_rhymes
	@echo
	@echo "===== IdentVerbs ====="
	./test_identverbs

clean:
	rm -f test_rhymes test_identverbs test_diagnostic test_rhymes_precision test_rhymes_complex