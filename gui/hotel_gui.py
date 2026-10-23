# tkinter GUI for our hotel project
# all the real work is done by our C functions (libhotel.so), python just calls them
# with ctypes and shows the linked lists in tables
#
# run with: make gui PYTHON=<python with tkinter>

import ctypes
import os
import sys
from ctypes import c_void_p

try:
    import tkinter as tk
    from tkinter import messagebox, ttk
except ImportError:
    sys.exit("This Python has no tkinter. Run: make gui PYTHON=<python with tkinter>")

from hotel_lib import ROOT, SAVE_FILE, load_library, text, walk


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

    # building the window
    def build_ui(self):
        pad = {"padx": 6, "pady": 4}

        top = ttk.Frame(self)
        top.pack(fill="x", **pad)
        self.chain_label = ttk.Label(top, text="No hotel chain", font=("TkDefaultFont", 12, "bold"))
        self.chain_label.pack(side="left")
        ttk.Button(top, text="Load saved", command=self.on_load_saved).pack(side="right")
        ttk.Button(top, text="Save", command=self.on_save).pack(side="right", padx=4)
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

        # customers of the location you clicked on
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

        # search (in all locations)
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

    # redraw tables after every change
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

    # button handlers
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
            # addSite failed so we have to free it ourselves (like in main.c)
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
        self.lib.processingfile(self.hotel)   # it prints skipped lines in the terminal, not in the gui
        self.refresh()
        self.say(f"{self.total_clients() - before} booking(s) loaded from data/bookings.txt.")

    def on_save(self):
        if not self.need_hotel():
            return
        written = self.lib.saveHotel(self.hotel, SAVE_FILE.encode())
        self.say(f"Cannot write {SAVE_FILE}." if written < 0 else f"{written} booking(s) saved to {SAVE_FILE}.")

    def on_load_saved(self):
        if not self.need_hotel():
            return
        before = self.total_clients()
        self.lib.loadBookings(self.hotel, SAVE_FILE.encode())
        self.refresh()
        self.say(f"{self.total_clients() - before} booking(s) loaded from {SAVE_FILE}.")

    def on_close(self):
        if self.hotel:
            self.lib.freeHotel(self.hotel)
            self.hotel = None
        self.destroy()


def main():
    os.chdir(ROOT)   # so data/bookings.txt is found
    HotelApp(load_library()).mainloop()


if __name__ == "__main__":
    main()
