# web version of the gui, works without tkinter
# same idea as hotel_gui.py: python only calls our C functions
# no pip install needed, only standard library
#
# two ways in:
#   - an account (login + password): you get a "my chains" list, chains are saved automatically
#     to data/chains/<login>/ every time something changes
#   - open a .txt file as a guest: asks for the file password only if the file has one
# passwords are never stored, only salted pbkdf2 hashes
#
# run with: make web  then open http://127.0.0.1:8000

import ctypes
import hashlib
import hmac
import json
import os
import re
import secrets
import tempfile
from ctypes import c_void_p
from http.server import BaseHTTPRequestHandler, HTTPServer

from hotel_lib import ROOT, load_library, text, walk

HOST, PORT = "127.0.0.1", 8000
PAGE = os.path.join(os.path.dirname(os.path.abspath(__file__)), "hotel_web.html")
ACCOUNTS_FILE = "data/accounts.txt"   # login;hash per line (relative to ROOT)
CHAINS_DIR = "data/chains"            # data/chains/<login>/<chain>.txt
MAX_TEXT = 49   # MAX_NAME - 1, longer names would get cut when loading the saved file
MAX_BODY = 1_100_000   # a bit more than the 1 MB file limit of the page
ITERATIONS = 200_000
HEADER = "lastname;firstname;age;room number;city"
HASH_RE = re.compile(r"pbkdf2\$[1-9][0-9]{0,6}\$(?:[0-9a-f]{2})+\$[0-9a-f]{64}")
LOGIN_RE = re.compile(r"[A-Za-z0-9_-]{1,32}")   # also the folder name, so keep it simple
CHAIN_FILE_RE = re.compile(r"[A-Za-z0-9_-]+\.txt")

lib = load_library()
libc = ctypes.CDLL(None)
libc.free.argtypes = [c_void_p]
# one user at a time, its fine since HTTPServer does one request at a time
account = None   # login of the signed in account
hotel = None     # the open chain (C linked lists)
chain = None     # {"hash": file password hash or "", "path": account file or None, "filename": for guests}
pending = None   # a password protected file waiting for its password


class BadRequest(Exception):
    pass


def ok_text(value):
    return bool(value) and len(value.encode()) <= MAX_TEXT and not any(c in value for c in ";\r\n")


def field(body, key):
    value = str(body.get(key, "")).strip()
    if not value:
        raise BadRequest(f"Type a {key.replace('_', ' ')}.")
    # ; or a newline in a name would break the file format
    if not ok_text(value):
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


def need_account():
    if not account:
        raise BadRequest("Log in first.")
    return account


def need_hotel():
    if not hotel:
        raise BadRequest("Open a chain first.")
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


def info():
    """what the page needs to know to draw itself, sent with every answer"""
    return {
        "state": state(),
        "user": account,
        "chain": {"protected": bool(chain["hash"]), "guest": not chain["path"],
                  "filename": chain["filename"]} if hotel else None,
    }


# ---------- passwords ----------
def make_hash(password):
    salt = secrets.token_bytes(16)
    digest = hashlib.pbkdf2_hmac("sha256", password.encode(), salt, ITERATIONS)
    return f"pbkdf2${ITERATIONS}${salt.hex()}${digest.hex()}"


def check_hash(password, stored):
    _, rounds, salt, digest = stored.split("$")
    mine = hashlib.pbkdf2_hmac("sha256", password.encode(), bytes.fromhex(salt), int(rounds))
    return hmac.compare_digest(mine.hex(), digest)


DUMMY_HASH = make_hash("not a real password")   # so a wrong login takes as long as a wrong password


def new_password(body, optional=False):
    """password + password2 from the page. returns the hash, or "" if optional and both are empty"""
    password, password2 = str(body.get("password", "")), str(body.get("password2", ""))
    if optional and not password and not password2:
        return ""
    if len(password) < 4:
        raise BadRequest("The password must be at least 4 characters.")
    if password != password2:
        raise BadRequest("The two passwords are different.")
    return make_hash(password)


