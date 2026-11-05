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
        # TODO (exercise C7): implement, the tests in tests/test_store.py say exactly how.
        # - validate instrument_id with self._instrument() (unknown id -> StoreError),
        # - assay: None or an upper-case code (ASSAY) that the instrument runs,
        # - limit: 1 to MAX_LIMIT, keep only the newest `limit` results,
        # - results from qc_results.json of this instrument (and assay), oldest first,
        # - return {"instrument_id": ..., "assay": ..., "results": [...]}.
        raise NotImplementedError("get_results is not implemented yet")

    # -- write tool --------------------------------------------------------------------

    def set_instrument_status(self, instrument_id: str, status: str, reason: str) -> Dict[str, Any]:
        """Records a status change. Refused unless the server runs with LAB_ALLOW_WRITES=1."""
        # TODO (exercise C7): implement after get_results. Refuse unless allow_writes,
        # validate status (STATUSES) and reason (3 to MAX_REASON characters, whitespace
        # collapsed), append one JSON line to self._status_log and return the change with
        # instrument_id, old_status, new_status, reason and changed_at.
        raise NotImplementedError("set_instrument_status is not implemented yet")
