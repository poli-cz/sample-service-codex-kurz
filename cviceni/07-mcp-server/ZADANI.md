# C7 – Vlastní MCP server nad daty přístrojů

**Blok:** 5b, MCP: vlastní server a jeho zabezpečení · **Čas:** 50 minut

## Cíl

Dokončíte malý MCP server v Pythonu (stdio) se smyšlenými daty laboratorních přístrojů. Zapojíte
ho do Codexu jen s nástroji pro čtení. Pak přidáte zapisovací nástroj a ochráníte ho dvěma
nezávislými pojistkami: schválením v klientovi a zámkem na serveru.

## Výchozí stav

Složka `mcp-lab-server` v repozitáři kurzu, výchozí stav je na `main` (z cvičení C0 máte `.venv`
s balíčky).
Server má tři nástroje: `list_instruments` (jen čtení, hotový), `get_results` (jen čtení, TODO)
a `set_instrument_status` (zápis, TODO). Logika je v `mcp_lab_server/store.py`, MCP vrstva
v `mcp_lab_server/server.py`, data v `mcp_lab_server/data/`.

Potřebujete Python 3.10 nebo novější (oficiální SDK `mcp` starší Python nepodporuje). Python
spouštějte jako `python` nebo `python3` podle vaší instalace. Pomocný skript `scripts/dev.py` funguje
stejně v každém terminálu a Python v `.venv` najde sám. Má tyto příkazy: `setup` (založí `.venv`), `test` (pytest),
`smoke` (surové JSON-RPC), `run` (spustí server) a `codex-config` (blok konfigurace pro Codex).

## Vlastní server od nuly (15 minut, lektor ukazuje v bloku 5)

Než budete dokončovat připravený server, postavte si nejmenší vlastní: server nad tikety uloženými
jako soubory Markdown. Hotové soubory jsou ve složce `od-nuly/` vedle tohoto zadání
(`server.py` a `tickets/`). Postup v PowerShellu, mimo repo kurzu:

