#include <gtest/gtest.h>
#include "RhymesAnalysis.h"

// ---------------- is_vowel ----------------

TEST(IsVowelTest, AllRussianVowelsReturnTrue) {
    string_view vowels[] = {"а","е","ё","и","о","у","ы","э","ю","я"};
    for (auto c : vowels) 
		EXPECT_TRUE(is_vowel(c));
}

TEST(IsVowelTest, UppercaseVowelsAreNotVowels) {
    EXPECT_FALSE(is_vowel("А"));
    EXPECT_FALSE(is_vowel("Я"));
}

TEST(IsVowelTest, NonLettersAreNotVowels) {
    EXPECT_FALSE(is_vowel("1"));
    EXPECT_FALSE(is_vowel(" "));
    EXPECT_FALSE(is_vowel("-"));
    EXPECT_FALSE(is_vowel(""));
}

// ---------------- is_consonant ----------------

TEST(IsConsonantTest, AllRussianConsonantsReturnTrue) {
    string_view consonants[] = {"б","в","г","д","ж","з","й","к","л","м","н","п","р","с","т","ф","х","ц","ч","ш","щ"};
    for (string_view c : consonants) {
        EXPECT_TRUE(is_consonant(c));
    }
}

TEST(IsConsonantTest, VowelsAreNotConsonants) {
    string_view vowels[] = {"а","е","ё","и","о","у","ы","э","ю","я"};
    for (string_view c : vowels) {
        EXPECT_FALSE(is_consonant(c));
    }
}

TEST(IsConsonantTest, UppercaseConsonantsAreNotConsonants) {
    EXPECT_FALSE(is_consonant("Б"));
    EXPECT_FALSE(is_consonant("Щ"));
}

TEST(IsConsonantTest, NonLettersAreNotConsonants) {
    EXPECT_FALSE(is_consonant("5"));
    EXPECT_FALSE(is_consonant(","));
    EXPECT_FALSE(is_consonant(" "));
}

// ---------------- lower_word ----------------

TEST(LowerWordTest, CyrillicUppercaseIsLowered) {
    EXPECT_EQ(lower_word("ПРИВЕТ"), "привет");
}

TEST(LowerWordTest, MixedCaseIsLowered) {
    EXPECT_EQ(lower_word("ПрИвЕт"), "привет");
}

TEST(LowerWordTest, AlreadyLowercaseUnchanged) {
    EXPECT_EQ(lower_word("привет"), "привет");
}

TEST(LowerWordTest, LatinAlsoLowered) {
    EXPECT_EQ(lower_word("HeLLo"), "hello");
}

TEST(LowerWordTest, EmptyStringReturnsEmpty) {
    EXPECT_EQ(lower_word(""), "");
    EXPECT_EQ(lower_word(std::string_view{}), "");
}

TEST(LowerWordTest, DigitsAndSymbolsUntouched) {
    EXPECT_EQ(lower_word("AbC-123!"), "abc-123!");
}

TEST(LowerWordTest, StringViewArgument) {
    std::string src = "МОСКВА";
    std::string_view sv(src);
    EXPECT_EQ(lower_word(sv), "москва");
    // исходная строка не меняется
    EXPECT_EQ(src, "МОСКВА");
}

// ---------------- to_phonetic ----------------
TEST(ToPhoneticTest, EmptyString) {
    EXPECT_EQ(to_phonetic(""), "");
}

TEST(ToPhoneticTest, SimpleWordNoChange) {
    EXPECT_EQ(to_phonetic("кот"), "кот");
}

TEST(ToPhoneticTest, UppercaseLowered) {
    EXPECT_EQ(to_phonetic("КОТ"), "кот");
}

TEST(ToPhoneticTest, MixedCaseLowered) {
    EXPECT_EQ(to_phonetic("КоТ"), "кот");
}

// ----- йотированные гласные -----
TEST(ToPhoneticTest, YaToA) {
    EXPECT_EQ(to_phonetic("яма"), "ама");
}

