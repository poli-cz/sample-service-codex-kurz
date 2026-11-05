# Kontrolní seznam: AGENTS.md pro C++ projekt

Odškrtněte, co váš AGENTS.md obsahuje. U každé položky platí: dá se to ověřit příkazem nebo v diffu?
Co agent nemůže ověřit, je šum.

## Build a testy

- [ ] Build a všechny testy jedním příkazem (`python scripts/check_all.py debug`).
- [ ] Kdy navíc `tidy` a kdy `asan` (konkrétní podmínka, ne „podle potřeby“).
- [ ] Rychlá smyčka pro jednu knihovnu: `cmake --build --preset debug --target <lib>_tests`
      a `ctest --preset debug -R <Suite>`.
- [ ] Zákaz vlastních adresářů pro build (všechno do `build/<preset>`).

## Závislosti

- [ ] Odkud jsou knihovny (systém, vcpkg, předpřipravený prefix, jinak kopie v `third_party/`) a že se
      `third_party/` neupravuje.
- [ ] Nová závislost jen po schválení člověkem; co dělat, když build selže na závislosti (zastavit a ohlásit).

## Struktura a konvence

- [ ] Směr závislostí mezi knihovnami (`apps -> labsched -> labio -> labcore`).
- [ ] Pojmenování a ošetření chyb tak, jak je v kódu (výjimky vs. `std::optional` a výsledkové typy).
- [ ] Zákaz `NOLINT` a `-Wno-*` jako zkratky.
- [ ] Kde jsou testy a jak se jmenují (`TEST(Suite, BehaviourInWords)`).

## Znalosti týmu, které v kódu nejsou

- [ ] Doménová pravidla (u kontroly kvality: ostré meze, SD ≤ 0 je chyba, `z_score` jen pro zobrazení).
- [ ] Co se nesmí změnit a proč (struktura `LEGACY_RESULT`, kalibrátory v `config/service.json`).
- [ ] Na co nesahat bez dotazu (`CMakePresets.json`, `cmake/`, soubory `.clang-tidy`).

## Hotovo a předání

- [ ] Definice hotovo jako číslovaný seznam příkazů.
- [ ] Co má obsahovat popis merge requestu.
- [ ] Co má agent napsat na konci: spuštěné příkazy s výsledkem a co neověřil.

## Forma

- [ ] Krátké (desítky řádků, ne stovky); Codex načítá AGENTS.md celé při každém startu (limit 32 KiB pro všechny soubory dohromady).
- [ ] Pravidla jen pro jeden modul ve vnořeném `AGENTS.md` v jeho adresáři (načte se jen, když Codex běží v tom adresáři nebo pod ním).
- [ ] Uložené v Gitu a revidované jako kód; má vlastníka.
