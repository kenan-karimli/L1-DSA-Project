# Handoff: Hotel Chain Manager, web version (UFAZ deck style)

## Overview
This is a redesign of the web version in this repo (`gui/hotel_web.html` + `gui/hotel_web.py`). It adds:
- an animated start screen where you choose **Create a chain** or **Load from file**
- a **login and password for each chain**: you set them when you create a chain, and you type them when you load a saved file
- file validation, with a clear error when you pick a file in the wrong format
- **Save** writes a file named after the chain, for example `UFAZ_Hotels.txt`
- a new look that matches the project deck: cream background, forest green and terracotta colors, Instrument Serif, DM Sans and DM Mono fonts, Phosphor Duotone icons

## About the design files
The files in `design/` are **HTML design references**. They show the intended look and behavior and are not production code. `Hotel Chain Manager v5.dc.html` runs entirely in the browser: `hotel_sim.js` simulates the C lists and the Python API.

**Task:** recreate this design in the existing repo:
- Plain HTML/CSS/vanilla JS in `gui/hotel_web.html`, with no build step and no npm. Keep it like the current file.
- Python standard library only in `gui/hotel_web.py`.
- The C code in `src/` stays the single owner of the linked lists. Python only calls C through `hotel_lib.py`, as today.
- Follow the rules in the repo README:
  - `main()` exists only in `src/main.c`.
  - No `scanf` in the list functions.
  - Every `malloc` has a matching `free`. Run `make debug` before merging.
  - `make test` must pass.

## Fidelity
**High fidelity.** Colors, type, spacing, radii, copy and animations are final. Recreate them exactly.

---

## Screens

The page has one root and swaps between 5 phases: `intro`, `create`, `login`, `setup` and `app`. A full-screen "veil" transition plays between phases (see *Interactions*).

### 1. Intro (start screen)
- Full viewport on `#f5ede0`, with `overflow:hidden`.
- Two decorative circles slowly drift (`drift` keyframes, 14s and 18s, ease-in-out, infinite):
  - a filled `#ebe4d2` circle, 520px, at top right (−120px, −140px)
  - a `1px #d8cfbc` outline circle, 460px, at bottom left
- The content column is max 1080px wide, padding `24px 40px 64px`, vertically centered, with a 48px gap.
- **Top line:** "UFAZ · L1 Data Structures & Algorithms · Final project", 13px, `#6b675d`, fades in over .8s.
- **Kicker:** "Hotel chain management in C", 14px, `#c3512d`.
- **Title:** "Hotel Chain" on one line and "*Manager*" (italic, `#2e4a3a`) below it. Instrument Serif 400, `clamp(56px,10vw,128px)/.95`, letter-spacing −.02em.
- **Lead:** "Locations and bookings kept in sorted linked lists. Each chain is protected by its own login and password.", 16px, `#6b675d`, max 520px wide.
- **Two option cards** in a grid of `repeat(auto-fit,minmax(min(100%,300px),1fr))`, gap 16, max 760px wide:
  - **Create a chain:** background `#2e4a3a`, text `#f5ede0`, radius 16, padding 24, gap 28. Icon `ph-buildings` at 32px. Title in Instrument Serif 32px. Subtitle "Start empty and set a login and password." in 14px `#c9d3cb`.
  - **Load from file:** background `#fbf6ec`, border `1px #d8cfbc`, radius 16. Icon `ph-folder-open` at 32px in `#c3512d`. Subtitle "Open a saved chain or a bookings .txt file." in `#6b675d`.
  - Hover on either card: `translateY(-3px)`, .2s.
- **Entrance stagger:** each element uses `rise` (opacity 0→1, translateY 24px→0, `cubic-bezier(.2,.7,.2,1)`):
  - kicker: .1s delay, .6s
  - title: .2s delay, .8s
  - lead: .35s delay, .8s
  - card 1: .5s delay, .7s
  - card 2: .62s delay, .7s
- **Error banner** (shown below the cards when a file is rejected): see *Error banner* under Shared components.

### 2. Create a chain
- A centered card column, max 440px wide, gap 20. It enters with `rise` .5s.
- **Back** button: ghost, 32px tall, `ph-arrow-left`, 13px `#6b675d`.
- **Kicker** "New chain" in `#c3512d`. **H1** "Create a chain" in Instrument Serif 48/1. Helper text: "You'll need this login and password to open the saved file later."
- **Fields** (label 13px `#6b675d` above each input; input 44px tall, radius 22, border `1px #d8cfbc`, background `#fbf6ec`, 15px DM Sans):
  - Chain name (placeholder "e.g. UFAZ Hotels")
  - Login ("e.g. hotel_admin")
  - Password ("At least 4 characters")
  - Confirm password ("Repeat the password")