TEST(ToPhoneticTest, YaAtEnd) {
    EXPECT_EQ(to_phonetic("земля"), "зэмла");
}

TEST(ToPhoneticTest, YuToU) {
    EXPECT_EQ(to_phonetic("юг"), "ук");
}

TEST(ToPhoneticTest, YoToO) {
    EXPECT_EQ(to_phonetic("ёж"), "ош");
}

TEST(ToPhoneticTest, EToE) {
    EXPECT_EQ(to_phonetic("еда"), "эда");
}

TEST(ToPhoneticTest, YotirovannyeVowel) {
    EXPECT_EQ(to_phonetic("ёлка"), "олка");
}

// ----- выпадение ь / ъ -----
TEST(ToPhoneticTest, SoftSignRemoved) {
    EXPECT_EQ(to_phonetic("соль"), "сол");
}

TEST(ToPhoneticTest, SoftSignRemovedAtEnd) {
    EXPECT_EQ(to_phonetic("мышь"), "мыш");
}

TEST(ToPhoneticTest, HardSignRemoved) {
    EXPECT_EQ(to_phonetic("объём"), "обом");
}

TEST(ToPhoneticTest, OnlySoftSign) {
    EXPECT_EQ(to_phonetic("ь"), "");
}

TEST(ToPhoneticTest, FinalDevoicingB) {
    EXPECT_EQ(to_phonetic("зуб"), "зуп");
}

TEST(ToPhoneticTest, FinalDevoicingV) {
    EXPECT_EQ(to_phonetic("лев"), "лэф");
}

TEST(ToPhoneticTest, FinalDevoicingG) {
    EXPECT_EQ(to_phonetic("друг"), "друк");
}

TEST(ToPhoneticTest, FinalDevoicingD) {
    EXPECT_EQ(to_phonetic("сад"), "сат");
}

TEST(ToPhoneticTest, FinalDevoicingZh) {
    EXPECT_EQ(to_phonetic("нож"), "нош");
}

TEST(ToPhoneticTest, FinalDevoicingZ) {
    EXPECT_EQ(to_phonetic("мороз"), "морос");
}

TEST(ToPhoneticTest, VoicelessNotDevoiced) {
    EXPECT_EQ(to_phonetic("кот"), "кот");
    EXPECT_EQ(to_phonetic("суп"), "суп");
    EXPECT_EQ(to_phonetic("сок"), "сок");
}

TEST(ToPhoneticTest, SonorantNotDevoiced) {
    // сонорные м, н, л, р, й не оглушаются
    EXPECT_EQ(to_phonetic("дом"), "дом");
    EXPECT_EQ(to_phonetic("сон"), "сон");
    EXPECT_EQ(to_phonetic("стол"), "стол");
    EXPECT_EQ(to_phonetic("мир"), "мир");
    EXPECT_EQ(to_phonetic("май"), "май");
}

// ----- прочее -----
TEST(ToPhoneticTest, LatinPreserved) {
    EXPECT_EQ(to_phonetic("test"), "test");
    EXPECT_EQ(to_phonetic("HELLO"), "hello");
}
TEST(ToPhoneticTest, DigitsPreserved) {
    EXPECT_EQ(to_phonetic("кот123"), "кот123");
}

// ----- проверка рифмующихся пар -----
TEST(ToPhoneticTest, RhymePairDrugDruk) {
    // «друг» и «друк» звучат одинаково
    EXPECT_EQ(to_phonetic("друг"), to_phonetic("друк"));
}

TEST(ToPhoneticTest, RhymePairLedLet) {
    // «лед» и «лет» звучат одинаково
    EXPECT_EQ(to_phonetic("лед"), to_phonetic("лет"));
}

