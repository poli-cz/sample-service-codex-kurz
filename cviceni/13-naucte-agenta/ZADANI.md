# C13 – Naučte agenta váš projekt: AGENTS.md, skill a hook

**Blok:** 2. den ráno, první cvičení dne · **Čas:** 65 minut

## Cíl

Agent v čistém repu bez dokumentace selže na úloze, kterou by člověk z týmu udělal správně. Neví, co
ví tým. Nejdřív to uvidíte na vlastní obrazovce, pak mu to předáte třemi nástroji a změříte rozdíl:

| nástroj | co do něj patří | kdy se uplatní |
|---|---|---|
| `AGENTS.md` | fakta a pravidla, která platí vždy | agent ho dostane na začátku každé session |
| skill | postup pro opakovaný druh úlohy | agent si ho načte, když úloha odpovídá popisu |
| hook | zákaz, který nesmí záviset na tom, jestli agent poslechne | spouští ho Codex, ne model |

## Výchozí stav

Nové repo **analyzer-link**: malá C++ knihovna, která překládá textové řádky z laboratorního analyzátoru
a posílá chyby přístroje do LIS. Bez AGENTS.md, bez dokumentace, bez tiketů. Klonujte ho **vedle**
repa kurzu, ne dovnitř:

```sh
cd ..
git clone https://github.com/poli-cz/analyzer-link
cd analyzer-link
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

Co repo dělá: laboratorní analyzátor posílá textové řádky a knihovna je překládá.
`HB|AN-07|2.6` je heartbeat (přístroj hlásí, že žije, a verzi protokolu, kterou mluví),
`RES|AN-07|S-1001|GLU|5.4|mmol/L` je výsledek, `ERR|AN-07|9|S-1003` je chyba přístroje s číslem
(9 = došla reagencie). Chyby knihovna předá do LIS (laboratorní informační systém) s jedním písmenem,
podle kterého LIS rozhodne, co s výsledky:

| příznak | co se stalo | co LIS udělá |
|---|---|---|
| `S` | problém se vzorkem | požádá o nový vzorek |
| `R` | došla reagencie, nic se nezměřilo | počká na doplnění a změří znovu |
| `H` | porucha přístroje | počká na servis |
| `Q` | už naměřené výsledky mohou být špatně | výsledky nevydá, dokud je někdo neprověří |

Ve složce `tests/data/recorded/` jsou nahrávky: skutečné řádky ze dvou přístrojů a co z nich má
knihovna vypsat. Podle nich běží regresní test.

Kontrolní skript je v repu kurzu, agent ho nevidí. Spouštějte ho z kořene `analyzer-link`
(na Windows `..\sample-service\cviceni\...`, cesta podle toho, kam jste naklonovali repo kurzu):

```sh
python ../sample-service/cviceni/13-naucte-agenta/zkontroluj.py .
```

Na nedotčeném repu vypíše „Splněno 4 z 7“: tiket ještě nikdo neřešil.

## Úloha pro agenta (pokaždé stejná, pokaždé nová session)

> Vyřeš tiket LAB-318.
>
> LAB-318: Firmware 3.2 hlásí novou chybu přístroje
> Analyzátory s firmwarem 3.2 posílají novou chybu „reagent lot expired“ s kódem 14, například
> `ERR|AN-07|14|S-1042`. analyzer-link ji dnes odmítne jako `invalid: unknown error code 14`.
> Přidej ji: dekódování, text pro obsluhu „Reagent lot expired“ a export do LIS.
> Přidej testy a ověř, že všechno projde.

Zadání je slušné: říká co, kde je příklad, jaký text a že chceme testy. Úloha je vybraná a vyzkoušená
tak, aby agent bez znalostí týmu skoro jistě selhal (výsledky zkušebních běhů ukáže lektor po kroku 1).

## Co ví tým (v kódu to není)

Toto vám řekl vedoucí týmu. Rozhodněte, kam která věta patří.

1. **Hotovo** znamená: build a testy projdou v presetu `debug` **i** `tidy`
   (`cmake --preset tidy`, `cmake --build --preset tidy`, `ctest --preset tidy`).
   `NOLINT`, `-Wno-*` ani úprava `.clang-tidy` a `CMakePresets.json` nejsou cesta, jak projít.
2. **Číslo chyby se nikdy nemění.** Čísla v `InstrumentError` posílá firmware přístroje a ukládají se
   i do archivů výsledků. Seznam nikdy nepřeřazovat ani do něj nevkládat doprostřed, nová chyba se
   přidává na konec a její číslo musí sedět s číslem z firmwaru.
3. **Každá nová chyba je změna protokolu:** `kProtocolMinor` v `include/alink/protocol.hpp`
   o jedna nahoru. Firmware 3.2 se v heartbeatu hlásí verzí 2.7; bez zvýšení knihovna u každého
   heartbeatu varuje „newer than supported“.
4. **Příznak pro LIS:** `Q` (quality hold) pro všechno, kvůli čemu mohou být už naměřené výsledky
   špatně: teplota, prošlá nebo špatná šarže reagencie, kalibrace. LIS takové výsledky nevydá, dokud
   je někdo neprověří. `R` jen pro zásobu reagencie (došla, otevřené víko), kdy se ještě nic
   nenaměřilo. Když si agent není jistý, má se zeptat.
5. **`tests/data/recorded/` jsou nahrávky ze skutečných přístrojů.** Nikdy je neupravovat a nikdy
   do nich nepřidávat ručně psané řádky. Nové případy patří do jednotkových testů
   `tests/*_test.cpp`. Když nahrávka přestane sedět, je chyba v kódu, ne v nahrávce.
6. **Postup pro nový kód chyby** (dělá se několikrát do roka, pokaždé s jiným vývojářem):
   enum na konec, text, příznak LIS podle tabulky z bodu 4, verze protokolu, testy, oba presety.
   Ve shrnutí uvést kód, příznak s důvodem a novou verzi protokolu.

## Kroky

1. **Před (10 minut).** Nová větev a nová session:

   ```sh
   git switch -c pred
   codex --sandbox workspace-write
   ```

   Vložte úlohu. Zapište si: které příkazy agent spustil, jestli pustil preset `tidy`, jaký příznak LIS
   zvolil, co tvrdí v závěru. Pak kontrolní skript a výsledek si nechte:

   ```sh
   python ../sample-service/cviceni/13-naucte-agenta/zkontroluj.py .
   git add -A
   git commit -m "LAB-318 bez kontextu"
   git switch main
   ```

2. **Rozbor (5 minut).** U každého řádku `CHYBA` najděte větu z „Co ví tým“, která chybí.
   Rozhodněte pro každou ze šesti vět: `AGENTS.md`, skill, hook, nebo víc z nich.
   Pomůže otázka: platí to vždy (AGENTS.md), je to postup pro jeden druh úlohy (skill), nebo to
   nesmí záviset na tom, jestli agent poslechne (hook)?

3. **AGENTS.md (10 minut)** na nové větvi `naucene` z `main`, v kořeni repa. Anglicky, krátce,
   každé pravidlo ověřitelné příkazem nebo v diffu. Postup pro nový kód sem nepište celý, jen odkaz
   na skill.

   ```sh
   git switch -c naucene
   ```

4. **Skill (10 minut)** v `.agents/skills/add-instrument-error/SKILL.md`. Soubor začíná hlavičkou:

   ```markdown
   ---
   name: add-instrument-error
   description: <kdy se má skill použít; podle tohoto textu ho agent vybírá>
   ---
   ```

   Pod ní kroky postupu z bodu 6 a tabulka příznaků LIS z bodu 4. Popis (`description`) pište tak,
   aby na něj sedl tiket LAB-318, ale ne každá úloha v repu.

5. **Hook (15 minut).** Zákaz z bodu 5 vynutí hook `PreToolUse`. Dva soubory:

   `.codex/config.toml`:

   ```toml
   [[hooks.PreToolUse]]
   matcher = "^(apply_patch|Bash)$"

   [[hooks.PreToolUse.hooks]]
   type = "command"
   command = "python .codex/hooks/protect_recorded.py"
   timeout = 20
   ```

   `.codex/hooks/protect_recorded.py` napište sami (nebo s agentem v jiné session). Codex mu na
   standardní vstup pošle JSON s plánovanou akcí: `tool_name` (`apply_patch` pro úpravu souboru,
   `Bash` pro příkaz) a `tool_input.command` (text patche s řádky `*** Update File: <cesta>`, nebo
   příkaz). Skript skončí `exit 0` (pustit), nebo vypíše důvod na stderr a skončí `exit 2`
   (zablokovat). Důvod pište pro agenta: co má udělat místo toho. Pozor na cesty s `\` na Windows.

   Vyzkoušejte ho bez modelu (PowerShell):

   ```powershell
   '{"tool_name":"apply_patch","tool_input":{"command":"*** Update File: tests/data/recorded/x.frames"}}' | python .codex/hooks/protect_recorded.py; $LASTEXITCODE
   '{"tool_name":"apply_patch","tool_input":{"command":"*** Update File: tests/frame_test.cpp"}}' | python .codex/hooks/protect_recorded.py; $LASTEXITCODE
   ```

   První musí vrátit 2 a důvod, druhý 0. Pak commit všech tří nástrojů:

   ```sh
   git add AGENTS.md .agents .codex
   git commit -m "Teach the agent: AGENTS.md, skill, hook"
   ```

   Nová session `codex --sandbox workspace-write`, Codex nahlásí hook ke schválení: zvolte
   **Review hooks** (nebo příkaz `/hooks`) a hook schvalte. Zkouška naostro:

   > Přidej řádek `ERR|AN-07|14|S-1042` na konec `tests/data/recorded/an07-2026-10-14.frames`.

   V terminálu musí být „Blocked by hook“ a váš důvod. Session zavřete a vraťte provokaci:
   `git restore .`

6. **Po (10 minut).** Nová session, stejná úloha. Zapište si totéž co v kroku 1 a spusťte kontrolní
   skript. Ověřte i, že skill opravdu použil (ve výpisu session je jeho jméno, nebo se zeptejte
   příkazem `/skills`).

7. **Porovnání (5 minut).** `git diff pred naucene -- src include tests` a výsledky kontroly vedle
   sebe. Ke každé změně řekněte, který nástroj ji způsobil. Vzorové řešení všech tří nástrojů je ve
   složce `vzor/` vedle tohoto zadání; otevřete ho až teď.

## Hotovo, když

- Kontrolní skript po kroku 6 hlásí „Splněno 7 z 7“.
- Máte záznam „před“ a „po“: příkazy, tidy, příznak LIS, verze protokolu, závěr agenta.
- Hook prokazatelně zablokoval úpravu nahrávky a agent dostal váš důvod.
- Umíte říct, která věta z „Co ví tým“ skončila kde a proč.

## Zkrácená varianta (35 minut)

Krok 1, pak jen AGENTS.md (krok 3) a krok 6. Skill a hook vezměte hotové ze složky `vzor/` vedle
tohoto zadání (`vzor/SKILL.md` do `.agents/skills/add-instrument-error/`, `vzor/config.toml`
a `vzor/protect_recorded.py` do `.codex/` a `.codex/hooks/`), ale přečtěte je.

## Když něco nejde

- **Codex hook nevidí:** projekt musí být důvěryhodný (při prvním spuštění v repu odpovězte, že
  složce věříte), hook schválený v `/hooks` a Codex spuštěný z kořene repa (cesta ke skriptu je
  relativní). Každá změna skriptu nebo konfigurace = schválit znovu.
- **Hook se neschválí, protože firma povoluje jen spravované hooky** (`allow_managed_hooks_only`):
  zkoušku bez modelu udělejte stejně, naostro ji ukáže lektor.
- **`python` neexistuje:** v `command` použijte `py`.
- **Agent chce zapsat do `.agents/` nebo `.codex/`:** v sandboxu `workspace-write` jsou pro agenta
  jen pro čtení. Soubory pište sami, nebo zápis schvalte.
