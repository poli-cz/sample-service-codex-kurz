"""Unit tests of the data access, without MCP."""

from __future__ import annotations

import json
from pathlib import Path

import pytest

from mcp_lab_server.store import PACKAGE_DATA, LabStore, Settings, StoreError


def make_store(tmp_path: Path, allow_writes: bool = False) -> LabStore:
    return LabStore(Settings(data_dir=PACKAGE_DATA, state_dir=tmp_path, allow_writes=allow_writes))


def test_list_instruments_returns_all_three(tmp_path: Path) -> None:
    ids = [i["id"] for i in make_store(tmp_path).list_instruments()]
    assert ids == ["AN-01", "AN-02", "AN-03"]


def test_list_instruments_filters_by_status(tmp_path: Path) -> None:
    assert [i["id"] for i in make_store(tmp_path).list_instruments("maintenance")] == ["AN-03"]


def test_list_instruments_rejects_unknown_status(tmp_path: Path) -> None:
    with pytest.raises(StoreError):
        make_store(tmp_path).list_instruments("broken")


# -- get_results ---------------------------------------------------------------------------


def test_get_results_returns_oldest_first_with_lot_targets(tmp_path: Path) -> None:
    out = make_store(tmp_path).get_results("AN-02", "GLU")
    assert out["instrument_id"] == "AN-02"
    assert out["assay"] == "GLU"
    values = [r["value"] for r in out["results"]]
    assert len(values) == 10
    assert values[-2:] == [5.95, 5.98]
    times = [r["time"] for r in out["results"]]
    assert times == sorted(times)
    first = out["results"][0]
    assert first["lot"] == "QC-GLU-L1-2611"
    assert (first["target_mean"], first["target_sd"]) == (5.5, 0.2)


def test_get_results_limit_keeps_the_newest(tmp_path: Path) -> None:
    out = make_store(tmp_path).get_results("AN-02", "GLU", limit=3)
    assert [r["value"] for r in out["results"]] == [5.77, 5.95, 5.98]


def test_get_results_without_assay_returns_every_assay_of_the_instrument(tmp_path: Path) -> None:
    out = make_store(tmp_path).get_results("AN-01", limit=100)
    assert {r["assay"] for r in out["results"]} == {"GLU", "K", "CRP"}
    assert len(out["results"]) == 30


@pytest.mark.parametrize(
    "instrument_id, assay, limit",
    [
        ("AN-99", None, 20),          # unknown instrument
        ("../data", None, 20),        # anything that is not an instrument id
        ("AN-03", "GLU", 20),         # assay the instrument does not run
        ("AN-01", "glu; rm", 20),     # not an assay code
        ("AN-01", None, 0),           # limit out of range
        ("AN-01", None, 101),
    ],
)
def test_get_results_rejects_bad_input(tmp_path: Path, instrument_id, assay, limit) -> None:
    with pytest.raises(StoreError):
        make_store(tmp_path).get_results(instrument_id, assay, limit)


# -- set_instrument_status -----------------------------------------------------------------


def test_writes_are_refused_by_default(tmp_path: Path) -> None:
    with pytest.raises(StoreError, match="disabled"):
        make_store(tmp_path).set_instrument_status("AN-02", "maintenance", "QC failure")
    assert not (tmp_path / "status_changes.jsonl").exists()


def test_status_change_is_logged_and_visible(tmp_path: Path) -> None:
    store = make_store(tmp_path, allow_writes=True)
    change = store.set_instrument_status("AN-02", "maintenance", "QC 2-2s\non GLU")
    assert change["old_status"] == "online"
    assert change["new_status"] == "maintenance"
    assert change["reason"] == "QC 2-2s on GLU"
    lines = (tmp_path / "status_changes.jsonl").read_text(encoding="utf-8").splitlines()
    assert json.loads(lines[0])["instrument_id"] == "AN-02"
    assert [i["id"] for i in store.list_instruments("maintenance")] == ["AN-02", "AN-03"]


@pytest.mark.parametrize(
    "instrument_id, status, reason",
    [("AN-02", "broken", "QC failure"), ("AN-02", "maintenance", "x"), ("AN-77", "online", "ok now")],
)
def test_status_change_rejects_bad_input(tmp_path: Path, instrument_id, status, reason) -> None:
    with pytest.raises(StoreError):
        make_store(tmp_path, allow_writes=True).set_instrument_status(instrument_id, status, reason)
