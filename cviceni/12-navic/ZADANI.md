# Úlohy navíc ze slidů

**Pro koho:** pro účastníky po kurzu nebo pro ty, kdo jsou se cvičením hotovi dřív · **Čas:** podle úlohy, viz tabulka

## K čemu to je

Na slidech zazněly úlohy, které nemají vlastní cvičení: kvíz, kontrolní otázka, zadání „zkuste si“, tipy
v poznámkách a bonusové ukázky. Tady jsou všechny sepsané tak, abyste je udělali jen s tímto
repozitářem, bez slidů a bez lektora. Každá úloha má kontext, výchozí větev, kroky, zadání pro agenta
doslova a kritérium hotovo. Čísla slidů odkazují na PDF ve složce `prezentace/`.

Úlohy označené **volitelné** potřebují prostředí mimo repozitář (GitLab, Codex cloud, GCC a gcov, gdb);
u každé je napsané, co přesně.

## Než začnete

```sh
git clone https://github.com/poli-cz/sample-service-codex-kurz sample-service
cd sample-service
python scripts/check_all.py debug
```

- Python spouštějte jako `python`, nebo `python3` podle vaší instalace. Příkazy pište po jednom na řádek.
- Každou úlohu dělejte ve vlastní větvi: `git switch <výchozí větev>`, pak `git switch -c navic/<název>`.
  Větve `cviceni/*` a `reference/agents-md` vám `git switch` založí z `origin` sám.
- Mezi pokusy vrátíte pracovní kopii příkazy `git restore .` a `git clean -fd`. Před přepnutím větve
  rozpracované změny commitněte, nebo je takto zahoďte: jinak je `git switch` přenese do nové větve.
- Úlohy 2. dne předpokládají, že máte za sebou cvičení C5. Několik z nich míří na stejný kód a jeho
  chybu by vám prozradily.
- Příkazy Codexu jsou ověřené ve verzi 0.163.0. U novější verze si přepínače zkontrolujte přes
  `codex <příkaz> --help`.

### Pomocný skript prompt_input.py

Několik úloh se dívá, co Codex pošle modelu v první zprávě: které AGENTS.md načetl, které skilly vidí
a kolik to stojí tokenů. Ukáže to příkaz `codex debug prompt-input` bez volání modelu. Jeho výstup
(JSON) přehledně vypíše tento skript. Uložte ho jako `prompt_input.py` **vedle** složky `sample-service`
(ne do repozitáře) a spouštějte z kořene repa jako `python ../prompt_input.py`.

```python
"""Co Codex pošle modelu v první zprávě (codex debug prompt-input), bez volání modelu.

    python ../prompt_input.py                      tokeny po blocích
    python ../prompt_input.py --agents             nadpisy načtených AGENTS.md
    python ../prompt_input.py --skilly             skilly, které Codex vidí
    python ../prompt_input.py --prazdna            s prázdnou konfigurací Codexu
    python ../prompt_input.py --slozka src/labio   jako by Codex běžel v jiné složce
"""
import argparse
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile

try:
    import tiktoken
    ENC = tiktoken.get_encoding("o200k_base")
except ImportError:
    ENC = None


def count(text):
    return len(ENC.encode(text)) if ENC else round(len(text) / 4)


def prompt_input(prompt, folder, empty):
    codex = shutil.which("codex")
    if codex is None:
        sys.exit("codex nebyl nalezen v PATH")
    env = dict(os.environ)
    with tempfile.TemporaryDirectory() as tmp:
        if empty:
            for name in ("HOME", "USERPROFILE", "CODEX_HOME"):
                env[name] = tmp
        run = subprocess.run([codex, "debug", "prompt-input", prompt], cwd=folder, env=env,
                             capture_output=True, check=False)
    if run.returncode != 0:
        sys.exit(run.stderr.decode("utf-8", "replace"))
    return json.loads(run.stdout.decode("utf-8"))


def texts(items):
    for item in items:
        for part in item.get("content") or []:
            yield part.get("text", "")


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--zadani", default="Přidej evaluate_westgard podle LAB-201.")
    parser.add_argument("--slozka", default=".")
    parser.add_argument("--prazdna", action="store_true")
    parser.add_argument("--agents", action="store_true")
    parser.add_argument("--skilly", action="store_true")
    args = parser.parse_args()
    items = prompt_input(args.zadani, args.slozka, args.prazdna)

    if args.agents:
        found = False
        for text in texts(items):
            if text.startswith("# AGENTS.md"):
                found = True
                print("\n".join(line for line in text.splitlines() if line.startswith("#")))
        if not found:
            print("žádný AGENTS.md se nenačetl")
        return
    if args.skilly:
        for text in texts(items):
            if text.lstrip().startswith("<skills_instructions>"):
                for line in text.splitlines():
                    match = re.match(r"- ([\w.:-]+): ", line)
                    if match:
                        print(match.group(1))
        return

    total = 0
    for text in texts(items):
        match = re.match(r"\s*<([a-z_ ]+)>|# (AGENTS\.md)", text)
        label = (match.group(1) or match.group(2)) if match else "vaše zadání"
        n = count(text)
        total += n
        print(f"{label:<28} {n:6d}")
    unit = "tokenů" if ENC else "tokenů (odhad: znaky / 4)"
    print(f"{'celkem':<28} {total:6d} {unit}")


if __name__ == "__main__":
    main()
```

Přesný počet tokenů dá balíček `tiktoken` (`python -m pip install tiktoken`, nejlépe ve virtuálním
prostředí; při prvním použití stáhne tabulku kódování). Bez něj skript ukáže odhad.

## Přehled

| slide | úloha | minut | výchozí větev | potřebujete navíc |
|---|---|---|---|---|
| 1. den, 42 | Kolik z první zprávy je vaše zadání | 15 | `reference/agents-md` | – |
| 1. den, 48 a 49 | Kvíz: projde to? | 15 | `main` | – |
| 1. den, 80 | Kontrolní otázka před úlohou | 10 | `reference/agents-md` | – |
| 1. den, 83 a 86 | Skill `new-gtest-suite` | 20 | `main` | – |
| 1. den, 93 | Orientační zadání nad `lr_parse` | 15 | `main` | – |
| 1. den, 100 | Agent navrhne scénáře, hodnoty potvrdíte vy | 15 | `cviceni/04-start` | – |
| 2. den, 13 | Výstup gcov jako zadání na testy | 30 | `fix/lab-214` z C5, jinak `main` | volitelné: GCC a gcov |
| 2. den, 14 | gdb jako příkaz, ne jako okno | 15 | `main` | volitelné: gdb |
| 2. den, 45 | Tři průzkumníci a sekce `## Sub-agents` | 25 | `reference/agents-md` | – |
| 2. den, 63 | První úloha přes GitLab MCP | 20 | váš projekt v GitLabu | volitelné: GitLab 19.5+ |
| 2. den, 85 | Kam s pravidlem: od prosby po zákaz | 10 | bez repozitáře | – |
| 2. den, 98 | Varianta D v Pythonu nebo C# | 45 | `main` | – |
| 2. den, 99 | Stejná úloha s levnějším modelem | 60 | `cviceni/10-start` | – |
| 2. den, 104 | Kdo vlastní co (RACI) pro váš tým | 15 | bez repozitáře | – |
| bonus, 110 | `codex exec` se schématem výstupu | 15 | `main` | – |
| bonus, 111 | Revize MR v GitLab CI | 30 | váš testovací projekt v GitLabu | volitelné: GitLab, runner, API klíč |
| bonus, 114 | Úloha v Codex cloud | 30 | `cviceni/10-start` ve forku na GitHubu | volitelné: Codex cloud |
| bonus, 116 a 120 | Plán v souboru: ExecPlan a Mikado | 30 + 30 | `cviceni/10c-start` a `main` | – |

