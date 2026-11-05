#!/usr/bin/env python3
"""Summarize the JSON lines written by sample-service.

Example:
    build/debug/apps/sample-service/sample-service --batch examples/basic.batch \
        | scripts/summarize_results.py
"""
from __future__ import annotations

import argparse
import json
import statistics
import sys
from collections import defaultdict
from dataclasses import dataclass, field
from typing import Iterable, TextIO


@dataclass
class AssaySummary:
    unit: str = ""
    values: list[float] = field(default_factory=list)
    flagged: int = 0
    errors: int = 0

    @property
    def count(self) -> int:
        return len(self.values) + self.errors


def summarize(lines: Iterable[str]) -> dict[str, AssaySummary]:
    summaries: dict[str, AssaySummary] = defaultdict(AssaySummary)
    for raw in lines:
        raw = raw.strip()
        if not raw:
            continue
        event = json.loads(raw)
        if event.get("event") != "result":
            continue
        summary = summaries[event["assay"]]
        summary.unit = event.get("unit") or summary.unit
        flags = event.get("flags", [])
        if "E" in flags:
            summary.errors += 1
            continue
        if flags:
            summary.flagged += 1
        summary.values.append(float(event["concentration"]))
    return dict(summaries)


def render(summaries: dict[str, AssaySummary], out: TextIO) -> None:
    out.write(f"{'assay':<6} {'unit':<8} {'n':>4} {'mean':>10} {'sd':>10} {'flagged':>8} {'errors':>7}\n")
    for assay in sorted(summaries):
        s = summaries[assay]
        mean = statistics.fmean(s.values) if s.values else float("nan")
        sd = statistics.stdev(s.values) if len(s.values) > 1 else 0.0
        out.write(f"{assay:<6} {s.unit:<8} {s.count:>4} {mean:>10.3f} {sd:>10.3f} "
                  f"{s.flagged:>8} {s.errors:>7}\n")


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("file", nargs="?", help="JSON lines file (default: standard input)")
    parser.add_argument("--fail-on-errors", action="store_true",
                        help="exit with status 1 when any instrument error was reported")
    args = parser.parse_args(argv)
    if args.file:
        with open(args.file, encoding="utf-8") as handle:
            summaries = summarize(handle)
    else:
        summaries = summarize(sys.stdin)
    render(summaries, sys.stdout)
    if args.fail_on_errors and any(s.errors for s in summaries.values()):
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
