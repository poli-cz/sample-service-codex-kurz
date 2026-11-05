# C2 – Slabé a dobré zadání (LAB-201)

**Blok:** 2, Jak coding agent uvažuje a jak mu zadávat · **Čas:** 40 minut, ve dvojicích

## Cíl

Stejnou úlohu zadáte agentovi dvakrát. Nejdřív jednou větou, potom se strukturou cíl, výstup,
chování, rozsah, omezení, postup a kritérium hotovo. Výsledky porovnáte podle hodnoticí mřížky a zjistíte,
co všechno agent u slabého zadání rozhodl za vás.

## Úloha

Tiket LAB-201: laboratoř chce, aby služba vyhodnocovala Westgardova pravidla pro kontrolní vzorky.
Kontrolní vzorek má známý cílový průměr a směrodatnou odchylku (SD) šarže. Pravidla říkají, kdy je
série kontrolních měření v pořádku, kdy je to varování a kdy se běh zamítá.

## Výchozí stav

`sample-service`, větev `main`, postavený (`python scripts/check_all.py debug` zelený).
Codex v režimu, který smí měnit soubory v pracovním adresáři:

```sh
codex --sandbox workspace-write
```

## Kroky

Ve dvojici: jeden z vás začne slabým zadáním, druhý dobrým. Po 15 minutách si výsledky ukážete.

1. **Čistý start** (před každým pokusem):

   ```sh
   git switch main
   git restore .
   git clean -fd
   ```

   `build/` zůstane (je v `.gitignore`), další build bude rychlý.

2. **Slabé zadání.** Nová session Codexu, vložte přesně:

   > Přidej do labcore vyhodnocení Westgardových pravidel (1-2s, 1-3s, 2-2s, R-4s, 4-1s, 10-x) pro kontrolní vzorky.

   Až agent skončí: `git status`, `git diff --stat`, přečtěte jeho shrnutí. Vyplňte mřížku
   (`hodnotici-mrizka.md`, sloupec „slabé“).

3. **Dobré zadání.** Čistý start (krok 1), **nová** session, vložte celé zadání níže. Vyplňte sloupec „dobré“.

4. **Porovnání ve dvojici** (10 minut): projděte mřížku řádek po řádku. U slabého zadání vypište
   všechna rozhodnutí, která agent udělal za vás (rozhraní, hranice, co vrací, co přidal navíc).

5. **Kontrola tabulkou:** pro výsledek dobrého zadání ověřte ručně nebo krátkým testem aspoň řádky
   „120“, „130“ a „101 až 109, pak 100“ z tabulky na konci. Vyšlo to?

### Dobré zadání (vložte celé)

> **Cíl:** přidej do knihovny `labcore` vyhodnocení Westgardových pravidel pro kontrolní vzorky (tiket LAB-201).
>
> **Rozhraní:** nový hlavičkový soubor `src/labcore/include/labcore/qc_rules.hpp`, implementace v `src/labcore/src/qc_rules.cpp`:
>
> ```cpp
> namespace lab::qc {
> struct Violation {
>   std::string rule;   // "1-2s", "1-3s", "2-2s", "R-4s", "4-1s" or "10-x"
>   std::size_t index;  // index of the value that completed the rule
>   friend bool operator==(const Violation&, const Violation&) = default;
> };
> [[nodiscard]] std::vector<Violation> evaluate_westgard(std::span<const double> values,
>                                                        double target_mean, double target_sd);
> }  // namespace lab::qc
> ```
>
> **Chování** (M = `target_mean`, S = `target_sd`, hodnoty jsou v pořadí měření):
>
> - `1-2s`: hodnota za M ± 2S (varování),
> - `1-3s`: hodnota za M ± 3S,
> - `2-2s`: dvě po sobě jdoucí hodnoty za stejným limitem 2S (obě nad M + 2S, nebo obě pod M − 2S),
> - `R-4s`: dvě po sobě jdoucí hodnoty, jedna nad M + 2S a druhá pod M − 2S,
> - `4-1s`: čtyři po sobě jdoucí hodnoty za stejným limitem 1S,
> - `10-x`: deset po sobě jdoucích hodnot na stejné straně průměru.
> - „Za limitem“ znamená ostře větší než horní limit, nebo ostře menší než dolní limit. Hodnota přesně na limitu ho nepřekračuje; hodnota přesně na průměru není na žádné straně a sérii `10-x` přeruší.
> - Pravidlo se hlásí na každém indexu, kde ho splní okno hodnot končící tímto indexem. Jedna hodnota může spustit víc pravidel; `1-2s` se hlásí vždy, i vedle ostatních.
> - Pořadí výstupu: podle indexu vzestupně, v rámci jednoho indexu v pořadí 1-2s, 1-3s, 2-2s, R-4s, 4-1s, 10-x.
> - `target_sd <= 0` vyhodí `std::invalid_argument`. Prázdný vstup vrátí prázdný vektor.
>
> **Rozsah:** jen nové soubory `qc_rules.hpp`, `qc_rules.cpp`, `tests/labcore/qc_rules_test.cpp` a jejich zápis do `src/labcore/CMakeLists.txt` a `tests/CMakeLists.txt`. Službu v `apps/` zatím nenapojuj, nic jiného neměň.
>
> **Technická omezení:** C++20, jen standardní knihovna, žádné nové závislosti, žádné `NOLINT` ani `-Wno-*`, styl okolního kódu.
>
> **Postup:** nejdřív testy v GoogleTest: prázdný vstup, každé pravidlo zvlášť, hodnoty přesně na limitech 1, 2 a 3 SD, série delší než okno pravidla, obě strany průměru, `target_sd` nulové i záporné. Spusť je a ukaž, že padají. Pak implementace.
>
> **Hotovo, když:** `python scripts/check_all.py debug tidy` projde: testy zelené, tidy nic nehlásí. Na konci shrň, co jsi změnil, které příkazy jsi spustil s jakým výsledkem a co jsi neověřil.

### Kontrolní tabulka (průměr 100, SD 10)

| hodnoty | očekávaný výsledek |
|---|---|
| 120 | nic (přesně na limitu 2 SD) |
| 130 | jen `1-2s` na indexu 0 (přesně na limitu 3 SD) |
| 110, 110, 110, 110 | nic (přesně na limitu 1 SD) |
| 101 až 109, pak 100 | nic (hodnota na průměru sérii přeruší) |
| 121, 122 | `1-2s` na 0, `1-2s` na 1, `2-2s` na 1 |

## Hotovo, když

- Máte vyplněnou mřížku pro obě zadání a seznam rozhodnutí, která agent u slabého zadání udělal sám.
- Víte, jestli agent u slabého zadání spustil testy a tidy, a jestli to řekl ve shrnutí.

## Zkrácená varianta

Ve dvojici jen jedno slabé a jedno dobré zadání (každý jedno) a z mřížky jen řádky 1, 2, 5, 7 a 8.
