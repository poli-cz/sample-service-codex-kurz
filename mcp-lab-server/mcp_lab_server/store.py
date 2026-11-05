"""Data access for the lab MCP server.

Everything here is plain Python without MCP, so it can be tested directly. The data is
invented: three simulated analyzers and their control (QC) results.

Reads come from JSON files in the data directory. The only write is a status change of an
instrument, which is appended to a log file in the state directory; the data files are
never modified.
"""

from __future__ import annotations

import json
import os
import re
from dataclasses import dataclass
from datetime import datetime, timezone
from pathlib import Path
from typing import Any, Dict, List, Optional

PACKAGE_DATA = Path(__file__).resolve().parent / "data"

INSTRUMENT_ID = re.compile(r"^AN-\d{2}$")
ASSAY = re.compile(r"^[A-Z]{1,8}$")
STATUSES = ("online", "maintenance", "out_of_service")
MAX_LIMIT = 100
MAX_REASON = 200


class StoreError(ValueError):
    """A request the store refuses; the message is safe to show to the caller."""


@dataclass(frozen=True)
class Settings:
    data_dir: Path
    state_dir: Path
    allow_writes: bool

    @staticmethod
    def from_env() -> "Settings":
        """LAB_DATA_DIR, LAB_STATE_DIR and LAB_ALLOW_WRITES (writes are off by default)."""
        data_dir = Path(os.environ.get("LAB_DATA_DIR", str(PACKAGE_DATA)))
        state_dir = Path(os.environ.get("LAB_STATE_DIR", str(Path.cwd() / "var")))
        allow_writes = os.environ.get("LAB_ALLOW_WRITES", "0") == "1"
        return Settings(data_dir=data_dir, state_dir=state_dir, allow_writes=allow_writes)


class LabStore:
    def __init__(self, settings: Settings) -> None:
        self._settings = settings

    # -- helpers -----------------------------------------------------------------------

    def _load(self, name: str) -> Any:
        with open(self._settings.data_dir / name, encoding="utf-8") as f:
            return json.load(f)

    @property
    def _status_log(self) -> Path:
        return self._settings.state_dir / "status_changes.jsonl"

    def _status_overrides(self) -> Dict[str, str]:
        """Latest status per instrument from the change log (later lines win)."""
        overrides: Dict[str, str] = {}
        if not self._status_log.exists():
            return overrides
        with open(self._status_log, encoding="utf-8") as f:
            for line in f:
                line = line.strip()
                if line:
                    change = json.loads(line)
                    overrides[change["instrument_id"]] = change["new_status"]
        return overrides

    def _instrument(self, instrument_id: str) -> Dict[str, Any]:
        if not INSTRUMENT_ID.match(instrument_id):
            raise StoreError("instrument_id must look like AN-01")
        for instrument in self.list_instruments():
            if instrument["id"] == instrument_id:
                return instrument
        raise StoreError(f"unknown instrument {instrument_id}")

    # -- read tools --------------------------------------------------------------------

    def list_instruments(self, status: Optional[str] = None) -> List[Dict[str, Any]]:
        """All instruments, optionally only those with the given status."""
        if status is not None and status not in STATUSES:
            raise StoreError(f"status must be one of {', '.join(STATUSES)}")
        overrides = self._status_overrides()
        instruments = []
        for instrument in self._load("instruments.json"):
            instrument = dict(instrument)
            instrument["status"] = overrides.get(instrument["id"], instrument["status"])
            if status is None or instrument["status"] == status:
                instruments.append(instrument)
        return instruments

    def get_results(
        self, instrument_id: str, assay: Optional[str] = None, limit: int = 20
    ) -> Dict[str, Any]:
        """Control results of one instrument, oldest first, at most `limit` newest ones.

        Each result carries the control lot with its target mean and SD, so the caller can
        evaluate Westgard rules.
        """
        instrument = self._instrument(instrument_id)
        if assay is not None:
            if not ASSAY.match(assay):
                raise StoreError("assay must be an upper-case code like GLU")
            if assay not in instrument["assays"]:
                raise StoreError(f"instrument {instrument_id} does not run assay {assay}")
        if not 1 <= limit <= MAX_LIMIT:
            raise StoreError(f"limit must be between 1 and {MAX_LIMIT}")
        results = [
            r
            for r in self._load("qc_results.json")
            if r["instrument_id"] == instrument_id and (assay is None or r["assay"] == assay)
        ]
        results.sort(key=lambda r: r["time"])
        return {"instrument_id": instrument_id, "assay": assay, "results": results[-limit:]}

    # -- write tool --------------------------------------------------------------------

    def set_instrument_status(self, instrument_id: str, status: str, reason: str) -> Dict[str, Any]:
        """Records a status change. Refused unless the server runs with LAB_ALLOW_WRITES=1."""
        if not self._settings.allow_writes:
            raise StoreError("writes are disabled on this server (LAB_ALLOW_WRITES is not 1)")
        if status not in STATUSES:
            raise StoreError(f"status must be one of {', '.join(STATUSES)}")
        reason = " ".join(reason.split())  # no newlines or control whitespace in the log
        if not 3 <= len(reason) <= MAX_REASON:
            raise StoreError(f"reason must have 3 to {MAX_REASON} characters")
        instrument = self._instrument(instrument_id)
        change = {
            "instrument_id": instrument_id,
            "old_status": instrument["status"],
            "new_status": status,
            "reason": reason,
            "changed_at": datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ"),
        }
        self._settings.state_dir.mkdir(parents=True, exist_ok=True)
        with open(self._status_log, "a", encoding="utf-8") as f:
            f.write(json.dumps(change) + "\n")
        return change
