# C8 – Sub-agenti a dvě úlohy naráz

**Blok:** 5a, Více agentů: role, koordinace, sub-agenti, pracovní kopie · **Čas:** 40 minut

## Cíl

Napíšete dvě vlastní role sub-agentů (`tester` a `reviewer`). Pak spustíte dvě nezávislé úlohy paralelně
ve dvou oddělených pracovních kopiích (`git worktree`) a po revizi je sloučíte. Uvidíte, kde druhý
agent šetří čas a kde jen přesouvá čekání na vás.

## Výchozí stav

`sample-service`, větev `main`, postavený. Zadání úloh jsou v souborech `BRIEF-A.md` (validace
konfigurace) a `BRIEF-B.md` (validace v JSON kodeku) vedle tohoto zadání. Úlohy mění různé soubory,
takže by při sloučení neměl vzniknout konflikt. Po C5 a C6 jste na jiné větvi. Práci z C6, kterou si
chcete nechat, commitněte ve větvi `refactor/legacy-parser` (`FINDINGS.md` přesuňte mimo repozitář).
Zbytek zahoďte, jinak ho `git switch` přenese do `main`:

```sh
git restore .
git clean -fd
git switch main
```

## Kroky

1. **Role (10 minut).** V kořeni repa vytvořte `.codex/agents/tester.toml` a `.codex/agents/reviewer.toml`.
   Každý soubor musí mít tři pole: `name`, `description`, `developer_instructions`. Příklad:

   ```toml
   name = "tester"
   description = "Writes failing GoogleTest cases first. Edits files under tests/ only."
   developer_instructions = """
   Write failing GoogleTest cases for the agreed behaviour before any implementation exists.
   Only edit files under tests/. Build and run only the affected test executable and report
   which tests fail and why.
   """
   ```

   Role `reviewer`: čte diff, hlásí nálezy se souborem a řádkem podle závažnosti, nic nemění. Commitněte:

   ```sh
   git add .codex
   git commit -m "Add agent roles tester and reviewer"
   ```

   Ověření: nová session (`codex`), zadání „Jaké vlastní role agentů máš k dispozici? Nic neměň.“ Codex načítá
   projektovou konfiguraci jen v důvěryhodném projektu; při prvním spuštění v repu důvěru potvrďte.

2. **Dva worktree (5 minut).**

   ```sh
   git worktree add ../ss-a -b c8/config-validation
   git worktree add ../ss-b -b c8/json-validation
   code .git/info/exclude
   ```

   Do souboru `.git/info/exclude` v hlavním repu připište dva řádky, `BRIEF.md` a `REVIEW.md`, a uložte
   ho (v libovolném editoru, `code` je VS Code). Pak zkopírujte `BRIEF-A.md` do `../ss-a` a `BRIEF-B.md`
   do `../ss-b`, v obou pod jménem `BRIEF.md` (ve správci souborů nebo ve VS Code).
   `BRIEF.md` a `REVIEW.md` se necommitují (jinak by při sloučení obou větví vznikl konflikt). `git status`
   v `../ss-a` je po zkopírování čistý.
   Každý worktree má vlastní `build/`; první build v něm chvíli trvá.

3. **Dvě úlohy paralelně (10 minut).** Dva terminály, oba v hlavním adresáři repa:
   `codex --sandbox workspace-write -C ../ss-a` a `codex --sandbox workspace-write -C ../ss-b`.
   (Worktree patří k repu, kterému jste důvěru potvrdili v kroku 1; Codex se znovu neptá.) V obou
   stejné zadání:

   > Použij agenta `tester`: ať podle `BRIEF.md` napíše padající testy a ukáže mi, jak padají. Pak se zastav.

   Přečtěte testy. Po vašem souhlasu:

   > Implementuj podle `BRIEF.md`, dokud testy neprojdou. Ověř `debug`, `asan` a `tidy`.

   Zatímco jeden agent pracuje, čtěte výsledek druhého.

4. **Revize (10 minut).** V každém worktree commitněte a spusťte revizi. Role reviewer v souboru
   **nezajistí**, že nic nezmění (sub-agent dědí oprávnění session), proto revizi spusťte v režimu jen
   pro čtení:

   ```sh
   cd ../ss-a                                     # pro úlohu B: cd ../ss-b a "Task B"
   git add -A
   git commit -m "Task A"
   codex exec -s read-only -o REVIEW.md "Použij agenta reviewer: ať zreviduje diff proti větvi main. Nic neměň."
   git status                                     # reviewer nesmí nic změnit
   ```

   Když role reviewer nefunguje, použijte vestavěnou revizi:
   `codex exec -s read-only -o REVIEW.md review --base main`.

   Ke každému nálezu v `REVIEW.md` rozhodněte: opravit, nebo ne, a proč. Rozhodnutí si před krokem 5
   přeneste do poznámek: `git worktree remove` smaže worktree i s `REVIEW.md`.

5. **Sloučení a úklid (5 minut).** Nejdřív ukončete obě session Codexu z kroku 3 (`/quit`) a zavřete
   soubory z `../ss-a` a `../ss-b` v editoru: složku, ve které ještě něco běží, nejde smazat. Pak
   v hlavním adresáři repa:

   ```sh
   cd ../sample-service
   git merge --no-ff --no-edit c8/config-validation
   git merge --no-ff --no-edit c8/json-validation
   python scripts/check_all.py debug
   git worktree remove ../ss-a
   git worktree remove ../ss-b
   ```

## Hotovo, když

- Dva TOML soubory rolí jsou v commitu a Codex je vidí.
- Obě větve mají zelené testy v debug i asan a tidy bez nálezů. U každé je `REVIEW.md` s rozhodnutím.
- Revize nic nezměnila (`git status` po revizi je čistý).
- Obě větve jsou sloučené, sloučený stav je zelený a pracovní kopie jsou uklizené (`git worktree list`).

## Zkrácená varianta (20 minut)

Jen role reviewer a jedna úloha (A) v jednom worktree, revize podle kroku 4.

## K zamyšlení

- Ušetřil vám druhý agent čas, nebo jen přesunul čekání na vás?
- Udržel se tester jen v `tests/`? Čím byste to vynutili, kdyby ne?
