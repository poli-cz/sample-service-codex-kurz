# C0 – Kontrola prostředí

**Blok:** před kurzem, kontrola na začátku bloku 1 · **Čas:** 10 minut (instalace musí proběhnout předem)

## Cíl

Na začátku kurzu máte funkční Codex, Git, CMake, překladač C++20 a postavený vzorový projekt
`sample-service` se všemi testy zelenými. Během dne se pak řeší agent, ne instalace.

## Výchozí stav

- Nainstalované nástroje podle návodu `NAVOD.pdf` z balíčku kontroly prostředí (Codex CLI, VS Code s rozšířením Codex,
  Git, Python 3.10+ a k tomu buď Visual Studio 2022+ se sadou Desktop development with C++, nebo
  CMake 3.22+, Ninja, GCC 12+ nebo Clang 16+ a clang-tidy). Knihovny GoogleTest, fmt a nlohmann_json
  instalovat nemusíte: co CMake nenajde, postaví z kopií ve vzorovém repu.
- Balíček kontroly prostředí (`over_prostredi.py`, `sample-service.bundle` a návod), který jste
  dostali před kurzem; kdo ho nemá, dostane ho od lektora.
- Repozitář kurzu https://github.com/poli-cz/sample-service-codex-kurz: vzorový projekt, MCP server pro C7, zadání
  cvičení a slidy. Kdo se na GitHub nedostane, dostane od lektora soubor `sample-service-codex-kurz.bundle`
  se stejným obsahem.

Python spouštějte jako `python` nebo `python3` podle vaší instalace (platí pro celý kurz). Všechny
příkazy jsou stejné v každém terminálu; zadávejte je po jednom řádku. Na Windows pracujte (a Codex
spouštějte) v Developer PowerShell for VS, kde jsou překladač, CMake a Ninja; kontrolní skript
v kroku 1 si prostředí najde sám. Repozitáře klonujte do běžné složky (například Dokumenty nebo
`C:\dev`), ne pod `%TEMP%` (`AppData\Local\Temp`): sandbox Codexu tam příkazům nedovolí číst.
Rozšíření Codex ve VS Code dědí prostředí od VS Code. Kdo s ním chce pracovat, ať VS Code spustí
z Developer PowerShell příkazem `code .` ve složce repozitáře.

## Kroky

1. Kontrola nástrojů. Skript nic neinstaluje a mění jen dočasnou složku, kterou na konci smaže.
   Ve složce balíčku kontroly prostředí (vedle skriptu leží `sample-service.bundle`):

   ```sh
   python over_prostredi.py --bez-codexu
   ```

   Na konci vypíše „Výsledek: N v pořádku, M upozornění, 0 chyb.“ a „PŘIPRAVENO NA KURZ“. Celá kontrola
   trvá několik minut; kdo ji se stejným výsledkem spustil před kurzem, krok přeskočí. Dotaz Codexu
   (`--bez-codexu` ho vynechá) uděláte v kroku 5.

2. Naklonujte repozitář kurzu do složky `sample-service` a postavte vzorový projekt jedním příkazem:

   ```sh
   git clone https://github.com/poli-cz/sample-service-codex-kurz sample-service
   cd sample-service
   python scripts/check_all.py debug    # konfigurace, build a všechny testy
   ```

   Bez přístupu na GitHub klonujte ze souboru od lektora: `git clone sample-service-codex-kurz.bundle
   sample-service`. Větve i příkazy v dalších cvičeních jsou pak stejné.

3. Ověřte, že funguje i preset se sanitizery a statická analýza:

   ```sh
   python scripts/check_all.py asan tidy
   ```

4. Připravte prostředí pro MCP server (cvičení C7, 2. den), dokud je čas a síť:

   ```sh
   cd mcp-lab-server
   python scripts/dev.py setup          # založí .venv a nainstaluje balíčky
   python scripts/dev.py test           # očekáváno: 16 failed, 4 passed (kostra, doplníte v C7)
   ```

   Skript `scripts/dev.py` najde Python v `.venv` sám, prostředí není potřeba aktivovat.

5. Jedno zadání v Codexu ověří, že funguje licence i síť. Zpátky v kořeni `sample-service`:

   ```sh
   cd ..
   codex --sandbox read-only "Vypiš, jakými příkazy se tento projekt staví a testuje. Nic neměň."
   ```

## Hotovo, když

- `over_prostredi.py` hlásí 0 chyb (PŘIPRAVENO NA KURZ).
- `python scripts/check_all.py debug` vypíše `100% tests passed, 0 tests failed out of 49`
  (CMake 4 píše zkráceně `100% tests passed out of 49`) a na konci `all presets passed: debug`.
- `asan` a `tidy` projdou.
- V `mcp-lab-server` je `.venv` s nainstalovanými balíčky a pytest doběhne (s očekávanými chybami kostry).
- Codex odpoví a v repozitáři nic nezmění (`git status` je čistý). Když se Codex při prvním spuštění
  zeptá, jestli složce důvěřujete, potvrďte to.

## Zkrácená varianta

Jen kroky 1, 2 a 5. Krok 4 nejpozději v polední přestávce 2. dne.

## Když něco nejde

| příznak | řešení |
|---|---|
| chyba u knihovny fmt, GoogleTest nebo nlohmann_json při konfiguraci nebo buildu | knihovny se berou z instalace, jinak z kopií ve vzorovém repu (`third_party/`); nic se nestahuje ani neinstaluje. Pošlete lektorovi celý výpis příkazu |
| `CMake 3.22 or higher is required` nebo `Could not read presets` | CMake je starší než 3.22; novější z cmake.org nebo `pip install cmake` |
| `over_prostredi.py` hlásí „clang-tidy: není v PATH“ (info, ne chyba) | rozhoduje preset `tidy`: když projde, je vše v pořádku. Když clang-tidy nenajde, nainstalujte ho (balík LLVM) a složku s ním přidejte do `PATH` |
| `over_prostredi.py` nebo `scripts/dev.py` hlásí Python 3.9 nebo starší, nebo pip hlásí `No matching distribution found for mcp==2.3.0` | nainstalujte Python 3.10 nebo novější a spusťte jím `python scripts/dev.py setup --recreate` (smaže `.venv` a založí ho znovu); verzi ukáže `python --version` |
| `codex login status` vrací chybu | `codex login`; bez prohlížeče `codex login --device-auth` (musí to povolit správce workspace) |
| pip nestáhne balíčky (proxy) | nastavte proxy pro pip, nebo požádejte lektora o předem stažené balíčky |
