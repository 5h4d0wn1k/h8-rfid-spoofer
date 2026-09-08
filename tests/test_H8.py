import os
import sys
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "host"))
import h8_cli as m


class TestUid(unittest.TestCase):
    def test_parse_uid(self):
        u = m.parse_uid("card0: 04:5A:67:1B")
        self.assertEqual(u["label"], "card0")
        self.assertEqual(u["uid"], bytes.fromhex("045A671B"))

    def test_emulate_sim_rfc5737_placeholder(self):
        u = m.parse_uid("card0: 04:5A:67:1B")
        s = m.emulate_sim(u["uid"])
        self.assertIn("SIM", s)
        self.assertIn("no RF field", s)

    def test_invalid(self):
        self.assertIsNone(m.parse_uid("zz"))


if __name__ == "__main__":
    unittest.main()
