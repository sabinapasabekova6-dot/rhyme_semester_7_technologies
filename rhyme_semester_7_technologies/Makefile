# ---------- Компилятор и флаги ----------
CXX := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -Werror -Wpedantic -g -Isrc
LDLIBS := -lgtest -lgtest_main -pthread

# ---------- Общие объектные файлы ----------
# Собираются один раз, переиспользуются несколькими тестами.
OBJ_RHYMES := RhymesAnalysis.o
OBJ_VERB := IdentVerbs.o MorphRules.o
OBJ_PREVIEW := $(OBJ_RHYMES) $(OBJ_VERB) result_file.o
OBJ_APP := $(OBJ_PREVIEW) file_processing.o rhyme_semester_7_technologies.o
APP_EXE := bin/rhyme_semester_7_technologies.exe

# ---------- Имена собранных тестов ----------
TEST_RHYMES := test_rhymes
TEST_PRECISION := test_rhymes_precision
TEST_COMPLEX := test_rhymes_complex
TEST_VERBS := test_identverbs
TEST_DIAG := test_diagnostic
TEST_STRESS := test_rhyme_stress

ALL_TESTS := $(TEST_RHYMES) $(TEST_PRECISION) $(TEST_COMPLEX) \
             $(TEST_VERBS) $(TEST_DIAG) $(TEST_STRESS)

.PHONY: all exe test clean help \
        run_rhymes run_rhymes_precision run_rhymes_complex \
        run_identverbs run_diagnostic run_rhyme_stress

# ============================================================
#  Основные цели
# ============================================================

# Собрать все тесты без запуска
all: $(ALL_TESTS)

# Собрать консольное приложение компилятором MinGW.
exe: $(APP_EXE)

$(APP_EXE): $(OBJ_APP)
	mkdir -p bin
	$(CXX) $(LDFLAGS) -static $^ -o $@

# В isLetterChar верхняя граница unsigned char проверяется явно.
file_processing.o: CXXFLAGS += -Wno-type-limits

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

$(TEST_STRESS): $(OBJ_PREVIEW) tests/test_rhyme_stress.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDLIBS)

# ---------- Правило: .cpp -> .o ----------
%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

-include $(OBJ_APP:.o=.d)

rhyme_preview: rhyme_preview.cpp $(OBJ_PREVIEW)
	$(CXX) $(CXXFLAGS) $^ -o $@

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
	@echo "===== Ударения и статистика рифм ====="
	./$(TEST_STRESS)
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

run_rhyme_stress: $(TEST_STRESS)
	./$(TEST_STRESS)

# ============================================================
#  Очистка и справка
# ============================================================

clean:
	rm -f $(ALL_TESTS) rhyme_preview $(APP_EXE) $(OBJ_APP) $(OBJ_APP:.o=.d)

help:
	@echo "Доступные цели:"
	@echo "  make all - собрать все тесты (без запуска)"
	@echo "  make exe - собрать $(APP_EXE) с MinGW"
	@echo "  make test - собрать и запустить все тесты"
	@echo "  make run_rhymes - запустить test_rhymes"
	@echo "  make run_rhymes_precision - запустить test_rhymes_precision"
	@echo "  make run_rhymes_complex - запустить test_rhymes_complex"
	@echo "  make run_identverbs - запустить test_identverbs"
	@echo "  make run_diagnostic - запустить test_diagnostic"
	@echo "  make run_rhyme_stress - проверить ударения и вывести статистику рифм"
	@echo "  make rhyme_preview - собрать консольную проверку для Linux"
	@echo "  make clean - удалить собранные файлы"
	@echo "  make help - показать эту справку"