- Error banner, if any.
- **Primary button** "Create chain" with `ph-buildings`: 48px tall, radius 24, background `#2e4a3a`, text `#f5ede0`, 15px/500. Hover background `#233a2d`.
- Pressing Enter in any field submits.

### 3. Login (a file saved from this app was chosen)
- Same card layout as Create.
- **Kicker:** `ph-lock-key` icon plus the file name. **H1:** the chain name read from the file. **Info:** "N location(s), M booking(s)."
- **Fields:** Login, Password.
- **Button:** "Open chain" with `ph-lock-key-open`.
- A wrong pair shows the error banner "Wrong login or password." with a shake.

### 4. Setup (a plain bookings file with no `#HOTEL` line was chosen)
- **Kicker:** `ph-file-text` icon plus the file name. **H1:** "Protect this chain". Info: "N location(s), M booking(s). This file has no login yet. Name the chain and set one."
- **Fields:** Chain name (pre-filled from the file name, with `_` and `-` turned into spaces), Login, Password, Confirm password.
- **Button:** "Open chain".

### 5. App (main screen)
This is the layout from `Hotel Chain Manager v4/v5`. The content is max 1280px wide with 40px side padding.

**Top bar**
- Left: the UFAZ line.
- Right: three pill buttons, 36px tall, radius 18, border `1px #d8cfbc`, 13px/500, each with a terracotta icon. Hover background `#ebe3d1`.
  - "Add sample bookings" (`ph-file-arrow-up`): runs the existing `load_file` action with `data/bookings.txt`
  - "Save" (`ph-floppy-disk`): after the first save the label becomes "Save to <file>"
  - "Lock" (`ph-sign-out`)

**Header** (padding `40px 40px 32px`)
- Kicker "Hotel chain".
- H1: the chain name, Instrument Serif `clamp(48px,7vw,84px)/1`.
- Sub line: "X locations · Y customers".
- Right side: a "Signed in as **login**" chip. Background `#ebe4d2`, 36px tall, radius 18, with a 26px green avatar circle showing `ph-user`.

**Locations section**
- A `1px #d8cfbc` rule on top. Kicker "hotel->first · sorted by city", H2 "Locations" (serif 36). Right-hand helper: "Select a location to manage its bookings."
- **Card grid:** `repeat(auto-fill,minmax(190px,1fr))`, gap 12.
  - **Location card:** radius 12, padding `16px 16px 14px`, gap 18. Top row: `ph-map-pin` at 22px, and "N customer(s)" in DM Mono 12px. City name below in Instrument Serif `30px/1.2`, single line with ellipsis.
    - Normal: background `#fbf6ec`, border `#e2d9c6`, terracotta icon.
    - Selected: background `#2e4a3a`, text `#f5ede0`, sub text `#c9d3cb`.
    - Hover: border `#2e4a3a`.
  - The **last card** is dashed (`1px dashed #c9bfaa`) and holds "New location" with `ph-plus-circle`, a City input and a 36px round green "+" button.

**Customers panel**
- Flex `999 1 520px`, background `#ebe4d2`, radius 12, padding 24.
- Kicker "site->first". H2 "Customers at <city>", or "Customers (select a location)" when none is selected.
- "Delete location" outline pill when a location is selected: border `#d9a38f`, text `#a8411f`, hover background `#f3d9cc`.
- **Table** on `#f5ede0`, radius 8:
  - headers `last_name / first_name / age / num_room` in DM Mono 12px `#6b675d`
  - rows have 11px vertical padding and a `#e7dfcd` divider
  - age and room are right-aligned in DM Mono 13px
  - each row has a trash icon button (32px round, hover `#f3d9cc` / `#a8411f`)
- **Empty states:** "Pick a location in the list above." (`ph-hand-pointing`) and "No customers yet." (`ph-user-circle-dashed`).
- **New booking row:** caption "New booking · inserted in sorted order". Pill inputs Last name / First name (flex 130px), Age (80px), Room (92px), then the green "Add booking" button.

