# C5 – LAB-214: občas se ztratí rutinní vzorek

**Blok:** 4b, Od požadavku po MR, část 2 (ladění, sanitizery, revize) · **Čas:** 55 minut, každý sám

## Cíl

Dostanete hlášení z provozu, ne zadání: symptom, výstup služby z jedné noci a nic víc. S Codexem
chybu zopakujete testem a najdete příčinu. Opravíte ji nejmenším diffem a doložíte presety.
Pak opravu necháte zrevidovat v nové session a napíšete popis merge requestu. Hotovo je důkaz, ne věta
„opraveno“.

Pokud jste si v C1 všimli, že agent u některé funkce zaváhal, tím lépe. Podezření ze čtení kódu ale ještě
není důkaz: tady jde o test, který chybu zopakuje, a o opravu, kterou doložíte.

## Výchozí stav

Větev `cviceni/05-start`: `main` plus tiket `docs/tickets/LAB-214.md` a příloha
`docs/tickets/LAB-214-service-output.jsonl` (výstup služby z té noci).

Práci z C4, kterou si chcete nechat, nejdřív commitněte ve větvi `lab-201` (`git add -A`, pak
`git commit -m "LAB-201: Westgard rules for control samples"`). Zbytek zahoďte, jinak ho `git switch`
přenese do nové větve:

```sh
git restore .
git clean -fd
git switch cviceni/05-start
git switch -c fix/lab-214
python scripts/check_all.py debug        # všechno zelené: chybu žádný test nepokrývá
codex --sandbox workspace-write
```

## Kroky

1. **Reprodukce (10 minut).** Zadání:

   > Přečti `docs/tickets/LAB-214.md` a přiložený výstup služby. Zopakuj chybu: přidej do `tests/labsched` test v GoogleTest, který bez opravy padá. Do `src/` zatím nesahej. Zastav se a ukaž mi, jak test padá.

   Přečtěte test. Padá ze stejného důvodu, jaký popisuje tiket (ztracený vzorek, chybné ID v logu)?
   Dokud test nepadá správně, nepouštějte agenta dál.

2. **Diagnóza (10 minut).** Zadání:

   > Spusť `python scripts/check_all.py asan` a z výpisu AddressSanitizeru vysvětli příčinu: soubor, řádek a proč to v debug buildu nepadá.

   Porovnejte vysvětlení s výpisem ASanu sami.

3. **Oprava (10 minut).** Zadání:

   > Oprav příčinu nejmenším možným diffem, bez refaktoringu. Test neměň.

4. **Důkaz (5 minut).** Sami nebo agentem:

   ```sh
   python scripts/check_all.py debug asan tidy
   ```

   Ověřte i opačný směr: s vrácenou opravou test padá. Odložte jen opravu v `src/` (test ani oprava ještě
   nejsou v commitu a samotné `git stash` by odložilo i test):

   ```sh
   git stash push -- src/
   python scripts/check_all.py asan     # nový test padá s hlášením AddressSanitizeru
   git stash pop
   ```

5. **Revize v nové session (10 minut).** Commitněte a spusťte revizi jen pro čtení:

   ```sh
   git add -A
   git commit -m "LAB-214: keep the preempted sample"
   codex exec -s read-only -o review.md review --base cviceni/05-start
   ```

   Základem revize je větev, ze které jste vyšli (`cviceni/05-start`): revize pak vidí jen vaši opravu
   a test. (Nebo nová session v TUI a `/review`.) Do `review.md` ke **každému** nálezu připište: opravit,
   nebo ne, a proč.

6. **Popis MR (10 minut).** Zadání v původní session:

   > Napiš `mr.md`: co a proč, oprava, jak ověřeno (příkazy a jejich skutečné výsledky), kam se má recenzent dívat, s čím pomáhal agent. Vycházej jen z diffu a z výsledků příkazů.

   `review.md` a `mr.md` se necommitují. Po cvičení je přesuňte mimo repozitář (ve správci souborů
   nebo ve VS Code), jinak je `git add -A` v dalších cvičeních přidá do commitu.

## Hotovo, když

- Nový test bez opravy padá (v `asan` s hlášením AddressSanitizeru) a s opravou projde v debug i asan.
- `tidy` projde bez nálezů.
- Diff mění jen příčinu a přidává test; žádné oslabené nebo smazané testy.
- `review.md` má u každého nálezu rozhodnutí a `mr.md` obsahuje příkazy a jejich výsledky.

## Zkrácená varianta (30 minut)

Kroky 1 (10 minut), 3 (5), 4 (5) a 6 (10). Revizi ukáže lektor. Kdo skončí dřív: zkuste, jestli agent chybu najde,
i když v zadání nezmíníte `asan`.
