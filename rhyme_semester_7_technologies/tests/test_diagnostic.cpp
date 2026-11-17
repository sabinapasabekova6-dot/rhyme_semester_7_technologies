// tests/test_diagnostic.cpp
#include <gtest/gtest.h>
#include "MorphRules.h"
#include "IdentVerbs.h"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

static std::string hex_bytes(const std::string& s) {
    std::string out;
    char buf[4];
    for (unsigned char c : s) {
        snprintf(buf, sizeof(buf), "%02X ", c);
        out += buf;
    }
    if (!out.empty() && out.back() == ' ') out.pop_back();
    return out;
}

// ---- 1. Доступны ли файлы из текущей рабочей директории ----
TEST(Diagnostic, FilesOpenableFromCwd) {
    const char* files[] = {
        "lib/endings.txt", "lib/suffixes.txt",
        "lib/participles.txt", "lib/nominal_suffixes.txt",
        "lib/blacklist.txt",
    };
    for (const char* p : files) {
        std::ifstream f(p, std::ios::binary);
        bool ok = static_cast<bool>(f);
        std::cout << "[FileAccess] " << p << " -> "
                  << (ok ? "OK" : "NOT FOUND") << "\n";
        EXPECT_TRUE(ok) << "Не открывается " << p
                        << " — вероятно, не та CWD.";
    }
}

// ---- 2. Байты файлов: UTF-8? BOM? CJK-мусор? ----
TEST(Diagnostic, RawFileBytesLookLikeUtf8) {
    const char* files[] = { "lib/endings.txt", "lib/suffixes.txt" };
    for (const char* p : files) {
        std::ifstream f(p, std::ios::binary);
        if (!f) { ADD_FAILURE() << "cannot open " << p; continue; }
        std::string data((std::istreambuf_iterator<char>(f)),
                         std::istreambuf_iterator<char>());
        std::cout << "[RawBytes] " << p
                  << " size=" << data.size() << "\n";

        if (data.size() >= 3
            && (unsigned char)data[0] == 0xEF
            && (unsigned char)data[1] == 0xBB
            && (unsigned char)data[2] == 0xBF) {
            ADD_FAILURE() << p << ": есть UTF-8 BOM. BOM сломает "
                          << "разбор [section] — сохрани без BOM.";
        }
        // ни одного байта 0xD0 — значит не кириллица UTF-8
        if (data.find("\xD0") == std::string::npos) {
            ADD_FAILURE() << p << ": нет ни одного байта 0xD0 — "
                          << "это не UTF-8 кириллица.";
        }
        // характерный мусор (褜 = E8 A4 8C)
        if (data.find("\xE8\xA4") != std::string::npos) {
            ADD_FAILURE() << p << ": найден CJK-мусор 0xE8 0xA4... — "
                          << "файл сохранён не в UTF-8, а содержимое — "
                          << "уже испорченные байты.";
        }
    }
}

// ---- 3. Что реально попало в таблицы ----
TEST(Diagnostic, LoadedTablesNonEmpty) {
    const auto& r = MorphRules::instance();
    struct Row { const char* name; size_t n; };
    Row rows[] = {
        {"endingsInfinitive",  r.endingsInfinitive.size()},
        {"endingsReflexive",   r.endingsReflexive.size()},
        {"endingsPast",        r.endingsPast.size()},
        {"endingsPresent",     r.endingsPresent.size()},
        {"endingsImperative",  r.endingsImperative.size()},
        {"suffixesVerbal",     r.suffixesVerbal.size()},
        {"suffixesNominal",    r.suffixesNominal.size()},
        {"blacklist",          r.blacklist.size()},
        {"gerundSuffixes",     r.gerundSuffixes.size()},
        {"participleSuffixes", r.participleSuffixes.size()},
    };
    for (auto& row : rows) {
        std::cout << "[Rules] " << row.name << " = " << row.n << "\n";
        EXPECT_GT(row.n, 0u) << "Таблица " << row.name << " пуста.";
    }
}

// ---- 4. Есть ли правило "ать" (D0 B0 D1 82 D1 8C) ----
TEST(Diagnostic, InfinitiveAtRuleExists) {
    const auto& r = MorphRules::instance();
    const std::string at = "\xD0\xB0\xD1\x82\xD1\x8C";
    bool found = false;
    for (const auto& rule : r.endingsInfinitive)
        if (rule.text == at) { found = true; break; }

    if (!found) {
        std::cout << "Первые 5 правил endingsInfinitive (hex):\n";
        int i = 0;
        for (const auto& rule : r.endingsInfinitive) {
            if (++i > 5) break;
            std::cout << "  " << hex_bytes(rule.text)
                      << "  score=" << (int)rule.score << "\n";
        }
    }
    EXPECT_TRUE(found) << "Правила 'ать' нет. Сравни hex выше с ожидаемым "
                       << "D0 B0 D1 82 D1 8C.";
}

// ---- 5. Сквозной прогон analysis_word ----
TEST(Diagnostic, AnalysisWordProbe) {
    const char* words[] = {"читать", "говорить", "делать", "идти"};
    for (const char* w : words) {
        VerbInfo info = IdentVerbs::analysis_word(w);
        std::cout << "word=" << w
                  << " score=" << (int)info.score
                  << " stem=[" << info.stem << "]"
                  << " ending=[" << info.ending << "]"
                  << " ending_hex=[" << hex_bytes(info.ending) << "]\n";
    }
    VerbInfo r = IdentVerbs::analysis_word("читать");
    EXPECT_GT(r.score, 5) << "score=5 => ни одно окончание не совпало.";
}