**Search panel**
- Flex `1 1 340px`, same panel style. Kicker "findClients", H2 "Search by last name", helper "All locations, not case sensitive."
- **Pill search field:** the wrapper carries the border and focus state. The inner input has **no border or outline** (this was a bug that has been fixed: use `:focus-within` on the wrapper). Then a "Search" button.
- **Results:** cards on `#f5ede0`, radius 8, each with `ph-user`, "LAST First", "age N · room N" in mono, and a green city chip (`ph-map-pin`). Below them a "Clear results" text button. Empty result: "No customer matches that prefix." with `ph-prohibit`.

### Shared components

**Error banner**
- Background `#f3d9cc`, text `#7a2e15`, radius 12, padding `12px 14px`, with a `ph-warning-circle` icon.
- Plays `shake` for .45s each time a new error appears (re-key the element).

**Toast** (fixed, bottom-left 24px)
- Radius 24, padding `10px 8px 10px 16px`, DM Mono 13px.
- Background `#2e4a3a` with `ph-check-circle` for success; `#a8411f` with `ph-warning-circle` for errors. A round close button.
- Enters with `toastIn` (.2s, from translateY 10px).
- Messages are exactly those returned by `hotel_web.py`.

**Confirm dialog**
- Scrim `rgba(42,41,37,.35)`. Card on `#f5ede0`, radius 16, padding 28, `ph-warning-circle` at 32px in terracotta.
- Heading "Are you sure?" in serif 32. Buttons: Cancel (outline) and Delete (`#c3512d`).
- Used for "Delete <city> and all its bookings?".

---

## Interactions and behavior

### Veil transition (between every phase)
1. A fixed full-screen layer (z-index 60) with a color, a 56px Phosphor icon and a serif label (`clamp(36px,6vw,64px)`).
2. **Grow:** `clip-path: circle(0 at X Y)` → `circle(R at X Y)`, where (X, Y) is the center of the clicked button (or the viewport center for an Enter key) and R is the distance to the farthest corner + 20. Duration 620ms, `cubic-bezier(.75,0,.25,1)`.
3. The label and icon enter at 300ms delay over 480ms: opacity 0→1, translateY 22px→0, scale .94→1, blur 6px→0. When `spin` is set, the icon also goes rotate(−25deg) scale(.6) → rotate(8deg) scale(1.1) → none over 700ms.
4. Once the grow finishes, **swap the phase**. Hold for 520ms if there is a label, 40ms if not.
5. **Wipe out:** `clip-path: inset(0 0 0 0)` → `inset(0 0 100% 0)` (the veil lifts upward) over 560ms with the same easing. At the same time the label goes to opacity 0 and translateY −40px over 360ms, ease-in.
6. Block clicks while the veil runs. Skip the whole effect when `prefers-reduced-motion: reduce`.
7. Use the Web Animations API (`el.animate`).

| Trigger | Veil color / text color | Icon | Label | Spin |
|---|---|---|---|---|
| Create a chain card | `#2e4a3a` / `#f5ede0` | ph-buildings | New chain | no |
| Valid saved file chosen | `#c3512d` / `#fff7ef` | ph-lock-key | chain name from file | yes |
| Valid plain bookings file chosen | `#c3512d` / `#fff7ef` | ph-file-text | file name | no |
| Login / create / setup succeeded | `#2e4a3a` / `#f5ede0` | ph-lock-key-open | Welcome, <login> | yes |
| Lock | `#c3512d` / `#fff7ef` | ph-lock-key | Locked | yes |
| Back | `#ebe4d2` / `#2a2925` | none | none | no |

**App entrance after the veil:** the root fades in over .5s, the header uses `rise` .6s with .05s delay, and main uses `rise` .7s with .15s delay.

### Load from file
- Use `showOpenFilePicker` when it's available, so the same file handle can be reused for saving. Otherwise fall back to a hidden `<input type="file" accept=".txt,.csv,text/plain">`.
- **Rejections** (shown in the intro error banner, no phase change):
  - not `.txt` or `.csv`: `"<name>" is not a .txt file. Choose a hotel chain file saved from this app or a bookings .txt file.`
  - larger than 1 MB: `The file is too big (max 1 MB).`
  - fails the format check: `"<name>" is not in the right format. Line N: <reason>`
