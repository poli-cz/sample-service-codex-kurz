# C6 – Volitelně: refaktoring legacy parseru

**Blok:** 4b (volitelně, pro ty, kdo C5 stihli) · **Čas:** 40 minut

## Cíl

Nejdřív záchranná síť, pak malé kroky. Charakterizačními testy zachytíte současné chování starého
parseru řádků z analyzátoru (včetně podivností). Pak odstraníte globální buffery a smažete výjimky
statické analýzy, které tím ztratí důvod. Testy musí projít před refaktoringem i po něm beze změny.

## Výchozí stav

Větev `main`. Parser je v `src/labio/legacy/legacy_result_parser.cpp` (funkce `lr_parse`, ve stylu C),
moderní fasáda kolem něj je `lab::io::parse_reading` v `src/labio/src/result_parser.cpp`. Dnes ho
pokrývají jen tři testy v `tests/labio/result_parser_test.cpp`.

Opravu z C5 máte v commitu ve větvi `fix/lab-214` (kdo dělal zkrácenou variantu bez kroku 5, commitne
ji teď: `git add -A`, `git commit -m "LAB-214: keep the preempted sample"`) a `review.md` s `mr.md` jste
přesunuli mimo repozitář. Zbytek zahoďte, jinak ho `git switch` přenese do nové větve:

```sh
git restore .
git clean -fd
git switch main
git switch -c refactor/legacy-parser
codex --sandbox workspace-write
```

## Kroky

1. **Vysvětlení, jen čtení (5 minut).** Zadání:

   > Nic neměň. Vysvětli, co dělá `lr_parse` a jaké formáty řádků přijímá (firmware v1 a v2, záznamy R, C, E, H). Vypiš všechno, co se chová nečekaně: tiché ořezání, ignorované hodnoty, slabá kontrola vstupu. Zapiš to do `FINDINGS.md`.

2. **Charakterizační testy (15 minut).** Zadání:

   > Napiš charakterizační testy v GoogleTest, které zachytí současné chování přes `lab::io::parse_reading`, včetně podivností z `FINDINGS.md`. Nic neopravuj, i když je chování špatně: test popisuje, co kód dělá dnes. Pokryj firmware v1 a v2 a záznamy R, C, E, H. Diff smí být jen v `tests/`. Spusť je; musí projít.

   Zkontrolujte: je diff opravdu jen v `tests/`? Má každá podivnost svůj test? Pak testy commitněte
   samostatně, ještě před refaktoringem:

   ```sh
   git add tests
   git commit -m "Characterization tests for the legacy result parser"
   ```

3. **Jeden krok refaktoringu (10 minut).** Zadání:

   > Odstraň statické globální buffery ve funkci `split` (parser má být reentrantní). Chování se nemění, testy neměň. Struktura `LEGACY_RESULT` a konstanty `LR_*` v hlavičce zůstávají beze změny. Ověř `debug` a `asan`.

4. **Odstranění výjimek (10 minut).** Zadání:

   > Zjisti, které výjimky v `src/labio/legacy/.clang-tidy` a v `COMPILE_OPTIONS` v `src/labio/CMakeLists.txt` už po refaktoringu nejsou potřeba, smaž je i s komentářem a ověř `python scripts/check_all.py tidy`.

   Commitněte refaktoring jako jeden commit (jen `src/`, testy už v commitu jsou):

   ```sh
   git add src
   git commit -m "Legacy parser: reentrant split without global buffers"
   ```

   `FINDINGS.md` se necommituje. Po cvičení ho přesuňte mimo repozitář, jinak ho `git add -A`
   v dalších cvičeních přidá do commitu.

## Hotovo, když

- Charakterizační testy projdou před refaktoringem i po něm, beze změny.
- `debug`, `asan` a `tidy` projdou a v `.clang-tidy` legacy parseru je méně výjimek.
- Fasáda `parse_reading` a struktura `LEGACY_RESULT` se nezměnily.
- Máte `FINDINGS.md`, commit s testy a commit s refaktoringem.

## Zkrácená varianta (20 minut)

Kroky 1 a 2. Refaktoring a výjimky si nechte na později; cenné jsou hlavně testy.