---

## 1. den

### Z 1. dne, slide 58: Kolik z první zprávy je vaše zadání

**Čas:** 15 minut · **Výchozí větev:** `reference/agents-md`

Codex kolem vaší věty složí první zprávu pro model: pravidla sandboxu, seznam skillů, AGENTS.md
a popis prostředí. Na kurzu jsme ji změřili s prázdnou konfigurací a zadání v ní bylo 13 tokenů
z 3 119. Měření zopakujete u sebe bez volání modelu a uvidíte, kolik stojí vaše osobní skilly
a nastavení.

**Kroky**

1. `git switch reference/agents-md`
2. Měření s vaší konfigurací (osobní skilly, osobní AGENTS.md, `config.toml`):

   ```sh
   python ../prompt_input.py
   ```

3. Totéž s prázdnou konfigurací, jako na kurzu:

   ```sh
   python ../prompt_input.py --prazdna
   ```

4. Porovnejte oba výpisy mezi sebou a s naším měřením (Codex 0.163.0, prázdná konfigurace, větev
   `reference/agents-md`). Bloky se jmenují podle značek ve zprávě:

   | blok ve výpisu | co to je | naše měření |
   |---|---|---|
   | `AGENTS.md` | AGENTS.md z repa, včetně osobního, pokud ho máte | 858 |
   | `permissions instructions` | pravidla sandboxu a schvalování | 799 |
   | `collaboration_mode`, `multi_agent_role`, `multi_agent_mode` | role a režim agenta | 761 |
   | `skills_instructions` | seznam skillů (jméno, popis, cesta) | 498 |
   | `environment_context` | složka, shell, datum | 190 |
   | `vaše zadání` | vaše věta | 13 |

   Čísla se o pár tokenů liší podle délky cesty ke složce a podle verze Codexu. Bloky navíc (například
   doporučené pluginy) přidává vaše konfigurace.

5. Volitelně: totéž ve větvi `main` (bez AGENTS.md) a s `--slozka src/labio` (načtou se dva soubory
   AGENTS.md).

**Hotovo, když**

- Víte, kolik tokenů ve vaší první zprávě stojí seznam skillů a AGENTS.md.
- Víte, kolik z toho přidala vaše osobní konfigurace (rozdíl kroků 2 a 3), a platíte to v každé zprávě.

### Z 1. dne, slidy 37 a 38: Kvíz: projde to?

**Čas:** 15 minut · **Výchozí větev:** `main`

Nejdražší chyby agenta se přeloží bez jediného varování. Tahle funkce používá vzor, který je
v tréninkových datech častý. Nejdřív odpovězte sami, pak se zeptejte agenta a nakonec to ověřte
v repozitáři.

```cpp
// labcore: průměr kontrolních hodnot
double mean_of(const std::vector<double>& v) {
  return std::accumulate(v.begin(), v.end(), 0)
         / static_cast<double>(v.size());
}

// vstup: 99.6, 100.4, 100.9, 98.7
// správný průměr: 99.90
```

**Otázka:** Projekt překládá s `-Wall -Wextra -Wconversion -Wsign-conversion -Werror`. Co se stane?

- A: Nepřeloží se, `-Wconversion` to chytí.
- B: Přeloží se a vrátí 99,90.
- C: Přeloží se a vrátí 99,25.
- D: Přeloží se, ale chytí to clang-tidy.

Správných odpovědí může být víc.

**Kroky**

1. Zapište si svou odpověď.
2. V kořeni repa spusťte `codex --sandbox read-only` a zeptejte se agenta:

   ```text
   Projekt překládá s -Wall -Wextra -Wconversion -Wsign-conversion -Werror
   a v definici hotovo má clang-tidy s konfigurací .clang-tidy z kořene repa.
   Co udělá tahle funkce se vstupem 99.6, 100.4, 100.9, 98.7? Přeloží se? Jakou hodnotu
   vrátí? Ohlásí něco clang-tidy? Nic nespouštěj a nic neměň, odpověz a zdůvodni to.

   double mean_of(const std::vector<double>& v) {
     return std::accumulate(v.begin(), v.end(), 0) / static_cast<double>(v.size());
   }
   ```

3. Ověřte to v repozitáři. Funkce `lab::stats::mean` v `labcore` počítá průměr stejně, jen s počáteční
   hodnotou `0.0`. Na chvíli ji změníte na tvar z kvízu:

   ```sh
   git switch main
   git switch -c navic/kviz
   ```

   V `src/labcore/src/statistics.cpp` ve funkci `mean` přepište v `std::accumulate(…, 0.0)` počáteční
   hodnotu `0.0` na `0`. Do `tests/labcore/statistics_test.cpp` přidejte za otevření
   `namespace {` test s hodnotou, kterou určil člověk:

   ```cpp
   TEST(Statistics, MeanOfControlValues) {
     const std::vector<double> v{99.6, 100.4, 100.9, 98.7};
     EXPECT_NEAR(mean(v), 99.90, 1e-9);
   }
   ```

   Pak:

   ```sh
   python scripts/check_all.py debug
   python scripts/check_all.py tidy
   ```

   Přeložilo se to? Co ukázal nový test a co ostatní testy statistiky? Co hlásí `tidy`? Preset `debug`
   používá přepínače z otázky (`cmake/CompilerWarnings.cmake`); jiný překladač s jinými přepínači může
   hlásit něco jiného, pak si zapište, co hlásí ten váš.

4. Změny zahoďte: `git restore .`, pak `git switch main`.
5. Porovnejte svou odpověď, odpověď agenta a výsledek. Rozbor je až na slidu 38 1. dne.

**Hotovo, když**

- Víte, které z odpovědí A až D platí, a výsledek obou presetů umíte vysvětlit.
- Víte, jestli se agent trefil, a co by takovou chybu v týmu chytilo dřív než revize.

### Z 1. dne, slide 99: Kontrolní otázka před úlohou

**Čas:** 10 minut · **Výchozí větev:** `reference/agents-md` (nebo vaše větev `c3-agents-md` z C3)

AGENTS.md pomůže jen tehdy, když ho agent opravdu načetl. Session spuštěná mimo repozitář nebo
v jiné složce ho nevidí. Proto v nové session před první úlohou položte kontrolní otázku. Pravidlo:
když agent vaše sekce nevyjmenuje, úlohu nepouštějte.

**Kroky**

1. `git switch reference/agents-md`
2. Zjistěte bez modelu, které nadpisy AGENTS.md Codex do první zprávy vloží:

   ```sh
   python ../prompt_input.py --agents
   ```

   Máte-li osobní AGENTS.md (ve složce `.codex` v domovské složce), uvidíte i jeho nadpisy.

3. Nová session `codex --sandbox read-only` a kontrolní otázka:

   ```text
   Jaké instrukce máš načtené? Vypiš nadpisy sekcí AGENTS.md a příkaz, kterým ověříš svou
   změnu. Nic neměň.
   ```