- **Format reasons** (the first failing line wins):
  - `The file is empty.`
  - `This is not a text file.` (a NUL byte was found)
  - `the #HOTEL line must be the first line.`
  - `the #HOTEL line is damaged.`
  - `the #SITE line is damaged.`
  - `expected 5 values separated by ";" (last name;first name;age;room;city), found K.`
  - `names and city must be 1 to 49 bytes.`
  - `age "x" must be a whole number from 0 to 150.`
  - `room "x" must be a whole number from 1 to 99999.`
  - `LAST First is booked twice in City.`
  - `No bookings found in this file.` (a file without a `#HOTEL` line must have at least 1 booking)
- A file with a valid `#HOTEL` line goes to **Login**. A valid file without one goes to **Setup**.

### Login, create and setup validation
- Chain name and login: 1–49 bytes, no `;`, no newline.
- Password: at least 4 characters. Confirm password must match.
- Error copy:
  - `Type a chain name (up to 49 bytes, no ";").`
  - `Type a login (up to 49 bytes, no ";").`
  - `The password must be at least 4 characters.`
  - `The two passwords are different.`
  - `Type your login and password.`
  - `Wrong login or password.`

### Save
- File name: the chain name with every run of characters outside `[A-Za-z0-9_-]` replaced by `_`, plus `.txt`. For example "UFAZ Hotels" becomes `UFAZ_Hotels.txt`.
- In the real app, **the server writes it to `data/chains/<file>` through C** (the user asked for the file to appear in the project files) and also returns the text so the browser can download it. If the browser has `showSaveFilePicker`, the first save asks where to save and later saves write to the same handle silently. If the chain was opened through `showOpenFilePicker`, save writes back to that same file.
- Toast: `N booking(s) saved to <file>.`

### Lock
Frees the hotel (`freeHotel`), clears the session and goes back to Intro with the "Locked" veil.

### Everything else
Adding and deleting locations and bookings, search, and the messages are **unchanged** from the current `hotel_web.py`. Keep the same validation and messages.

---

## Chain file format (new)
```
#HOTEL;<chain name>;<login>;<password hash>
#SITE;<city>            one line per location, including empty ones
LastName;FirstName;Age;Room Number;City
<last>;<first>;<age>;<room>;<city>
...
```
- The prototype stores `sha256(login + ":" + password)` as 64 hex characters. **For the real app, use a salted hash** from Python's standard library: `hashlib.pbkdf2_hmac("sha256", password, salt, 200_000)`, stored as `pbkdf2$200000$<salt hex>$<hash hex>` (contains no `;`). Compare with `hmac.compare_digest`. Regenerate `data/UFAZ_Hotels.txt` in that format (the prototype sample uses login `ufaz`, password `ufaz2026`).
- Plain `data/bookings.txt` (no `#` lines) must keep loading as before.

## Implementation plan for the repo

### C (`src/file.c`, `include/file.h`, plus tests)
1. In `loadBookings`: lines that start with `#` are not bookings.
   - `#SITE;<city>` creates the location if it's missing (`findSite`, `createSite`, `addSite`, freeing on failure as in the existing code).
   - `#HOTEL;...` is ignored by C, because Python reads it.
   - Any other `#` line is skipped. Keep the existing `LastName;` header skip.
2. Add `int saveChainFile(T_Hotel *hotel, const char *filename, const char *login, const char *passHash)`:
   - writes `#HOTEL;name;login;hash`, then one `#SITE;city` line for every site, then the same body as `saveHotel`
   - share the body through a static helper so `saveHotel` keeps its behavior for menu option 11
   - returns the number of bookings written, or -1
3. Add tests in `tests/test_file.c`:
   - round trip: save, load, and get the same sites (including empty ones) and the same clients
   - `#` lines are skipped
4. Run `make test` and `make debug`. `make submit` must still compile.

### Python (`gui/hotel_lib.py`, `gui/hotel_web.py`)
- Register `saveChainFile` in `hotel_lib.signatures`: `([POINTER(Hotel), c_char_p, c_char_p, c_char_p], c_int)`.
- Server state:
  - `hotel` (as today)
  - `session = {"login", "hash"}`, or `None` when locked
  - `pending = {"meta", "sites", "count", "filename"}` between choosing a file and logging in
