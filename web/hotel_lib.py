"""Load our C code (build/libhotel.so, run `make lib` first) with ctypes.

Used by both hotel_gui.py and hotel_web.py.
"""

import ctypes
import os
import sys
from ctypes import POINTER, Structure, c_char_p, c_int

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
LIB_PATH = os.path.join(ROOT, "build", "libhotel.so")
SAVE_FILE = "data/saved_chain.txt"   # has to match SAVE_FILE in web.h


# same structs as in submit/tp3.h, the order of fields has to be exactly the same!
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
    """Load build/libhotel.so and set the argument/return types of every C function."""
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
        "loadBookings": ([POINTER(Hotel), c_char_p], None),
        "saveHotel": ([POINTER(Hotel), c_char_p], c_int),
        "saveChainFile": ([POINTER(Hotel), c_char_p, c_char_p], c_int),
    }
    for name, (args, res) in signatures.items():
        func = getattr(lib, name)
        func.argtypes = args
        func.restype = res
    return lib


def text(raw):
    """Turn a C string (bytes or None) into a Python str."""
    return raw.decode("utf-8", errors="replace") if raw else ""


def walk(node):
    """Yield every node of a C linked list, like for (temp = head; temp; temp = temp->next)."""
    while node:
        yield node.contents
        node = node.contents.next
