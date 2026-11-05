# C3 – AGENTS.md pro vzorový projekt (a volitelně C3b: skill)

**Blok:** 3, Instrukce a kontext projektu · **Čas:** 50 minut, C3b dalších 15 minut (volitelně)

## Cíl

Napíšete AGENTS.md pro `sample-service` a změříte, co změní: stejnou malou úlohu dáte agentovi
před a po. AGENTS.md nemá opakovat, co agent v kódu najde sám; má obsahovat to, co ví tým.

## Výchozí stav

`sample-service`, větev `main` (bez AGENTS.md), postavený. Kdo chce, může cvičení dělat na vlastním
necitlivém repozitáři; pak si malou úlohu pro test před a po vymyslete sami.

## Malá úloha pro test před a po (pokaždé stejná, pokaždé nová session)

> Přidej do `labcore` funkci `lab::qc::control_zone(double value, double target_mean, double target_sd)`, která vrátí zónu kontrolní hodnoty jako `int`: 0 uvnitř ±1 SD, 1 za 1 SD, 2 za 2 SD, 3 za 3 SD; kladné číslo nad průměrem, záporné pod ním. Přidej testy a ověř, že všechno projde.

## Co ví tým (v kódu to není)

Toto vám řekl vedoucí týmu. Rozhodněte, co z toho patří do AGENTS.md a jak to napsat, aby to agent
mohl dodržet a ověřit.

- Hotovo znamená: projde `python scripts/check_all.py debug tidy`; když změna sahá na paměť,
  kontejnery nebo životnost objektů (fronta, stojan, plánovač, parser), tak i preset `asan`.
- Nové závislosti jen po schválení člověkem; kopie knihoven v `third_party/` se neupravují, žádný
  `FetchContent`.
- `NOLINT` a `-Wno-*` nejsou cesta, jak projít buildem.
- Kontrola kvality: „za limitem“ znamená ostře větší (nebo menší) než limit, hodnota přesně na limitu
  je uvnitř. Cílová SD ≤ 0 je chyba konfigurace a vyhazuje `std::invalid_argument`. Funkce
  `lab::stats::z_score` vrací pro SD ≤ 0 nulu schválně (je jen pro zobrazení), pro rozhodování
  o kvalitě se nepoužívá.
- Výsledky s příznakem `E` (chyba přístroje) nikdy nevstupují do statistik ani do kontroly kvality.
- Struktura `LEGACY_RESULT` v legacy parseru se nesmí měnit (staré nástroje čtou soubory s ní).
  Před refaktoringem parseru nejdřív charakterizační testy.
- Popis merge requestu: co a proč, jak otestováno (příkazy), rizika, co zůstalo otevřené.

## Kroky

1. **Před (10 minut).** Čistý stav (`git switch main`, `git restore .`, `git clean -fd` jako v C2), nová session
   `codex --sandbox workspace-write`, vložte malou úlohu. Zapište si: které příkazy agent spustil,
   jestli spustil tidy, co vrací pro SD 0 a pro hodnotu přesně na limitu (například 120 při průměru
   100 a SD 10), co napsal ve shrnutí. Pak zase čistý stav.

2. **Napište AGENTS.md (20 minut)** do kořene repa. Pomůže vám `kontrolni-seznam.md`. Pravidla:
   krátce, anglicky (jako kód a README), každé pravidlo ověřitelné příkazem nebo v diffu.
   Kostru můžete nechat vygenerovat příkazem `/init` v Codexu, ale pak ji projděte a seškrtejte.

3. **Commit.** AGENTS.md patří do Gitu jako kód:

   ```sh
   git switch -c c3-agents-md
   git add AGENTS.md
   git commit -m "Add AGENTS.md"
   ```

4. **Po (10 minut).** Codex načítá AGENTS.md při startu, proto **nová** session. Stejná malá úloha.
   Zapište si totéž co v kroku 1 a porovnejte.

5. **Porovnání s referencí (5 minut).** Referenční AGENTS.md je ve větvi `reference/agents-md`:

   ```sh
   git diff c3-agents-md origin/reference/agents-md -- AGENTS.md src/labio/AGENTS.md
   ```

   Všimněte si `src/labio/AGENTS.md`: pravidla jen pro jeden modul ve vnořeném souboru. Pozor,
   Codex skládá instrukce od kořene repa jen po adresář, ve kterém běží: vnořený soubor se načte,
   když Codex spustíte v `src/labio` (nebo pod ním), ne když pracuje z kořene repa.

## Hotovo, když

- AGENTS.md je v commitu ve vaší větvi.
- Máte vedle sebe záznam „před“ a „po“: příkazy, tidy, chování pro SD 0, shrnutí.
- Umíte říct, které pravidlo z AGENTS.md změnilo chování agenta a které nezměnilo nic.

## Zkrácená varianta

Jen „po“: AGENTS.md jen se sekcemi Build and test, Domain rules a Done means, jedna session
s malou úlohou. Výsledek „před“ ukáže lektor ze zkušebního běhu; kdo cvičení dělá po kurzu, udělá i krok 1.

## C3b (volitelně, 15 minut): skill

Skill je zabalený opakovaný postup, který si agent načte, až když ho potřebuje (na rozdíl od AGENTS.md,
který čte vždy). V repu patří do `.agents/skills/<jméno>/SKILL.md`.

1. Vytvořte skill `add-service-command`: postup, jak do služby přidat nový příkaz (kde `CommandProcessor`
   rozděluje příkazy, kam patří logika, jak vypadá chyba použití, kam testy, README).
   Soubor založte sami, nebo agentovi zápis schvalte: v sandboxu `workspace-write` je adresář `.agents/`
   pro agenta jen pro čtení. Codex se na zápis zeptá dvakrát, obojí schvalte. Soubor začíná hlavičkou:

   ```markdown
   ---
   name: add-service-command
   description: <kdy se má skill použít; podle tohoto textu ho agent vybírá>
   ---
   ```

2. Commitněte ho a v nové session ověřte, že ho Codex vidí: příkaz `/skills`, nebo zadání
   začínající `$add-service-command`.
3. Skill se vám bude hodit v závěrečném cvičení (varianta B, příkaz `PRIORITY`).
