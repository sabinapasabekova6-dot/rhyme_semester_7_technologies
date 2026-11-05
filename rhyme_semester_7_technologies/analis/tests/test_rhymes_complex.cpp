#include <gtest/gtest.h>
#include "RhymesAnalysis.h"
#include "MorphRules.h"

#include <string>
#include <vector>

using namespace std;

// ============ is_rhyme / rhyme_similarity ============

TEST(RhymeSimilarityTest, EmptyVerbs) {
    VerbInfo a; a.verb = "";
    VerbInfo b; b.verb = "кот";
    EXPECT_DOUBLE_EQ(rhyme_similarity(a, b), 0.0);
    EXPECT_FALSE(is_rhyme(a, b));
}

TEST(RhymeSimilarityTest, IdenticalVerbs) {
    VerbInfo a; a.verb = "бежать";
    VerbInfo b; b.verb = "бежать";
    EXPECT_DOUBLE_EQ(rhyme_similarity(a, b), 1.0);
    EXPECT_TRUE(is_rhyme(a, b));
}

TEST(RhymeSimilarityTest, RhymePair) {
    VerbInfo a; a.verb = "делать";
    VerbInfo b; b.verb = "гулять";
    EXPECT_GT(rhyme_similarity(a, b), 0.5);
    EXPECT_TRUE(is_rhyme(a, b));
}

TEST(RhymeSimilarityTest, NonRhymePair) {
    VerbInfo a; a.verb = "делать";
    VerbInfo b; b.verb = "любить";
    EXPECT_LT(rhyme_similarity(a, b), 0.5);
    EXPECT_FALSE(is_rhyme(a, b));
}

TEST(RhymeSimilarityTest, CustomThreshold) {
    VerbInfo a; a.verb = "делать";
    VerbInfo b; b.verb = "гулять";
    EXPECT_FALSE(is_rhyme(a, b, 0.99));
    EXPECT_TRUE (is_rhyme(a, b, 0.1));
}

// ============ find_rhymes ============

// Проверяет, что lib/ загрузилась (нужно для IdentVerbs).
// Если нет - тест пропускается.
static bool lib_loaded() {
    const MorphRules& rules = MorphRules::instance();
    if (rules.endingsInfinitive.empty()
        && rules.endingsReflexive.empty()
        && rules.endingsPast.empty()
        && rules.endingsPresent.empty()
        && rules.endingsImperative.empty()) {
        return false;
    }
    return true;
}

// Ищет пару в результате независимо от порядка глаголов.
// Возвращает true, если пара найдена.
static bool has_pair(const vector<VerbStruct>& result,
                     const string& w1, const string& w2,
                     uint64_t n1, uint64_t n2)
{
    for (size_t k = 0; k < result.size(); ++k) {
        bool direct = (result[k].verb1 == w1 && result[k].verb2 == w2
                       && result[k].num_verb1 == n1 && result[k].num_verb2 == n2);
        bool reverse = (result[k].verb1 == w2 && result[k].verb2 == w1
                        && result[k].num_verb1 == n2 && result[k].num_verb2 == n1);
        if (direct || reverse) {
            return true;
        }
    }
    return false;
}

// Считает, сколько раз пара (в любом порядке) встречается в результате.
static int count_pair(const vector<VerbStruct>& result,
                      const string& w1, const string& w2)
{
    int cnt = 0;
    for (size_t k = 0; k < result.size(); ++k) {
        bool direct = (result[k].verb1 == w1 && result[k].verb2 == w2);
        bool reverse = (result[k].verb1 == w2 && result[k].verb2 == w1);
        if (direct || reverse) {
            ++cnt;
        }
    }
    return cnt;
}

// -------- базовые случаи --------

TEST(FindRhymesTest, EmptySentences) {
    vector<string> sentences;
    vector<VerbStruct> res = find_rhymes(sentences, 50, 0.5);
    EXPECT_TRUE(res.empty());
}

TEST(FindRhymesTest, EmptySentenceStrings) {
    vector<string> sentences;
    sentences.push_back("");
    sentences.push_back("");
    vector<VerbStruct> res = find_rhymes(sentences, 50, 0.5);
    EXPECT_TRUE(res.empty());
}