4. Porovnejte odpověď s výpisem z kroku 2 a s příkazy v AGENTS.md:

   ```sh
   git grep -n check_all -- AGENTS.md
   ```

5. Totéž ve větvi `main` (`git switch main`, `python ../prompt_input.py --agents`, nová session,
   stejná otázka). Pustili byste podle pravidla výše úlohu?
6. Zpět ve větvi `reference/agents-md`: `python ../prompt_input.py --agents --slozka src/labio`, pak
   `cd src/labio`, `codex --sandbox read-only` a stejná otázka. Na konci `cd ../..`.

**Hotovo, když**

- Ve větvi `reference/agents-md` odpověď agenta sedí s výpisem z kroku 2.
- Víte, co agent odpoví ve větvi `main`, a víte, co se změní, když Codex běží v `src/labio`.

### Z 1. dne, slide 102: Skill new-gtest-suite

**Čas:** 20 minut · **Výchozí větev:** `main`

Skill je zabalený opakovaný postup, který si agent načte, až když ho potřebuje. Tenhle řeší skutečnou
past projektu: testovací soubor, který není zapsaný v `tests/CMakeLists.txt`, se nepřeloží ani
nespustí a `ctest` je přesto zelený. Hodí se i jako hotový vzor k cvičení C3b, pokud jste vlastní skill
`add-service-command` nestihli.

**Kroky**

1. `git switch main`, pak `git switch -c navic/skill-gtest`
2. Soubor `.agents/skills/new-gtest-suite/SKILL.md` založte sami v editoru (pro agenta je `.agents/`
   v sandboxu jen pro čtení):

   ```markdown
   ---
   name: new-gtest-suite
   description: Use when adding a new GoogleTest file to sample-service, for example tests for a source file that has no test file yet.
   ---

   # New GoogleTest file

   1. Put the file in `tests/<library>/<topic>_test.cpp`, where `<library>` is `labcore`,
      `labio` or `labsched`. Use a test suite name that no other test uses yet.
   2. Add the file to the `sample_add_test_suite(<library>_tests ...)` call in
      `tests/CMakeLists.txt`. A file that is not listed there is never built or run,
      and ctest still reports success.
   3. Build only that test executable and run the new suite:
      `cmake --build --preset debug --target <library>_tests`
      `ctest --preset debug -R '^<Suite>\.'`
   4. Check that ctest lists the new tests: `ctest --preset debug -N -R '^<Suite>\.'`,
      and that the total in `ctest --preset debug -N` grew by the number of new tests.
   5. Before you report done: `python scripts/check_all.py debug tidy`.
   ```

3. Commit:

   ```sh
   git add .agents
   git commit -m "Add skill new-gtest-suite"
   ```

4. Ověřte bez modelu, že ho Codex vidí: `python ../prompt_input.py --skilly` vypíše `new-gtest-suite`
   vedle vestavěných skillů. V nové session totéž ukáže `/skills`.
5. Použijte ho. Nová session `codex --sandbox workspace-write`:

   ```text
   $new-gtest-suite Přidej nový testovací soubor pro flag_codes, has_flag a add_flag
   ze src/labcore/include/labcore/measurement.hpp: každý ze šesti příznaků zvlášť,
   pořadí kódů při více příznacích a prázdnou sadu příznaků. Kód v src/ neměň.
   ```

6. Zkontrolujte sami: `git status` a `git diff tests/CMakeLists.txt`, pak
   `ctest --preset debug -N -R '^<Suite>\.'` se jménem sady, které agent zvolil. Prošel agent krok 4
   skillu sám?
7. Volitelně si past vyzkoušejte: odložte jen zápis v CMake a pusťte všechny testy.

   ```sh
   git stash push -- tests/CMakeLists.txt
   cmake --build --preset debug
   ctest --preset debug
   ctest --preset debug -N -R '^<Suite>\.'
   git stash pop
   ```

   Celý běh `ctest` je zelený, a přitom nové testy v něm nejsou.

**Hotovo, když**

- Codex skill vidí (`/skills` nebo výpis z kroku 4).
- Nové testy jsou zapsané v `tests/CMakeLists.txt` a `ctest -N` je vypíše.
- `python scripts/check_all.py debug tidy` projde a diff agenta je jen v `tests/`.

### Z 1. dne, slide 108: Orientační zadání nad lr_parse

**Čas:** 15 minut · **Výchozí větev:** `main`

Rozšíření cvičení C1 na jednu konkrétní funkci. Agent jen čte a každé tvrzení dokládá souborem
a řádkem; vy odpověď kontrolujete namátkou. Když namátková kontrola nesedí, nevěřte ani zbytku.
Na stejném parseru pak stojí cvičení C6.

**Kroky**

1. `git switch main`, pak `codex --sandbox read-only`
2. Zadání:

   ```text
   Vysvětli, co dělá lr_parse v src/labio/legacy/legacy_result_parser.cpp
   a jaké formáty řádků přijímá.

   Nic neměň. U každého tvrzení uveď soubor a řádek.

   Vyjmenuj chování, které vypadá jako chyba, ale může být záměr.
   Nic neopravuj.
   ```

3. Vyberte dvě tvrzení a otevřete soubor v editoru na uvedených řádcích. Když tvrzení na řádku není,
   zeptejte se:

   ```text
   Ukaž mi řádek, na kterém to je.
   ```

4. U každého podezřelého chování si zapište: chyba, nebo záměr? Kde to z kódu poznat nejde, zapište,
   koho byste se zeptali.

**Hotovo, když**

- Každé tvrzení v odpovědi má soubor a řádek a dvě z nich jste ověřili v kódu.
- Máte seznam podivností s vaším rozhodnutím u každé.
- `git status` je čistý.

### Z 1. dne, slide 116: Agent navrhne scénáře, hodnoty potvrdíte vy

**Čas:** 15 minut · **Výchozí větev:** `cviceni/04-start`

Před testy napřed (cvičení C4, krok 2) se hodí seznam scénářů. Ten agent vymyslí dobře, ale očekávané
hodnoty do testů píše člověk. U hraničních případů má agent říct, jestli je tiket rozhoduje.
Nejlépe před C4, nebo jako jeho rozšíření.

**Kroky**

1. `git switch cviceni/04-start`, pak `codex --sandbox read-only`
2. Zadání:

   ```text
   Navrhni testovací scénáře pro evaluate_westgard podle tiketu LAB-201.
   U každého: co testuje a proč je kritický.
   Hodnoty nevymýšlej, popiš jen vstup.
   Zvlášť vypiš hraniční případy a u každého, jestli ho tiket rozhoduje.
   Nic neimplementuj.
   ```

3. U každého hraničního případu, o kterém agent tvrdí, že ho tiket rozhoduje, najděte příslušnou větu
   v `docs/tickets/LAB-201.md`. U případů „tiket neříká“ ověřte, že opravdu neříká.
4. Zkontrolujte, že agent nevymyslel očekávané hodnoty.
5. Porovnejte scénáře s tabulkou očekávaných výsledků v zadání C4 (`cviceni/04-tdd-westgard/ZADANI.md`).
   Které scénáře agent navrhl navíc a které v jeho seznamu chybí? U scénářů navíc rozhodněte, jestli
   patří do testů, a očekávaný výsledek napište sami podle tiketu.