# ---------- chain files ----------
def parse(content):
    """checks a chain / bookings file. returns (meta, sites, count) or raises BadRequest with the reason.
    sites is {city: [(last, first, age, room), ...]}, meta is {"name", "hash"} or None for a plain bookings file"""
    def bad(n, why):
        raise BadRequest(f"Line {n}: {why}" if n else why)

    if not content.strip():
        bad(0, "The file is empty.")
    if "\0" in content:
        bad(0, "This is not a text file.")
    meta, sites, count, first = None, {}, 0, True
    for n, line in enumerate(content.removeprefix("﻿").split("\n"), 1):
        line = line.strip()
        if not line:
            continue
        p = line.split(";")
        if p[0] == "#HOTEL":
            if not first:
                bad(n, "the #HOTEL line must be the first line.")
            # #HOTEL;name;hash  (hash empty = no password). old files had #HOTEL;name;login;hash
            if len(p) == 4:
                p = [p[0], p[1], p[3]]
            if len(p) != 3 or not ok_text(p[1]) or (p[2] and not HASH_RE.fullmatch(p[2])):
                bad(n, "the #HOTEL line is damaged.")
            meta = {"name": p[1], "hash": p[2]}
        elif p[0] == "#SITE":
            if len(p) != 2 or not ok_text(p[1]):
                bad(n, "the #SITE line is damaged.")
            sites.setdefault(p[1], [])
        elif line.lower() != HEADER:
            if len(p) != 5:
                bad(n, f'expected 5 values separated by ";" (last name;first name;age;room;city), found {len(p)}.')
            last, first_name, age, room, city = (x.strip() for x in p)
            if not ok_text(last) or not ok_text(first_name) or not ok_text(city):
                bad(n, "names and city must be 1 to 49 bytes.")
            if not re.fullmatch(r"[0-9]+", age) or int(age) > 150:
                bad(n, f'age "{age}" must be a whole number from 0 to 150.')
            if not re.fullmatch(r"[0-9]+", room) or not 1 <= int(room) <= 99999:
                bad(n, f'room "{room}" must be a whole number from 1 to 99999.')
            clients = sites.setdefault(city, [])
            if any(c[0] == last and c[1] == first_name for c in clients):
                bad(n, f"{last} {first_name} is booked twice in {city}.")
            clients.append((last, first_name, int(age), int(room)))
            count += 1
        first = False
    if not meta and not count:
        bad(0, "No bookings found in this file.")
    return meta, sites, count


def free_site(site):
    # addSite failed so we have to free it ourselves (like in main.c)
    libc.free(ctypes.cast(site.contents.city, c_void_p))
    libc.free(ctypes.cast(site, c_void_p))


def build(name, sites):
    """makes a new C hotel from parsed sites, using only our C functions"""
    new_hotel = lib.createHotel(name.encode())
    if not new_hotel:
        raise BadRequest("Out of memory.")
    for city, clients in sites.items():
        site = lib.createSite(city.encode())
        if not site:
            lib.freeHotel(new_hotel)
            raise BadRequest("Out of memory.")
        if not lib.addSite(new_hotel, site):
            free_site(site)   # cant happen, parse already merged same cities
            continue
        for last, first, age, room in clients:
            client = lib.createClient(last.encode(), first.encode(), age, room)
            if not client:
                lib.freeHotel(new_hotel)
                raise BadRequest("Out of memory.")
            if not lib.addClient(site, client):
                lib.freeClients(client)
    return new_hotel


def safe_name(name):
    return (re.sub(r"[^A-Za-z0-9_-]+", "_", name) or "chain") + ".txt"


def account_dir():
    return f"{CHAINS_DIR}/{need_account()}"


def close_chain():
    global hotel, chain
    if hotel:
        lib.freeHotel(hotel)
    hotel = chain = None


