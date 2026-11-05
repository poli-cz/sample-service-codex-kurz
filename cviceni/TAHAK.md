# Codex v C++ týmu: tahák

Kurz OpenAI Codex v C++ týmu (AICODX). Stav dokumentace Codexu ověřen 18. 9. 2026. Codex se
mění rychle. Při rozporu platí odkazy na konci.

## 1. Struktura dobrého zadání

| část | otázka | příklad |
|---|---|---|
| Cíl | Co má být jinak, až agent skončí? | „Přidej `lab::stats::percentile` do `labcore`.“ |
| Očekávaný výstup | Jak výsledek vypadá? | signatura funkce, formát výstupu, JSON řádek |
| Chování a hraniční případy | Co u prázdného vstupu, chyby, limitu? | „Prázdný vstup vrací prázdné `std::optional`.“ |
| Rozsah | Které soubory smí měnit, co nechat být? | „Jen tyhle dva soubory a test. `Summary` neměň.“ |
| Technická omezení | Standard, kompilátor, povolené knihovny, styl | „C++20, jen standardní knihovna, žádné nové závislosti.“ |
| Postup | V jakém pořadí? | „Nejdřív testy, ukaž, že padají, pak implementace.“ |
| Hotovo, když | Spustitelné kritérium | „`python scripts/check_all.py debug tidy` projde (tidy bez nových nálezů).“ |

Praktická pravidla:

- U větší úlohy si nejdřív nechte ukázat **plán** a teprve pak pusťte implementaci.
- Kritérium hotovo musí jít **spustit**; pak ho agent ověří sám.
- Když agent odbočí, zastavte ho a upřesněte zadání, místo abyste opravovali výsledek po kouscích.
- Na konci chtějte shrnutí: co změnil, co ověřil, co neověřil.

## 2. Kostra AGENTS.md pro C++ projekt

Soubor v kořeni repa, verzovaný v Gitu. Krátký a spustitelný; co agent nemůže ověřit, je šum.
Specifická pravidla modulu patří do `AGENTS.md` v adresáři modulu.

```markdown
# AGENTS.md

<One paragraph: what the service does, main libraries and how they depend on each other.>

## Build and test
- Configure, build and test in one step: `python scripts/check_all.py debug`
- Presets: `debug`, `release`, `asan` (AddressSanitizer + UBSan), `tidy` (clang-tidy)
- Build one target: `cmake --build --preset debug --target <target>`
- Run selected tests: `ctest --preset debug -R <regex>`
- Never create other build directories; everything goes to `build/<preset>`.

## Dependencies
- Third-party libraries come from <system packages | vcpkg preset | /opt/sdk prefix | copies in third_party/ (do not edit)>.
- Headers of the shared SDK are in `<path>`; read them, do not copy them into the repo.
- Do not add a dependency without asking.

## Conventions
- C++20, <naming convention>, `#pragma once`, namespaces `<ns>::...`.
- Errors: <exceptions | expected | error codes> as in the surrounding code.
- Every behaviour change comes with a GoogleTest test in `tests/<module>/`.

## Domain rules (not visible in the code)
- <What the team knows and the code does not show.>

## Do not touch without asking
- `<generated code, vendored code, legacy module with its own AGENTS.md>`
- CI configuration and release scripts, unless the task says so.

