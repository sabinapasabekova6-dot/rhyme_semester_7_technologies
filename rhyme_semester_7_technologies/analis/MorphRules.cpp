#include "MorphRules.h"
#include <cctype>
#include <fstream>
#include <map>
#include <string_view>
using namespace std;

// Функция убирает пробельные символы (пробелы, табы, \r, \n)
// в начале и в конце строки, не копируя данные.
// Параметры:
//		s - исходная строка.
// Возвращаемое значение:
//		 Та же строка без ведущих и хвостовых пробелов
string_view trim(string_view s) {
    // сдвигаем начало, пока первый символ - пробельный
    while (!s.empty() && isspace(static_cast<unsigned char>(s.front())))
        s.remove_prefix(1);
    // сдвигаем конец, пока последний символ - пробельный
    while (!s.empty() && isspace(static_cast<unsigned char>(s.back())))
        s.remove_suffix(1);
    return s;
}

// Функция готовит строку к разбору
// Параметры:
//		 line строка, полученная из getline (копируется)
// Возвращаемые значения: 
// 		 отредактированная строка
string strip_line(string line) {
    if (!line.empty() && line.back() == '\r')
        line.pop_back();
    return line;
}

// Функция преобразующая строку из цифр в число 0..100.
// Параметры:
//		s - строка с числом
// 		out - результат
// Возвращаемые значения:
//   	true - строка корректна, out заполнен
//   	false - строка не является числом
bool parse_score(string_view s, uint8_t& out) {
    if (s.empty()) return false; // пусто => не число
    unsigned int v = 0;
    for (char c : s) {
        if (c < '0' || c > '9') return false;
        v = v * 10 + static_cast<unsigned int>(c - '0'); // добавляем цифру
        if (v > 100) return false; // вышли за диапазон => в файле ошибка
    }
    out = static_cast<uint8_t>(v);
    return true;
}

// Функция читающая файл с секциями и весами, формат:
//     # комментарий
//     [имя секции]
//     морфема вес
//
// Параметры:
//   	path - путь к файлу
//   	out - карта "имя секции  список правил". Ключи создаются
//          при встрече [секция], правила дописываются по порядку
// Возвращаемые значения:
//   	true - файл открылся и хотя бы одно правило разобрано
//   	false - файл не открылся или правил не найдено
bool read_scored_sections(const string& path, map<string, vector<MorphemeRule>>& out) {
    ifstream f(path); // открываем файл на чтение
    if (!f) return false; // не открылся

    bool any = false; // был ли хотя бы один валидный правило
    string line; // буфер под строку
    vector<MorphemeRule>* target = nullptr; // указатель на текущую секцию

    while (getline(f, line)) { // построчно читаем весь файл
        string_view sv = trim(strip_line(line)); // чистим \r и пробелы
        if (sv.empty() || sv.front() == '#') continue; // пусто или комментарий

        if (sv.front() == '[') { // встретили начало новой секции
            string_view::size_type rb = sv.find(']');
            if (rb == string_view::npos) continue; // нет ']' - битая строка
            // имя секции = содержимое между '[' и ']'
            target = &out[string(sv.substr(1, rb - 1))];
            continue;
        }

        if (target == nullptr) continue; // правило до первой секции — игнор

        // строка правила имеет вид "морфема вес"
        string_view::size_type sp = sv.find_first_of(" \t");
        if (sp == string_view::npos) continue; // нет разделителя — пропуск

        uint8_t score = 0; // сюда кладём вес
        if (!parse_score(trim(sv.substr(sp + 1)), score)) continue; // вес не разобран

        MorphemeRule rule; // собираем правило
        rule.text  = string(sv.substr(0, sp)); // морфема - до разделителя
        rule.score = score; // вес - после
        target->push_back(move(rule)); // кладём в текущую секцию
        any = true; // отметили что что-то было прочитано
    }
    return any;
}

