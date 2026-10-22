#include <gtest/gtest.h>
#include "IdentVerbs.h"

using namespace iverbs;

// проверка корректности парсинга

TEST(Parse, SimpleSentence) {
    IdentVerbs iv("Жил да был черный кот за углом.");
    ASSERT_EQ(iv.words().size(), 7u);
    EXPECT_EQ(iv.words()[0], "жил");
    EXPECT_EQ(iv.words()[6], "углом");
}

TEST(Parse, PunctuationAttached) {
    IdentVerbs iv("И кота ненавидел: весь дом (кто-то)!");
    ASSERT_EQ(iv.words().size(), 6u);
    EXPECT_EQ(iv.words()[5], "кто-то");
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
    ASSERT_EQ(iv.words().size(), 5u);
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

// ================= analysis_word: точечные проверки =================

TEST(AnalysisWord, FilterEnglish) {
    auto info = IdentVerbs::analysis_word("hello");
    EXPECT_EQ(info.score, 0);
    EXPECT_EQ(info.verb, "hello");
}

TEST(AnalysisWord, FilterTooShort) {
    auto info = IdentVerbs::analysis_word("он");
    // 2 буквы = 4 байта, но окончания нет → не глагол
    EXPECT_LT(info.score, 50);
}

TEST(AnalysisWord, Infinitive) {
    auto info = IdentVerbs::analysis_word("читать");
    EXPECT_GE(info.score, 80);
    EXPECT_EQ(info.ending, "ать");
    // "читать" = 12 байт, "ать" = 6 байт → stem = "чит"
    EXPECT_EQ(info.stem, "чит");
}

TEST(AnalysisWord, InfinitiveReflexive) {
    auto info = IdentVerbs::analysis_word("мыться");
    EXPECT_GE(info.score, 90);
    // самое длинное совпадение — "ться" (8 байт, 95), а не "ся"
    EXPECT_EQ(info.ending, "ться");
}

TEST(AnalysisWord, PastReflexive) {
    auto info = IdentVerbs::analysis_word("мылся");
    // "лся" (6 байт, 85) > "ся" (4 байта, 40)
    EXPECT_GE(info.score, 80);
    EXPECT_EQ(info.ending, "лся");
}

TEST(AnalysisWord, Present) {
    auto info = IdentVerbs::analysis_word("читаешь");
    EXPECT_GE(info.score, 60);
    EXPECT_EQ(info.ending, "ешь");
}

TEST(AnalysisWord, NounEndingInL) {
    auto info = IdentVerbs::analysis_word("стол");
    // окончание "-л" даёт 25 → ниже порога
    EXPECT_LT(info.score, 5);
}

// ================= analysis_all_words / ident_verbs =================

TEST(IdentVerbsSentence, SimpleVerbs) {
    IdentVerbs iv("Кот бежал и спал.");
    auto verbs = iv.ident_verbs(40);
    ASSERT_EQ(verbs.size(), 2u);
    EXPECT_EQ(verbs[0].verb, "бежал");
    EXPECT_EQ(verbs[1].verb, "спал");
}

TEST(IdentVerbsSentence, InfinitiveOnly) {
    IdentVerbs iv("Он хочет читать книгу.");
    auto verbs = iv.ident_verbs(40);
    // "хочет" — "-ет" (45), "читать" — "-ать" (85)
    ASSERT_EQ(verbs.size(), 2u);
    EXPECT_EQ(verbs[0].verb, "хочет");
    EXPECT_EQ(verbs[1].verb, "читать");
}

// ================= отчёт по точности =================

TEST(Accuracy, ReportOnBulgakov) {
    struct Case {
        std::string sentence;
        std::set<std::string> expected;
    };

    std::vector<Case> cases = {
        {"Маргарита Николаевна сидела перед трюмо в одном купальном халате, наброшенном на голое тело, и в замшевых черных туфлях.",
            {"сидела"}},

        {"Золотой браслет с часиками лежал перед Маргаритой Николаевной рядом с коробочкой, полученной от Азазелло, и Маргарита не сводила глаз с циферблата.",
            {"лежал", "сводила"}},

        {"Временами ей начинало казаться, что часы сломались и стрелки не движутся.",
            {"начинало", "казаться", "сломались", "движутся"}},

        {"Но они двигались, хотя и очень медленно, как будто прилипая, и наконец длинная стрелка упала на двадцать девятую минуту десятого.",
            {"двигались", "упала"}},

        {"Сердце Маргариты страшно стукнуло, так что она не смогла даже сразу взяться за коробочку.",
            {"стукнуло", "смогла", "взяться"}},

        {"Справившись с собою, Маргарита открыла ее и увидела в коробочке жирный желтоватый крем.",
            {"открыла", "увидела"}},

        {"Ей показалось, что он пахнет болотной тиной.",
            {"показалось", "пахнет"}},

        {"Кончиком пальца Маргарита выложила небольшой мазочек крема на ладонь, причем сильнее запахло болотными травами и лесом, и затем ладонью начала втирать крем в лоб и щеки.",
            {"выложила", "запахло", "начала", "втирать"}},

        {"Крем легко мазался и, как показалось Маргарите, тут же испарялся.",
            {"мазался", "показалось", "испарялся"}},

        {"Сделав несколько втираний, Маргарита глянула в зеркало и уронила коробочку прямо на стекло часов, от чего оно покрылось трещинами.",
            {"глянула", "уронила", "покрылось"}},

        {"Маргарита закрыла глаза, потом глянула еще раз и бурно расхохоталась.",
            {"закрыла", "глянула", "расхохоталась"}},

        {"Ощипанные по краям в ниточку пинцетом брови сгустились и черными ровными дугами легли над зазеленевшими глазами.",
            {"сгустились", "легли"}},

        {"Тонкая вертикальная морщинка, перерезавшая переносицу, появившаяся тогда, в октябре, когда пропал мастер, бесследно пропала.",
            {"пропал", "пропала"}},

        {"Исчезли и желтенькие тени у висков, и две чуть заметные сеточки у наружных углов глаз.",
            {"исчезли"}},

        {"Кожа щек налилась ровным розовым цветом, лоб стал бел и чист, а парикмахерская завивка волос развилась.",
            {"налилась", "стал", "развилась"}},

        {"На тридцатилетнюю Маргариту из зеркала глядела от природы кудрявая черноволосая женщина лет двадцати, безудержно хохочущая, скалящая зубы.",
            {"глядела"}},
    };

    const uint8_t threshold = 50;
	// tp - (true positive) Нашли и это глагол
	// fp - (false positive) Нашли, но это не глагол
	// fn - (false negative) Не нашли, но это глагол
    int tp = 0, fp = 0, fn = 0;

    for (const auto& c : cases) {
        IdentVerbs iv(c.sentence);
        auto found_vec = iv.ident_verbs(threshold);

        std::set<std::string> found;
        for (const auto& f : found_vec) found.insert(f.verb);

        for (const auto& f : found) {
            if (!c.expected.count(f)) {
                ++fp;
                std::cout << "[FP] " << f << "  в: " << c.sentence << "\n";
            }
        }
        for (const auto& e : c.expected) {
            if (!found.count(e)) {
                ++fn;
                std::cout << "[FN] пропущен: " << e << "  в: " << c.sentence << "\n";
            }
        }
        for (const auto& f : found) {
            if (c.expected.count(f)) ++tp;
        }
    }

    const double precision = (tp + fp) ? double(tp) / (tp + fp) : 0.0;
    const double recall    = (tp + fn) ? double(tp) / (tp + fn) : 0.0;

    std::cout << "\n========== Отчёт по точности (пример Булгаков) ==========\n"
              << "Предложений: " << cases.size() << "\n"
              << "Порог: " << int(threshold) << "\n"
              << "Нашли гл. = " << tp << "  Нашли не гл. = " << fp << "  Не нашли гл. = " << fn << "\n"
              << "Точность = " << precision << "\n"
              << "Сколько из всех гл. найдено    = " << recall << "\n"
              << "===========================================================\n";
}