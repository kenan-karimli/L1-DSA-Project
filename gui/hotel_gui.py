"""Tkinter GUI for the hotel chain project.

It does not reimplement anything: every operation calls the C functions from
build/libhotel.so (built with `make lib`) through ctypes, and the tables are
filled by walking the same C linked lists.

Run from the repository root:  make gui PYTHON=<a python that has tkinter>
"""

import ctypes
import os
import sys
from ctypes import POINTER, Structure, c_char_p, c_int, c_void_p

try:
    import tkinter as tk
    from tkinter import messagebox, ttk
except ImportError:
    sys.exit("This Python has no tkinter. Run: make gui PYTHON=<python with tkinter>")

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
LIB_PATH = os.path.join(ROOT, "build", "libhotel.so")


# Mirrors of the structs in include/tp3.h (same field order and types)
class Client(Structure):
    pass


Client._fields_ = [
    ("last_name", c_char_p),
    ("first_name", c_char_p),
    ("age", c_int),
    ("num_room", c_int),
    ("next", POINTER(Client)),
]


class Site(Structure):
    pass


Site._fields_ = [
    ("city", c_char_p),
    ("number_clients", c_int),
    ("first", POINTER(Client)),
    ("next", POINTER(Site)),
]


class Hotel(Structure):
    _fields_ = [("name", c_char_p), ("first", POINTER(Site))]


class Result(Structure):
    pass


Result._fields_ = [
    ("client", POINTER(Client)),
    ("city", c_char_p),
    ("next", POINTER(Result)),
]


def load_library():
    if not os.path.exists(LIB_PATH):
        sys.exit(f"{LIB_PATH} not found. Build it first with: make lib")
    lib = ctypes.CDLL(LIB_PATH)
    signatures = {
        "createHotel": ([c_char_p], POINTER(Hotel)),
        "createSite": ([c_char_p], POINTER(Site)),
        "createClient": ([c_char_p, c_char_p, c_int, c_int], POINTER(Client)),
        "addSite": ([POINTER(Hotel), POINTER(Site)], c_int),
        "addClient": ([POINTER(Site), POINTER(Client)], c_int),
        "deleteSite": ([POINTER(Hotel), c_char_p], c_int),
        "deleteClient": ([POINTER(Site), c_char_p, c_char_p], c_int),
        "findSite": ([POINTER(Hotel), c_char_p], POINTER(Site)),
        "findClients": ([POINTER(Hotel), c_char_p], POINTER(Result)),
        "freeResults": ([POINTER(Result)], None),
        "freeClients": ([POINTER(Client)], None),
        "freeHotel": ([POINTER(Hotel)], None),
        "processingfile": ([POINTER(Hotel)], None),
    }
    for name, (args, res) in signatures.items():
        func = getattr(lib, name)
        func.argtypes = args
        func.restype = res
    return lib


def text(raw):
    return raw.decode("utf-8", errors="replace") if raw else ""


def walk(node):
    """Yields each struct of a C linked list, starting from a pointer."""
    while node:
        yield node.contents
        node = node.contents.next