**Hotovo, když**

- Máte seznam scénářů a u každého hraničního případu větu z tiketu, nebo ověřenou poznámku „tiket neříká“.
- Víte, které scénáře v tabulce z C4 chybí (nebo jste ověřili, že žádný), a jejich očekávané hodnoty
  jste napsali vy.

---

## 2. den

### Z 2. dne, slide 155: Výstup gcov jako zadání na testy

**Čas:** 30 minut · **Výchozí větev:** vaše větev `fix/lab-214` z cvičení C5, jinak `main` ·
**Volitelné, potřebujete:** GCC a gcov (preset `coverage` je jen pro GCC)

Pokrytí neříká, že kód je správně. Ukáže, kam žádný test nesáhl, a tam nic nenajde ani AddressSanitizer.
Řádky, které nikdy neběžely, jsou dobré zadání pro agenta. Past: agent rád napíše test, který funkci
jen zavolá a nic neověří. Pokrytí stoupne, jistota ne.

**Kroky**

1. `git switch fix/lab-214` (nebo `main`), pak `git switch -c navic/gcov-testy`
2. Pokud složka `build/coverage` existuje z dřívějška, smažte ji (počty běhů se jinak sčítají). Pak:

   ```sh
   python scripts/check_all.py coverage
   python scripts/coverage_report.py
   ```

   Zapište si procento u `src/labsched/src/scheduler.cpp`.

3. Výstup gcov pro plánovač uložte do `build/coverage` (Git tu složku ignoruje):

   ```sh
   cd build/coverage
   gcov src/labsched/CMakeFiles/labsched.dir/src/scheduler.cpp.gcda
   cd ../..
   ```

   gcov zapíše soubor `.gcov` pro každý zdrojový soubor, který plánovač používá, včetně hlaviček
   standardní knihovny. Náš je `build/coverage/scheduler.cpp.gcov`: řádky, které nikdy neběžely, mají místo počtu
   běhů `#####` (jen výjimková cesta `=====`).

4. `codex --sandbox workspace-write` a zadání:

   ```text
   V souboru build/coverage/scheduler.cpp.gcov je výstup gcov pro src/labsched/src/scheduler.cpp.
   Řádky označené ##### nespustil žádný test. Napiš do tests/labsched/scheduler_test.cpp testy
   v GoogleTest, které je spustí. U každého testu napiš, jaké chování ověřuje a odkud víš, že
   je správné (README, komentář v kódu, existující test). Každý test musí ověřit výsledek nebo
   stav fronty a stojanu, nestačí funkci zavolat. Kód v src/ neměň. Když narazíš na chování,
   které vypadá jako chyba, test na něj nepiš, zastav se a popiš mi ho.
   ```

5. Přečtěte nové testy: má každý `EXPECT_` nebo `ASSERT_` na něco, co se dá pokazit?
6. Změřte znovu (krok 2, včetně smazání `build/coverage`) a spusťte
   `python scripts/check_all.py debug asan tidy`. Ve větvi `main` bez opravy z C5 může preset `asan`
   nahlásit chybu, kterou řeší tiket LAB-214. Tady ji neopravujte.

**Hotovo, když**

- Pokrytí `scheduler.cpp` vzrostlo a umíte říct, které řádky zůstaly nepokryté a proč.
- Každý nový test něco ověřuje a diff je jen v `tests/`.
- `debug` a `tidy` projdou (a `asan` ve větvi s opravou z C5).

### Z 2. dne, slide 152: gdb jako příkaz, ne jako okno

**Čas:** 15 minut · **Výchozí větev:** `main` · **Volitelné, potřebujete:** gdb a build presetu `asan`
přeložený GCC nebo Clangem

Agent neumí klikat v debuggeru, ale umí spustit gdb v dávkovém režimu jako každý jiný příkaz a přečíst
backtrace. Jestli to v sandboxu Codexu půjde, záleží na tom, jestli sandbox dovolí ladění procesu.
Když ne, gdb spustíte vy a výstup agentovi vložíte.

**Kroky**

1. `git switch main`, pak `python scripts/check_all.py asan`
2. Sami (jeden příkaz):

   ```sh
   gdb -batch -ex 'break lab::sched::Rack::unload' -ex run -ex 'bt 3' --args build/asan/apps/sample-service/sample-service --batch examples/stat-burst.batch
   ```

   Měli byste vidět zastavení v `Rack::unload` a tři rámce zásobníku se soubory a řádky. Varování, že
   gdb nemohl vypnout náhodné rozložení adres, můžete ignorovat.

3. `codex --sandbox read-only` a zadání:

   ```text
   Zjisti, kdo volá lab::sched::Rack::unload, když služba zpracuje examples/stat-burst.batch.
   Použij gdb v dávkovém režimu (gdb -batch) nad build/asan/apps/sample-service/sample-service
   s breakpointem na lab::sched::Rack::unload a výpisem tří rámců zásobníku. Příkaz mi nejdřív
   ukaž. Z backtrace vysvětli, ve které funkci a na kterém řádku se vzorek ze stojanu vyjímá
   a proč. Nic neměň.
   ```

   Když gdb v sandboxu nepoběží, vložte agentovi svůj výstup z kroku 2:

   ```text
   Tady je výstup gdb: <výstup z kroku 2>. Vysvětli z něj, ve které funkci a na kterém řádku
   se vzorek ze stojanu vyjímá a proč. Nic neměň.
   ```

4. Porovnejte vysvětlení s rámci backtrace a s kódem na uvedených řádcích.

**Hotovo, když**

- Máte backtrace se třemi rámci.
- Vysvětlení agenta se opírá jen o řádky, které v backtrace a v kódu opravdu jsou.
- Víte, jestli gdb v sandboxu Codexu na vašem počítači běží.

### Z 2. dne, slide 182: Tři průzkumníci a sekce Sub-agents

**Čas:** 25 minut · **Výchozí větev:** `reference/agents-md`

Sub-agenti zrychlí průzkum: jedna otázka, tři knihovny, souběžně. Hlavní agent ví jen to, co mu pomocník
napíše, proto dobré zadání pomocníka určuje rozsah, otázku a tvar výsledku (soubor a řádek). Codex
pomocníky sám od sebe nespouští: musíte o ně požádat vy, nebo pravidlo v AGENTS.md. Pravidla pro
pomocníky proto patří do AGENTS.md.

**Kroky**

1. `git switch reference/agents-md`, pak `git switch -c navic/sub-agents`
2. Na konec `AGENTS.md` připište sekci:

   ```markdown
   ## Sub-agents

   - Exploring several libraries: one explorer per library.
   - Writing: at most one worker per directory, never two in one file.
   - Legacy struct, presets, .clang-tidy: no sub-agent touches them.
   - Sub-agents return paths and lines, not just conclusions.
   - More than 3 sub-agents only on explicit request.
   ```

3. Commit a kontrola bez modelu (ve výpisu je `## Sub-agents`):

   ```sh
   git commit -am "Add Sub-agents section to AGENTS.md"
   python ../prompt_input.py --agents
   ```

4. Volitelně levnější pomocníci. Bez nastavení pomocník dědí model rodiče; pomocník s celou historií
   vlákna ho dědí vždy. Do `config.toml` Codexu (složka `.codex` v domovské složce):

   ```toml
   [agents]
   default_subagent_model = "gpt-6-luna"
   default_subagent_reasoning_effort = "low"
   ```