def open_hotel(new_hotel, pass_hash, path, filename):
    close_chain()
    global hotel, chain
    hotel, chain = new_hotel, {"hash": pass_hash, "path": path, "filename": filename}


def write_chain(path):
    written = lib.saveChainFile(hotel, path.encode(), chain["hash"].encode())
    if written < 0:
        raise BadRequest(f"Cannot write {path}.")
    return written


def autosave(message):
    """account chains are saved after every change"""
    if hotel and chain["path"]:
        try:
            write_chain(chain["path"])
        except BadRequest:
            raise BadRequest(f"{message} But it could not be saved to your account.") from None
    return message


def read_accounts():
    try:
        with open(ACCOUNTS_FILE, encoding="utf-8") as accounts:
            return dict(line.rstrip("\n").split(";", 1) for line in accounts if ";" in line)
    except FileNotFoundError:
        return {}


# ---------- accounts ----------
def signup(body):
    global account
    login = str(body.get("login", "")).strip()
    if not LOGIN_RE.fullmatch(login):
        raise BadRequest("The login can only use letters, numbers, _ and - (up to 32).")
    pass_hash = new_password(body)
    if login.lower() in (name.lower() for name in read_accounts()):
        raise BadRequest("This login is already taken.")
    os.makedirs(os.path.dirname(ACCOUNTS_FILE), exist_ok=True)
    with open(ACCOUNTS_FILE, "a", encoding="utf-8") as accounts:
        accounts.write(f"{login};{pass_hash}\n")
    close_chain()
    account = login
    return f"Account {login} created."


def login(body):
    global account
    user, password = str(body.get("login", "")).strip(), str(body.get("password", ""))
    if not user or not password:
        raise BadRequest("Type your login and password.")
    stored = read_accounts().get(user)
    # check a hash even if the login is wrong, so both cases take the same time
    if not check_hash(password, stored or DUMMY_HASH) or not stored:
        raise BadRequest("Wrong login or password.")
    close_chain()
    account = user
    return f"Welcome back, {user}."


def logout(_body):
    global account, pending
    close_chain()
    account = pending = None
    return "Logged out."


# ---------- my chains ----------
def list_chains(_body):
    folder = account_dir()
    chains = []
    for file in sorted(os.listdir(folder)) if os.path.isdir(folder) else []:
        if not CHAIN_FILE_RE.fullmatch(file):
            continue
        try:
            with open(f"{folder}/{file}", encoding="utf-8") as saved:
                meta, sites, count = parse(saved.read())
        except (BadRequest, OSError, UnicodeDecodeError):
            continue   # a broken file is just not shown
        chains.append({"file": file, "name": meta["name"] if meta else file[:-4], "locations": len(sites),
                       "bookings": count, "protected": bool(meta and meta["hash"])})
    chains.sort(key=lambda c: c["name"].lower())
    return {"chains": chains}


def chain_path(body):
    file = str(body.get("file", ""))
    path = f"{account_dir()}/{file}"
    if not CHAIN_FILE_RE.fullmatch(file) or not os.path.isfile(path):
        raise BadRequest("No such chain.")
    return path


def new_chain(body):
    folder = account_dir()
    name = str(body.get("name", "")).strip()
    if not ok_text(name):
        raise BadRequest('Type a chain name (up to 49 bytes, no ";").')
    pass_hash = new_password(body, optional=True)
    path = f"{folder}/{safe_name(name)}"
    if os.path.exists(path):
        raise BadRequest(f"You already have a chain saved as {safe_name(name)}.")
    os.makedirs(folder, exist_ok=True)
    open_hotel(build(name, {}), pass_hash, path, None)
    write_chain(path)
    return f"Hotel chain {name} created."