class HotelApp(tk.Tk):
    def __init__(self, lib):
        super().__init__()
        self.lib = lib
        self.libc = ctypes.CDLL(None)
        self.libc.free.argtypes = [c_void_p]
        self.hotel = None

        self.title("Hotel Chain Manager")
        self.geometry("1000x680")
        self.minsize(820, 560)
        self.protocol("WM_DELETE_WINDOW", self.on_close)
        self.build_ui()
        self.refresh()

    # ---------- layout ----------
    def build_ui(self):
        pad = {"padx": 6, "pady": 4}

        top = ttk.Frame(self)
        top.pack(fill="x", **pad)
        self.chain_label = ttk.Label(top, text="No hotel chain", font=("TkDefaultFont", 12, "bold"))
        self.chain_label.pack(side="left")
        ttk.Button(top, text="Load bookings file", command=self.on_load_file).pack(side="right")
        ttk.Button(top, text="Create chain", command=self.on_create_chain).pack(side="right", padx=4)
        self.chain_entry = ttk.Entry(top, width=24)
        self.chain_entry.pack(side="right")
        ttk.Label(top, text="Chain name:").pack(side="right", padx=4)

        middle = ttk.Frame(self)
        middle.pack(fill="both", expand=True, **pad)

        # locations
        left = ttk.LabelFrame(middle, text="Locations")
        left.pack(side="left", fill="both", padx=(0, 6))
        self.sites = self.make_table(left, ("City", "Customers"), (150, 90))
        self.sites.bind("<<TreeviewSelect>>", lambda _e: self.refresh_clients())
        row = ttk.Frame(left)
        row.pack(fill="x", **pad)
        self.city_entry = ttk.Entry(row, width=16)
        self.city_entry.pack(side="left")
        ttk.Button(row, text="Add location", command=self.on_add_site).pack(side="left", padx=4)
        ttk.Button(left, text="Delete selected location", command=self.on_delete_site).pack(fill="x", **pad)

        # customers of the selected location
        right = ttk.LabelFrame(middle, text="Customers")
        right.pack(side="left", fill="both", expand=True)
        self.clients_frame = right
        self.clients = self.make_table(right, ("Last name", "First name", "Age", "Room"), (170, 170, 60, 70))
        form = ttk.Frame(right)
        form.pack(fill="x", **pad)
        self.form = {}
        for i, (label, width) in enumerate((("Last name", 16), ("First name", 16), ("Age", 5), ("Room", 6))):
            ttk.Label(form, text=label).grid(row=0, column=i, sticky="w", padx=2)
            entry = ttk.Entry(form, width=width)
            entry.grid(row=1, column=i, padx=2)
            self.form[label] = entry
        ttk.Button(form, text="Add booking", command=self.on_add_client).grid(row=1, column=4, padx=4)
        ttk.Button(right, text="Delete selected booking", command=self.on_delete_client).pack(fill="x", **pad)

        # search across all locations
        bottom = ttk.LabelFrame(self, text="Search customers by last name (all locations)")
        bottom.pack(fill="both", expand=True, **pad)
        row = ttk.Frame(bottom)
        row.pack(fill="x", **pad)
        ttk.Label(row, text="Last name starts with:").pack(side="left")
        self.search_entry = ttk.Entry(row, width=20)
        self.search_entry.pack(side="left", padx=4)
        self.search_entry.bind("<Return>", lambda _e: self.on_search())
        ttk.Button(row, text="Search", command=self.on_search).pack(side="left")
        self.results = self.make_table(bottom, ("Last name", "First name", "Age", "Room", "City"),
                                       (170, 170, 60, 70, 150), height=6)

        self.status = ttk.Label(self, text="", relief="sunken", anchor="w")
        self.status.pack(fill="x", side="bottom")

    @staticmethod
    def make_table(parent, columns, widths, height=10):
        frame = ttk.Frame(parent)
        frame.pack(fill="both", expand=True, padx=6, pady=4)
        tree = ttk.Treeview(frame, columns=columns, show="headings", height=height, selectmode="browse")
        for col, width in zip(columns, widths):
            tree.heading(col, text=col)
            tree.column(col, width=width, anchor="w")
        scroll = ttk.Scrollbar(frame, orient="vertical", command=tree.yview)
        tree.configure(yscrollcommand=scroll.set)
        tree.pack(side="left", fill="both", expand=True)
        scroll.pack(side="right", fill="y")
        return tree

    # ---------- helpers ----------
    def say(self, message):
        self.status.config(text=" " + message)

    def need_hotel(self):
        if not self.hotel:
            self.say("Create a hotel chain first.")
        return bool(self.hotel)

    def selected_city(self):
        selection = self.sites.selection()
        return self.sites.item(selection[0], "values")[0] if selection else None

    def selected_site(self):
        city = self.selected_city()
        if not self.hotel or city is None:
            return None
        site = self.lib.findSite(self.hotel, city.encode())
        return site if site else None

    def total_clients(self):
        return sum(site.number_clients for site in walk(self.hotel.contents.first)) if self.hotel else 0

    # ---------- refresh ----------
    def refresh(self, select_city=None):
        keep = select_city or self.selected_city()
        self.sites.delete(*self.sites.get_children())
        if self.hotel:
            self.chain_label.config(text=f"Hotel chain: {text(self.hotel.contents.name)}")
            for site in walk(self.hotel.contents.first):
                city = text(site.city)
                item = self.sites.insert("", "end", values=(city, site.number_clients))
                if city == keep:
                    self.sites.selection_set(item)
        else:
            self.chain_label.config(text="No hotel chain")
        self.refresh_clients()

    def refresh_clients(self):
        self.clients.delete(*self.clients.get_children())
        site = self.selected_site()
        if not site:
            self.clients_frame.config(text="Customers (select a location)")
            return
        self.clients_frame.config(text=f"Customers at {text(site.contents.city)}")
        for client in walk(site.contents.first):
            self.clients.insert("", "end", values=(text(client.last_name), text(client.first_name),
                                                   client.age, client.num_room))

    # ---------- actions ----------
    def on_create_chain(self):
        name = self.chain_entry.get().strip()
        if not name:
            self.say("Type a chain name first.")
            return
        if self.hotel and not messagebox.askyesno(
                "Replace chain", "This deletes the current chain and all its bookings. Continue?"):
            return
        new_hotel = self.lib.createHotel(name.encode())
        if not new_hotel:
            self.say("Out of memory.")
            return
        if self.hotel:
            self.lib.freeHotel(self.hotel)
        self.hotel = new_hotel
        self.results.delete(*self.results.get_children())
        self.refresh()
        self.say(f"Hotel chain {name} created.")

    def on_add_site(self):
        city = self.city_entry.get().strip()
        if not self.need_hotel():
            return
        if not city:
            self.say("Type a city first.")
            return
        if self.lib.findSite(self.hotel, city.encode()):
            self.say(f"Location {city} already exists.")
            return
        site = self.lib.createSite(city.encode())
        if not site:
            self.say("Out of memory.")
            return
        if not self.lib.addSite(self.hotel, site):
            # same cleanup as the C menu does when addSite refuses a site
            self.libc.free(ctypes.cast(site.contents.city, c_void_p))
            self.libc.free(ctypes.cast(site, c_void_p))
            self.say(f"Location {city} could not be added.")
            return
        self.city_entry.delete(0, "end")
        self.refresh(select_city=city)
        self.say(f"Location {city} added.")

    def on_delete_site(self):
        city = self.selected_city()
        if not self.need_hotel() or city is None:
            self.say("Select a location first.")
            return
        if not messagebox.askyesno("Delete location", f"Delete {city} and all its bookings?"):
            return
        if self.lib.deleteSite(self.hotel, city.encode()):
            self.say(f"Location {city} and all its bookings deleted.")
        else:
            self.say(f"No location in {city}.")
        self.results.delete(*self.results.get_children())
        self.refresh()

    def on_add_client(self):
        site = self.selected_site()
        if not self.need_hotel():
            return
        if not site:
            self.say("Select a location first.")
            return
        last = self.form["Last name"].get().strip()
        first = self.form["First name"].get().strip()
        try:
            age = int(self.form["Age"].get())
            room = int(self.form["Room"].get())
        except ValueError:
            self.say("Age and room must be whole numbers.")
            return
        if not last or not first:
            self.say("Type a last name and a first name.")
            return
        if not 0 <= age <= 150 or room < 1:
            self.say("Age must be 0-150 and room at least 1.")
            return
        client = self.lib.createClient(last.encode(), first.encode(), age, room)
        if not client:
            self.say("Out of memory.")
            return
        if not self.lib.addClient(site, client):
            self.lib.freeClients(client)
            self.say(f"{last} {first} already has a booking here.")
            return
        for entry in self.form.values():
            entry.delete(0, "end")
        self.refresh()
        self.say(f"Booking for {last} {first} added.")

    def on_delete_client(self):
        site = self.selected_site()
        selection = self.clients.selection()
        if not site or not selection:
            self.say("Select a booking first.")
            return
        last, first = self.clients.item(selection[0], "values")[:2]
        if self.lib.deleteClient(site, last.encode(), first.encode()):
            self.say(f"Booking for {last} {first} deleted.")
        self.results.delete(*self.results.get_children())
        self.refresh()

    def on_search(self):
        self.results.delete(*self.results.get_children())
        prefix = self.search_entry.get().strip()
        if not self.need_hotel():
            return
        results = self.lib.findClients(self.hotel, prefix.encode())
        count = 0
        for result in walk(results):
            client = result.client.contents
            self.results.insert("", "end", values=(text(client.last_name), text(client.first_name),
                                                   client.age, client.num_room, text(result.city)))
            count += 1
        self.lib.freeResults(results)
        self.say(f"{count} customer(s) found.")

    def on_load_file(self):
        if not self.need_hotel():
            return
        before = self.total_clients()
        self.lib.processingfile(self.hotel)   # its line-by-line report goes to the terminal
        self.refresh()
        self.say(f"{self.total_clients() - before} booking(s) loaded from data/bookings.txt.")

    def on_close(self):
        if self.hotel:
            self.lib.freeHotel(self.hotel)
            self.hotel = None
        self.destroy()


def main():
    os.chdir(ROOT)   # processingfile opens data/bookings.txt relative to the repo root
    HotelApp(load_library()).mainloop()


if __name__ == "__main__":
    main()
