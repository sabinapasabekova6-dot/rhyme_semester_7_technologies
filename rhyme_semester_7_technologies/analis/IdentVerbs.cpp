#include "IdentVerbs.h"
#include <iterator> // для size()

using std::string_view, std::move, std::string, std::size;

namespace {

	struct EndingRule { 
		string_view ending; 
		uint8_t score;
	};
	struct SuffixRule { 
		string_view suffix; 
		uint8_t score; 
	};
	
	// ----------------------ОКОНЧАНИЯ----------------------------
	// Окончания инфинитивов
	EndingRule kEndingsInfinitive[] = {
		{"ться", 100}, {"тись", 100}, {"ать", 100},
		{"ять", 99}, {"ить", 100}, {"еть", 98},
		{"оть", 78}, {"уть", 99}, {"ыть", 94},
		{"ть", 78},	{"ти", 13}, {"чь", 90},
	};

	// Окончания возвратных форм глаголов
	EndingRule kEndingsReflexive[] = {
		{"лись", 100}, {"лась", 100}, {"лось", 100},
		{"лся", 100}, {"ешься", 100}, {"ишься", 100},
		{"етесь", 100},	{"итесь", 100}, {"ется", 100},
		{"ится", 100}, {"емся", 100}, {"имся", 100},
		{"утся", 100}, {"ются", 100}, {"атся", 100},
		{"ятся", 100}, {"усь", 99}, {"юсь", 100},
		{"йся", 98}, {"ись", 100}, {"ся", 99}, 
		{"сь", 100},
	};

	// Окончание глаголов прошедшего времени
	EndingRule kEndingsPast[] = {
		{"или", 99}, {"ила", 97}, {"ило", 99}, 
		{"ил", 96}, {"али", 99}, {"ала", 97}, 
		{"ало", 100}, {"ал", 96}, {"яли", 100},
		{"яла", 100}, {"яло", 99}, {"ял", 99},
		{"ели", 53}, {"ела", 90}, {"ело", 96},
		{"ел", 85}, {"ули", 94}, {"ула", 86},
		{"уло", 99},{"ул", 87}, {"ли", 93}, 
		{"ла", 93}, {"ло", 98}, {"л", 90}
	};

	// Окончания глаголов настоящего и прошедшего времени
	EndingRule kEndingsPresent[] = {
		{"ешь", 100}, {"ишь", 100}, {"ете", 97},
		{"ите", 88}, {"ет", 95}, {"ит", 83},
		{"ем", 72}, {"им", 77}, {"ут", 94},
		{"ют", 100}, {"ат", 41}, {"ят", 97},
		{"ю", 46}, {"у", 37} 
	};

	// Окончания повелительного наклонения глаголов
	EndingRule kEndingsImperative[] = {
		{"йте", 100}, {"ите", 88}, {"ай", 98}, 
		{"ей", 68}, {"уй", 98}, {"ой", 24}, 
		{"й", 48}, {"и", 37}, 
	};

	// ------------------- суффиксы -----------------------------
	SuffixRule kSuffixes[] = {
		{"изирова", 75}, {"изова", 70},   
		{"ирова", 70}, {"ствова", 65}, 
		{"ествова", 65}, {"нича", 55},   			
		{"ича", 50}, {"ива", 50},
		{"ыва", 50}, {"ова", 45},   
		{"ева", 45}, {"ну", 35},   
		{"ва", 25},   
	};

