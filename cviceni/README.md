# Cvičení

Každé cvičení má složku se zadáním `ZADANI.md`. Zadání stačí samo, cvičení se dají projít i po
kurzu bez slidů. Každé cvičení začíná ve vlastní větvi, takže můžete začít kterýmkoli.

| cvičení | minut | výchozí větev |
|---|---|---|
| [C0 Kontrola prostředí](00-priprava/ZADANI.md) | 10 | `main` |
| [C1 První úloha: orientace jen pro čtení](01-prvni-uloha/ZADANI.md) | 15 | `main` |
| [C2 Slabé a dobré zadání](02-zadani/ZADANI.md) | 40 | `main` |
| [C3 AGENTS.md pro vzorový projekt, C3b skill](03-agents-md/ZADANI.md) | 50 + 20 | `main`, pak `reference/agents-md` |
| [C4 LAB-201: TDD a implementace](04-tdd-westgard/ZADANI.md) | 80 | `cviceni/04-start` |
| [C5 LAB-214: ztracený vzorek (samostudium)](05-ztraceny-vzorek/ZADANI.md) | 55 | `cviceni/05-start` |
| [C6 Refaktoring legacy parseru (volitelně)](06-legacy-refaktoring/ZADANI.md) | 40 | `main` |
| [C7 Vlastní MCP server](07-mcp-server/ZADANI.md) | 50 | `main`, složka `mcp-lab-server` |
| [C8 Sub-agenti a dvě úlohy naráz](08-subagenti-worktrees/ZADANI.md) | 40 | `main` |
| [C9 Prompt injection a matice autonomie](09-prompt-injection/ZADANI.md) | 20 | `cviceni/09-start` |
| [C10 Závěrečné cvičení](10-zaverecne/ZADANI.md) | 55 | `cviceni/10-start`, `cviceni/10c-start` |
| [C11 Plán zavedení pro váš tým](11-plan-zavedeni/ZADANI.md) | 15 | bez repozitáře |
| [Úlohy navíc ze slidů](12-navic/ZADANI.md) | podle úlohy | podle úlohy |
| [C13 Naučte agenta váš projekt: AGENTS.md, skill a hook](13-naucte-agenta/ZADANI.md) (2. den ráno) | 65 | samostatné repo [analyzer-link](https://github.com/poli-cz/analyzer-link) |

## Větve

| větev | obsah |
|---|---|
| `main` | výchozí stav projektu, bez AGENTS.md |
| `reference/agents-md` | referenční AGENTS.md pro krok 5 cvičení C3 |
| `cviceni/04-start` | `main` a tiket LAB-201 |
| `cviceni/05-start` | `main`, tiket LAB-214 a výstup služby z té noci |
| `cviceni/09-start` | `main` a tiket LAB-233 |
| `cviceni/10-start` | `main` a tikety LAB-240 a LAB-241 (C10, varianty A a B) |
| `cviceni/10c-start` | hotové LAB-201 a tiket LAB-242 (C10, varianta C); obsahuje řešení C4, proto až po něm |
| `cviceni/07-hotovo` | hotový MCP server pro zkrácenou variantu C7 |

Struktura zadání, kostra AGENTS.md, konfigurace MCP, režimy oprávnění a kontrolní seznam revize
najdete v [taháku kurzu](TAHAK.md).

Vzorová řešení v repozitáři nejsou, kromě složky `13-naucte-agenta/vzor/` pro poslední krok C13. Mezi pokusy vrátíte pracovní kopii do výchozího stavu příkazy
`git restore .` a `git clean -fd`.
