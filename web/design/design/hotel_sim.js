// Browser-side stand-in for gui/hotel_web.py + the C lists (same rules and messages).
(function () {
  const MAX_TEXT = 49, SAVE_FILE = 'data/saved_chain.txt', SAVE_KEY = 'ufaz-hotel-saved-chain';
  const BOOKINGS = `ANTOINE;Michel;40;38;Bordeaux
MARTIN;Lucie;32;45;Lille
DUPOND;Antoine;42;25;Toulouse
LECLERC;Denis;25;112;Lyon
DUPOND;Agathe;23;12;Bordeaux
DUPOND;François;35;29;Paris
DUPONT;Amélie;28;17;Lille
DURAND;Antoine;42;15;Paris
DUVAL;Magali;40;38;Bordeaux
MARTIN;Alain;42;112;Paris`;
  const cmp = (a, b) => (a < b ? -1 : a > b ? 1 : 0);
  const cmpNames = (a, b) => cmp(a.last_name, b.last_name) || cmp(a.first_name, b.first_name);
  class Bad extends Error {}
  const G = window.__hotelSim = window.__hotelSim || { hotel: null };
  let hotel = G.hotel;

  const label = k => k.replace('_', ' ');
  function field(b, k) {
    const v = String(b[k] ?? '').trim();
    if (!v) throw new Bad(`Type a ${label(k)}.`);
    if (new TextEncoder().encode(v).length > MAX_TEXT || /[;\r\n]/.test(v))
      throw new Bad(`The ${label(k)} must be at most ${MAX_TEXT} bytes, without ';'.`);
    return v;
  }
  function number(b, k, lo, hi) {
    const s = String(b[k] ?? '').trim(), v = /^[+-]?\d+$/.test(s) ? parseInt(s, 10) : null;
    if (v === null || v < lo || v > hi) throw new Bad(`${k[0].toUpperCase() + k.slice(1)} must be a whole number from ${lo} to ${hi}.`);
    return v;
  }
  const needHotel = () => { if (!hotel) throw new Bad('Create a hotel chain first.'); return hotel; };
  const findSite = city => needHotel().sites.find(s => s.city === city);
  const needSite = b => { const s = findSite(field(b, 'city')); if (!s) throw new Bad('No such location.'); return s; };
  const total = () => hotel ? hotel.sites.reduce((n, s) => n + s.clients.length, 0) : 0;
  function insertSite(city) {
    const s = { city, clients: [] }; hotel.sites.push(s); hotel.sites.sort((a, b) => cmp(a.city, b.city)); return s;
  }
  function insertClient(site, c) {
    if (site.clients.some(x => cmpNames(x, c) === 0)) return false;
    site.clients.push(c); site.clients.sort(cmpNames); return true;
  }
  function loadText(txt) {
    for (const line of txt.split('\n').slice(0)) {
      const p = line.trim().split(';');
      if (p.length !== 5 || p[0] === 'LastName') continue;
      const age = parseInt(p[2], 10), room = parseInt(p[3], 10);
      if (isNaN(age) || isNaN(room)) continue;
      const site = findSite(p[4]) || insertSite(p[4]);
      insertClient(site, { last_name: p[0], first_name: p[1], age, room });
    }
  }
  const state = () => hotel && JSON.parse(JSON.stringify(hotel));
  const A = {
    create(b) { const name = field(b, 'name'); hotel = { name, sites: [] }; return `Hotel chain ${name} created.`; },
    add_site(b) {
      const city = field(b, 'city');
      if (findSite(city)) throw new Bad(`Location ${city} already exists.`);
      insertSite(city); return `Location ${city} added.`;
    },
    delete_site(b) {
      const city = field(b, 'city'), i = needHotel().sites.findIndex(s => s.city === city);
      if (i < 0) throw new Bad(`No location in ${city}.`);
      hotel.sites.splice(i, 1); return `Location ${city} and all its bookings deleted.`;
    },
    add_client(b) {
      const site = needSite(b), last = field(b, 'last_name'), first = field(b, 'first_name');
      const age = number(b, 'age', 0, 150), room = number(b, 'room', 1, 99999);
      if (!insertClient(site, { last_name: last, first_name: first, age, room })) throw new Bad(`${last} ${first} already has a booking here.`);
      return `Booking for ${last} ${first} added.`;
    },
    delete_client(b) {
      const site = needSite(b), last = field(b, 'last_name'), first = field(b, 'first_name');
      const i = site.clients.findIndex(c => c.last_name === last && c.first_name === first);
      if (i < 0) throw new Bad(`No booking for ${last} ${first}.`);
      site.clients.splice(i, 1); return `Booking for ${last} ${first} deleted.`;
    },
    save() {
      needHotel();
      const lines = ['LastName;FirstName;Age;Room Number;City'];
      hotel.sites.forEach(s => s.clients.forEach(c => lines.push([c.last_name, c.first_name, c.age, c.room, s.city].join(';'))));
      try { localStorage.setItem(SAVE_KEY, lines.join('\n')); } catch (e) { throw new Bad(`Cannot write ${SAVE_FILE}.`); }
      return `${lines.length - 1} booking(s) saved to ${SAVE_FILE}.`;
    },
    load_file() { needHotel(); const n = total(); loadText(BOOKINGS); return `${total() - n} booking(s) loaded from data/bookings.txt.`; },
    load_saved() { needHotel(); const n = total(); loadText(localStorage.getItem(SAVE_KEY) || ''); return `${total() - n} booking(s) loaded from ${SAVE_FILE}.`; },
  };
  function api(path, body) {
    hotel = G.hotel;
    try {
      if (path === 'search') {
        const pre = String(body.prefix ?? '').trim().toUpperCase(), out = [];
        needHotel().sites.forEach(s => s.clients.forEach(c => { if (c.last_name.toUpperCase().startsWith(pre)) out.push({ ...c, city: s.city }); }));
        out.sort((a, b) => cmpNames(a, b) || cmp(a.city, b.city));
        return { ok: true, message: `${out.length} customer(s) found.`, results: out };
      }
      return { ok: true, message: A[path](body || {}), state: state() };
    } catch (e) {
      if (!(e instanceof Bad)) throw e;
      return { ok: false, error: e.message, state: state() };
    } finally {
      G.hotel = hotel;
    }
  }

  // Chain files: optional "#HOTEL;name;login;sha256" line, optional "#SITE;city" lines, then bookings lines.
  const HEADER = 'lastname;firstname;age;room number;city';
  const okText = v => v && new TextEncoder().encode(v).length <= MAX_TEXT && !/[;\r\n]/.test(v);
  window.hotelFile = {
    parse(text) {
      const bad = (n, why) => ({ ok: false, error: n ? `Line ${n}: ${why}` : why });
      if (typeof text !== 'string' || !text.trim()) return bad(0, 'The file is empty.');
      if (/\u0000/.test(text)) return bad(0, 'This is not a text file.');
      let meta = null, count = 0, first = true;
      const sites = [];
      const site = city => sites.find(s => s.city === city) || (sites.push({ city, clients: [] }), sites[sites.length - 1]);
      const lines = text.replace(/^\uFEFF/, '').split(/\r?\n/);
      for (let i = 0; i < lines.length; i++) {
        const n = i + 1, line = lines[i].trim();
        if (!line) continue;
        const p = line.split(';');
        if (p[0] === '#HOTEL') {
          if (!first) return bad(n, 'the #HOTEL line must be the first line.');
          if (p.length !== 4 || !okText(p[1]) || !okText(p[2]) || !/^[0-9a-f]{64}$/.test(p[3])) return bad(n, 'the #HOTEL line is damaged.');
          meta = { name: p[1], login: p[2], hash: p[3] };
        } else if (p[0] === '#SITE') {
          if (p.length !== 2 || !okText(p[1])) return bad(n, 'the #SITE line is damaged.');
          site(p[1]);
        } else if (line.toLowerCase() === HEADER) {
        } else {
          if (p.length !== 5) return bad(n, `expected 5 values separated by ";" (last name;first name;age;room;city), found ${p.length}.`);
          const [last, firstName, a, r, city] = p.map(x => x.trim());
          if (!okText(last) || !okText(firstName) || !okText(city)) return bad(n, 'names and city must be 1 to 49 bytes.');
          if (!/^\d+$/.test(a) || +a > 150) return bad(n, `age "${a}" must be a whole number from 0 to 150.`);
          if (!/^\d+$/.test(r) || +r < 1 || +r > 99999) return bad(n, `room "${r}" must be a whole number from 1 to 99999.`);
          const s = site(city), c = { last_name: last, first_name: firstName, age: +a, room: +r };
          if (s.clients.some(x => cmpNames(x, c) === 0)) return bad(n, `${last} ${firstName} is booked twice in ${city}.`);
          s.clients.push(c); count++;
        }
        first = false;
      }
      if (!meta && !count) return bad(0, 'No bookings found in this file.');
      sites.sort((a, b) => cmp(a.city, b.city)).forEach(s => s.clients.sort(cmpNames));
      return { ok: true, meta, sites, count };
    },
    open(name, sites) { G.hotel = { name, sites: JSON.parse(JSON.stringify(sites || [])) }; return JSON.parse(JSON.stringify(G.hotel)); },
    serialize(meta) {
      const h = G.hotel, out = [`#HOTEL;${h.name};${meta.login};${meta.hash}`];
      h.sites.forEach(s => out.push(`#SITE;${s.city}`));
      out.push('LastName;FirstName;Age;Room Number;City');
      h.sites.forEach(s => s.clients.forEach(c => out.push([c.last_name, c.first_name, c.age, c.room, s.city].join(';'))));
      return out.join('\n') + '\n';
    },
    sample: () => BOOKINGS,
    okText,
    async hash(login, pw) {
      const buf = await crypto.subtle.digest('SHA-256', new TextEncoder().encode(login + ':' + pw));
      return [...new Uint8Array(buf)].map(b => b.toString(16).padStart(2, '0')).join('');
    },
  };

  // Shared UI controller: both DCs call hotelVals(this, theme) from renderVals().
  window.hotelSeed = name => { api('create', { name }); const d = api('load_file'); return d.state; };
  window.hotelInitial = () => ({ chain: null, selected: null, results: [], searched: false, status: '', err: false, confirm: null,
    f: { chain: '', city: '', last_name: '', first_name: '', age: '', room: '', prefix: '' } });
  window.hotelVals = function (cmpnt, theme) {
    const st = cmpnt.state, set = p => cmpnt.setState(p);
    const say = d => ({ status: d.error || d.message, err: !d.ok });
    const act = (path, body, clear) => {
      const d = api(path, body), patch = say(d);
      if (d.state !== undefined) patch.chain = d.state;
      if (d.ok && clear) { patch.results = []; patch.searched = false; }
      set(patch); return d.ok;
    };
    const ask = (text, yes) => set({ confirm: { text, yes } });
    const setF = (k, v) => set(s => ({ f: { ...s.f, [k]: v } }));
    const on = {}; Object.keys(st.f).forEach(k => { on[k] = e => setF(k, e.target.value); });
    const chain = st.chain, sites = chain ? chain.sites : [];
    const selected = sites.some(s => s.city === st.selected) ? st.selected : null;
    const site = sites.find(s => s.city === selected);
    const pick = city => set({ selected: city });
    const v = {
      f: st.f, on,
      hasChain: !!chain, noChain: !chain,
      chainName: chain ? chain.name : '',
      title: chain ? 'Hotel chain: ' + chain.name : 'No hotel chain',
      totalLocations: sites.length,
      totalCustomers: sites.reduce((n, s) => n + s.clients.length, 0),
      sites: sites.map(s => {
        const sel = s.city === selected;
        return { city: s.city, count: s.clients.length, sel, onPick: () => pick(s.city),
          bg: sel ? theme.selBg : 'transparent', fg: sel ? theme.selFg : theme.fg, sub: sel ? theme.selSub : theme.sub };
      }),
      noSites: !!chain && !sites.length,
      hasSelected: !!site, noSelected: !site,
      selectedCity: site ? site.city : '',
      clientsTitle: site ? 'Customers at ' + site.city : 'Customers (select a location)',
      clients: (site ? site.clients : []).map(c => ({ ...c, onDelete: () => act('delete_client', { city: site.city, last_name: c.last_name, first_name: c.first_name }, true) })),
      noClients: !!site && !site.clients.length,
      results: st.results, hasResults: st.results.length > 0, noResults: st.searched && !st.results.length,
      status: st.status, hasStatus: !!st.status, statusErr: st.err,
      statusColor: st.err ? theme.bad : theme.fg,
      statusDot: st.err ? theme.bad : theme.ok,
      createChain: () => {
        const go = () => act('create', { name: st.f.chain }, true) && setF('chain', '');
        if (chain) { ask('This deletes the current chain and all its bookings. Continue?', go); return true; }
        return go();
      },
      loadFile: () => act('load_file'), save: () => act('save'), loadSaved: () => act('load_saved'),
      addSite: () => { const city = st.f.city.trim(); if (act('add_site', { city })) set(s => ({ selected: city, f: { ...s.f, city: '' } })); },
      deleteSite: () => {
        if (!selected) return set({ status: 'Select a location first.', err: true });
        ask(`Delete ${selected} and all its bookings?`, () => act('delete_site', { city: selected }, true));
      },
      addClient: () => {
        if (!selected) return set({ status: 'Select a location first.', err: true });
        const ids = ['last_name', 'first_name', 'age', 'room'], body = { city: selected };
        ids.forEach(k => body[k] = st.f[k]);
        const ok = act('add_client', body);
        if (ok) set(s => ({ f: { ...s.f, last_name: '', first_name: '', age: '', room: '' } }));
        return ok;
      },
      search: e => { e && e.preventDefault(); const d = api('search', { prefix: st.f.prefix }); set({ ...say(d), results: d.results || [], searched: true }); },
      clearSearch: () => set(s => ({ results: [], searched: false, f: { ...s.f, prefix: '' } })),
      confirmOpen: !!st.confirm, confirmText: st.confirm ? st.confirm.text : '',
      confirmYes: () => { const y = st.confirm.yes; set({ confirm: null }); y(); },
      confirmNo: () => set({ confirm: null }),
      dismiss: () => set({ status: '' }),
    };
    const enter = fn => e => { if (e.key === 'Enter') fn(); };
    v.keys = { chain: enter(v.createChain), city: enter(v.addSite), client: enter(v.addClient) };
    return v;
  };
})();