// Функция читающая файл с секциями без весов, формат:
//     [имя секции]
//     слово
//
// Параметры:
//   	path - путь к файлу
//   	out - карта "имя секции - список слов"
// Возвращаемые значения:
//   	true - файл открылся и хотя бы одно слово прочитано
//   	false - иначе.
bool read_word_sections(const string& path, map<string, vector<string>>& out) {
    ifstream f(path); // открываем на чтение
    if (!f) return false; // не открылся

    bool any = false; // признак того что что-то прочитали
    string line; // буфер под строку
    vector<string>* target = nullptr; // текущая секция

    while (getline(f, line)) { // построчно
        string_view sv = trim(strip_line(line)); // чистим строку
        if (sv.empty() || sv.front() == '#') continue; // пусто/комментарий

        if (sv.front() == '[') { // начало секции
            string_view::size_type rb = sv.find(']');
            if (rb == string_view::npos) continue; // битая строка
            target = &out[string(sv.substr(1, rb - 1))]; // выбираем секцию
            continue;
        }

        if (target == nullptr) continue; // слово вне секции пропускаем

        target->push_back(string(sv)); // кладём слово в текущую секцию
        any = true; // отметили прочитанное
    }
    return any;
}

// Функция читающая список слов (без секций, без весов), по одному в строке.
// Параметры:
//   	path - путь к файлу
//   	out - вектор, куда добавляются слова
// Возвращаемые значения:
//   	true - файл открылся и прочитано хотя бы одно слово
//   	false - иначе
bool read_word_list(const string& path, vector<string>& out) {
    ifstream f(path); // открываем на чтение
    if (!f) return false; // не открылся

    bool any = false;
    string line; // буфер строки

    while (getline(f, line)) { // построчно
        string_view sv = trim(strip_line(line)); // чистим строку
        if (sv.empty() || sv.front() == '#') continue; // пусто или комментарий
        out.push_back(string(sv)); // сохраняем слово
        any = true; // отметили прочитанное
    }
    return any;
}

// Функция читающая все пять .txt файлов и раскладывающая содержимое по полям структуры
//
// Возвращаемые значения:
// 		true - если был успешно прочитан хотя бы один файл

bool MorphRules::load_from_lib() {
    bool any = false; // флаг - прочитан хотя бы один файл

    // endings.txt
    map<string, vector<MorphemeRule>> sections; // карта секция => правила
    if (read_scored_sections("lib/endings.txt", sections)) {
        map<string, vector<MorphemeRule>>::iterator it; // явный итератор

        // для каждой секции - найти её в карте и переложить в нужное поле
        it = sections.find("infinitive");
        if (it != sections.end() && !it->second.empty())
            endingsInfinitive = move(it->second);

        it = sections.find("reflexive");
        if (it != sections.end() && !it->second.empty())
            endingsReflexive = move(it->second);

        it = sections.find("past");
        if (it != sections.end() && !it->second.empty())
            endingsPast = move(it->second);

        it = sections.find("present");
        if (it != sections.end() && !it->second.empty())
            endingsPresent = move(it->second);

        it = sections.find("imperative");
        if (it != sections.end() && !it->second.empty())
            endingsImperative = move(it->second);

        any = true; // хотя бы один файл был прочитан
    }
    sections.clear(); // освобождаем карту под следующий файл

    // suffixes.txt
    if (read_scored_sections("lib/suffixes.txt", sections)) {
        map<string, vector<MorphemeRule>>::iterator it = sections.find("verbal");
        if (it != sections.end() && !it->second.empty())
            suffixesVerbal = move(it->second);
        any = true;
    }
    sections.clear();

    // participles.txt
    map<string, vector<string>> wordSections; // карта "секция и слова"
    if (read_word_sections("lib/participles.txt", wordSections)) {
        map<string, vector<string>>::iterator it;

        it = wordSections.find("gerund");
        if (it != wordSections.end() && !it->second.empty())
            gerundSuffixes = move(it->second);

        it = wordSections.find("participle");
        if (it != wordSections.end() && !it->second.empty())
            participleSuffixes = move(it->second);

        any = true;
    }
    wordSections.clear();

    // простые списки
    if (read_word_list("lib/nominal_suffixes.txt", suffixesNominal)) 
		any = true;
    if (read_word_list("lib/blacklist.txt", blacklist))
		any = true;

    return any;
}

// Функция возвращает единственный экземпляр правил,
// заполненный при первом обращении. Последующие вызовы отдают
// уже загруженный словарь
//
// Возвращаемые значения:
//		 ссылку на статический словарь MorphRules
const MorphRules& MorphRules::instance() {
    static MorphRules rules; // сам словарь
    static bool loaded = false; // проверка, загружался ли словарь ранее

    if (!loaded) { // при первом вызове - грузим из файлов
        rules.load_from_lib();
        loaded = true;
    }
    return rules; // возвращаем ссылку
}