TEST(ToPhoneticTest, RhymePairLyubovKrov) {
    string p1 = to_phonetic("любовь");
    string p2 = to_phonetic("кровь");
    ASSERT_GE(p1.size(), 4u);
    ASSERT_GE(p2.size(), 4u);
    EXPECT_EQ(p1.substr(p1.size() - 4), p2.substr(p2.size() - 4));
}

TEST(ToPhoneticTest, RhymePairKlyuchLuch) {
    string p1 = to_phonetic("ключ");   // клуч
    string p2 = to_phonetic("луч");    // луч
    ASSERT_GE(p1.size(), 4u);
    ASSERT_GE(p2.size(), 4u);
    EXPECT_EQ(p1.substr(p1.size() - 4), p2.substr(p2.size() - 4));
}

//------------- common_suffix_len -----------------------

TEST(CommonSuffixLenTest, BothEmpty) {
    EXPECT_EQ(common_suffix_len("", ""), 0u);
}

TEST(CommonSuffixLenTest, OneEmpty) {
    EXPECT_EQ(common_suffix_len("кот", ""), 0u);
    EXPECT_EQ(common_suffix_len("", "кот"), 0u);
}

TEST(CommonSuffixLenTest, IdenticalSingleChar) {
    EXPECT_EQ(common_suffix_len("а", "а"), 1u);
    EXPECT_EQ(common_suffix_len("т", "т"), 1u);
}

TEST(CommonSuffixLenTest, IdenticalWords) {
    EXPECT_EQ(common_suffix_len("кот", "кот"), 3u);
    EXPECT_EQ(common_suffix_len("привет", "привет"), 6u);
}

TEST(CommonSuffixLenTest, NoCommonSuffix) {
    EXPECT_EQ(common_suffix_len("кот", "пёс"), 0u);   // т vs с
    EXPECT_EQ(common_suffix_len("аб", "вг"), 0u);     // б vs г
    EXPECT_EQ(common_suffix_len("дом", "лес"), 0u);   // м vs с
}

TEST(CommonSuffixLenTest, PartialSuffixOneChar) {
    // "кот" vs "кит": общий только "т"
    EXPECT_EQ(common_suffix_len("кот", "кит"), 1u);
}

TEST(CommonSuffixLenTest, PartialSuffixTwoChars) {
    // "кот" vs "крот": общий "от"
    EXPECT_EQ(common_suffix_len("кот", "крот"), 2u);
    // "кот" vs "бегемот": общий "от"
    EXPECT_EQ(common_suffix_len("кот", "бегемот"), 2u);
}

TEST(CommonSuffixLenTest, PartialSuffixThreeChars) {
    // "делат" vs "гулат": общий "лат"
    EXPECT_EQ(common_suffix_len("делат", "гулат"), 3u);
}

TEST(CommonSuffixLenTest, OneIsSuffixOfOther) {
    EXPECT_EQ(common_suffix_len("мот", "бегемот"), 3u);
    EXPECT_EQ(common_suffix_len("от", "кот"), 2u);
    EXPECT_EQ(common_suffix_len("кот", "кот"), 3u);
}

TEST(CommonSuffixLenTest, IdenticalLatin) {
    EXPECT_EQ(common_suffix_len("cat", "cat"), 3u);
    EXPECT_EQ(common_suffix_len("abc", "xbc"), 2u);
}

TEST(CommonSuffixLenTest, MixedLatinCyrillic) {
    // "aкот" vs "бкот" - общий суффикс "кот" (3 символа)
    EXPECT_EQ(common_suffix_len("aкот", "бкот"), 3u);
}

TEST(CommonSuffixLenTest, Utf8Boundaries) {
    // Один символ = один, не два/три байта
    EXPECT_EQ(common_suffix_len("я", "я"), 1u);
    EXPECT_EQ(common_suffix_len("яя", "яя"), 2u);
    EXPECT_EQ(common_suffix_len("ая", "уя"), 1u);
}

//-------------- similarity ------------------------

