#include "RhymesAnalysis.h"
#include "Total_Header.h"
#include <gtest/gtest.h>
#include <iomanip>
#include <set>
#include <sstream>

const uint8_t min_verb_score = 50;
const double min_rhyme_score = 0.5;

TEST(RhymeStress, FindsStressInDifferentForms) {
    EXPECT_EQ(prepare_rhyme_word("ПОНЯЛА").stressed_tail, "а");
    EXPECT_EQ(prepare_rhyme_word("понял").stressed_tail, "онал");
    EXPECT_EQ(prepare_rhyme_word("Возражала").stressed_tail, "ала");
    EXPECT_EQ(prepare_rhyme_word("учиться").stressed_tail, "ица");
    EXPECT_EQ(prepare_rhyme_word("взять").stressed_tail, "ат");
    EXPECT_EQ(prepare_rhyme_word("идёт").stressed_tail, "от");
    EXPECT_EQ(prepare_rhyme_word("идет").stressed_tail, "от");
}

TEST(RhymeStress, FallsBackForUnknownOrAmbiguousWords) {
    RhymeWord ambiguous = prepare_rhyme_word("узнаю");
    RhymeWord unknown = prepare_rhyme_word("псевдогулять");
    RhymeWord known = prepare_rhyme_word("знаю");
    ASSERT_FALSE(known.stressed_tail.empty());
    EXPECT_TRUE(ambiguous.stressed_tail.empty());
    EXPECT_TRUE(unknown.stressed_tail.empty());
    double old_score = similarity(ambiguous.phonetic, known.phonetic);
    EXPECT_DOUBLE_EQ(compare_rhyme_words(ambiguous, known), old_score);
}

TEST(RhymeStress, ChecksStressedTailInBothSearchPaths) {
    RhymeWord first = prepare_rhyme_word("возражала");
    RhymeWord second = prepare_rhyme_word("провожала");
    RhymeWord third = prepare_rhyme_word("поняла");
    RhymeWord fourth = prepare_rhyme_word("вспоминала");
    EXPECT_LT(similarity(first.phonetic, second.phonetic), min_rhyme_score);
    EXPECT_GE(similarity(third.phonetic, fourth.phonetic), min_rhyme_score);
    EXPECT_DOUBLE_EQ(compare_rhyme_words(first, second), 1.0);
    EXPECT_DOUBLE_EQ(compare_rhyme_words(third, fourth), 0.0);

    vector<string> sentences = {"1) Возражала.", "2) Провожала.",
                                "3) Поняла.", "4) Вспоминала."};
    vector<VerbInfo> verbs = collect_verbs(sentences, min_verb_score);
    ASSERT_EQ(verbs.size(), 4u);
    vector<VerbStruct> pairs = find_rhymes(sentences, min_verb_score, min_rhyme_score);
    vector<RhymeStats> stats = buildRhymeStats(verbs, min_rhyme_score);
    ASSERT_EQ(pairs.size(), 3u);
    ASSERT_EQ(stats.size(), pairs.size());
    EXPECT_EQ(pairs[0].verb1, "возражала");
    EXPECT_EQ(pairs[0].verb2, "провожала");
    EXPECT_EQ(stats[0].verb1, "возражала");
    EXPECT_EQ(stats[0].verb2, "вспоминала");
}

// Сохраняем нумерацию строк, как в rhyme_preview.
vector<VerbInfo> read_corpus_verbs(istream& input) {
    vector<string> sentences;
    string line;
    size_t number = 0;
    while (getline(input, line)) {
        ++number;
        string prefix = to_string(number);
        sentences.push_back(prefix + ") " + line);
    }
    return collect_verbs(sentences, min_verb_score);
}

