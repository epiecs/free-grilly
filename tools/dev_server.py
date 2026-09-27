"""Frontend development server.

    python tools/dev_server.py 192.168.1.50    serve web/ and proxy /api/* to that grill
    python tools/dev_server.py --mock          serve web/ with mock data (tools/mock_api.py)

Then open http://localhost:8000. Pages and api share one origin, so saving works: the grill blocks
cross-origin posts. Files are served straight from web/, reload the page after editing.
"""
import http.server
import json
import os
import sys
import urllib.error
import urllib.request

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
WEB_DIR = os.path.join(ROOT, "web")
sys.path.insert(0, os.path.join(ROOT, "tools"))

GRILL = None
PORT = 8000


class Handler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=WEB_DIR, **kwargs)

    def end_headers(self):
        self.send_header("Cache-Control", "no-store")
        super().end_headers()

    def do_GET(self):
        if self.path.startswith("/api/"):
            return self.api()
        return super().do_GET()

    def do_POST(self):
        if self.path.startswith("/api/"):
            return self.api()
        self.send_error(404)

    def api(self):
        length = int(self.headers.get("Content-Length", 0))
        body = self.rfile.read(length) if length else None
        if GRILL is None:
            import mock_api
            status, data = mock_api.handle(self.command, self.path.split("?")[0], body)
            return self.reply(status, "application/json", json.dumps(data).encode("utf-8"))

        request = urllib.request.Request("http://%s%s" % (GRILL, self.path), data=body, method=self.command)
        for name in ("Content-Type", "Authorization", "X-Grilly-Update"):
            if self.headers.get(name):
                request.add_header(name, self.headers[name])
        try:
            with urllib.request.urlopen(request, timeout=60) as response:
                self.reply(response.status, response.headers.get("Content-Type", "application/json"), response.read())
        except urllib.error.HTTPError as error:
            self.reply(error.code, error.headers.get("Content-Type", "application/json"), error.read())
        except OSError as error:
            self.reply(502, "application/json", json.dumps({"error": "Grill unreachable: %s" % error}).encode("utf-8"))

    def reply(self, status, content_type, data):
        self.send_response(status)
        self.send_header("Content-Type", content_type)
        self.send_header("Content-Length", str(len(data)))
        self.end_headers()
        self.wfile.write(data)


def main():
    global GRILL
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    GRILL = None if sys.argv[1] == "--mock" else sys.argv[1]
    server = http.server.ThreadingHTTPServer(("127.0.0.1", PORT), Handler)
    print("Serving web/ on http://localhost:%d (%s)" % (PORT, "mock data" if GRILL is None else "grill " + GRILL))
    server.serve_forever()


if __name__ == "__main__":
    main()