	// Черный список частых существительных/прилагательных, которые маскируются под глаголы
	bool is_in_blacklist(string_view word) {
		string_view blacklist[] = {
			// местоимения
			"ней","нею","ним","ними","них", "нем", 
			"меня","тебя","себя","мною","тобою","собою",
			"его","ему","ей","ею","кем","чем","никем","ничем","коем",
			"тем","том","сём","всём","всем","этом",
			"моей","твоей","своей","нашей","вашей",
			"моем","твоем","своем","нашем","вашем",
			"моим","твоим","своим","нашим","вашим",
			"мою","твою","свою","нашу","вашу",
			"мной","тобой","собой",
			"этой","той","всей","самой",
			"этим","тем","всем","самым",
			"эту","ту","всю","саму",

			// наречия
			"тут","там","затем","потом","зачем","почем","нипочём",
			"отчего","почему","засим","чуть","едва",

			// предлоги
			"около","возле","подле","после","кроме","между","пред","перед",

			// частицы, союзы, вводные 
			"нет","да","ну","вот","ведь","мол","бишь", "или", "ли",

			// краткие прилагательные
			"мал", "стар", "молод", "холод", "голод", "светел", "тепел",
			"бел", "зол", "смел", "весел", "гол", "стол",

			// существительные
			"дело","тело","мыло","крыло","весло","полотно","зеркало",
			"стекло","масло","сало","шило","рыло","жало","помело",
			"правило","слово","покрывало","золото","серебро","молоко","долото",


			"людей","детей","матерей","дочерей","дверей","зверей",
			"царей","князей","бурей","пустыней","тучей","дачей",
			"встречей","ночей","печей","речей","мечей","ручей",

			// прочее
			"сон","свет","гранит","банкет","билет","скелет",
			"вид","род","лад","нос",
			"пять","шесть","девять","десять","двадцать","тридцать",
			"крем","причем",
			"зал","бал","вал","нал","мил","бил","лил",
			"синим","синем","синего","синие","синих","синими","синей","синюю",
			"тепло", "весело"
			// TODO: вопрос винительного падежа открыт
		};
		for (string_view w : blacklist)
			if (word == w) return true;
		return false;
	}

	// Основа (после отрезания окончания) оканчивается на именной суффикс
	// Обратная идея, поиск не глагольных частей
	bool has_nominal_suffix(string_view word) {
		string_view markers[] = {
			"тель", // учитель
			"ник", // работник
			"чик", // лётчик
			"щик", // наборщик
			"ость", // радость
			"есть", // свежесть
			"изм", // коммунизм
			"ист", // артист
			"ств", // царство
			"тельств", // правительство
			"атор", // оратор
			"итор", // кредитор
		};
		for (string_view m : markers) {
			if (word.size() > m.size() &&
				word.compare(word.size() - m.size(), m.size(), m) == 0)
				return true;
		}
		return false;
	}

	bool is_participle(string_view word) {
		// деепричастия (только однозначные длинные формы)
		string_view gerund_endings[] = {
			"вшись", "авшись", "ившись", "явшись", "евшись", "вши", "ясь"
		};
		for (string_view suf : gerund_endings) {
			if (word.size() > suf.size() &&
				word.compare(word.size() - suf.size(), suf.size(), suf) == 0)
				return true;
		}

		// причастия (все падежные формы прилагательного типа)
		string_view participle_endings[] = {
			// действительные наст. вр. (-ущ/-ющ/-ащ/-ящ)
			"ущий","ущая","ущее","ущие","ущего","ущей","ущему","ущим","ущем","ущих","ущими","ущую",
			"ющий","ющая","ющее","ющие","ющего","ющей","ющему","ющим","ющем","ющих","ющими","ющую",
			"ащий","ащая","ащее","ащие","ащего","ащей","ащему","ащим","ащем","ащих","ащими","ащую",
			"ящий","ящая","ящее","ящие","ящего","ящей","ящему","ящим","ящем","ящих","ящими","ящую",
			// действительные прош. вр. (-вш), включая возвратные
			"вший","вшая","вшее","вшие","вшего","вшей","вшему","вшим","вшем","вших","вшими","вшую",
			"вшийся","вшаяся","вшееся","вшиеся","вшегося","вшейся","вшемуся","вшимся",
			"вшемся","вшихся","вшимися","вшуюся",
			// страдательные наст. вр. (-ем/-им)
			"емый","емая","емое","емые","емого","емой","емому","емым","емом","емых","емыми","емую",
			"имый","имая","имое","имые","имого","имой","имому","имым","имом","имых","имыми","имую",
			// страдательные прош. вр. (-нн/-енн/-т)
			"нный","нная","нное","нные","нного","нной","нному","нным","нном","нных","нными","нную",
			"енный","енная","енное","енные","енного","енной","енному","енным","енном","енных","енными","енную",
			"тый","тая","тое","тые","того","той","тому","тым","том","тых","тыми","тую",
			// прилагательные на -нний (та же идея, тоже не глаголы)
			"нний","нняя","ннее","нние","ннего","нней","ннему","нним","ннем","нних","нними","ннюю",
		};
		for (string_view end : participle_endings) {
			if (word.size() > end.size() &&
				word.compare(word.size() - end.size(), end.size(), end) == 0)
				return true;
		}
		return false;
	}

