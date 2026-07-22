#!/usr/bin/env python3
"""
prepare_bible.py
Convert a plain text KJV (one verse per line, format "Book Chapter:Verse text...")
into the simple SD card format used by CardBible.

Example input line:
Genesis 1:1 In the beginning God created the heaven and the earth.

Usage:
    python3 prepare_bible.py kjv.txt ./bible_output

Then copy the generated "bible" folder to the root of your microSD card.
"""

import sys
import os
import re
from collections import defaultdict

# Canonical order + chapter counts (KJV 66 books)
BOOKS = [
    ("Genesis", 50, "GEN"),
    ("Exodus", 40, "EXO"),
    ("Leviticus", 27, "LEV"),
    ("Numbers", 36, "NUM"),
    ("Deuteronomy", 34, "DEU"),
    ("Joshua", 24, "JOS"),
    ("Judges", 21, "JDG"),
    ("Ruth", 4, "RUT"),
    ("1 Samuel", 31, "1SA"),
    ("2 Samuel", 24, "2SA"),
    ("1 Kings", 22, "1KI"),
    ("2 Kings", 25, "2KI"),
    ("1 Chronicles", 29, "1CH"),
    ("2 Chronicles", 36, "2CH"),
    ("Ezra", 10, "EZR"),
    ("Nehemiah", 13, "NEH"),
    ("Esther", 10, "EST"),
    ("Job", 42, "JOB"),
    ("Psalms", 150, "PSA"),
    ("Proverbs", 31, "PRO"),
    ("Ecclesiastes", 12, "ECC"),
    ("Song of Solomon", 8, "SNG"),
    ("Isaiah", 66, "ISA"),
    ("Jeremiah", 52, "JER"),
    ("Lamentations", 5, "LAM"),
    ("Ezekiel", 48, "EZK"),
    ("Daniel", 12, "DAN"),
    ("Hosea", 14, "HOS"),
    ("Joel", 3, "JOL"),
    ("Amos", 9, "AMO"),
    ("Obadiah", 1, "OBA"),
    ("Jonah", 4, "JON"),
    ("Micah", 7, "MIC"),
    ("Nahum", 3, "NAM"),
    ("Habakkuk", 3, "HAB"),
    ("Zephaniah", 3, "ZEP"),
    ("Haggai", 2, "HAG"),
    ("Zechariah", 14, "ZEC"),
    ("Malachi", 4, "MAL"),
    ("Matthew", 28, "MAT"),
    ("Mark", 16, "MRK"),
    ("Luke", 24, "LUK"),
    ("John", 21, "JHN"),
    ("Acts", 28, "ACT"),
    ("Romans", 16, "ROM"),
    ("1 Corinthians", 16, "1CO"),
    ("2 Corinthians", 13, "2CO"),
    ("Galatians", 6, "GAL"),
    ("Ephesians", 6, "EPH"),
    ("Philippians", 4, "PHP"),
    ("Colossians", 4, "COL"),
    ("1 Thessalonians", 5, "1TH"),
    ("2 Thessalonians", 3, "2TH"),
    ("1 Timothy", 6, "1TI"),
    ("2 Timothy", 4, "2TI"),
    ("Titus", 3, "TIT"),
    ("Philemon", 1, "PHM"),
    ("Hebrews", 13, "HEB"),
    ("James", 5, "JAS"),
    ("1 Peter", 5, "1PE"),
    ("2 Peter", 3, "2PE"),
    ("1 John", 5, "1JN"),
    ("2 John", 1, "2JN"),
    ("3 John", 1, "3JN"),
    ("Jude", 1, "JUD"),
    ("Revelation", 22, "REV"),
]

