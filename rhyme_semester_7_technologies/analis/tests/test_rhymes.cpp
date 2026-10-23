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