## Done means
- `python scripts/check_all.py debug tidy` passes (tidy reports nothing new in touched files).
- Changes to memory ownership, containers or lifetimes: `python scripts/check_all.py asan` passes too.
- README / docs updated when the command line or protocol changes.
```

Vrstvení a limity: viz oddíl 6 „Rychlá fakta“.

## 3. Konfigurace MCP

Konfigurace je v `~/.codex/config.toml` (sdílí ji CLI i rozšíření ve VS Code), pro projekt v
`.codex/config.toml` v repu (načte se jen v důvěryhodném projektu).

Lokální server přes stdio (příklad pro server z cvičení bloku 5; celý blok s cestami pro váš počítač
vypíše v repu serveru `python scripts/dev.py codex-config`):

```toml
[mcp_servers.lab]
command = "/cesta/k/mcp-lab-server/.venv/…/python"     # Python z .venv, cestu vypíše codex-config
args = ["-m", "mcp_lab_server"]
cwd = "/cesta/k/mcp-lab-server"
enabled_tools = ["list_instruments", "get_results"]   # jen tyhle nástroje uvidí agent
startup_timeout_sec = 20                               # výchozí 10
tool_timeout_sec = 30                                  # výchozí 60
default_tools_approval_mode = "prompt"                 # ptát se před voláním nástroje
env = { LAB_DATA_DIR = "/cesta/k/datum" }              # inline: Codex si na konec souboru připisuje vlastní tabulky
```

Názvy nástrojů v `enabled_tools` jsou ilustrační; skutečné vypíše `/mcp` nebo dokumentace serveru.

Totéž z příkazové řádky a kontrola:

```sh
codex mcp add lab --env LAB_DATA_DIR=/cesta/k/datum -- /cesta/k/python -m mcp_lab_server
# cwd tenhle příkaz nezapíše: doplňte ho do config.toml ručně
codex mcp list            # přehled serverů
codex mcp get lab         # podrobnosti jednoho serveru
# v TUI: /mcp (servery a nástroje), /mcp verbose (diagnostika)
```

Další klíče: `disabled_tools` (uplatní se po `enabled_tools`), `env_vars` (proměnné předané z prostředí),
`enabled`, `required`, `tools.<nástroj>.approval_mode`. Vzdálený server: `codex mcp add <jméno> --url <URL>`,
token přes `--bearer-token-env-var <PROMĚNNÁ>`.

Bezpečnost MCP:

- Token s **minimálním rozsahem** (jen čtení, jen jeden projekt), nikdy osobní token s plnými právy.
- `enabled_tools` jako výčet povolených nástrojů; zapisovací nástroje jen se schválením člověkem.
- Tajemství do proměnných prostředí nebo správce tajemství, nikdy do `config.toml` v repu.
- Výstup MCP nástroje je pro agenta vstup jako každý jiný: může obsahovat prompt injection.

## 4. Režimy oprávnění a sandbox

Dvě nezávislá nastavení: **sandbox** (co příkazy agenta technicky smí) a **schvalování** (kdy se agent ptá vás).

| sandbox (`--sandbox`, `-s`, v config `sandbox_mode`) | co smí |
|---|---|
| `read-only` | jen číst |
| `workspace-write` | číst a měnit v pracovním adresáři; `.git`, `.agents`, `.codex` jen pro čtení; síť vypnutá |
| `danger-full-access` | vše, co smíte vy. Jen v jednorázovém kontejneru nebo VM. |

| schvalování (`--ask-for-approval`, `-a`, v config `approval_policy`) | chování |
|---|---|
| `on-request` | agent se ptá, když potřebuje víc, než sandbox dovolí (zápis mimo repo, síť) |
| `never` | neptá se nikdy; co sandbox nedovolí, selže (pro neinteraktivní běhy) |

Běžné kombinace:

```sh
codex --sandbox read-only --ask-for-approval on-request        # průzkum, plán, vysvětlení
codex --sandbox workspace-write --ask-for-approval on-request  # běžná práce (předvolba Auto)
codex --add-dir ../sdk                                         # další zapisovatelný adresář, místo plného přístupu
```

V TUI: `/permissions` přepíná režim, `/status` ukáže model, schvalování a zapisovatelné adresáře.

Trvale v `~/.codex/config.toml`:

```toml
sandbox_mode    = "workspace-write"
approval_policy = "on-request"

