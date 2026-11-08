# Hodnoticí mřížka: výsledek agenta proti zadání

Body: 0 = ne, 1 = částečně, 2 = ano. Do poznámky napište, co konkrétně jste viděli (soubor, test, příkaz).

| # | kritérium | slabé | dobré | poznámka |
|---|---|---|---|---|
| 1 | **Rozhraní:** hlavička, jméno funkce, typy a jmenný prostor odpovídají tiketu | | | |
| 2 | **Hranice:** hodnota přesně na limitu 1, 2 a 3 SD se chová podle zadání | | | |
| 3 | **Hlášení:** vrací pravidlo i index; `1-2s` se hlásí i vedle ostatních; hlásí se každé okno | | | |
| 4 | **Neplatný vstup:** SD 0 nebo záporná vyhodí `std::invalid_argument` | | | |
| 5 | **Testy existují** a testují požadavek (konkrétní očekávané hodnoty), ne jen to, co kód zrovna dělá | | | |
| 6 | **Testy napřed:** agent ukázal, že testy před implementací padají | | | |
| 7 | **Ověření:** agent sám spustil `debug` i `tidy` a obojí prošlo | | | |
| 8 | **Rozsah:** změnil jen to, co povoluje tiket (žádná dokumentace ani soubory navíc) | | | |
| 9 | **Shrnutí:** uvádí spuštěné příkazy s výsledkem a říká, co neověřil | | | |
| | **Součet** (max. 18) | | | |

**Druhý názor:** po každém pokusu, ještě před čistým startem, pusťte v kořeni repa druhého agenta. Pracuje v nové session a jen čte:

```
codex exec --sandbox read-only "Ohodnoť změny v pracovní kopii podle cviceni/02-zadani/hodnotici-mrizka.md. Požadavek je tiket LAB-201 v cviceni/02-zadani/ZADANI.md. Projdi git status, git diff i nové soubory. Ke každému řádku dej 0, 1 nebo 2 a jeden důkaz (soubor a řádek). Řádky 6, 7 a 9 hodnotí průběh session, který nevidíš, proto u nich napiš nehodnoceno. Nic neměň."
```

Jeho body zapište do poznámky vedle svých. Kde se lišíte, otevřete soubor, který uvedl jako důkaz, a rozhodněte vy.

**Tichá rozhodnutí agenta** (u slabého zadání vypište všechna, která v zadání nebyla):

1.
2.
3.
4.
5.

**Otázka na konec:** Které z tichých rozhodnutí by vás při revizi kódu stálo nejvíc času?