5. Nová session `codex --sandbox read-only`. Zapište si spotřebu z `/status`, pak zadání:

   ```text
   Spusť souběžně tři agenty explorer, jednoho na knihovnu: src/labcore, src/labio,
   src/labsched.

   Otázka: kde může reference, ukazatel, iterátor nebo string_view přežít objekt,
   na který ukazuje?

   Každý vrátí: soubor:řádek, objekt a co ho zneplatní. Nic neměň.

   Pak odpovědi slouč do jedné tabulky a označ, co má pokrýt test.
   ```

6. Po skončení znovu `/status`. Vyberte z tabulky dva řádky a v kódu ověřte, že objekt a místo, kde se
   zneplatní, opravdu sedí.
7. Volitelně stejná otázka v nové session bez pomocníků (zadání začněte „Odpověz sám, bez pomocníků.“)
   a porovnejte čas, spotřebu a nálezy.

**Hotovo, když**

- Sekce `## Sub-agents` je v commitu a Codex ji načte (krok 3).
- Máte tabulku nálezů ze všech tří knihoven se souborem a řádkem a dva řádky jste ověřili v kódu.
- Víte, kolik běh se třemi pomocníky stál.

### Z 2. dne, slide 211: První úloha přes GitLab MCP

**Čas:** 20 minut · **Výchozí větev:** váš projekt v GitLabu · **Volitelné, potřebujete:**

- GitLab s MCP serverem: obecně dostupný od verze 19.5 (beta od 18.6, experiment od 18.3); verzi
  ukáže stránka `/help` vaší instance.
- Přihlášení přes OAuth se scope `mcp`, nebo osobní token s co nejmenšími právy (jen čtení, jeden
  projekt). Starší typy tokenů dostanou odpověď 403.
- Adresu GitLabu dosažitelnou z počítače, na kterém běží Codex.
- Souhlas firmy s napojením Codexu na firemní GitLab, skutečný tiket a merge request s pipeline.

Napojení je hotové, až agent přečte váš skutečný tiket. Do té doby je to jen konfigurace. První úloha
je jen čtení a agent dostane jen čtyři nástroje pro čtení z 56, které GitLab nabízí.

**Kroky**

1. Do `config.toml` Codexu (složka `.codex` v domovské složce) připište blok, adresu doplňte:

   ```toml
   [mcp_servers.gitlab]
   url = "https://<váš-gitlab>/api/v4/mcp"
   enabled_tools = ["get_work_item", "get_merge_request", "get_pipeline", "get_job"]
   ```

2. Přihlášení: `codex mcp login gitlab` (OAuth v prohlížeči; s `--no-browser` vypíše adresu). Nebo
   s osobním tokenem: do bloku připište `bearer_token_env_var = "GITLAB_TOKEN"` a token uložte do
   proměnné prostředí `GITLAB_TOKEN` podle vaší instalace.
3. Kontrola bez agenta: `codex mcp list` a `codex mcp get gitlab --json` (v `enabled_tools` jsou čtyři
   nástroje).
4. Nová session `codex --sandbox read-only`, příkaz `/mcp`: server a jeho nástroje. Názvy nástrojů jsou
   podle dokumentace GitLabu. Když `/mcp` žádný neukáže, vaše verze je pojmenovává jinak: `enabled_tools`
   dočasně vynechte, názvy si v `/mcp` přečtěte a vyberte jen nástroje pro čtení.
5. Zadání (čísla a projekt doplňte):

   ```text
   Přečti work item <číslo> v projektu <skupina/projekt> a stav pipeline merge requestu
   !<číslo> ve stejném projektu. Shrň, co tiket chce a proč pipeline prošla nebo neprošla.
   U každého tvrzení uveď nástroj, ze kterého ho máš. Nic nezapisuj.
   ```

6. Kontrolní otázka:

   ```text
   Jaké nástroje GitLabu máš k dispozici? Žádný nevolej.
   ```

7. Výsledek zapište do `PLAN.md` z cvičení C11, sekce 4 (sloupec „MCP nástroje“).

**Hotovo, když**

- Agent přečetl váš skutečný tiket a stav pipeline a u tvrzení uvádí nástroj.
- Agent má k dispozici jen nástroje pro čtení.
- Sekce 4 vašeho `PLAN.md` uvádí, které nástroje GitLabu tým povolí.

### Z 2. dne, slide 238: Kam s pravidlem: od prosby po zákaz

**Čas:** 10 minut · **Výchozí větev:** bez repozitáře

Stejné pravidlo může být prosba v AGENTS.md, nebo zákaz, který agent nemá jak obejít. Čím víc záleží na
tom, aby se něco nestalo, tím níž na škále patří. Typická chyba: „nikdy neměň presety“ jen
v AGENTS.md. Agent to většinou dodrží, ale ne vždy, a právě ta výjimka je drahá.

| stupeň | kdy platí | příklad z kurzu |
|---|---|---|
| AGENTS.md | v každém vlákně; model ho většinou dodrží, ne vždy | build jedním příkazem, konvence, kde co je |
| skill (`$jméno`) | když se úloha hodí, nebo když ho zavoláte | oprava s regresním testem, nový modul |
| hook (`[[hooks.PreToolUse]]`) | při každé akci; model ho neobejde | zákaz úprav presetů a `.clang-tidy` |
| sandbox (`-s workspace-write`) | vždy; ohrada procesů, ne text | zápis jen do repa, bez sítě |
| `requirements.toml` (spravovaná konfigurace od IT) | vždy; uživatel ho nepřebije | schvalování nejde vypnout, MCP jen schválené |

Prosba je nahoře, zákaz dole. Ukázka hooku je na slidu 236 2. dne.

**Kroky**

1. Zapište si tři pravidla, která dnes agentovi píšete do zadání (například „nesahej na…“, „spusť
   testy“, „nepřidávej závislosti“).
2. U každého se zeptejte: co se stane, když ho agent jednou nedodrží? Podle toho ho zařaďte na stupeň.
3. Ke každému připište mechanismus (soubor, nastavení) a kdo ho vlastní.
4. Pravidla přeneste do `PLAN.md` z cvičení C11 (sekce 3 a 4).

**Hotovo, když**

- Máte tabulku tří pravidel se stupněm, mechanismem a vlastníkem.
- U každého pravidla umíte říct, proč nestačí stupeň nad ním.

### Z 2. dne, slide 251: Varianta D v Pythonu nebo C#

**Čas:** 45 minut · **Výchozí větev:** `main`

Postup z obou dnů platí v každém jazyce. Mění se příkazy v AGENTS.md a v definici hotovo, ne postup.
Ve vzorovém repozitáři je Python dvakrát: skripty ve `scripts/` (testy unittest, ctest je pouští jako
`scripts.unittest`) a MCP server v `mcp-lab-server/` (pytest). Úloha je na skriptech, protože nepotřebují
virtuální prostředí. C# v kurzu ukázku nemá; použijte tabulku na vlastním repozitáři.