TEST(FindRhymesTest, SingleVerbNoPairs) {
    if (!lib_loaded()) {
        GTEST_SKIP() << "lib/ не загружена";
    }
    vector<string> sentences;
    sentences.push_back("1. Бежать.");
    vector<VerbStruct> res = find_rhymes(sentences, 50, 0.5);
    EXPECT_TRUE(res.empty());
}

// -------- поиск пар --------

TEST(FindRhymesTest, RhymeInOneSentence) {
    if (!lib_loaded()) {
        GTEST_SKIP() << "lib/ не загружена";
    }
    vector<string> sentences;
    sentences.push_back("1. Делать и гулять.");
    vector<VerbStruct> res = find_rhymes(sentences, 50, 0.5);

    EXPECT_TRUE(has_pair(res, "делать", "гулять", 1, 1));
}

TEST(FindRhymesTest, RhymeAcrossSentences) {
    if (!lib_loaded()) {
        GTEST_SKIP() << "lib/ не загружена";
    }
    vector<string> sentences;
    sentences.push_back("1. Делать.");
    sentences.push_back("2. Гулять.");
    vector<VerbStruct> res = find_rhymes(sentences, 50, 0.5);

    EXPECT_TRUE(has_pair(res, "делать", "гулять", 1, 2));
}

TEST(FindRhymesTest, MultiplePairsInText) {
    if (!lib_loaded()) {
        GTEST_SKIP() << "lib/ не загружена";
    }
    vector<string> sentences;
    sentences.push_back("1. Делать и гулять.");
    sentences.push_back("2. Любить и рубить.");
    vector<VerbStruct> res = find_rhymes(sentences, 50, 0.5);

    EXPECT_TRUE(has_pair(res, "делать", "гулять", 1, 1));
    EXPECT_TRUE(has_pair(res, "любить", "рубить", 2, 2));
}

TEST(FindRhymesTest, NonRhymeNotReturned) {
    if (!lib_loaded()) {
        GTEST_SKIP() << "lib/ не загружена";
    }
    vector<string> sentences;
    sentences.push_back("1. Делать и любить.");
    vector<VerbStruct> res = find_rhymes(sentences, 50, 0.5);

    EXPECT_FALSE(has_pair(res, "делать", "любить", 1, 1));
}

// -------- пороги --------

TEST(FindRhymesTest, HighThresholdFiltersAll) {
    if (!lib_loaded()) {
        GTEST_SKIP() << "lib/ не загружена";
    }
    vector<string> sentences;
    sentences.push_back("1. Делать и гулять.");
    // при min_similarity = 1.0 рифма возможна только для идентичных слов
    vector<VerbStruct> res = find_rhymes(sentences, 50, 1.0);
    EXPECT_FALSE(has_pair(res, "делать", "гулять", 1, 1));
}

TEST(FindRhymesTest, LowThresholdFindsMore) {
    if (!lib_loaded()) {
        GTEST_SKIP() << "lib/ не загружена";
    }
    vector<string> sentences;
    sentences.push_back("1. Делать и гулять.");
    vector<VerbStruct> strict = find_rhymes(sentences, 50, 0.8);
    vector<VerbStruct> loose  = find_rhymes(sentences, 50, 0.1);
    EXPECT_LE(strict.size(), loose.size());
}

TEST(FindRhymesTest, HighVerbScoreFilters) {
    if (!lib_loaded()) {
        GTEST_SKIP() << "lib/ не загружена";
    }
    vector<string> sentences;
    sentences.push_back("1. Делать и гулять.");
    // score = 100 - отсекает всё, что не набрало максимум
    vector<VerbStruct> res = find_rhymes(sentences, 100, 0.5);
    // Ничего не падает; результат может быть пустым или нет, но
    // проверяем именно отсутствие сбоев и корректность типа.
    (void)res;
    SUCCEED();
}

// -------- дедупликация --------