def open_chain(body):
    path = chain_path(body)
    with open(path, encoding="utf-8") as saved:
        meta, sites, count = parse(saved.read())
    # its your own chain, so no file password needed here
    open_hotel(build(meta["name"] if meta else os.path.basename(path)[:-4], sites),
               meta["hash"] if meta else "", path, None)
    return f"{count} booking(s) loaded."


def delete_chain(body):
    path = chain_path(body)
    if hotel and chain["path"] == path:
        close_chain()
    os.remove(path)
    return "Chain deleted."


# ---------- open a file ----------
def import_path(name):
    """a free file in the account folder for an imported chain: "Name", "Name (2)", ..."""
    folder = account_dir()
    os.makedirs(folder, exist_ok=True)
    candidate, n = name, 2
    while os.path.exists(f"{folder}/{safe_name(candidate)}"):
        suffix = f" ({n})"
        base = name
        while not ok_text(base + suffix):
            base = base[:-1]
        candidate, n = base + suffix, n + 1
    return candidate, f"{folder}/{safe_name(candidate)}"


def finish_open(meta, sites, count, filename):
    """opens a checked file, as a guest or copied into the account"""
    name = meta["name"] if meta else re.sub(r"[_\-;\r\n]+", " ", os.path.splitext(filename)[0]).strip()
    while name and not ok_text(name):
        name = name[:-1]
    name = name.strip() or "Chain"
    pass_hash = meta["hash"] if meta else ""
    message = f"{count} booking(s) loaded from {filename}."
    if account:
        name, path = import_path(name)
        open_hotel(build(name, sites), pass_hash, path, None)
        write_chain(path)
        return message + " Added to your chains."
    open_hotel(build(name, sites), pass_hash, None, filename)
    return message


def open_file(body):
    global pending
    name = str(body.get("filename", "")).strip() or "file"
    if not re.search(r"\.(txt|csv)$", name, re.IGNORECASE):
        raise BadRequest(f'"{name}" is not a .txt file. Choose a hotel chain file saved from this app '
                         f'or a bookings .txt file.')
    try:
        meta, sites, count = parse(str(body.get("text", "")))
    except BadRequest as error:
        raise BadRequest(f'"{name}" is not in the right format. {error}') from None
    if meta and meta["hash"]:
        pending = {"meta": meta, "sites": sites, "count": count, "filename": name}
        return {"needs": "password", "name": meta["name"], "locations": len(sites), "bookings": count}
    pending = None
    return {"message": finish_open(meta, sites, count, name)}


def unlock(body):
    global pending
    if not pending:
        raise BadRequest("Choose a file first.")
    password = str(body.get("password", ""))
    if not password:
        raise BadRequest("Type the file password.")
    if not check_hash(password, pending["meta"]["hash"]):
        raise BadRequest("Wrong password.")
    p, pending = pending, None
    return finish_open(p["meta"], p["sites"], p["count"], p["filename"])


def close(_body):
    need_hotel()
    close_chain()
    return "Chain closed."


def set_file_password(body):
    need_hotel()
    chain["hash"] = new_password(body, optional=True)
    return autosave("File password set." if chain["hash"] else "File password removed.")


def save(_body):
    """gives the page the chain file to download (and saves it to the account too)"""
    need_hotel()
    filename = chain["filename"] or safe_name(text(hotel.contents.name))
    if not filename.lower().endswith(".txt"):
        filename = os.path.splitext(filename)[0] + ".txt"
    if chain["path"]:
        write_chain(chain["path"])
    with tempfile.TemporaryDirectory() as folder:
        written = write_chain(f"{folder}/chain.txt")
        with open(f"{folder}/chain.txt", encoding="utf-8") as saved:
            content = saved.read()
    return {"message": f"{written} booking(s) saved to {filename}.", "filename": filename, "text": content}