	const EndingRule* try_ending_table(string_view word, const EndingRule* table,
										size_t n, const EndingRule* best) {
		size_t word_s = word.size(); // длина слова в байтах
		for (size_t i = 0; i < n; ++i) {
			const EndingRule& rule = table[i];
			size_t rule_end_s = rule.ending.size(); // длина окончания из словаря в байтах

			// если длина слова <= длине окончания из словаря
			if (word_s <= rule_end_s) 
				continue;
			// если окончания не совпадают
			if (word.compare(word_s - rule_end_s, rule_end_s, rule.ending) != 0)
				continue;
			// если лучший не определен или длина найденного окончания больше или
			// если длина совпадает, но score больше
			if (!best || rule_end_s > best->ending.size() ||
				(rule_end_s == best->ending.size() && rule.score > best->score)) {
				best = &rule;
			}
		}
		return best;
	}

	// Функция поиска суффикса (аналог try_ending_table)
	const SuffixRule* try_suffix_table(string_view word, const SuffixRule* table,
									size_t n, const SuffixRule* best) {
		size_t word_s = word.size();
		for (size_t i = 0; i < n; ++i) {
			const SuffixRule& rule = table[i];
			size_t rule_suf_s = rule.suffix.size();
			if (word_s <= rule_suf_s) continue;
			
			// если суффикс не совпадает с концом слова
			if (word.compare(word_s - rule_suf_s, rule_suf_s, rule.suffix) != 0) continue;
			
			// берем самый длинный суффикс (или с большим score, если длина равна)
			if (!best || rule_suf_s > best->suffix.size() ||
				(rule_suf_s == best->suffix.size() && rule.score > best->score)) {
				best = &rule;
			}
		}
		return best;
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
	VerbInfo info;
	info.verb = word;
	if (!is_cyrillic_word(word) || word.size() < 4) {
		info.score = 0;
		return info;
	}
	// Проверка по черному списку
	if (is_in_blacklist(word)) {
		info.score = 0;
		return info;
	}
	
	string_view sv = word; // Будем "обрезать" слово справа налево
	uint32_t total_score = 0;
	string found_ending = "";
	string found_suffix = "";

	if (is_participle(sv)) {
		info.score = 0;
		return info;
	}

	// Ищем самое длинное окончание таблицах
	const EndingRule* best_end = nullptr;
	best_end = try_ending_table(sv, kEndingsInfinitive, size(kEndingsInfinitive), best_end);
	best_end = try_ending_table(sv, kEndingsReflexive,  size(kEndingsReflexive),  best_end);
	best_end = try_ending_table(sv, kEndingsPast,       size(kEndingsPast),       best_end);
	best_end = try_ending_table(sv, kEndingsPresent,    size(kEndingsPresent),    best_end);
	best_end = try_ending_table(sv, kEndingsImperative, size(kEndingsImperative), best_end);
	if (!best_end) {
		info.score = 5; // не нашли окончания
		return info;
	}

	// Отрезаем найденное окончание
	sv.remove_suffix(best_end->ending.size());
	found_ending = string(best_end->ending);
	total_score += best_end->score;

	if (has_nominal_suffix(sv)) { info.score = 0; return info; }

	const SuffixRule* best_suf = try_suffix_table(sv, kSuffixes, size(kSuffixes), nullptr);

	if (best_suf) {
		sv.remove_suffix(best_suf->suffix.size());
		found_suffix = string(best_suf->suffix);
		total_score += best_suf->score;
	}
	
	info.stem = string(sv);
	info.ending = found_suffix + found_ending;
	info.score = static_cast<uint8_t>(std::min(total_score, 100u));

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