void print_corpus_coverage(const string& path) {
    ifstream input(path);
    ASSERT_TRUE(input.is_open()) << path;
    vector<VerbInfo> verbs = read_corpus_verbs(input);
    ASSERT_FALSE(input.bad()) << path;
    ASSERT_FALSE(verbs.empty()) << path;
    map<string, size_t> forms;
    for (const VerbInfo& verb : verbs) ++forms[verb.verb];

    vector<RhymeWord> sounds;
    size_t covered_forms = 0;
    size_t covered_tokens = 0;
    for (const auto& form : forms) {
        RhymeWord word = prepare_rhyme_word(form.first);
        if (!word.stressed_tail.empty()) {
            ++covered_forms;
            covered_tokens += form.second;
        }
        sounds.push_back(word);
    }

    size_t old_pairs = 0;
    size_t verified = 0;
    size_t candidates = 0;
    size_t added = 0;
    size_t removed = 0;
    for (size_t i = 0; i < sounds.size(); ++i) {
        for (size_t j = i + 1; j < sounds.size(); ++j) {
            bool old_match = similarity(sounds[i].phonetic, sounds[j].phonetic) >= min_rhyme_score;
            bool new_match = compare_rhyme_words(sounds[i], sounds[j]) >= min_rhyme_score;
            if (old_match) ++old_pairs;
            if (old_match && !new_match) ++removed;
            if (!old_match && new_match) ++added;
            if (!new_match) continue;
            if (sounds[i].stressed_tail.empty() || sounds[j].stressed_tail.empty()) ++candidates;
            else ++verified;
        }
    }

    cout << path << '\n'
         << "  Известно ударение у форм: " << covered_forms << '/' << forms.size()
         << ", у вхождений: " << covered_tokens << '/' << verbs.size() << '\n'
         << "  Пар до: " << old_pairs << ", после: " << verified + candidates
         << ", добавилось: " << added << ", убрано: " << removed << '\n'
         << "  С совпавшими ударными хвостами: " << verified
         << ", без полного покрытия ударений: " << candidates << '\n';
}

TEST(RhymeStress, CorpusCoverage) {
    print_corpus_coverage("test_MiM.txt");
    print_corpus_coverage("tests/text2.txt");
}

struct RhymeMetrics {
    size_t tp = 0;
    size_t fp = 0;
    size_t fn = 0;
    size_t tn = 0;
    size_t fp_pos = 0;

    void count(const string& label, bool predicted) {
        if (label == "rhyme") {
            if (predicted) ++tp;
            else ++fn;
        } else if (predicted) {
            ++fp;
            if (label == "pos") ++fp_pos;
        } else {
            ++tn;
        }
    }
};

double metric_ratio(size_t numerator, size_t denominator) {
    if (denominator == 0) return 0.0;
    return static_cast<double>(numerator) / static_cast<double>(denominator);
}

void print_rhyme_metrics(const string& name, const RhymeMetrics& metrics) {
    double precision = metric_ratio(metrics.tp, metrics.tp + metrics.fp);
    double recall = metric_ratio(metrics.tp, metrics.tp + metrics.fn);
    double f1 = metric_ratio(2 * metrics.tp, 2 * metrics.tp + metrics.fp + metrics.fn);
    cout << name << "  TP=" << metrics.tp << " FP=" << metrics.fp
         << " (часть речи: " << metrics.fp_pos << ") FN=" << metrics.fn
         << " TN=" << metrics.tn << fixed << setprecision(1)
         << "  precision=" << 100 * precision << "% recall=" << 100 * recall
         << "% F1=" << 100 * f1 << "%\n";
}

TEST(RhymeStress, MetricsOnLabeledPairs) {
    ifstream input("tests/mim_rhyme_pairs.tsv");
    ifstream corpus("test_MiM.txt");
    ASSERT_TRUE(input.is_open());
    ASSERT_TRUE(corpus.is_open());
    vector<VerbInfo> verbs = read_corpus_verbs(corpus);
    ASSERT_FALSE(corpus.bad());
    set<string> recognized;
    for (const VerbInfo& verb : verbs) recognized.insert(verb.verb);

    string line;
    ASSERT_TRUE(getline(input, line));
    size_t uncertain = 0;
    RhymeMetrics before;
    RhymeMetrics after;
    while (getline(input, line)) {
        istringstream row(line);
        string a;
        string b;
        string label;
        string extra;
        ASSERT_TRUE(row >> a >> b >> label) << line;
        ASSERT_FALSE(row >> extra) << line;
        if (label == "uncertain") {
            ++uncertain;
            continue;
        }
        ASSERT_TRUE(label == "rhyme" || label == "sound" || label == "pos") << label;
        RhymeWord first = prepare_rhyme_word(a);
        RhymeWord second = prepare_rhyme_word(b);
        bool both_verbs = recognized.count(a) != 0 && recognized.count(b) != 0;
        bool old_match = both_verbs && similarity(first.phonetic, second.phonetic) >= min_rhyme_score;
        bool new_match = both_verbs && compare_rhyme_words(first, second) >= min_rhyme_score;
        before.count(label, old_match);
        after.count(label, new_match);
    }
    ASSERT_TRUE(input.eof());
    ASSERT_GT(before.tp + before.fn, 0u);
    ASSERT_GT(before.fp + before.tn, 0u);
    cout << "Размеченная выборка MiM, спорных пар исключено: " << uncertain << '\n';
    print_rhyme_metrics("  До  ", before);
    print_rhyme_metrics("  После", after);
}