| | C++ (kurz) | Python | C# |
|---|---|---|---|
| Build a testy jedním příkazem | `python scripts/check_all.py debug` | skript v repu, u nás `python scripts/dev.py test` (MCP server) | skript v repu, který postaví a pustí testy |
| Testy napřed | GoogleTest | pytest, unittest | xUnit nebo NUnit |
| Statická kontrola v definici hotovo | preset `tidy` (clang-tidy) | linter a kontrola typů podle týmu | analyzátory Roslyn, varování jako chyby |
| Paměť a nedefinované chování | preset `asan` | hlídá interpret, sanitizery odpadají | spravovaná paměť, sanitizery odpadají |
| Ukázka z kurzu | `sample-service` | `mcp-lab-server`, `scripts/` v `sample-service` | žádná, postup stejný |

**Kroky**

1. `git switch main`, pak `git switch -c navic/python`
2. Nejdřív AGENTS.md. V editoru založte vnořený soubor `scripts/AGENTS.md` (kostru upravte):

   ```markdown
   # AGENTS.md for scripts/

   Python helpers for sample-service. Standard library only, Python 3.9+.

   ## Build and test

   - From `scripts/`: `python -m unittest discover -s tests -t .`
   - The same tests run in ctest as `scripts.unittest`:
     `python scripts/check_all.py debug` from the repository root.

   ## Rules

   - No third-party packages; a new dependency needs a human decision.
   - New behavior starts with a failing test in `tests/test_scripts.py`.
   - Keep the output format of existing commands unless the task says otherwise.

   ## Done means

   1. The unittest command above passes.
   2. New or changed behavior has tests.
   3. The usage example in the module docstring is updated when an option changes.
   ```

   Commit:

   ```sh
   git add scripts/AGENTS.md
   git commit -m "Add AGENTS.md for scripts"
   ```

3. Vnořený AGENTS.md se načte, jen když Codex běží ve `scripts/`. Ověřte to:
   `python ../prompt_input.py --agents --slozka scripts`. Pak `cd scripts` a `codex --sandbox read-only`.
4. Projděte kroky 1 až 6 cvičení C10 s touto úlohou (zadání pro krok 1):

   ```text
   Úloha: scripts/summarize_results.py dostane volbu --assay <metoda>, kterou jde zadat
   víckrát. Souhrn pak obsahuje jen zadané metody. Metoda, která ve vstupu není, se ohlásí
   na standardním chybovém výstupu a skript skončí s kódem 1. Bez volby se chování nemění.
   Nejdřív si přečti související kód a testy, nic neměň. Vypiš soubory, kterých se změna
   týká, a otázky, které k zadání máš.
   ```

   Na otázky agenta odpovězte sami. Testy ve `scripts/`: `python -m unittest discover -s tests -t .`.
   Revizi v nové session pusťte z kořene repa (`cd ..`):
   `codex exec -s read-only -o review.md review --base main`.

**Hotovo, když**

- `scripts/AGENTS.md` je v commitu a Codex ho ve `scripts/` načte.
- Nové testy bez změny padají a se změnou projdou (unittest ve `scripts/` i
  `python scripts/check_all.py debug` z kořene).
- Diff je jen ve `scripts/`, máte `review.md` s rozhodnutími a `mr.md` s příkazy a výsledky.

### Z 2. dne, slide 245: Stejná úloha s levnějším modelem

**Čas:** 60 minut · **Výchozí větev:** `cviceni/10-start`

Dražší model není vždy lepší volba: na úloze s jasným zadáním a testy může stačit levnější. Zjistíte
to jen měřením na stejné úloze se stejnými zadáními. Porovnáváte počet kol do zelených testů,
spotřebu a výsledek.

**Kroky**

1. Dvakrát projdete zkrácenou variantu cvičení C10 (varianta A, kroky 1, 3, 4 a 6) se stejnými zadáními.
   Zadání si zkopírujte ze `cviceni/10-zaverecne/ZADANI.md`, ať jsou v obou bězích doslova stejná.
2. První běh s výchozím modelem:

   ```sh
   git switch cviceni/10-start
   git switch -c c10-a-vychozi
   codex --sandbox read-only
   ```

   Model a úsilí si ověřte v `/status`. Před krokem 3 povolte zápis příkazem `/permissions` jako v C10.

3. Druhý běh s levnějším modelem (které modely váš účet nabízí, ukáže v session `/model`):

   ```sh
   git switch cviceni/10-start
   git switch -c c10-a-levny
   codex -m gpt-6-luna --sandbox read-only
   ```

4. U každého běhu si zapište:

   | | výchozí model | levnější model |
   |---|---|---|
   | model a úsilí (`/status`) | | |
   | vašich zpráv do zelených testů | | |
   | kolikrát agent pustil testy | | |
   | spotřeba tokenů (`/status`) | | |
   | `git diff --stat` | | |
   | `python scripts/check_all.py debug asan tidy` | | |

5. `git diff c10-a-vychozi c10-a-levny -- tests/`: testují obě větve totéž? Který diff byste poslali
   do revize?

**Hotovo, když**

- Máte vyplněnou tabulku pro oba běhy.
- Umíte říct, u kterého kroku levnější model stačil a kde ne.

### Z 2. dne, slide 257: Kdo vlastní co (RACI) pro váš tým

**Čas:** 15 minut · **Výchozí větev:** bez repozitáře (váš `PLAN.md` z cvičení C11)

Instrukce bez vlastníka zastarají do měsíce a agent pak dělá chyby, které už jednou opravil. Tabulka
je náš návrh pro C++ tým s GitLabem. Přepíšete ji na jména.

| | Tech lead | Správce instrukcí | Vývojář | QA | DevOps | IT |
|---|---|---|---|---|---|---|
| AGENTS.md, skilly, role | A | R | C | | C | |
| Týmová konfigurace a hooky | A | R | I | | C | C |
| `requirements.toml` | A | C | | | | R |
| MCP napojení a tokeny | A | C | | | C | R |
| Presety, sanitizery, pipeline | A | | C | C | R | |
| Revize a sloučení MR | A | | R | C | | |
| Měření přínosu | A | R | | C | | |

R = dělá, A = rozhoduje a odpovídá, C = konzultuje se s ním, I = je informován. Správce instrukcí je
role, ne funkce: jeden člověk, který slučuje změny AGENTS.md a skillů a jednou za sprint je pročistí.

**Kroky**

1. Tabulku zkopírujte do svého `PLAN.md` z C11 jako novou sekci „7. Kdo vlastní co“.
2. Role nahraďte jmény. Roli, kterou tým nemá, sloučte s jinou; řádek, který se vás netýká, smažte.
3. Zkontrolujte: každý řádek má právě jedno A a aspoň jedno R; R u řádku „AGENTS.md, skilly, role“ je
   stejný člověk jako vlastník instrukcí v sekci 3 vašeho `PLAN.md`.

**Hotovo, když**

- Tabulka má místo rolí jména a žádný řádek bez A nebo bez R.
- Vlastník instrukcí je v sekci 3 i v tabulce stejný.

---

## Bonus

### Bonus, slide 264: codex exec se schématem výstupu

**Čas:** 15 minut · **Výchozí větev:** `main`

Ve skriptu nečtěte volný text: první změna formulace ho rozbije. `codex exec --output-schema` dostane
JSON Schema a poslední odpověď agenta mu musí odpovídat. Skript pak čte JSON. Výchozí sandbox
`codex exec` je jen pro čtení.

**Kroky**

