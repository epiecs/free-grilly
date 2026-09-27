"""Tests for tools/build_web.py: python -m unittest discover -s tools -p "test_*.py" """
import importlib.util
import os
import tempfile
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
spec = importlib.util.spec_from_file_location("build_web", os.path.join(HERE, "build_web.py"))
build_web = importlib.util.module_from_spec(spec)
spec.loader.exec_module(build_web)


def write(folder, name, content):
    path = os.path.join(folder, name)
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", encoding="utf-8") as f:
        f.write(content)


class InlinePageTest(unittest.TestCase):
    def test_inlines_stylesheet_and_scripts_in_order(self):
        with tempfile.TemporaryDirectory() as folder:
            write(folder, "index.html",
                  '<link rel="stylesheet" href="app.css">'
                  '<script src="js/a.js"></script><script src="js/b.js"></script>')
            write(folder, "app.css", "body{color:red}")
            write(folder, "js/a.js", "let a = 1;")
            write(folder, "js/b.js", "let b = 2;")

            page = build_web.inline_page(folder)

            self.assertEqual(page,
                             "<style>\nbody{color:red}\n</style>"
                             "<script>\nlet a = 1;\n</script><script>\nlet b = 2;\n</script>")

    def test_missing_file_fails(self):
        with tempfile.TemporaryDirectory() as folder:
            write(folder, "index.html", '<script src="js/missing.js"></script>')
            with self.assertRaises(FileNotFoundError):
                build_web.inline_page(folder)


class HeaderTest(unittest.TestCase):
    def test_writes_bytes_length_and_quoted_etag(self):
        text = build_web.header(bytes([0x1f, 0x8b, 0x00]), "abc123", bytes([0x89, 0x50]))

        self.assertIn('const char WEB_APP_ETAG[] = "\\"abc123\\"";', text)
        self.assertIn("const size_t WEB_APP_GZ_LEN = 3;", text)
        self.assertIn("0x1f, 0x8b, 0x00,", text)
        self.assertIn("PROGMEM", text)

    def test_writes_touch_icon(self):
        text = build_web.header(bytes([0x1f]), "abc123", bytes([0x89, 0x50, 0x4e]))

        self.assertIn("const size_t WEB_TOUCH_ICON_LEN = 3;", text)
        self.assertIn("const uint8_t WEB_TOUCH_ICON[] PROGMEM = {\n    0x89, 0x50, 0x4e,\n};", text)


if __name__ == "__main__":
    unittest.main()
