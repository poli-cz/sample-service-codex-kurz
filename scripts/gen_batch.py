#!/usr/bin/env python3
"""Generate a command batch for sample-service.

Example:
    scripts/gen_batch.py --samples 40 --stat-share 0.1 --seed 3 > /tmp/load.batch
"""
from __future__ import annotations

import argparse
import random
import sys

ASSAYS = ("GLU", "CRP", "K")


def generate(samples: int, stat_share: float, urgent_share: float, rack_size: int,
             seed: int) -> list[str]:
    rng = random.Random(seed)
    lines = [f"# generated: samples={samples} seed={seed}"]
    for i in range(samples):
        roll = rng.random()
        if roll < stat_share:
            priority = "STAT"
        elif roll < stat_share + urgent_share:
            priority = "URGENT"
        else:
            priority = "ROUTINE"
        lines.append(f"SUBMIT S-{500000 + i:06d} {rng.choice(ASSAYS)} {priority}")
        if (i + 1) % rack_size == 0:
            lines.append("RUN")
    lines.append("RUN")
    lines.append("QUIT")
    return lines


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--samples", type=int, default=20)
    parser.add_argument("--stat-share", type=float, default=0.05)
    parser.add_argument("--urgent-share", type=float, default=0.15)
    parser.add_argument("--rack-size", type=int, default=4)
    parser.add_argument("--seed", type=int, default=1)
    args = parser.parse_args(argv)
    if args.samples < 0 or not 0 <= args.stat_share + args.urgent_share <= 1:
        parser.error("samples must be >= 0 and the shares must add up to at most 1")
    sys.stdout.write("\n".join(generate(args.samples, args.stat_share, args.urgent_share,
                                        args.rack_size, args.seed)) + "\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