# ---------- app actions: each gets the json from the page and returns the message to show ----------
def add_site(body):
    city = field(body, "city")
    if lib.findSite(need_hotel(), city):
        raise BadRequest(f"Location {city.decode()} already exists.")
    site = lib.createSite(city)
    if not site:
        raise BadRequest("Out of memory.")
    if not lib.addSite(hotel, site):
        free_site(site)
        raise BadRequest(f"Location {city.decode()} could not be added.")
    return autosave(f"Location {city.decode()} added.")


def delete_site(body):
    city = field(body, "city")
    if not lib.deleteSite(need_hotel(), city):
        raise BadRequest(f"No location in {city.decode()}.")
    return autosave(f"Location {city.decode()} and all its bookings deleted.")


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
    return autosave(f"Booking for {last.decode()} {first.decode()} added.")


def delete_client(body):
    site = need_site(body)
    last, first = field(body, "last_name"), field(body, "first_name")
    if not lib.deleteClient(site, last, first):
        raise BadRequest(f"No booking for {last.decode()} {first.decode()}.")
    return autosave(f"Booking for {last.decode()} {first.decode()} deleted.")


def load_file(_body):
    need_hotel()
    before = total_clients()
    lib.processingfile(hotel)   # skipped lines get printed in the terminal
    return autosave(f"{total_clients() - before} booking(s) loaded from data/bookings.txt.")


def search(body):
    results = lib.findClients(need_hotel(), str(body.get("prefix", "")).strip().encode())
    found = [{"last_name": text(r.client.contents.last_name), "first_name": text(r.client.contents.first_name),
              "age": r.client.contents.age, "room": r.client.contents.num_room, "city": text(r.city)}
             for r in walk(results)]
    lib.freeResults(results)
    return {"message": f"{len(found)} customer(s) found.", "results": found}


# actions that return a message string, the page gets {message, state, user, chain}
ACTIONS = {
    "signup": signup, "login": login, "logout": logout,
    "new_chain": new_chain, "open_chain": open_chain, "delete_chain": delete_chain,
    "unlock": unlock, "close": close, "set_file_password": set_file_password,
    "add_site": add_site, "delete_site": delete_site,
    "add_client": add_client, "delete_client": delete_client, "load_file": load_file,
}
# actions that return their own dict
RAW_ACTIONS = {"chains": list_chains, "open": open_file, "save": save, "search": search}


class Handler(BaseHTTPRequestHandler):
    def send(self, code, payload, content_type="application/json"):
        data = payload if isinstance(payload, bytes) else json.dumps(payload).encode()
        self.send_response(code)
        self.send_header("Content-Type", content_type)
        self.send_header("Content-Length", str(len(data)))
        self.send_header("Cache-Control", "no-store")
        self.end_headers()
        self.wfile.write(data)

    def do_GET(self):
        if self.path == "/":
            with open(PAGE, "rb") as page:
                self.send(200, page.read(), "text/html; charset=utf-8")
        elif self.path == "/api/state":
            self.send(200, info())
        else:
            self.send(404, {"error": "Not found."})

    def do_POST(self):
        name = self.path.removeprefix("/api/")
        action, raw = ACTIONS.get(name), RAW_ACTIONS.get(name)
        if not action and not raw:
            self.send(404, {"error": "Not found."})
            return
        try:
            length = int(self.headers.get("Content-Length") or 0)
            body = json.loads(self.rfile.read(length) or b"{}") if length <= MAX_BODY else None
            if not isinstance(body, dict):
                raise BadRequest("Bad request." if length <= MAX_BODY else "The file is too big (max 1 MB).")
            answer = {"message": action(body)} if action else raw(body)
            self.send(200, {**answer, **info()})
        except (BadRequest, ValueError, OSError) as error:
            self.send(400, {"error": str(error) or "Bad request.", **info()})


def main():
    os.chdir(ROOT)   # so the C code finds data/
    server = HTTPServer((HOST, PORT), Handler)
    print(f"Hotel chain web app on http://{HOST}:{PORT}  (Ctrl+C to stop)")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        close_chain()


if __name__ == "__main__":
    main()
