"""Собирает stress.txt из глагольного TSV OpenRussian."""

import csv
import re
import sys
from pathlib import Path


VOWELS = "аеёиоуыэюя"
FORM = re.compile("[а-яё']+")
COLUMNS = ("accented", "imperative_sg", "imperative_pl",
           "past_m", "past_f", "past_n", "past_pl",
           "presfut_sg1", "presfut_sg2", "presfut_sg3",
           "presfut_pl1", "presfut_pl2", "presfut_pl3")


def parse_form(marked):
    if not FORM.fullmatch(marked):
        return None
    word = marked.replace("'", "")
    vowels = [letter for letter in word if letter in VOWELS]
    if marked.count("'") == 1:
        prefix = marked.split("'")[0]
        if not prefix or prefix[-1] not in VOWELS:
            return None
        return word, sum(letter in VOWELS for letter in prefix)
    if "'" not in marked:
        if vowels.count("ё") == 1:
            return word, vowels.index("ё") + 1
        if len(vowels) == 1:
            return word, 1
    return None


def build(source):
    forms = {}
    with source.open(encoding="utf-8-sig", newline="") as stream:
        for row in csv.DictReader(stream, delimiter="\t"):
            for column in COLUMNS:
                for marked in re.split("[,;/]", row[column]):
                    form = parse_form(marked.strip())
                    if form is None:
                        continue
                    word, stress = form
                    key = word.replace("ё", "е")
                    forms.setdefault(key, set()).add((word, stress))

    # Сначала собираем варианты по всему источнику, чтобы не потерять
    # второе ударение, даже если оно записано после первых 20 000 форм.
    unambiguous = [next(iter(variants)) for variants in forms.values()
                   if len(variants) == 1]
    return sorted(unambiguous[:20_000])


if __name__ == "__main__":
    if len(sys.argv) != 2:
        sys.exit("Использование: python3 lib/build_stress.py verbs.csv")
    forms = build(Path(sys.argv[1]))
    if len(forms) != 20_000:
        sys.exit("В источнике меньше 20 000 однозначных форм")
    output = Path(__file__).with_name("stress.txt")
    with output.open("w", encoding="utf-8", newline="\n") as stream:
        for word, stress in forms:
            stream.write(f"{word} {stress}\n")