TEST(SimilarityTest, BothEmpty) {
    EXPECT_DOUBLE_EQ(similarity("", ""), 0.0);
}

TEST(SimilarityTest, OneEmpty) {
    EXPECT_DOUBLE_EQ(similarity("кот", ""), 0.0);
    EXPECT_DOUBLE_EQ(similarity("", "кот"), 0.0);
}

TEST(SimilarityTest, Identical) {
    EXPECT_DOUBLE_EQ(similarity("кот", "кот"), 1.0);
    EXPECT_DOUBLE_EQ(similarity("привет", "привет"), 1.0);
    EXPECT_DOUBLE_EQ(similarity("а", "а"), 1.0);
}

TEST(SimilarityTest, NoCommonSuffix) {
    EXPECT_DOUBLE_EQ(similarity("кот", "пёс"), 0.0);
    EXPECT_DOUBLE_EQ(similarity("аб", "вг"), 0.0);
}

TEST(SimilarityTest, OneIsSuffixOfOther) {
    // "кот" - суффикс "бегемот" => common=2, min=3 => 2/3
    double s = similarity("кот", "бегемот");
    EXPECT_NEAR(s, 2.0 / 3.0, 1e-9);

    // "от" - суффикс "кот" => common=2, min=2 => 1.0
    EXPECT_DOUBLE_EQ(similarity("от", "кот"), 1.0);
}

TEST(SimilarityTest, PartialMatch) {
    // "делат" vs "гулат" => common=3, min=5 => 0.6
    EXPECT_NEAR(similarity("делат", "гулат"), 0.6, 1e-9);
}

TEST(SimilarityTest, Symmetric) {
    EXPECT_DOUBLE_EQ(similarity("кот", "крот"), similarity("крот", "кот"));
    EXPECT_DOUBLE_EQ(similarity("кот", "бегемот"), similarity("бегемот", "кот"));
    EXPECT_DOUBLE_EQ(similarity("делат", "гулат"), similarity("гулат", "делат"));
}

TEST(SimilarityTest, RangeZeroToOne) {
    EXPECT_GE(similarity("а", "яблоко"), 0.0);
    EXPECT_LE(similarity("а", "яблоко"), 1.0);
    EXPECT_GE(similarity("кот", "крот"), 0.0);
    EXPECT_LE(similarity("кот", "крот"), 1.0);
}

TEST(SimilarityTest, LatinWorks) {
    EXPECT_DOUBLE_EQ(similarity("cat", "cat"), 1.0);
    EXPECT_DOUBLE_EQ(similarity("cat", "scat"), 1.0);   // "cat" - суффикс "scat"
    EXPECT_NEAR(similarity("cat", "cut"), 1.0 / 3.0, 1e-9); // только "t"
}

TEST(SimilarityTest, IdenticalPhoneticRhymes) {
    // Фонетически идентичные слова (после to_phonetic) => 1.0
    EXPECT_DOUBLE_EQ(similarity(to_phonetic("друг"), to_phonetic("друк")), 1.0);
    EXPECT_DOUBLE_EQ(similarity(to_phonetic("лед"),  to_phonetic("лет")),  1.0);
    EXPECT_DOUBLE_EQ(similarity(to_phonetic("зуб"),  to_phonetic("зуп")),  1.0);
}

TEST(SimilarityTest, RealRhymesHighSimilarity) {
    // Неплохие глагольные рифмы => высокий коэффициент
    EXPECT_GT(similarity(to_phonetic("делать"), to_phonetic("гулять")), 0.5);
    EXPECT_GT(similarity(to_phonetic("любить"), to_phonetic("губить")), 0.5);
}

TEST(SimilarityTest, NonRhymesLowSimilarity) {
    // Совсем не рифмуются
    EXPECT_LT(similarity(to_phonetic("делать"), to_phonetic("писать")), 0.5);
    EXPECT_LT(similarity(to_phonetic("кот"),    to_phonetic("пёс")),    0.5);
}