# Map common abbreviations and variants to canonical name
ABBR_MAP = {}
for name, ch, abbr in BOOKS:
    ABBR_MAP[name.lower()] = name
    ABBR_MAP[abbr.lower()] = name
    ABBR_MAP[abbr.lower().replace("1", "i").replace("2", "ii").replace("3", "iii")] = name
    # add short forms
    if name.startswith("1 "):
        ABBR_MAP[name[2:].lower() + "1"] = name
    if name.startswith("2 "):
        ABBR_MAP[name[2:].lower() + "2"] = name
    if name.startswith("3 "):
        ABBR_MAP[name[2:].lower() + "3"] = name

# Common short names people type
EXTRA = {
    "ps": "Psalms", "psa": "Psalms", "psalm": "Psalms",
    "pro": "Proverbs", "prov": "Proverbs",
    "jn": "John", "jhn": "John",
    "rom": "Romans",
    "rev": "Revelation", "revelation": "Revelation",
    "gen": "Genesis",
    "exo": "Exodus", "ex": "Exodus",
    "lev": "Leviticus",
    "num": "Numbers",
    "deu": "Deuteronomy", "deut": "Deuteronomy",
}
ABBR_MAP.update(EXTRA)


def normalize_book(raw):
    raw = raw.strip().lower()
    if raw in ABBR_MAP:
        return ABBR_MAP[raw]
    # try without numbers prefix
    m = re.match(r'([123])\s*(.+)', raw)
    if m:
        key = m.group(2) + m.group(1)
        if key in ABBR_MAP:
            return ABBR_MAP[key]
    return None


def parse_line(line):
    """Parse a line like 'Genesis 1:1 In the beginning...' """
    line = line.strip()
    if not line:
        return None
    # Match Book Chapter:Verse rest
    m = re.match(r'^([1-3]?\s*[A-Za-z]+)\s+(\d+):(\d+)\s+(.*)$', line)
    if not m:
        return None
    book_raw, ch, vs, text = m.groups()
    book = normalize_book(book_raw)
    if not book:
        return None
    return book, int(ch), int(vs), text.strip()


def main():
    if len(sys.argv) < 3:
        print("Usage: python3 prepare_bible.py <input_kjv.txt> <output_dir>")
        print("Example: python3 prepare_bible.py kjv.txt ./bible_data")
        sys.exit(1)

    infile = sys.argv[1]
    outdir = sys.argv[2]
    bibledir = os.path.join(outdir, "bible")
    os.makedirs(bibledir, exist_ok=True)

    books_data = defaultdict(lambda: defaultdict(list))  # book -> chapter -> list of (verse, text)

    print(f"Reading {infile}...")
    with open(infile, "r", encoding="utf-8", errors="ignore") as f:
        for lineno, line in enumerate(f, 1):
            res = parse_line(line)
            if res:
                book, ch, vs, text = res
                books_data[book][ch].append((vs, text))
            if lineno % 5000 == 0:
                print(f"  processed {lineno} lines...")

    print(f"Found {len(books_data)} books. Writing files...")

    # Write books.txt index
    with open(os.path.join(bibledir, "books.txt"), "w", encoding="utf-8") as idx:
        for name, ch_count, abbr in BOOKS:
            if name in books_data:
                idx.write(f"{name}|{ch_count}|{abbr}\n")

    # Write per-book files
    for name, ch_count, abbr in BOOKS:
        if name not in books_data:
            continue
        filepath = os.path.join(bibledir, f"{name}.txt")
        with open(filepath, "w", encoding="utf-8") as bf:
            for ch in sorted(books_data[name].keys()):
                bf.write(f"## {ch}\n")
                for vs, text in sorted(books_data[name][ch]):
                    bf.write(f"{vs} {text}\n")
        print(f"  wrote {name}.txt ({len(books_data[name])} chapters)")

    print(f"\nDone! Copy the folder '{bibledir}' to the root of your SD card as /bible/")
    print("Example tree:")
    print("  /bible/")
    print("    books.txt")
    print("    Genesis.txt")
    print("    ...")
    print("    Revelation.txt")


if __name__ == "__main__":
    main()