[sandbox_workspace_write]
network_access = false   # síť jen když ji úloha opravdu potřebuje
```

Pozor na zastaralé návody: `--full-auto` už v aktuálním CLI není a `approval_policy = "untrusted"`
Codex odmítne (nenastartuje). Sandbox, schvalování a povolené MCP servery může pro celý tým vynutit správce
souborem `/etc/codex/requirements.toml`.

### Kolik autonomie pro jakou úlohu (k diskusi v bloku 6)

| úloha | doporučený režim | proč |
|---|---|---|
| Orientace v cizím kódu, vysvětlení | jen čtení | nic nemění, nepotřebuje síť |
| Oprava testu, malá změna v jednom modulu | úpravy v pracovním adresáři, schvalování mimo něj | rychlá smyčka build a test, změny jsou vidět v diffu |
| Nová funkce s testy | úpravy v pracovním adresáři, bez sítě | build a testy lokálně, závislosti už jsou nainstalované |
| Aktualizace závislosti | se schvalováním příkazů, síť jen na vyžádání | stahuje z internetu, mění build |
| Cokoli nad tiketem nebo logem zvenčí | jako výše a vždy revize diffu | text tiketu může obsahovat vloženou instrukci |
| Plný přístup bez sandboxu | jen v jednorázovém kontejneru nebo VM | agent může cokoli, co může uživatel |

## 5. Kontrolní seznam revize kódu od agenta

Než změnu pustíte dál (a než napíšete „LGTM“):

- [ ] **Rozsah:** diff mění jen to, co zadání chtělo. Žádné „drobné vylepšení“ okolí, žádné přeformátování cizích souborů.
- [ ] **API existuje:** každá volaná funkce, hlavička a CMake cíl opravdu existuje v použité verzi knihovny (C++ halucinace č. 1).
- [ ] **Testy testují požadavek**, ne implementaci. Padaly před změnou? Pokrývají hraniční případy (prázdný vstup, limity, chyby)?
- [ ] **Testy nebyly oslabené:** žádné smazané nebo zakomentované asserty, žádné `GTEST_SKIP`, žádné zvednuté tolerance bez důvodu.
- [ ] **Životnost a vlastnictví:** reference a `string_view` na dočasné objekty, iterátory po změně kontejneru, `std::move` a pak použití.
- [ ] **Nedefinované chování:** přetečení, znaménkové/neznaménkové porovnání, přístup mimo meze. `asan` preset prošel.
- [ ] **Statická analýza:** `tidy` preset bez nových nálezů; žádné nové `NOLINT` bez komentáře proč.
- [ ] **Chyby a výjimky** se ošetřují stejně jako v okolním kódu.
- [ ] **Závislosti a build:** žádná nová závislost, žádná změna presetů nebo přepínačů kompilátoru, pokud to nebylo v zadání.
- [ ] **Tajemství a data:** žádné klíče, tokeny, interní URL ani skutečná data v kódu, testech a logu.
- [ ] **Dokumentace:** README, komentáře u veřejného API a popis MR odpovídají tomu, co kód dělá.
- [ ] **Druhý pohled:** review agentem v nové session (bez kontextu autora), pak člověk.

## 6. Rychlá fakta

| co | jak |
|---|---|
| Instalace CLI | `npm install -g @openai/codex` (všude, kde je Node.js), nebo samostatný instalátor či správce balíčků podle návodu OpenAI pro vaši instalaci |
| Přihlášení | `codex login` (účet ChatGPT), bez prohlížeče `codex login --device-auth`; kontrola `codex login status` |
| Verze | `codex --version` |
| Rozšíření VS Code | „Codex – OpenAI's coding agent“, ID `openai.chatgpt` |
| Konfigurace | `~/.codex/config.toml`, projekt `.codex/config.toml`, jednorázově `-c klíč=hodnota` |
| AGENTS.md | globální `~/.codex/AGENTS.md`, pak od kořene repa po aktuální adresář, v každém nejvýš jeden soubor (`AGENTS.override.md` má přednost před `AGENTS.md`); bližší má poslední slovo; limit 32 KiB pro součet (`project_doc_max_bytes`); načítá se při startu, po změně spustit znovu; `/init` vytvoří kostru |
| Skilly | `.agents/skills/` v repu, `~/.agents/skills/` osobní |
| Sub-agenti | vestavění `default`, `worker`, `explorer`; vlastní role jako TOML v `.codex/agents/` (pole `name`, `description`, `developer_instructions`) |
| Revize | v TUI `/review`, z příkazové řádky `codex review --uncommitted` nebo `--base main`; pravidla pro revize v AGENTS.md pod `## Code Review Rules` |
| Pokračování | `codex resume` (výběr), `codex resume --last` |
| Paralelní práce | `git worktree add ../repo-feature -b feature/x`, nebo `codex --worktree` |
| Neinteraktivně | `codex exec "<zadání>"` |
| Lokální modely | `codex --oss` (Ollama, LM Studio), vlastní poskytovatel `[model_providers.<id>]` |
| Firemní TLS proxy | `CODEX_CA_CERTIFICATE=/cesta/bundle.pem` |
| Data a trénink | plány Business a Enterprise netrénují na vašich datech; Compliance API a auditní logy jen Enterprise a Edu |

## 7. Odkazy na dokumentaci

Dokumentace Codexu (dříve developers.openai.com/codex, dnes přesměrováno):

- Přehled a CLI: https://learn.chatgpt.com/docs/codex/cli
- Rozšíření do IDE: https://learn.chatgpt.com/docs/codex/ide
- Přihlášení: https://learn.chatgpt.com/docs/auth
- Příkazy a přepínače: https://learn.chatgpt.com/docs/developer-commands?surface=cli
- Sandbox a schvalování: https://learn.chatgpt.com/docs/agent-approvals-security
- config.toml: https://learn.chatgpt.com/docs/config-file/config-basic a https://learn.chatgpt.com/docs/config-file/config-reference
- AGENTS.md: https://learn.chatgpt.com/docs/agent-configuration/agents-md
- Sub-agenti: https://learn.chatgpt.com/docs/agent-configuration/subagents
- Skilly: https://learn.chatgpt.com/docs/build-skills
- MCP: https://learn.chatgpt.com/docs/extend/mcp
- Hooky: https://learn.chatgpt.com/docs/hooks
- Spravovaná konfigurace: https://learn.chatgpt.com/docs/enterprise/managed-configuration
- GitLab: https://learn.chatgpt.com/docs/third-party/gitlab
- Plány a ceny: https://learn.chatgpt.com/docs/pricing
- Zdrojový kód CLI: https://github.com/openai/codex

Ostatní:

- Model Context Protocol: https://modelcontextprotocol.io
- CMake presety: https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html
- GoogleTest: https://google.github.io/googletest/
- clang-tidy: https://clang.llvm.org/extra/clang-tidy/
- AddressSanitizer: https://clang.llvm.org/docs/AddressSanitizer.html
