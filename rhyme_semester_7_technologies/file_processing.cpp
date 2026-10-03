#include "Total_Header.h"  
using namespace std;

//запрашиваем имя входного файла с проверками:
//   1) имя не пустое;
//   2) файл существует и открывается для чтения;
//   3) файл не пустой.
string requestValidInputFile() {
    while (true) {
        cout << "Введите имя входного файла: ";
        string filename;
        getline(cin, filename);

        // Проверка 1 не пустое  ли название  файла 
        if (filename.empty()) {
            cout << "Ошибка: имя файла не может быть пустым. Попробуйте снова.\n\n";
            continue;
        }

        // Проверка 2: открывается ли файл для чтения
        ifstream file(filename.c_str(), ios::binary);
        if (!file.is_open()) {
            cout << "Ошибка: не удалось открыть файл \"" << filename
                << "\". Проверьте названиe и попробуйте снова.\n\n";
            continue;
        }

        // Проверка 3: файл не пустой.
        file.seekg(0, ios::end);
        streampos size = file.tellg();
        file.close();

        if (size <= 0) {
            cout << "Ошибка: файл \"" << filename << "\" пустой. Введите другой файл.\n\n";
            continue;
        }

        // Все проверки пройдены 
        cout << "Файл \"" << filename << "\" успешно открыт \n";
        return filename;
    }
}

// строит имя выходного файла на основе имени входного:
// "input.txt" -> "input_numbered_sentences.txt"
string buildNumberedSentencesFileName(const string& inputFileName) {
    int dotPosition = -1; // позиция последней точки в имени файла (с неё начинается расширение, например ".txt"). Если точки в имени нет, остаётся -1.
    for (int i = (int)inputFileName.size() - 1; i >= 0; i--) {
        if (inputFileName[i] == '.') {
            dotPosition = i;
            break;
        }
    }

    
    string nameWithoutExtension;// имя файла без расширения
    if (dotPosition == -1) {
        // точки не нашли - значит, у файла вообще нет расширения,
        // используем имя целиком
        nameWithoutExtension = inputFileName;
    }
    else {
        // точку нашли - отрезаем всё начиная с неё
        nameWithoutExtension = inputFileName.substr(0, dotPosition);
    }

    return nameWithoutExtension + "_numbered_sentences.txt";
}

// проверяет, является ли символ русской буквой.
bool isLetterChar(char ch) {
    unsigned char c = (unsigned char)ch; // переводим в unsigned, чтобы сравнения работали правильно
    if (c >= 0xC0 && c <= 0xFF) return true;  // русские буквы А-Я, а-я
    if (c == 0xA8 || c == 0xB8) return true;  // буквы Ё и ё
    return false;
}

// проверяет, является ли символ закрывающей скобкой или закрывающей
// кавычкой. Такие символы, если они идут СРАЗУ ПОСЛЕ точки/!/?,
// нужно присоединять к текущему предложению, а не считать началом нового
bool isClosingMark(char ch) {
    unsigned char c = (unsigned char)ch;
    if (c == ')') return true;   
    if (c == ']') return true;    
    if (c == '"') return true;    
    if (c == '\'') return true;  
    if (c == 0xBB) return true;   // закрывающая кавычка-"ёлочка" » 
    return false;
}

// убирает пробелы, табуляцию и переводы строк с КОНЦА строки text
// (например, "Привет!\n\n" превратится в "Привет!")
string trimTrailingWhitespace(string text) {
    while (text.size() > 0) {
        char lastChar = text[text.size() - 1];
        if (lastChar == ' ' || lastChar == '\n' || lastChar == '\r' || lastChar == '\t') {
            text.erase(text.size() - 1); // удаляем последний символ
        }
        else {
            break; // дошли до обычного символа - дальше чистить не нужно
        }
    }
    return text;
}

// если в currentSentence после очистки пробелов осталось что-то кроме
// знаков препинания - печатает это как очередное пронумерованное
// предложение в outputFile и увеличивает sentenceNumber.
// В любом случае очищает currentSentence, чтобы начать собирать следующее
// предложение заново.
void flushSentence(ofstream& outputFile, string& currentSentence, int& sentenceNumber) {
    string cleanSentence = trimTrailingWhitespace(currentSentence);

    // cleanSentence.size() > 1 отсекает "пустые" предложения из одного
    // случайного знака препинания (например, второй "." из "..")
    if (cleanSentence.size() > 1) {
        sentenceNumber = sentenceNumber + 1;
        outputFile << sentenceNumber << ") " << cleanSentence << "\n";
    }

    currentSentence = ""; // буфер готов собирать следующее предложение
}