1. `git switch main`
2. Vedle složky `sample-service` (ne do repozitáře) uložte `findings.schema.json`:

   ```json
   {
     "type": "object",
     "additionalProperties": false,
     "required": ["findings"],
     "properties": {
       "findings": {
         "type": "array",
         "items": {
           "type": "object",
           "additionalProperties": false,
           "required": ["file", "line", "object", "invalidated_by", "severity", "test_idea"],
           "properties": {
             "file": { "type": "string", "description": "path relative to the repository root" },
             "line": { "type": "integer", "description": "line where the reference, pointer or view is kept" },
             "object": { "type": "string", "description": "the object the reference points to" },
             "invalidated_by": { "type": "string", "description": "what ends the object's lifetime or moves it" },
             "severity": { "type": "string", "enum": ["high", "medium", "low"] },
             "test_idea": { "type": "string", "description": "a test that would expose the problem" }
           }
         }
       }
     }
   }
   ```

3. V kořeni repa spusťte (jeden příkaz):

   ```sh
   codex exec -s read-only --output-schema ../findings.schema.json -o ../findings.json --json "Najdi v src/labio místa, kde reference, ukazatel nebo string_view může přežít objekt, na který ukazuje. Odpověz podle schématu."
   ```

   `-o` uloží poslední odpověď do souboru, `--json` vypisuje průběh jako události JSON, jednu na řádek.
   Spotřebu tokenů najdete v události `turn.completed` na konci výpisu.

4. Přečtěte výsledek skriptem, ne očima:

   ```sh
   python -c "import json; d = json.load(open('../findings.json', encoding='utf-8')); [print('{file}:{line}  {severity}  {object}'.format(**f)) for f in d['findings']]"
   ```

5. Dva nálezy ověřte v kódu.
6. Navázání na stejnou session (opět jen čtení):

   ```sh
   codex exec resume --last "Ke každému nálezu navrhni test v GoogleTest, který by problém odhalil. Nic neměň."
   ```

   Přepínač `--ephemeral` session neuloží na disk; pro jednorázové běhy ve skriptu se hodí, ale `resume`
   pak nemá na co navázat.

**Hotovo, když**

- Skript z kroku 4 vypíše nálezy bez chyby.
- Dva nálezy jste ověřili v kódu.
- Víte, kde je ve výstupu `--json` spotřeba tokenů.

### Bonus, slide 265: Revize MR v GitLab CI

**Čas:** 30 minut · **Výchozí větev:** váš testovací projekt v GitLabu · **Volitelné, potřebujete:**

- Projekt v GitLabu s runnerem, který spouští kontejnery (Docker executor).
- API klíč OpenAI jako proměnnou CI/CD `CODEX_REVIEW_KEY` (masked). API se platí podle ceníku API,
  mimo kredity předplatného; nastavte mu vlastní rozpočet.
- Souhlas firmy: kód z MR odchází k OpenAI.

Job je náš návrh, oficiální příklad pro GitLab CI není a v GitLabu jsme job nespouštěli. První krok do
CI je revize jen pro čtení s výsledkem jako artefakt; komentáře do MR až s důvěrou.

**Kroky**

1. Založte v GitLabu prázdný testovací projekt a nahrajte do něj vzorový repozitář (ne firemní kód):

   ```sh
   git remote add gitlab <adresa projektu>
   git push gitlab main
   ```

2. `git switch -c navic/ci-review` a v editoru přidejte `.gitlab-ci.yml` (když už nějaký máte, přidejte
   jen job a stage `review`):

   ```yaml
   stages:
     - review

   codex-review:
     stage: review
     image: node:22-bookworm
     rules:
       - if: $CI_PIPELINE_SOURCE == "merge_request_event"
     variables:
       GIT_DEPTH: "0"
     script:
       - npm install -g @openai/codex@0.166.0
       - git fetch origin "${CI_MERGE_REQUEST_TARGET_BRANCH_NAME}:refs/remotes/origin/${CI_MERGE_REQUEST_TARGET_BRANCH_NAME}"
       - CODEX_API_KEY="$CODEX_REVIEW_KEY" codex exec -s read-only -o review.md review --base "origin/${CI_MERGE_REQUEST_TARGET_BRANCH_NAME}"
     artifacts:
       when: always
       paths:
         - review.md
   ```

   Proč takto: verze Codexu je zamčená; klíč dostane jen příkaz Codexu, ne celý job, který spouští kód
   z MR; `-s read-only`, revize nic nemění; `GIT_DEPTH: "0"`, protože diff proti cílové větvi potřebuje
   historii; obraz s Node a npm (obraz z `Dockerfile` repa je nemá). Příkazy v `script` běží v shellu
   runneru, ne u vás.

3. Proměnná `CODEX_REVIEW_KEY`: Settings > CI/CD > Variables, Masked. Když ji označíte i jako
   Protected, dostanou ji jen pipeline chráněných větví; ověřte, že ji job MR na vaší instanci vidí.
4. Commit a nahrání:

   ```sh
   git add .gitlab-ci.yml
   git commit -m "Add Codex review job"
   git push gitlab navic/ci-review
   ```

   Pak v GitLabu založte merge request do `main`.
5. V pipeline MR otevřete log jobu `codex-review` a artefakt `review.md`. K nálezům připište: opravit,
   nebo ne, a proč.

**Hotovo, když**

- Pipeline MR má artefakt `review.md` a klíč se v logu jobu neobjevil.
- Víte, kolik jeden běh stál (spotřeba v přehledu API).

### Bonus, slide 267: Úloha v Codex cloud

**Čas:** 30 minut · **Výchozí větev:** `cviceni/10-start` ve vašem forku na GitHubu · **Volitelné, potřebujete:**

- Codex cloud povolený pro váš účet (ve firemním prostoru rozhoduje IT; v nových prostorech Enterprise
  je ve výchozím stavu vypnutý).
- Fork tohoto repozitáře na GitHubu se všemi větvemi (při forku zrušte volbu „Copy the main branch
  only“). Vzorový kód není firemní, ale i tak odchází do cloudu OpenAI.
- Prostředí (environment) ve webovém rozhraní Codexu napojené na fork, se setup skriptem, který
  doinstaluje nástroje z README (CMake, Ninja, překladač C++20, clang-tidy, Python); knihovny se
  postaví z `third_party/`.

Úloha běží v kontejneru v cloudu, výsledek si stáhnete jako diff. Víc pokusů znamená víc diffů ke čtení:
vyberete jeden a ostatní zahodíte. Příkaz `codex cloud` je ve verzi 0.163.0 označený jako experimentální.

**Kroky**

1. ID prostředí najdete ve webovém rozhraní, nebo v přehledu, který otevře příkaz `codex cloud`.
2. Pošlete úlohu se dvěma nezávislými pokusy (jeden příkaz):

   ```sh
   codex cloud exec --env <ENV_ID> --branch cviceni/10-start --attempts 2 "Přidej příkaz PRIORITY podle tiketu docs/tickets/LAB-241.md. Nejdřív testy, pak implementace. Hotovo znamená: python scripts/check_all.py debug asan tidy projde. Řiď se AGENTS.md, pokud v repu je."
   ```

3. Stav a výsledky:

   ```sh
   codex cloud list
   codex cloud status <TASK_ID>
   codex cloud diff <TASK_ID> --attempt 1
   codex cloud diff <TASK_ID> --attempt 2
   ```

