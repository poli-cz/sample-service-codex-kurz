# C10 – Závěrečné cvičení: od zadání po merge request

**Blok:** 7, Závěrečné cvičení a plán zavedení · **Čas:** 55 minut, samostatně

## Cíl

Projdete celý postup z obou dnů na jedné úloze sami: analýza a otázky, plán, testy napřed,
implementace malými kroky, revize v nové session a popis MR s důkazem.

## Varianty (vyberte jednu)

| | úloha | tiket | výchozí větev | obtížnost |
|---|---|---|---|---|
| **A** | `csv_escape` podle RFC 4180 a testy pro celý `csv_report.cpp` | `docs/tickets/LAB-240.md` | `cviceni/10-start` | lehčí: jeden soubor bez jediného testu |
| **B** | příkaz `PRIORITY <id> <STAT\|URGENT\|ROUTINE>` | `docs/tickets/LAB-241.md` | `cviceni/10-start` | střední: fronta, protokol, README |
| **C** | příkaz `QC`: Westgard na kontrolních záznamech `C\|...` z logu analyzátoru | `docs/tickets/LAB-242.md` | `cviceni/10c-start` (obsahuje hotové LAB-201) | těžší: parser, kalibrace, pravidla z 1. dne |
| **D** | úloha z vašeho repozitáře | vlastní | vlastní | jen necitlivý nebo anonymizovaný kód schválený firmou |

Kdo dělal C3b, má skill `add-service-command` ve větvi `c3-agents-md`; hodí se pro variantu B. Do větve
`c10-b` ho přeneste (a commitněte) příkazem `git checkout c3-agents-md -- .agents/skills/add-service-command`.

Nejdřív zahoďte změny agenta z C9, jinak je `git switch` přenese do vaší větve a skončí v diffu
závěrečného cvičení:

```sh
git restore .
git clean -fd
git switch cviceni/10-start                                # pro C: cviceni/10c-start
git switch -c c10-a                                        # nebo c10-b, c10-c
python scripts/check_all.py debug
code .git/info/exclude                                     # připište mr.md a review.md, uložte
codex --sandbox read-only                                  # první dva kroky jen čtení
```

`mr.md` a `review.md` do commitu nepatří, proto je `.git/info/exclude` (jako v C8). Jinak je `git add -A`
v kroku 5 přidá do větve a revize je bude číst jako součást změny.

## Kroky

1. **Analýza a otázky, jen čtení (5 minut).** Zadání:

   > Úloha: tiket `docs/tickets/LAB-24x.md`. Nejdřív si přečti související kód, nic neměň. Vypiš soubory, kterých se změna týká, a otázky, které k zadání máš.

   Na otázky odpovězte sami nebo je zapište do `mr.md` jako otevřené.

2. **Plán (5 minut).** Zadání:

   > Navrhni plán malých kroků, ke každému test, který ho dokáže. Hotovo znamená: nejdřív testy, pak kód; presety `debug`, `asan` a `tidy` zelené.

   Plán schvalujete vy. Teprve pak povolte zápis ve stejné session příkazem `/permissions` (plán zůstane
   v kontextu). Nová session se zápisem plán nezná, museli byste ho vložit znovu.

3. **Testy napřed (15 minut).** Nechte agenta napsat testy podle tiketu a ukázat, že padají. Přečtěte je:
   testují tiket, nebo současný kód?

4. **Implementace malými kroky (15 minut).** Krok za krokem podle plánu, po každém cílené testy. Na konci
   `debug`, `asan` a `tidy`.

5. **Revize v nové session (10 minut).**

   ```sh
   git add -A
   git commit -m "LAB-24x: ..."
   codex exec -s read-only -o review.md review --base cviceni/10-start   # u C: --base cviceni/10c-start
   ```

   Do `review.md` k nálezům připište: opravit, nebo ne, a proč.

6. **mr.md (5 minut).** Co a proč, jak ověřeno (příkazy a skutečné výsledky), rizika, otevřené otázky,
   s čím pomáhal agent.

## Hotovo, když

- Nové testy bez změny kódu padají a se změnou projdou v debug i asan; tidy bez nálezů.
- Diff obsahuje jen to, co tiket chce.
- Máte `review.md` s rozhodnutími a `mr.md` s příkazy a výsledky.

## Zkrácená varianta (30 minut)

Jen varianta A, kroky 1 (5 minut), 3 (10), 4 (10) a 6 (5); revize v nové session odpadá. Před krokem 3
povolte zápis příkazem `/permissions` jako v kroku 2.

## Varianta D: vlastní repozitář

- Jen kód, který firma pro práci s Codexem schválila; žádné tajné hodnoty, zákaznická data ani interní adresy.
- Nejdřív AGENTS.md (z bloku 3): build a testy jedním příkazem, definice hotovo, na co nesahat. Bez toho
  agent stráví první polovinu času zjišťováním, jak se projekt staví.
- Vyberte úlohu na 30 minut práce člověka s jasným kritériem hotovo (oprava malé chyby s testem, chybějící
  testy pro jeden soubor, malý příkaz nebo volba).
- Postup a kritérium hotovo jsou stejné jako výše.
