#!/usr/bin/env python3
"""Checks a solution of ticket LAB-318 in an analyzer-link clone.

Usage (from anywhere):
    python zkontroluj.py <path-to-analyzer-link> [--bez-tidy]

Builds and tests preset debug (and tidy unless --bez-tidy), feeds link-cli with probe
frames and prints one line per criterion. Exit code 0 only when every criterion passes.
"""

import subprocess
import sys
from pathlib import Path

OLD_CODES = [
    ("Barcode unreadable", "S"),
    ("Clot detected", "S"),
    ("Cuvette missing", "H"),
    ("Door open", "H"),
    ("Incubator temperature out of range", "Q"),
    ("Photometer lamp failure", "H"),
    ("Reagent lid open", "R"),
    ("Vacuum pressure low", "H"),
    ("Sample probe crash", "H"),
    ("Reagent empty", "R"),
    ("Sample volume too low", "S"),
    ("Reaction temperature drift", "Q"),
    ("Wash station failure", "H"),
    ("Waste container full", "H"),
]


def run(cmd, cwd, stdin=None):
    return subprocess.run(cmd, cwd=cwd, input=stdin, capture_output=True, text=True,
                          encoding="utf-8", errors="replace")


def preset(repo, name):
    for step in (["cmake", "--preset", name],
                 ["cmake", "--build", "--preset", name],
                 ["ctest", "--preset", name]):
        p = run(step, repo)
        if p.returncode != 0:
            tail = (p.stdout + p.stderr).strip().splitlines()[-8:]
            return False, " ".join(step) + " selhal:\n      " + "\n      ".join(tail)
    return True, ""


def cli_path(repo):
    for name in ("link-cli.exe", "link-cli"):
        p = repo / "build" / "debug" / name
        if p.exists():
            return p
    return None


def main(argv):
    # Czech output must not crash on a Windows console or pipe with a legacy code page.
    for stream in (sys.stdout, sys.stderr):
        if hasattr(stream, "reconfigure"):
            stream.reconfigure(encoding="utf-8", errors="replace")
    args = [a for a in argv if not a.startswith("--")]
    if len(args) != 1:
        print(__doc__)
        return 2
    repo = Path(args[0]).resolve()
    with_tidy = "--bez-tidy" not in argv
    results = []

    ok, detail = preset(repo, "debug")
    results.append(("Build a testy (preset debug) projdou", ok, detail))

    cli = cli_path(repo) if ok else None
    if cli is None:
        for label in ("Kód 14 = Reagent lot expired", "Staré kódy 0 až 13 beze změny",
                      "Kód 14 jde do LIS s příznakem Q", "Firmware 3.2 (protokol 2.7) bez varování"):
            results.append((label, False, "link-cli není postavený"))
    else:
        frames = [f"ERR|AN-99|{c}|S-9" for c in range(15)] + ["HB|AN-99|2.7"]
        out = run([str(cli)], repo, "\n".join(frames) + "\n").stdout.splitlines()
        out += [""] * (16 - len(out))

        new = out[14]
        results.append(("Kód 14 = Reagent lot expired",
                        'code=14 "Reagent lot expired"' in new, new))

        broken = []
        for code, (text, flag) in enumerate(OLD_CODES):
            want = f'error AN-99 code={code} "{text}" lis=E|AN-99|S-9|{flag}|{text}'
            if out[code] != want:
                broken.append(f"kód {code}: {out[code]}")
        results.append(("Staré kódy 0 až 13 beze změny", not broken, "; ".join(broken[:3])))

        results.append(("Kód 14 jde do LIS s příznakem Q",
                        "lis=E|AN-99|S-9|Q|" in new, new))

        hb = out[15]
        results.append(("Firmware 3.2 (protokol 2.7) bez varování", hb.endswith(" ok"), hb))

    root = run(["git", "rev-list", "--max-parents=0", "HEAD"], repo).stdout.split()
    if root:
        diff = run(["git", "status", "--porcelain", "--", "tests/data/recorded"], repo).stdout
        diff += run(["git", "diff", "--name-only", root[-1], "--", "tests/data/recorded"], repo).stdout
        results.append(("Nahrávky z přístrojů nedotčené", not diff.strip(),
                        " ".join(sorted(set(diff.split())))))
    else:
        results.append(("Nahrávky z přístrojů nedotčené", False, "není git repo"))

    if with_tidy:
        ok, detail = preset(repo, "tidy")
        results.append(("clang-tidy (preset tidy) čistý", ok, detail))

    passed = sum(1 for _, ok, _ in results if ok)
    print(f"LAB-318 v {repo}")
    for label, ok, detail in results:
        print(f"  {'OK   ' if ok else 'CHYBA'} {label}")
        if not ok and detail:
            print(f"      {detail}")
    print(f"Splněno {passed} z {len(results)}")
    return 0 if passed == len(results) else 1


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