1. **Prostředí a balíček.**

   ```powershell
   mkdir tickets-mcp; cd tickets-mcp
   python -m venv .venv
   .venv\Scripts\python -m pip install "mcp==2.3.0"
   ```

   Do složky `tickets\` zkopírujte `LAB-401.md` a `LAB-402.md` z `od-nuly/tickets/`.

2. **server.py.** Zkopírujte `od-nuly/server.py`, nebo ho napište podle slidu „Krok 2: celý server je
   25 řádků“. Každý nástroj je funkce s dekorátorem `@server.tool(annotations=READ_ONLY)`; docstring je
   popis, který čte model, typy parametrů jsou schéma, které model vyplní. Bez anotace jen pro čtení
   se vás Codex zeptá na každé volání.

3. **Ověření bez agenta.**

   ```powershell
   .venv\Scripts\python
   >>> import server
   >>> server.list_tickets()
   ['LAB-401', 'LAB-402']
   ```

4. **Registrace v Codexu** (ze složky `tickets-mcp`, cesty jsou absolutní):

   ```powershell
   codex mcp add tickets --env "TICKETS_DIR=$PWD\tickets" -- "$PWD\.venv\Scripts\python.exe" "$PWD\server.py"
   codex mcp list
   ```

5. **Dotaz.** Nová session v jiné složce, `/mcp` ukáže server `tickets` se dvěma nástroji. Zadání:

   > Použij MCP server `tickets`. Vypiš všechny tikety a k LAB-401 napiš jednou větou, co je špatně.

   Po cvičení server odeberte: `codex mcp remove tickets`.

Místo složky s tikety si pak můžete dosadit vlastní zdroj (API Jiry, GitLabu, výsledky buildu). Token
patří jen do proměnné prostředí (`--env`), nikdy do kódu.

## Kroky

1. **Prostředí a testy (5 minut).**

   ```sh
   cd mcp-lab-server                      # z kořene sample-service, kde jste skončili v C8
   git switch -c c7-mcp main
   python scripts/dev.py test             # 16 failed, 4 passed: to je správně
   ```

   Kdo nemá `.venv` z cvičení C0, spustí před testy `python scripts/dev.py setup`.

2. **Dopsat nástroje (15 minut).** V adresáři `mcp-lab-server` spusťte `codex --sandbox workspace-write` a zadejte:

   > Doplň v `mcp_lab_server/store.py` metody `get_results` a `set_instrument_status` podle komentářů TODO a podle testů v `tests/`. Testy neměň. Nepřidávej závislosti. Na konci spusť `python scripts/dev.py test`.

   Pak bez Codexu vyzkoušejte server surovým JSON-RPC přes stdio:

   ```sh
   python scripts/dev.py smoke
   ```

   Vidíte `initialize`, `tools/list` (tři nástroje, u dvou `readOnlyHint`) a odmítnutý vstup `../etc`?

3. **Zapojit do Codexu (10 minut).** Blok konfigurace s cestami pro váš počítač vypíše:

   ```sh
   python scripts/dev.py codex-config
   ```

   Vypsaný blok vložte do konfiguračního souboru Codexu (`~/.codex/config.toml`; přesnou cestu
   ukazuje první řádek výpisu) a soubor uložte. Stejný blok se zástupnými cestami je
   v `codex/config.example.toml`. Pak:

   ```sh
   codex mcp list
   codex mcp get lab        # musí ukázat cwd a enabled_tools: list_instruments, get_results
   ```

   Celá konfigurace je jedna tabulka `[mcp_servers.lab]`: `env` je v ní jako inline tabulka
   (`env = { … }`). Samostatnou podtabulku `[mcp_servers.lab.env]` nepoužívejte. Codex si do
   `config.toml` sám připisuje vlastní tabulky (například `[tui]`), i hned za blok `[mcp_servers.lab]`.
   Klíč, který byste později připsali pod tabulku nebo na konec souboru, by tak patřil do cizí tabulky
   a Codex by nenastartoval.

4. **Dotaz přes agenta (10 minut).** V **jiném** adresáři (například `sample-service`) spusťte novou session,
   napište `/mcp` (u serveru `lab` vidíte 2 nástroje, ne 3? jména ukáže `/mcp verbose`) a zadejte:

   > Použij jen MCP server `lab`. Vypiš přístroje, které jsou online, a pro každý z nich vezmi kontrolní výsledky GLU. Které hodnoty leží za limitem 2 SD cílové hodnoty šarže? Uveď přístroj, čas, hodnotu a limit. Nic neměň.

   Ověřte výsledek ručně u jednoho přístroje proti `mcp_lab_server/data/qc_results.json`. Pak agenta
   požádejte, aby přístroj s nálezem přepnul do údržby. Co udělá?

5. **Zápis se dvěma pojistkami (10 minut).** V konfiguračním souboru Codexu z kroku 3:

   ```toml
   enabled_tools = ["list_instruments", "get_results", "set_instrument_status"]
   tools.set_instrument_status.approval_mode = "prompt"
   ```

   (Oba řádky patří do tabulky `[mcp_servers.lab]` hned pod `env`, `enabled_tools` přepíše ten
   původní. Pod tabulku je nepište: Codex za ni mohl vložit vlastní tabulky.) Nová session, zadání „Přepni AN-02 do údržby, důvod: QC selhalo na GLU“. Codex zobrazí schvalovací
   dialog s parametry volání. Nejdřív **bez** `LAB_ALLOW_WRITES`: co se stane po vašem schválení? Pak
   do inline tabulky `env` přidejte `LAB_ALLOW_WRITES = "1"`
   (`env = { LAB_STATE_DIR = "…", LAB_ALLOW_WRITES = "1" }`). Spusťte novou session a zkuste to znovu. Změna je
   v `var/status_changes.jsonl`.

## Hotovo, když

- `pytest` je zelený (20 passed).
- `/mcp` ukazuje jen povolené nástroje; agent odpověděl na dotaz a volal jen nástroje pro čtení.
- Zápis bez vašeho schválení neprojde. Bez `LAB_ALLOW_WRITES` ho odmítne i server. S povoleným
  zápisem na serveru (`LAB_ALLOW_WRITES = "1"`) a po schválení projde a je v logu změn.

## Zkrácená varianta (20 minut)

Přeskočte krok 2: hotová verze serveru je ve větvi `cviceni/07-hotovo`. Ve složce `mcp-lab-server`:

```sh
git switch -c c7-hotovo origin/cviceni/07-hotovo
python scripts/dev.py test             # 20 passed
```

Kdo už v kroku 2 změnil `store.py`, ten přepnutí odmítne („Please commit your changes or stash
them“): před přepnutím spusťte `git stash`.

Pak kroky 3 (10 minut) a 4 (5) a z kroku 5 jen zápis bez `LAB_ALLOW_WRITES` (5).

## Po cvičení

Server `lab` zůstává v konfiguraci Codexu (`~/.codex/config.toml`) i se zapnutým zápisem. Tabulku
`[mcp_servers.lab]` i s řádky pod ní smažte, nebo z ní aspoň odstraňte `set_instrument_status`
a `LAB_ALLOW_WRITES`.

## K zamyšlení

- Server je pro agenta zdroj dat jako každý jiný: co kdyby pole `location` obsahovalo pokyn pro AI?
- Skutečný server pro GitLab, Jiru nebo LIMS potřebuje token. Jaký rozsah mu dáte, kde bude uložený
  a kdo ho vydá?
