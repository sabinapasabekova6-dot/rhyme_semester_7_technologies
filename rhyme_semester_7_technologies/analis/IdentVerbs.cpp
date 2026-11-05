#include "IdentVerbs.h"
#include "MorphRules.h"
#include <cctype>
#include <algorithm>
#include <utility>

using std::string_view, std::move, std::string;

namespace {

	// Универсальный поиск самого длинного (а при равной длине - самого
	// весомого) правила, которым заканчивается слово
	// Возвращает указатель на лучшее правило либо best (если ничего не нашли).
	// Параметры:
	//     word  - слово, в котором ищем окончание/суффикс;
	//     table - таблица правил (окончания или суффиксы);
	//     best  - текущий лучший результат (для последовательного вызова
	//             по нескольким таблицам; при первом вызове - nullptr).
	const MorphemeRule* try_rule_table(string_view word,
									const vector<MorphemeRule>& table,
									const MorphemeRule* best) {
		size_t word_s = word.size();
		for (const MorphemeRule& rule : table) {
			size_t rule_s = rule.text.size();
			if (word_s <= rule_s) continue; // слово не длиннее правила
			if (word.compare(word_s - rule_s, rule_s, rule.text) != 0) continue; // не совпало
			if (!best
				|| rule_s > best->text.size()
				|| (rule_s == best->text.size() && rule.score > best->score)) {
				best = &rule;
			}
		}
		return best;
	}

	// Проверка "слово в чёрном списке".
	bool is_in_blacklist(string_view word, const vector<string>& blacklist) {
		for (const string& w : blacklist)
			if (word == w) return true;
		return false;
	}

	// Проверяет, оканчивается ли word на suf
	// Возвращает true, если word длиннее suf и последние suf.size()
	// байт word совпадают с suf
	bool ends_with(string_view word, string_view suf) {
		if (word.size() <= suf.size()) return false; // слово не длиннее суффикса
		return word.compare(word.size() - suf.size(), suf.size(), suf) == 0;
	}

	// Проверка "основа оканчивается на именной суффикс"
	bool has_nominal_suffix(string_view word, const vector<string>& suffixes) {
		for (const string& m : suffixes) {
			if (ends_with(word, m)) return true;
		}
		return false;
	}

	// Проверка "слово - деепричастие/причастие"
	bool is_participle(string_view word, const vector<string>& gerunds, const vector<string>& participles) {
		for (const string& s : gerunds) 
			if (ends_with(word, s)) return true;
    	for (const string& s : participles) 
			if (ends_with(word, s)) return true;
    	return false;
	}
}

IdentVerbs::IdentVerbs(const string& src) {
	parse_sentence(src);
	normalize();

}

// Метод анализирующий одно слово и возвращающий по нему статистику 
// Возвращаемые значения:
// 		VerbInfo - информация по глаголу, где:
// 			score = 0 - слово не глагол
//				  = 5 - идентифицировать не вышло, вероятно не глагол
//                = <число> - найдено некоторое совпадение
VerbInfo IdentVerbs::analysis_word(const string& word) {
    // Один раз за вызов получаем ссылку на общий словарь.
    const MorphRules& rules = MorphRules::instance();

    VerbInfo info;
    info.verb = word;

    // слишком короткое или не кириллица - точно не наш случай
    if (!is_cyrillic_word(word) || word.size() < 4) {
        info.score = 0;
        return info;
    }

    // чёрный список
    if (is_in_blacklist(word, rules.blacklist)) {
        info.score = 0;
        return info;
    }

    string_view sv = word; // рабочая обрезаемая копия
    uint32_t total_score = 0;
    string found_ending;
    string found_suffix;

    // причастия/деепричастия — не глаголы
    if (is_participle(sv, rules.gerundSuffixes, rules.participleSuffixes)) {
        info.score = 0;
        return info;
    }

    // ищем самое длинное окончание по всем пяти таблицам сразу
    const MorphemeRule* best_end = nullptr;
    best_end = try_rule_table(sv, rules.endingsInfinitive, best_end);
    best_end = try_rule_table(sv, rules.endingsReflexive,  best_end);
    best_end = try_rule_table(sv, rules.endingsPast,       best_end);
    best_end = try_rule_table(sv, rules.endingsPresent,    best_end);
    best_end = try_rule_table(sv, rules.endingsImperative, best_end);

    // таблицы пусты (нет lib/) или ни одно окончание не подошло
    if (!best_end) {
        info.score = 5;        // «вероятно не глагол»
        return info;
    }

    // отрезаем найденное окончание
    sv.remove_suffix(best_end->text.size());
    found_ending = best_end->text;
    total_score += best_end->score;

    if (has_nominal_suffix(sv, rules.suffixesNominal)) {
        info.score = 0;
        return info;
    }

    // ищем глагольный суффикс в остатке
    const MorphemeRule* best_suf = try_rule_table(sv, rules.suffixesVerbal, nullptr);
    if (best_suf) {
        sv.remove_suffix(best_suf->text.size());
        found_suffix = best_suf->text;
        total_score += best_suf->score;
    }

    info.stem   = string(sv);
    info.ending = found_suffix + found_ending;
    info.score  = static_cast<uint8_t>(std::min<uint32_t>(total_score, 100u));
    return info;
}

