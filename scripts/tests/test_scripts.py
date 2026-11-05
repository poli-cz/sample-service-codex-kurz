import io
import json
import unittest

import gen_batch
import summarize_results


def result(assay, concentration, flags=()):
    return json.dumps({"event": "result", "assay": assay, "unit": "mmol/L",
                       "concentration": concentration, "flags": list(flags)})


class SummarizeTest(unittest.TestCase):
    def test_groups_by_assay_and_ignores_other_events(self):
        lines = [
            json.dumps({"event": "submit", "sample_id": "S-0001"}),
            result("GLU", 5.0),
            result("GLU", 7.0, ["H"]),
            result("K", 4.1),
            result("K", 0.0, ["E"]),
        ]
        s = summarize_results.summarize(lines)
        self.assertEqual(sorted(s), ["GLU", "K"])
        self.assertEqual(s["GLU"].values, [5.0, 7.0])
        self.assertEqual(s["GLU"].flagged, 1)
        self.assertEqual(s["K"].errors, 1)
        self.assertEqual(s["K"].count, 2)

    def test_render_prints_one_row_per_assay(self):
        out = io.StringIO()
        summarize_results.render(summarize_results.summarize([result("GLU", 5.0)]), out)
        self.assertEqual(len(out.getvalue().splitlines()), 2)


class GenBatchTest(unittest.TestCase):
    def test_is_deterministic_and_ends_with_quit(self):
        a = gen_batch.generate(10, 0.1, 0.2, 4, seed=5)
        b = gen_batch.generate(10, 0.1, 0.2, 4, seed=5)
        self.assertEqual(a, b)
        self.assertEqual(a[-1], "QUIT")
        self.assertEqual(sum(1 for line in a if line.startswith("SUBMIT")), 10)


if __name__ == "__main__":
    unittest.main()
