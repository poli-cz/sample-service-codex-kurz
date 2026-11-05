#!/usr/bin/env python3
"""Configure, build and test every preset: debug, release, asan and tidy.

Usage: python scripts/check_all.py [preset...]
"""

import subprocess
import sys
from pathlib import Path

DEFAULT_PRESETS = ["debug", "release", "asan", "tidy"]


def main(argv):
    presets = argv or DEFAULT_PRESETS
    root = Path(__file__).resolve().parent.parent
    for preset in presets:
        print(f"==> {preset}", flush=True)
        for step in (["cmake", "--preset", preset],
                     ["cmake", "--build", "--preset", preset],
                     ["ctest", "--preset", preset]):
            if subprocess.run(step, cwd=root).returncode != 0:
                return 1
    print("all presets passed: " + " ".join(presets))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
