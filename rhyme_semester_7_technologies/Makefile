# ---------- Компилятор и флаги ----------
CXX := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -Werror -Wpedantic -g -I.
LDLIBS := -lgtest -lgtest_main -pthread

# ---------- Общие объектные файлы ----------
# Собираются один раз, переиспользуются несколькими тестами.
OBJ_RHYMES := RhymesAnalysis.o
OBJ_VERB := IdentVerbs.o MorphRules.o

# ---------- Имена собранных тестов ----------
TEST_RHYMES := test_rhymes
TEST_PRECISION := test_rhymes_precision
TEST_COMPLEX := test_rhymes_complex
TEST_VERBS := test_identverbs
TEST_DIAG := test_diagnostic

ALL_TESTS := $(TEST_RHYMES) $(TEST_PRECISION) $(TEST_COMPLEX) \
             $(TEST_VERBS) $(TEST_DIAG)

.PHONY: all test clean help \
        run_rhymes run_rhymes_precision run_rhymes_complex \
        run_identverbs run_diagnostic

# ============================================================
#  Основные цели
# ============================================================

# Собрать все тесты без запуска
all: $(ALL_TESTS)

# ============================================================
#  Сборка отдельных тестов
# ============================================================

$(TEST_RHYMES): $(OBJ_RHYMES) $(OBJ_VERB) tests/test_rhymes.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDLIBS)

$(TEST_PRECISION): $(OBJ_RHYMES) $(OBJ_VERB) tests/test_rhymes_precision.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDLIBS)

$(TEST_COMPLEX): $(OBJ_RHYMES) $(OBJ_VERB) tests/test_rhymes_complex.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDLIBS)

$(TEST_VERBS): $(OBJ_VERB) tests/test_identverbs.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDLIBS)

$(TEST_DIAG): $(OBJ_VERB) tests/test_diagnostic.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDLIBS)

# ---------- Правило: .cpp -> .o ----------
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Собрать и запустить все тесты
test: $(ALL_TESTS)
	@echo
	@echo "===== RhymesAnalysis (базовые) ====="
	./$(TEST_RHYMES)
	@echo
	@echo "===== RhymesAnalysis (precision) ====="
	./$(TEST_PRECISION)
	@echo
	@echo "===== RhymesAnalysis (complex) ====="
	./$(TEST_COMPLEX)
	@echo
	@echo "===== IdentVerbs ====="
	./$(TEST_VERBS)
	@echo
	@echo "===== Diagnostic ====="
	./$(TEST_DIAG)
	@echo
	@echo "Все тесты пройдены."

# ---------- Запуск отдельных тестов ----------
run_rhymes: $(TEST_RHYMES)
	./$(TEST_RHYMES)

run_rhymes_precision: $(TEST_PRECISION)
	./$(TEST_PRECISION)

run_rhymes_complex: $(TEST_COMPLEX)
	./$(TEST_COMPLEX)

run_identverbs: $(TEST_VERBS)
	./$(TEST_VERBS)

run_diagnostic: $(TEST_DIAG)
	./$(TEST_DIAG)

# ============================================================
#  Очистка и справка
# ============================================================

clean:
	rm -f $(ALL_TESTS) *.o

help:
	@echo "Доступные цели:"
	@echo "  make all - собрать все тесты (без запуска)"
	@echo "  make test - собрать и запустить все тесты"
	@echo "  make run_rhymes - запустить test_rhymes"
	@echo "  make run_rhymes_precision - запустить test_rhymes_precision"
	@echo "  make run_rhymes_complex - запустить test_rhymes_complex"
	@echo "  make run_identverbs - запустить test_identverbs"
	@echo "  make run_diagnostic - запустить test_diagnostic"
	@echo "  make clean - удалить собранные файлы"
	@echo "  make help - показать эту справку"