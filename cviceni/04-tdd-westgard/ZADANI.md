# C4 – LAB-201 od tiketu po merge request: TDD a implementace

**Blok:** 4a, Od požadavku po MR, část 1 · **Čas:** 80 minut (kroky 1 až 5 asi 60 minut, zbytek rezerva nebo rozšíření)

## Cíl

Projdete s Codexem postup na jedné funkci: orientace, plán, testy napřed, implementace malými
kroky, statická analýza a vlastní revize diffu. Každý krok je samostatné zadání a po každém kroku
se díváte, co agent udělal, než zadáte další. Revize v nové session a popis merge requestu jsou
tady rozšíření pro ty, kdo skončí dřív; povinně je uděláte 2. den v C5.

## Výchozí stav

Větev `cviceni/04-start` (v bundlu z 1. dne): `main` plus tiket `docs/tickets/LAB-201.md`.
Nejdřív zahoďte rozpracované změny z C3 (AGENTS.md máte v commitu ve své větvi). Jinak je
`git switch` přenese do nové větve a diff v kroku 5 nebude sedět.

```sh
git restore .
git clean -fd
git switch cviceni/04-start
git switch -c lab-201
python scripts/check_all.py debug        # 49 testů zelených
codex --sandbox workspace-write
```

Všechny kroky dělejte v **jedné** session (agent si pamatuje plán).

## Kroky

1. **Orientace a plán (10 minut).** Zadání:

   > Přečti `docs/tickets/LAB-201.md`. Zatím nic neměň. Najdi v kódu místa, kterých se změna týká (knihovna, CMake, testy, konvence), a navrhni plán malých kroků; u každého kroku uveď test, který ho dokáže. Na konci vypiš, co v tiketu není jednoznačné.

   Projděte plán. Nesouhlasíte s něčím? Řekněte to agentovi teď, ne až v diffu.

2. **Testy napřed (15 minut).** Zadání:

   > Napiš testy v GoogleTest do `tests/labcore/qc_rules_test.cpp` podle tiketu. Přidej jen tolik kódu, aby se testy přeložily (hlavička a prázdná implementace), a zapiš soubory do CMake. Testy musí pokrýt nejméně případy z tabulky níže (průměr 100, SD 10). Spusť testy cíleně (jen `labcore_tests`) a ukaž, že padají ze správného důvodu, ne kvůli chybě překladu.

   Do zadání vložte i tabulku očekávaných výsledků (dole). Pak **sami** zkontrolujte, že každý řádek
   tabulky má v testech svůj případ s přesným očekáváním.

3. **Implementace malými kroky (20 minut).** Zadání:

   > Implementuj `evaluate_westgard` po malých krocích (pravidlo po pravidle) a po každém kroku spusť cílené testy. Chyby překladače čti a oprav sám. Testy neměň, pokud nejsou v rozporu s tiketem; když jsou, zastav se a řekni mi to.

4. **Kvalita (10 minut).** Zadání:

   > Spusť `python scripts/check_all.py debug tidy` a oprav nálezy bez `NOLINT` a bez `-Wno-*`.

5. **Vlastní revize (5 minut).** `git diff --stat` a `git diff`: mění diff jen to, co tiket chce?
   Použijte kontrolní seznam revize z [taháku kurzu](../TAHAK.md) (oddíl 5: rozsah, API existuje, testy
   testují požadavek, nic oslabeného).

## Tabulka očekávaných výsledků (kritérium hotovo)

Průměr 100, SD 10. Každý řádek musí mít test a test musí projít.

| hodnoty | očekávaný výsledek |
|---|---|
| prázdný vstup | nic |
| 100, 125 | `1-2s` na indexu 1 |
| 131 | `1-2s` a `1-3s` na indexu 0 |
| 121, 122 | `1-2s` na 0, `1-2s` na 1, `2-2s` na 1 |
| 121, 78 | `1-2s` na 0, `1-2s` na 1, `R-4s` na 1 |
| 111, 112, 113, 114 | `4-1s` na indexu 3 |
| 111, 112, 113, 114, 115 | `4-1s` na 3 a na 4 |
| 101 až 110 | `10-x` na indexu 9 |
| 101 až 109, pak 100 | nic |
| 120 | nic |
| 130 | jen `1-2s` na indexu 0 |
| 110, 110, 110, 110 | nic |
| SD 0 nebo záporná | `std::invalid_argument` |

## Hotovo, když

- Každý řádek tabulky má test a všechny testy procházejí.
- `python scripts/check_all.py debug tidy` projde.
- Diff obsahuje jen soubory z tiketu (hlavička, implementace, test, dva řádky v CMake).

## Rozšíření pro ty, kdo skončí dřív

1. **NaN.** Tiket neříká, co s NaN. Zeptejte se agenta (a sebe): co vrátí vaše implementace pro
   `target_sd` = NaN? Projde podmínkou `target_sd <= 0`? A co hodnota NaN uprostřed série, například
   131, NaN, 132? Napište testy na chování, které byste chtěli, a rozhodnutí zapište k tiketu.
2. **Druhý pohled.** Commitněte a spusťte revizi v nové session, bez kontextu autora:

   ```sh
   git add -A
   git commit -m "LAB-201: Westgard rules for control samples"
   codex review --base cviceni/04-start
   ```

   (Nebo `/review` v TUI.) Jsou nálezy oprávněné?
3. **Koncept popisu MR:**

   > Napiš popis merge requestu: co a proč, jak otestováno (příkazy a výsledky), rizika, co není v rozsahu.

## Zkrácená varianta (40 minut)

Kroky 2, 3 a 4 v jednom zadání (testy napřed, implementace, debug a tidy), pak krok 5.