- **Endpoints** (all POST with JSON, responses `{message|error, state, ...}` as today):
  - `create {name, login, password}`: validate, hash, `createHotel`, set the session.
  - `open {filename, text}`: run the format check (port the rules above from `hotel_sim.js`, `hotelFile.parse`), store `pending`, return `{needs: "login"|"setup", name, locations, bookings}` or a 400 error.
  - `login {login, password}`: verify against `pending.meta`, build the hotel with `createHotel` / `createSite` / `addSite` / `createClient` / `addClient` from the parsed rows (free on failure), set the session.
  - `setup {name, login, password}`: same as login, but takes the name and credentials from the body.
  - `save`: `saveChainFile` to `data/chains/<safe>.txt` (create the folder), then return `{message, filename, text}`.
  - `lock`: `freeHotel`, `session = None`.
  - The existing `add_site`, `delete_site`, `add_client`, `delete_client`, `search` and `load_file` actions now also need a session. Return `400 "Log in first."` otherwise.
  - Remove `load_saved` (replaced by Load from file).
  - `GET /api/state` returns `{state, user}` so a page reload stays in the app while the server is unlocked.
- Raise the request body limit from 10 000 bytes to about 1.1 MB for `open`.

### Front end (`gui/hotel_web.html`)
- Rebuild the screens above in plain HTML, CSS and JS. Here, unlike the prototype, normal CSS classes are fine.
- Load the fonts from Google Fonts (`Instrument Serif` 400 and italic, `DM Sans` 400/500/600, `DM Mono` 400/500) and the icons from `https://unpkg.com/@phosphor-icons/web@2.1.1/src/duotone/style.css`.
- Keep the dark-mode media query optional. The design is light only.
- Replace `confirm()` with the confirm dialog.

## Design tokens
**Colors**
- Background: `#f5ede0`
- Panel: `#ebe4d2`
- Field and card surface: `#fbf6ec`, focused field `#fffaf2`
- Line: `#d8cfbc`; card border `#e2d9c6`; dashed line `#c9bfaa`; table divider `#e7dfcd`; table header line `#ddd3bf`
- Ink: `#2a2925`; muted `#6b675d`; placeholder `#9a9384`
- Forest (primary): `#2e4a3a`; hover `#233a2d`; on-forest muted `#c9d3cb`
- Terracotta (accent and labels): `#c3512d`; dark `#a8411f`; danger border `#d9a38f`
- Error surface: `#f3d9cc`; error text `#7a2e15`

**Type**
- Display: Instrument Serif 400. Intro title clamp 56–128; app H1 clamp 48–84; screen H1 48; section H2 36; panel H2 32; veil clamp 36–64; card city 30.
- UI: DM Sans. Body 15/1.5; buttons 13–15/500; kickers 13 terracotta.
- Data: DM Mono. Table headers 12; numbers 13; toast 13; counts 12.

**Radii:** pills = height/2 (18, 20, 22, 24); panels 12; option cards 16; dialog 16; list items 8; error banner 12.

**Shadows:** toast `0 10px 30px rgba(46,74,58,.25)`; dialog `0 24px 60px rgba(42,41,37,.25)`.

**Keyframes**
- `rise`: opacity 0, translateY(24px) → none
- `fade`: opacity 0 → 1
- `shake`: ±2/4/6px horizontal steps over .45s
- `drift`: translate(12px,−10px) rotate(4deg) at 50%
- `toastIn`: opacity 0, translateY(10px) → none

## Assets
- Icons: Phosphor Duotone web font (unpkg CDN above). Icons used: buildings, folder-open, arrow-left, lock-key, lock-key-open, file-text, file-arrow-up, floppy-disk, sign-out, user, map-pin, plus, plus-circle, trash, hand-pointing, user-circle-dashed, user-plus, magnifying-glass, prohibit, warning-circle, check-circle, x.
- Fonts: Google Fonts.
- No images.

## Files in this bundle
- `design/Hotel Chain Manager v5.dc.html`: the main reference. Open it in a browser, keeping `support.js` and `hotel_sim.js` next to it.
- `design/Hotel Chain Manager v4.dc.html`: the same app screen without the auth flow (for reference).
- `design/hotel_sim.js`:
  - the in-browser simulation of the API and the C rules
  - `hotelFile.parse` is the **reference validator**, to port to Python
  - `hotelVals` holds all the app-screen handlers and messages
- `design/support.js`: the runtime needed to open the `.dc.html` files.
- `design/data/UFAZ_Hotels.txt`: a sample saved chain in the prototype format (login `ufaz`, password `ufaz2026`).