4. Vyberte lepší pokus a použijte ho u sebe:

   ```sh
   git switch cviceni/10-start
   git switch -c c10-b-cloud
   codex cloud apply <TASK_ID> --attempt <N>
   python scripts/check_all.py debug asan tidy
   ```

**Hotovo, když**

- Porovnali jste oba pokusy a umíte říct, proč jste vybrali ten svůj.
- Vybraný diff je u vás v pracovní kopii a `debug`, `asan` a `tidy` projdou.

### Bonus, slidy 269 a 273: Plán v souboru: ExecPlan a Mikado

**Čas:** 30 + 30 minut · **Výchozí větev:** `cviceni/10c-start` (ExecPlan) a `main` (Mikado)

U úlohy na víc než jedno vlákno se kontext session ztratí, soubor ne. Dva postupy se stejným principem:
stav práce je v souboru, ne v hlavě agenta. ExecPlan (podle OpenAI Cookbook, „Using PLANS.md for
multi-hour problem solving“) je plán dlouhé úlohy, na který naváže nové vlákno nebo kolega. Mikado
(O. Ellnestam, D. Brolund: The Mikado Method) rozloží změnu, která se při prvním pokusu rozsype, na strom
malých kroků.

**A. ExecPlan (30 minut)**

Větev `cviceni/10c-start` obsahuje řešení cvičení C4, proto ji otevírejte až po něm.

1. `git switch cviceni/10c-start`, pak `git switch -c navic/execplan`
2. V editoru uložte do kořene repa `PLANS.md`:

   ````markdown
   # Execution plans (ExecPlans)

   An ExecPlan is a design and work document that a coding agent or a colleague can follow
   from start to finish without any other context. Write one for any task that will not fit
   into a single session. Plans live in `docs/plans/<ticket>.md`.

   ## Rules

   - Self-contained: a reader who has only this repository and the plan can continue the
     work. Name files, functions, commands and terms; never write "as discussed".
   - Living: update Progress, Surprises & Discoveries and Decision Log at every stopping
     point, in the same commit as the work.
   - Verifiable: every step says how to prove it worked, with exact commands and the
     output you expect.
   - Small steps: each step leaves the build green and can be committed on its own.
   - Recoverable: say how to retry or roll back a step that fails halfway.

   ## Skeleton

   ```markdown
   # <goal in one sentence>

   ## Purpose / Big Picture
   What a user can do after this change that they cannot do now, and how to see it work.

   ## Progress
   - [ ] (YYYY-MM-DD HH:MM) <step>
   Split a step that is only partly done into a done part and a remaining part.

   ## Surprises & Discoveries
   Unexpected behavior found while working, with evidence (test output, file:line).

   ## Decision Log
   - Decision: <what>. Rationale: <why>. Date: <when>.

   ## Outcomes & Retrospective
   At milestones and at the end: what was achieved, what is left, what was learned.

   ## Context and Orientation
   The current state for someone new to the code: relevant files by full path, key types
   and functions, terms defined in plain words.

   ## Plan of Work
   The sequence of edits in prose: which file, which function, what changes.

   ## Concrete Steps
   Exact commands with the working directory, and the short output you expect.

   ## Validation and Acceptance
   How to prove the change works: which tests fail before and pass after, which behavior
   to observe, with expected results.

   ## Idempotence and Recovery
   Which steps can be repeated safely; how to retry or roll back a failed step.

   ## Artifacts and Notes
   Short excerpts of output, diffs or logs that prove a point.

   ## Interfaces and Dependencies
   Types, function signatures and modules the change adds or relies on, with full names.
   ```
   ````

   Commit:

   ```sh
   git add PLANS.md
   git commit -m "Add PLANS.md"
   ```

3. `codex --sandbox workspace-write` a zadání:

   ```text
   Přečti PLANS.md a vytvoř ExecPlan pro tiket docs/tickets/LAB-242.md do souboru
   docs/plans/LAB-242.md. Nic neimplementuj, ukaž mi ho.
   ```

4. Projděte plán podle pravidel z `PLANS.md`: má Validation příkazy a očekávané výsledky? Je tam
   Recovery? Jmenuje soubory a funkce celou cestou? Opravte, co chybí, a plán commitněte.
5. Session ukončete. V nové session:

   ```text
   Pokračuj podle docs/plans/LAB-242.md: proveď první nedokončený krok z Progress, ověř ho
   podle Validation and Acceptance, aktualizuj Progress a zastav se.
   ```

**Hotovo, když (A)**

- `docs/plans/LAB-242.md` je v commitu a má všechny sekce kostry.
- Nová session navázala jen podle souboru, bez doplňujících otázek, co je úkolem.
- Progress má po prvním kroku zaškrtnutou položku s časem.

**B. Mikado (30 minut)**

Cíl je stejný jako v kroku 3 cvičení C6: parser bez globálních bufferů. Nejlépe navazuje na kroky 1 a 2
cvičení C6 (charakterizační testy jsou záchranná síť); pak začněte z jeho větve `refactor/legacy-parser`.

1. `git switch main` (nebo `refactor/legacy-parser`), pak `git switch -c navic/mikado`
2. `MIKADO.md` nechte mimo Git, aby ho vracení pokusů nesmazalo: v editoru připište do souboru
   `.git/info/exclude` řádek `MIKADO.md`. Pak v kořeni repa založte `MIKADO.md`:

   ```markdown
   # Cíl: lr_parse bez globálních bufferů g_fld a g_nfld

   Struktura LEGACY_RESULT a konstanty LR_* se nemění.
   Hotovo, když projde python scripts/check_all.py debug tidy.

   ## Strom (překážky jako potomci, zaškrtnuté = hotové a v commitu)

   - [ ] odstranit g_fld a g_nfld z src/labio/legacy/legacy_result_parser.cpp

   ## Pokusy (vrácené)
   ```

   `git status` je potom čistý.

3. `codex --sandbox workspace-write` a zadání:

   ```text
   Cíl je kořen stromu v MIKADO.md. Postupuj metodou Mikado:
   1. Zkus nejbližší otevřený list (na začátku samotný cíl). Pak spusť
      python scripts/check_all.py debug.
   2. Při neúspěchu zapiš překážky do MIKADO.md jako potomky toho listu a pokus do sekce
      Pokusy. Pak pokus vrať: git restore . a git clean -fd. Nic necommituj.
      MIKADO.md Git ignoruje, vrácení ho nesmaže.
   3. Při úspěchu commitni jen tento list a v MIKADO.md ho zaškrtni.
   4. Po třech listech se zastav a ukaž mi MIKADO.md.
   Struktura LEGACY_RESULT a konstanty LR_* se nemění.
   ```

   Když si agent řekne o schválení příkazu `git`, posuďte ho a schvalte jen commit a vracení pokusu.

4. Zkontrolujte: `git log --oneline` (jeden commit na zaškrtnutý list), `git status` (čistý),
   `python scripts/check_all.py debug tidy` (projde) a strom v `MIKADO.md`.

**Hotovo, když (B)**

- `MIKADO.md` má strom s aspoň jednou zapsanou překážkou a záznam o vráceném pokusu.
- Každý commit je jeden list a po každém projde `debug` a `tidy`.
- V pracovní kopii nezůstal rozpracovaný pokus.
