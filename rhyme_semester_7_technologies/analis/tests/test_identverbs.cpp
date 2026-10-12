#include <gtest/gtest.h>
#include "IdentVerbs.h"

using namespace iverbs;

// проверка корректности парсинга

TEST(Parse, SimpleSentence) {
    IdentVerbs iv("Жил да был черный кот за углом.");
    ASSERT_EQ(iv.words().size(), 6u);
    EXPECT_EQ(iv.words()[0], "жил");
    EXPECT_EQ(iv.words()[5], "углом");
}

TEST(Parse, PunctuationAttached) {
    IdentVerbs iv("И кота ненавидел: весь дом (кто-то)!");
    ASSERT_EQ(iv.words().size(), 5u);
    EXPECT_EQ(iv.words()[4], "кто-то");
}

TEST(Parse, ApostropheInsideWord) {
    IdentVerbs iv("Д'Артаньян и кто-то ещё");
    ASSERT_EQ(iv.words().size(), 4u);
    EXPECT_EQ(iv.words()[0], "д'артаньян");
    EXPECT_EQ(iv.words()[1], "и");
    EXPECT_EQ(iv.words()[2], "кто-то");
    EXPECT_EQ(iv.words()[3], "ещё");
}

TEST(Parse, DashBetweenWords) {
    IdentVerbs iv("Печь пироги - это печь нужна.");
    ASSERT_EQ(iv.words().size(), 6u);
    EXPECT_EQ(iv.words()[0], "печь");
    EXPECT_EQ(iv.words()[1], "пироги");
    EXPECT_EQ(iv.words()[2], "это");
}

TEST(Parse, LotsOfSpacesAndDots) {
    IdentVerbs iv("  ...тут многоточие...  и   пробелы  ");
    ASSERT_EQ(iv.words().size(), 4u);
    EXPECT_EQ(iv.words()[0], "тут");
    EXPECT_EQ(iv.words()[1], "многоточие");
    EXPECT_EQ(iv.words()[2], "и");
    EXPECT_EQ(iv.words()[3], "пробелы");
}

TEST(Parse, EmptyString) {
    IdentVerbs iv("");
    EXPECT_TRUE(iv.words().empty());
}

TEST(Parse, OnlySeparators) {
    IdentVerbs iv("   !!!   ???   ");
    EXPECT_TRUE(iv.words().empty());
}

// проверка корректности нормализации

TEST(Normalize, Latin) {
    IdentVerbs iv("Hello, world!?");
    ASSERT_EQ(iv.words().size(), 2u);
    EXPECT_EQ(iv.words()[0], "hello");
    EXPECT_EQ(iv.words()[1], "world");
}

TEST(Normalize, CyrillicUpperToLower) {
    IdentVerbs iv("ПРИВЕТ Мир");
    ASSERT_EQ(iv.words().size(), 2u);
    EXPECT_EQ(iv.words()[0], "привет");
    EXPECT_EQ(iv.words()[1], "мир");
}

TEST(Normalize, YoLetter) {
    IdentVerbs iv("ЁЖИК ёжик Ёлка");
    ASSERT_EQ(iv.words().size(), 3u);
    EXPECT_EQ(iv.words()[0], "ёжик");
    EXPECT_EQ(iv.words()[1], "ёжик");
    EXPECT_EQ(iv.words()[2], "ёлка");
}

TEST(Normalize, MixedCase) {
    IdentVerbs iv("СтЕкЛо СТЕКЛО");
    ASSERT_EQ(iv.words().size(), 2u);
    EXPECT_EQ(iv.words()[0], "стекло");
    EXPECT_EQ(iv.words()[1], "стекло");
}