# Cardputer Pocket Bible - Final Distribution v1.0

**Pocket Bible v1.0 (M5Burner base)** — Clean, intentional, easy-to-use KJV reader for Cardputer Advanced.

Super simple 4-item main menu:
1. Read the Bible (with OT/NT)
2. Verse of the Day
3. Extras (Prayer, Hymns, Topical)
4. About (classic passage)

Most features self-contained. Optional SD only for full 66 books.

See build.sh and below for guide.

## What works

- **Read the Bible**: From Home → Old Testament or New Testament → live filterable book list (correct mapping, Genesis first in OT, Matthew in NT) → big Chapter selector with arrows + typing → reader (red letters for Matthew/Mark/Luke/John words of Jesus, full scroll, L/R chapter change with SD data).
- **Verse of the Day**: Smarter (day-seeded from popular list) on Home.
- **Extras** (now simple 3-item menu with good space):
  - Sinner's Prayer
  - Hymns & Worship (complete public-domain lyrics)
  - Topical Verses (Gideon-style categories — all references audited to load)
- **About** moved to main menu #4 for instant one-tap access (the full classic passage).
- Reader: verse numbers highlighted, red letters for Jesus' words in the Gospels, vertical scroll + scrollbar, L/R chapter change when using full SD data.
- No audio. No bookmarks. No search. No separate keyboard help screen (keeps real estate for content; footers + simple keys are sufficient).
- Consistent easy controls: ;/. or k/j up/down, Enter, B back, type to filter in book list, G for quick ref.

Huge_app build. Single final binary for M5Burner: PocketBible.bin

**SD card?** Core (VOTD, popular, hymns, topical, About, navigation) is self-contained. Copy sd-card-bible/bible/ to SD root as `/bible/` only for complete 66 books + L/R chapter navigation.

For flash: always DELETE prior launcher entries first, use huge_app, power cycle. See build.sh.

## Hardware
M5Stack Cardputer Advanced (or standard). microSD recommended for full 66-book KJV.

## Download & Flash with M5Burner (recommended)

**The only file you need to flash:** `PocketBible.bin` (the merged ~4 MB binary built with huge_app).

### Using M5Burner
1. Download the latest `PocketBible.bin`.
2. Open **M5Burner**.
3. In the device list on the left, select **Cardputer** (or Cardputer Advanced).
4. Switch to the **Burn** tab.
5. Choose **Local file**.
6. Browse and select the `PocketBible.bin` you downloaded.
7. Click **Burn**.
8. If M5Burner prompts for a partition/app size, select **huge_app** (or any large 3MB+ scheme such as "Doom").
9. Wait for the burn to complete.

**Critical after flashing:**
- On the Cardputer, open M5Launcher.
- Long-press any old "Pocket Bible", "CardBible", or similar entries and **DELETE** them completely.
- Reboot the launcher or power cycle the Cardputer.
- Launch the new Pocket Bible entry.

### SD Card (strongly recommended for full experience)
Copy the folder `sd-card-bible/bible/` from this project to the **root** of your microSD card so it appears as `/bible/`.

This gives you the complete 66-book text and L/R chapter navigation. The app works without an SD card (using built-in popular verses, hymns, etc.), but the full Bible experience requires it.

### Adding Pocket Bible to the M5Burner app list (for public distribution)
If you want the app to appear as a selectable item inside M5Burner for other users:

- Host `PocketBible.bin` publicly (GitHub Releases is the easiest and most reliable).
- Go to the M5Burner repository: https://github.com/m5stack/M5Burner
- Open an issue or follow the community contribution process for new apps.
- Provide the following information:
  - Direct link to the latest `PocketBible.bin`
  - App name: **Pocket Bible**
  - Version: **1.0**
  - Short description: Clean, focused KJV Bible reader for the M5Stack Cardputer. Features easy OT/NT navigation, resume last reading position, full hymns with complete lyrics, Gideon-style topical verses, and the classic "About the Bible" passage. Mostly self-contained; optional SD card for the complete 66-book text.
  - Target device: **Cardputer** (or Cardputer Advanced)
  - Partition requirement: **huge_app** recommended
  - SD card note: Copy the `bible/` folder to `/bible/` on the root of the SD card for full features.

The project's `FLASH-INSTRUCTIONS.txt` and this README are written to be easy to copy for submissions.

### Alternative flashers
- Web flasher (excellent for Cardputer): https://bmorcelli.github.io/Launcher/webflasher.html (select local bin)
- Or https://web.esphome.io/

The black screen / partition error is almost always fixed by (1) deleting old launcher entries and (2) using the merged binary with huge_app.

## Build yourself (after editing)

```bash
cd cardputer-bible
./build.sh
```

It forces `build.partitions=huge_app`.

Then take the `build/Pocket Bible-merged.bin` (or the .bin + bootloader etc.).

## Controls Summary
- Menus & lists: Up/Down (arrows or ; . j k), Enter to pick, B back.
- Reader: Up/Down scroll, L/R = prev/next chapter (full SD mode), B/Q = menu, M = bookmark, G = quick go-to ref, R = random.
- Book wizard: type letters to filter books live, digits + arrows for chapter/verse numbers.

## SD data preparation (if you want to refresh the text)
Use the included `prepare_bible.py` against a plain one-verse-per-line KJV text file, then copy the output `bible/` folder to SD root.

## Why this rewrite?
Previous versions accumulated duplicate function bodies (loadChapterFromSD, parseReference appeared 2-3 times), a dangling `SEARCH` state that would not compile, leftover speaker code, and bloated state handling. That is why reinstalls "did nothing", hymns never appeared, and menus stayed old (bad binaries or launcher still pointing at previous app slot).

This version is deliberately small, single-purpose functions, focused on the three things you said must work: Popular browse, Book-Chapter-Verse, and Hymns/Worship (visual).

Enjoy the Word on your pocket computer.