// Метод анализирующий и возвращающий статистику по всем словам предложения 
vector<VerbInfo> IdentVerbs::analysis_all_words() {
	vector<VerbInfo> out;
	for (const string& w : sentence_){
		VerbInfo info = analysis_word(w);
        info.num = num_;
        out.push_back(std::move(info));
	}
	return out;
}

// Метод возвращающий только слова с установленным порогом "похожести" на глагол
// по умолчанию вес более 50 (от 1 до 100)
vector<VerbInfo> IdentVerbs::ident_verbs(uint8_t min_score) {
	vector<VerbInfo> out;
	for (const auto& w : sentence_) {
		VerbInfo info = analysis_word(w);
		if (info.score >= min_score){
			info.num = num_;
			out.push_back(move(info));
		}
	}
	return out;
}	

// Метод проверки, является ли переданный символ (ch), знаком
// пунктуации или специальным символом
bool IdentVerbs::is_separator(unsigned char ch) {
	// все символы лежат в диапазоне значений <128, поэтому все что больше нас не интересует
	if(ch >= 128) return false;
	if (isspace(ch)) return true;
	string_view symbols = ".,;:!?\"()[]{}<>/\\|*&^%#@$~";
	// приведение к char допустимо, интересующие буквы с >=128 а символы < 128
	bool flag = (symbols.find(static_cast<char>(ch)) != string_view::npos);
	// npos максимально возможное значение для size_t 
	// как некое спец значение для идентификации значения "не найдено" 
	return flag;
}

// Метод возвращающая структуру слов, содержащихся в переданном предложении src без знаков пунктуации
void IdentVerbs::parse_sentence(const string& src){
	string current; // промежуточная строка для "накопления" букв слова
	for (size_t i = 0; i < src.size(); ++i){
		unsigned char c = static_cast<unsigned char>(src[i]);
		if (is_separator(c)){
			if (!current.empty()){
				sentence_.push_back(current); // кладем слово в массив
				current.clear(); // очищаем для нового слова
			}
			continue;
		}
		if (c == '-' || c == '\'' || c == '`'){
			bool has_prev = !current.empty();
			bool has_next = (i + 1 < src.size()) && !is_separator(static_cast<unsigned char>(src[i + 1]));
			// если предыдущая часть слова есть и следующая часть не символ пунктуации 
			if (has_prev && has_next) {
				current += static_cast<char>(c);
				continue;
			} else {
				if (!current.empty()) {
					sentence_.push_back(current);
					current.clear();
				}
				continue;
			}
		}
		current += static_cast<char>(c);
	}
	if (!current.empty()){
		sentence_.push_back(current);
	}
}

void IdentVerbs::extract_sentence_num() {
    num_ = 0; // безопасное значение по умолчанию
    if (sentence_.empty()) return;

    const string& first = sentence_[0];
    if (first.empty()) return;

    uint64_t val = 0;
    for (char ch : first) {
        if (ch < '0' || ch > '9') return; // не число, значит оставляем 0, слово не трогаем
		val = val * 10 + static_cast<uint64_t>(ch - '0');
    }
    num_ = val;
    sentence_.erase(sentence_.begin());
}

// Метод приведения одного слова к нижнему регистру
void IdentVerbs::normalize_word(string& word) {
	// ASCII - сдвиг; кириллица в UTF-8 идёт двумя байтами - сдвиг второго байта
	size_t i = 0;
	while (i < word.size()) {
		unsigned char c = static_cast<unsigned char>(word[i]);
		if (c < 128) {
			if (c >= 'A' && c <= 'Z') word[i] = static_cast<char>(c + 0x20);
			++i;
		} else if (c == 0xD0 && i + 1 < word.size()) {
			// U+0410...U+042F -> D0 90...D0 AF (А...Я)
			// U+0430...U+044F -> D0 B0...D1 8F (а...я)
			// U+0401 (Ё) D0 81 -> U+0451 (ё) D1 91
			unsigned char b = static_cast<unsigned char>(word[i + 1]);
			if (b >= 0x90 && b <= 0x9F) {  // А..П
				word[i + 1] = static_cast<char>(b + 0x20);
			} else if (b >= 0xA0 && b <= 0xAF) {    // Р..Я
				word[i]     = static_cast<char>(0xD1);
				word[i + 1] = static_cast<char>(b - 0x20);
			} else if (b == 0x81) {   // Ё
				word[i]     = static_cast<char>(0xD1);
				word[i + 1] = static_cast<char>(0x91);
			}
			i += 2;
		} else {
			++i; // прочие многобайтовые символы - не трогаем
		}
	}
}

// Метод нормализации всех слов в структуре класса
void IdentVerbs::normalize() {
	for (string& w : sentence_) normalize_word(w);
}

// Метод проверки принадлежности символа к кириллице
bool IdentVerbs::is_cyrillic_word(const string& w) {
	if (w.empty()) return false;
	bool has_cyr = false;
	size_t i = 0;
	// проходим по байтам
	while (i < w.size()) {
		unsigned char c = static_cast<unsigned char>(w[i]);
		if (c < 128) {
			// ASCII-символ внутри слова, значит не чисто кириллическое
			if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) return false;
			++i;
			continue;
		}
		// кириллица: D0/D1 + второй байт
		if ((c == 0xD0 || c == 0xD1) && i + 1 < w.size()) {
			has_cyr = true;
			i += 2;
			continue;
		}
		// прочий символ пропускаем
		return false;
	}
	return has_cyr;
}
