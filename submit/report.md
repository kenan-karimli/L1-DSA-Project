# DSA1 Project - Hotel chain management (report draft)

Team: ...

## 1. Organisation

- `tp3.h`: structures `T_Client`, `T_Site`, `T_Hotel`, `T_Result`, constants and prototypes.
- `tp3.c`: all list functions.
- `main.c`: the menu (options 1-9 from the subject, plus 10 to load the bookings file).

All names are copied with `malloc` (`copyString`), so the caller can reuse its buffers.
Every node and every string is freed when it is deleted (checked with AddressSanitizer).

## 2. Additional structure

**`T_Result`** (`client`, `city`, `next`): one node of a search result.
`searchClients` must print clients from *all* sites sorted by name, but each client list is
only sorted inside its own site. So we chose to create a specific structure instead of adapting
`T_Client`: we build a new sorted list of `T_Result` nodes that only *point* to the existing
clients and city names. Nothing is copied, the hotel lists are not changed, and freeing the
result list (`freeResults`) only frees the result nodes.

## 3. Additional functions

| Function | Why |
|---|---|
| `copyString` | `strdup` is not in standard C11; copies a name with `malloc`. |
| `compareNames` | Compare by last name then first name; used by add, delete and search. |
| `findSite` | Find a site by city; used by the menu and by `addBooking`. |
| `findClients` | Builds the sorted `T_Result` list, so `searchClients` only prints it. |
| `addBooking` | Adds one booking and creates the site if needed (used by `processingfile`). |
| `freeResults`, `freeClients`, `freeHotel` | Free memory. |

In `main.c`: `readLine`, `readText`, `readInt` read input safely with `fgets`
(empty text, letters instead of numbers and too long lines are refused), and one small
function per menu option.

Choices: the same person (same last and first name) cannot be booked twice in the same site,
otherwise `deleteClient` would not know which one to delete. The search is not case sensitive.

## 4. Time complexity

s = number of sites, n = clients in one site, N = clients in the whole chain,
k = number of search results, m = lines in the file. String copies/comparisons are
considered O(1) (names are short).

| Function | Complexity | Reason |
|---|---|---|
| `createClient`, `createSite`, `createHotel` | O(1) | one `malloc` + copies |
| `addClient` | O(n) | walk the list to find the sorted position |
| `addSite` | O(s) | same, by city |
| `displayHotel` | O(s) | one line per site |
| `displaySite` | O(n) | one line per client |
| `deleteClient` | O(n) | find the client, then O(1) unlink |
| `deleteSite` | O(s + n) | find the site, then free its n clients |
| `findSite` | O(s) | linear search |
| `searchClients` / `findClients` | O(N + k²) | visit every client; each sorted insert in the result list is O(k) |
| `addBooking` | O(s + n) | `findSite` + `addClient` |
| `processingfile` | O(m (s + n)) | one `addBooking` per line |
| `freeClients` / `freeHotel` | O(n) / O(s + N) | every node is freed once |
