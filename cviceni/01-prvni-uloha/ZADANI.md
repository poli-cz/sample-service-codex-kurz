# C1 – První úloha: orientace jen pro čtení

**Blok:** 1, Úvod: Codex a agentní přístup · **Čas:** 15 minut

## Cíl

Vyzkoušíte si, jak agent prochází neznámý repozitář: jaký si udělá plán, jaké nástroje volá
a kde odhaduje místo toho, aby četl. Výsledek porovnáte s tím, co o projektu říká README.

## Výchozí stav

Repozitář `sample-service`, větev `main`, postavený z cvičení C0. Codex v režimu **jen pro čtení**.

## Kroky

1. V adresáři `sample-service` spusťte Codex jen pro čtení:

   ```sh
   codex --sandbox read-only
   ```

   Kdo pracuje ve VS Code: v rozšíření Codex přepněte oprávnění na režim jen pro čtení.

2. Vložte zadání:

   > Jsi v C++ repozitáři, který neznáš. Nic neměň a nespouštěj build. Než začneš číst, napiš plán: co a v jakém pořadí projdeš.
   >
   > Pak odpověz, vždy s odkazem na soubor a funkci:
   > 1. Popiš cestu jednoho vzorku od příkazu `SUBMIT` až po řádek `result` na výstupu: funkce v pořadí, jak se volají.
   > 2. Jakým směrem jdou závislosti mezi knihovnami `labcore`, `labio`, `labsched` a `apps`? Ověř to podle `#include` a `CMakeLists.txt`, ne podle dokumentace. Najdeš nějaký `#include`, který jde proti směru?
   > 3. Kde je fronta vzorků a kde stojan? Jaké datové struktury používají a kde se nastavuje jejich kapacita?
   > 4. Které příkazy služba přijímá? Sedí to s tabulkou v `README.md`?
   > 5. Uveď tři místa, kde si nejsi jistý, a proč.

3. Sledujte, co agent dělá: které soubory otevřel, co hledal, kolik kroků potřeboval.

4. Porovnejte odpověď s `README.md` a `docs/architecture.md`. Kde se agent mýlí, nebo tvrdí
   něco, co v kódu není?

## Hotovo, když

- Máte odpovědi na pět otázek s odkazy na soubory.
- Víte o aspoň jednom místě, kde agent odhadoval, nebo jste si ověřili, že nikde.
- `git status` je čistý (agent nic nezměnil).

## Zkrácená varianta

Jen otázky 1 a 2.

## K zamyšlení

- Napsal agent plán dřív, než začal číst? Držel se ho?
- Ověřil směr závislostí v kódu, nebo ho opsal z README?
- Co z odpovědi byste zapsali do AGENTS.md (blok 3)?
