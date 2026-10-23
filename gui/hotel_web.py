# web version of the gui, works without tkinter
# same idea as hotel_gui.py: python only calls our C functions
# no pip install needed, only standard library
#
# run with: make web  then open http://127.0.0.1:8000

import ctypes
import json
import os
from ctypes import c_void_p
from http.server import BaseHTTPRequestHandler, HTTPServer

from hotel_lib import ROOT, SAVE_FILE, load_library, text, walk

HOST, PORT = "127.0.0.1", 8000
PAGE = os.path.join(os.path.dirname(os.path.abspath(__file__)), "hotel_web.html")
MAX_TEXT = 49   # MAX_NAME - 1, longer names would get cut when loading the saved file

lib = load_library()
libc = ctypes.CDLL(None)
libc.free.argtypes = [c_void_p]
hotel = None   # only one hotel for everyone, its fine since HTTPServer does one request at a time


class BadRequest(Exception):
    pass


def field(body, key):
    value = str(body.get(key, "")).strip()
    if not value:
        raise BadRequest(f"Type a {key.replace('_', ' ')}.")
    # ; or a newline in a name would break the file format
    if len(value.encode()) > MAX_TEXT or any(c in value for c in ";\r\n"):
        raise BadRequest(f"The {key.replace('_', ' ')} must be at most {MAX_TEXT} bytes, without ';'.")
    return value.encode()


def number(body, key, low, high):
    try:
        value = int(body.get(key))
    except (TypeError, ValueError):
        value = None
    if value is None or not low <= value <= high:
        raise BadRequest(f"{key.capitalize()} must be a whole number from {low} to {high}.")
    return value


def need_hotel():
    if not hotel:
        raise BadRequest("Create a hotel chain first.")
    return hotel


def need_site(body):
    site = lib.findSite(need_hotel(), field(body, "city"))
    if not site:
        raise BadRequest("No such location.")
    return site


def total_clients():
    return sum(site.number_clients for site in walk(hotel.contents.first)) if hotel else 0


def state():
    if not hotel:
        return None
    return {
        "name": text(hotel.contents.name),
        "sites": [{
            "city": text(site.city),
            "clients": [{"last_name": text(c.last_name), "first_name": text(c.first_name),
                         "age": c.age, "room": c.num_room} for c in walk(site.first)],
        } for site in walk(hotel.contents.first)],
    }


# each action gets the json from the page and returns the message to show
def create(body):
    global hotel
    name = field(body, "name")
    new_hotel = lib.createHotel(name)
    if not new_hotel:
        raise BadRequest("Out of memory.")
    if hotel:
        lib.freeHotel(hotel)
    hotel = new_hotel
    return f"Hotel chain {name.decode()} created."


def add_site(body):
    city = field(body, "city")
    if lib.findSite(need_hotel(), city):
        raise BadRequest(f"Location {city.decode()} already exists.")
    site = lib.createSite(city)
    if not site:
        raise BadRequest("Out of memory.")
    if not lib.addSite(hotel, site):
        # addSite failed so we have to free it ourselves (like in main.c)
        libc.free(ctypes.cast(site.contents.city, c_void_p))
        libc.free(ctypes.cast(site, c_void_p))
        raise BadRequest(f"Location {city.decode()} could not be added.")
    return f"Location {city.decode()} added."


def delete_site(body):
    city = field(body, "city")
    if not lib.deleteSite(need_hotel(), city):
        raise BadRequest(f"No location in {city.decode()}.")
    return f"Location {city.decode()} and all its bookings deleted."


def add_client(body):
    site = need_site(body)
    last, first = field(body, "last_name"), field(body, "first_name")
    age, room = number(body, "age", 0, 150), number(body, "room", 1, 99999)
    client = lib.createClient(last, first, age, room)
    if not client:
        raise BadRequest("Out of memory.")
    if not lib.addClient(site, client):
        lib.freeClients(client)
        raise BadRequest(f"{last.decode()} {first.decode()} already has a booking here.")
    return f"Booking for {last.decode()} {first.decode()} added."


def delete_client(body):
    site = need_site(body)
    last, first = field(body, "last_name"), field(body, "first_name")
    if not lib.deleteClient(site, last, first):
        raise BadRequest(f"No booking for {last.decode()} {first.decode()}.")
    return f"Booking for {last.decode()} {first.decode()} deleted."


def load_from(path):
    need_hotel()
    before = total_clients()
    lib.loadBookings(hotel, path.encode())   # skipped lines get printed in the terminal
    return f"{total_clients() - before} booking(s) loaded from {path}."


def save(_body):
    written = lib.saveHotel(need_hotel(), SAVE_FILE.encode())
    if written < 0:
        raise BadRequest(f"Cannot write {SAVE_FILE}.")
    return f"{written} booking(s) saved to {SAVE_FILE}."


ACTIONS = {
    "create": create, "add_site": add_site, "delete_site": delete_site,
    "add_client": add_client, "delete_client": delete_client, "save": save,
    "load_file": lambda _b: load_from("data/bookings.txt"),
    "load_saved": lambda _b: load_from(SAVE_FILE),
}


def search(prefix):
    results = lib.findClients(need_hotel(), prefix.encode())
    found = [{"last_name": text(r.client.contents.last_name), "first_name": text(r.client.contents.first_name),
              "age": r.client.contents.age, "room": r.client.contents.num_room, "city": text(r.city)}
             for r in walk(results)]
    lib.freeResults(results)
    return found


class Handler(BaseHTTPRequestHandler):
    def send(self, code, payload, content_type="application/json"):
        data = payload if isinstance(payload, bytes) else json.dumps(payload).encode()
        self.send_response(code)
        self.send_header("Content-Type", content_type)
        self.send_header("Content-Length", str(len(data)))
        self.end_headers()
        self.wfile.write(data)

    def do_GET(self):
        if self.path == "/":
            with open(PAGE, "rb") as page:
                self.send(200, page.read(), "text/html; charset=utf-8")
        elif self.path == "/api/state":
            self.send(200, {"state": state()})
        else:
            self.send(404, {"error": "Not found."})

    def do_POST(self):
        action = ACTIONS.get(self.path.removeprefix("/api/"))
        if not action and self.path != "/api/search":
            self.send(404, {"error": "Not found."})
            return
        try:
            length = int(self.headers.get("Content-Length") or 0)
            body = json.loads(self.rfile.read(length) or b"{}") if length <= 10_000 else None
            if not isinstance(body, dict):
                raise BadRequest("Bad request.")
            if action:
                self.send(200, {"message": action(body), "state": state()})
            else:
                found = search(str(body.get("prefix", "")).strip())
                self.send(200, {"message": f"{len(found)} customer(s) found.", "results": found})
        except (BadRequest, ValueError) as error:
            self.send(400, {"error": str(error) or "Bad request.", "state": state()})


def main():
    os.chdir(ROOT)   # so the C code finds data/
    server = HTTPServer((HOST, PORT), Handler)
    print(f"Hotel chain web app on http://{HOST}:{PORT}  (Ctrl+C to stop)")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        if hotel:
            lib.freeHotel(hotel)


if __name__ == "__main__":
    main()