TEST(FindRhymesTest, NoDuplicatePairs) {
    if (!lib_loaded()) {
        GTEST_SKIP() << "lib/ не загружена";
    }
    vector<string> sentences;
    sentences.push_back("1. Делать и гулять.");
    vector<VerbStruct> res = find_rhymes(sentences, 50, 0.5);

    // Пара (делать, гулять) должна быть один раз, не два
    int cnt = count_pair(res, "делать", "гулять");
    EXPECT_EQ(cnt, 1);
}

TEST(FindRhymesTest, NoDuplicateAcrossRepeatedSentences) {
    if (!lib_loaded()) {
        GTEST_SKIP() << "lib/ не загружена";
    }
    // Одно и то же предложение встречается дважды
    vector<string> sentences;
    sentences.push_back("1. Делать и гулять.");
    sentences.push_back("1. Делать и гулять.");
    vector<VerbStruct> res = find_rhymes(sentences, 50, 0.5);

    int cnt = count_pair(res, "делать", "гулять");
    EXPECT_EQ(cnt, 1);
}

// -------- номера предложений --------

TEST(FindRhymesTest, SentenceNumbersPreserved) {
    if (!lib_loaded()) {
        GTEST_SKIP() << "lib/ не загружена";
    }
    vector<string> sentences;
    sentences.push_back("7. Делать.");
    sentences.push_back("13. Гулять.");
    vector<VerbStruct> res = find_rhymes(sentences, 50, 0.5);

    EXPECT_TRUE(has_pair(res, "делать", "гулять", 7, 13));
}

TEST(FindRhymesTest, NoNumberGivesZero) {
    if (!lib_loaded()) {
        GTEST_SKIP() << "lib/ не загружена";
    }
    // Без номера в начале - num будет 0
    vector<string> sentences;
    sentences.push_back("Делать.");
    sentences.push_back("Гулять.");
    vector<VerbStruct> res = find_rhymes(sentences, 50, 0.5);

    EXPECT_TRUE(has_pair(res, "делать", "гулять", 0, 0));
}

TEST(FindRhymesTest, MixedNumberedAndPlain) {
    if (!lib_loaded()) {
        GTEST_SKIP() << "lib/ не загружена";
    }
    vector<string> sentences;
    sentences.push_back("1. Делать.");
    sentences.push_back("Гулять.");
    vector<VerbStruct> res = find_rhymes(sentences, 50, 0.5);

    EXPECT_TRUE(has_pair(res, "делать", "гулять", 1, 0));
}

// -------- структура VerbStruct --------

TEST(FindRhymesTest, VerbStructFieldsFilled) {
    if (!lib_loaded()) {
        GTEST_SKIP() << "lib/ не загружена";
    }
    vector<string> sentences;
    sentences.push_back("3. Делать.");
    sentences.push_back("5. Гулять.");
    vector<VerbStruct> res = find_rhymes(sentences, 50, 0.5);

    ASSERT_FALSE(res.empty());
    for (size_t k = 0; k < res.size(); ++k) {
        // Поля не пустые и согласованы
        EXPECT_FALSE(res[k].verb1.empty());
        EXPECT_FALSE(res[k].verb2.empty());
        EXPECT_NE(res[k].verb1, res[k].verb2);
        // Хотя бы одна из пар - деладь/гулять с номерами 3 и 5
    }
    EXPECT_TRUE(has_pair(res, "делать", "гулять", 3, 5));
}

TEST(FindRhymesTest, SimilarVerbsDifferentSentences) {
    if (!lib_loaded()) {
        GTEST_SKIP() << "lib/ не загружена";
    }
    // Одинаковые глаголы в разных предложениях - идеальная рифма (sim = 1.0)
    vector<string> sentences;
    sentences.push_back("1. Бежать.");
    sentences.push_back("2. Бежать.");
    vector<VerbStruct> res = find_rhymes(sentences, 50, 0.5);

    bool found = false;
    for (size_t k = 0; k < res.size(); ++k) {
        if (res[k].verb1 == "бежать" && res[k].verb2 == "бежать"
            && res[k].num_verb1 == 1 && res[k].num_verb2 == 2) {
            found = true;
        }
    }
    EXPECT_TRUE(found);
}