// читает входной файл, разбивает его на предложения и записывает их
// пронумерованными в новый файл, каждое предложение - с новой строки.
//
// Предложение считается законченным в двух случаях:
//   1) встретили точку, "!" или "?" (и, если сразу за ними идёт закрывающая
//      скобка/кавычка - она тоже присоединяется к этому предложению);
//   2) встретили перевод строки, а символ ПЕРЕД ним был не буквой, а каким-то
//      другим знаком (например, двоеточием перед началом прямой речи).
void writeNumberedSentences(const string& inputFileName) {
    ifstream inputFile(inputFileName.c_str());
    if (!inputFile.is_open()) {
        cout << "Ошибка: не удалось открыть файл для чтения предложений.\n";
        return;
    }

    // text - это весь входной файл, прочитанный целиком в одну строку итератор который читает все байты 
    string text((istreambuf_iterator<char>(inputFile)), istreambuf_iterator<char>());
    inputFile.close();

    string outputFileName = buildNumberedSentencesFileName(inputFileName);
    ofstream outputFile(outputFileName.c_str());
    if (!outputFile.is_open()) {
        cout << "Ошибка: не удалось создать файл \"" << outputFileName << "\".\n";
        return;
    }

    outputFile << "Пронумерованные предложения:\n\n";

    // currentSentence - буфер, в который мы постепенно собираем буквы текущего предложения, пока не найдём его конец
    string currentSentence = "";

    int sentenceNumber = 0; //  счётчик уже найденных и напечатанных предложений

    // textLength - длина всего текста в байтах (чтобы не вызывать text.size() на каждом шаге)
    int textLength = (int)text.size();

    for (int i = 0; i < textLength; i++) {
        char currentChar = text[i];

        // пока currentSentence пустой (новое предложение ещё не началось) -
        // пропускаем пробелы, табуляцию и переводы строк между предложениями
        if (currentSentence.empty() &&
            (currentChar == ' ' || currentChar == '\n' || currentChar == '\r' || currentChar == '\t')) {
            continue;
        }

        // добавляем текущий символ в собираемое предложение
        currentSentence += currentChar;

        // isTerminalPunctuation - true, если текущий символ это точка, восклицательный или вопросительный знак
        bool isTerminalPunctuation = (currentChar == '.' || currentChar == '!' || currentChar == '?');

        if (isTerminalPunctuation) {
            // если сразу после этого знака препинания идут ещё точки -
            // значит, дальше продолжается троеточие (например: "...",
            // "?..", "!.."), и все эти точки нужно присоединить к этому же
            // предложению целиком, а не обрывать на первой точке
            while (i + 1 < textLength && text[i + 1] == '.') {
                i = i + 1;
                currentSentence += text[i];
            }

            // после точки/!/?/троеточия сразу может идти закрывающая скобка
            // или закрывающая кавычка - она относится к этому же предложению,
            // поэтому присоединяем все такие символы подряд
            while (i + 1 < textLength && isClosingMark(text[i + 1])) {
                i = i + 1;
                currentSentence += text[i];
            }

            flushSentence(outputFile, currentSentence, sentenceNumber);
            continue;
        }

        // если текущий символ - перевод строки, проверяем символ ПЕРЕД ним
        // (в тексте, который мы уже собрали в currentSentence)
        if (currentChar == '\n' && currentSentence.size() > 1) {
            // previousChar - это символ, который стоял прямо перед "\n"
            char previousChar = currentSentence[currentSentence.size() - 2];

            if (!isLetterChar(previousChar)) {
                // перед переводом строки стоит не буква (например, ":") -
                // значит, здесь тоже заканчивается "предложение" (строка)
                flushSentence(outputFile, currentSentence, sentenceNumber);
            }
        }
    }

    // если после последнего символа текста в буфере что-то осталось
    // (текст закончился не точкой/!/?) - тоже записываем это как предложение
    flushSentence(outputFile, currentSentence, sentenceNumber);

    outputFile.close();

    cout << "Файл \"" << outputFileName << "\" создан. Найдено предложений: " << sentenceNumber << "\n";
}