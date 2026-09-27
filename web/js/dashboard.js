// Grill view: one card per connected probe in socket order. Tapping a card opens the probe editor.
(() => {
  let list;
  let emptyLine;
  let noProbes;
  const cards = {};   // probe_id -> card element

  function createCard(id) {
    const card = document.createElement("button");
    card.type = "button";
    card.className = "card probe-card live";
    card.dataset.probe = String(id);
    card.innerHTML =
      '<div class="card-head"><span class="probe-name"></span><span class="probe-alarm"></span></div>' +
      '<div class="probe-temp"><span class="value"></span><span class="unit"></span></div>' +
      '<div class="progress" hidden><i></i></div>' +
      '<div class="card-meta"><span class="probe-status"></span><span class="probe-time"></span></div>';
    card.addEventListener("click", () => {
      if (typeof Editor !== "undefined") Editor.open(id, card);
    });
    return card;
  }

  function fill(card, probe, unit) {
    const status = Format.probeStatus(probe);
    card.dataset.status = status.kind;
    card.querySelector(".probe-name").textContent = probe.probe_id + " · " + probe.name;
    card.querySelector(".probe-alarm").textContent = Format.alarmLabel(probe);
    card.querySelector(".value").textContent = Format.number(probe.temperature);
    card.querySelector(".unit").textContent = Format.unitSymbol(unit);
    const progress = card.querySelector(".progress");
    progress.hidden = status.progress === null;
    if (status.progress !== null) progress.firstElementChild.style.width = Math.round(status.progress * 100) + "%";
    card.querySelector(".probe-status").textContent = status.text;
    card.querySelector(".probe-time").textContent = Format.duration(probe.connected_seconds);
    card.setAttribute("aria-label",
      probe.name + ", " + Format.temperature(probe.temperature, unit) + (status.text ? ", " + status.text : "") + ". Edit probe");
  }

  function render(status) {
    const connected = status.probes.filter((probe) => probe.connected);
    const empty = status.probes.filter((probe) => !probe.connected).map((probe) => probe.probe_id);

    Object.keys(cards).forEach((id) => {
      if (!connected.some((probe) => String(probe.probe_id) === id)) {
        cards[id].remove();
        delete cards[id];
      }
    });

    connected.forEach((probe, index) => {
      let card = cards[probe.probe_id];
      if (!card) card = cards[probe.probe_id] = createCard(probe.probe_id);
      // Only move a card when its position changed, moving a focused element would drop focus
      if (list.children[index] !== card) list.insertBefore(card, list.children[index] || null);
      fill(card, probe, status.temperature_unit);
    });

    noProbes.hidden = connected.length > 0;
    emptyLine.hidden = connected.length === 0 || empty.length === 0;
    emptyLine.textContent = Format.emptySockets(empty);
  }

  function mount(el) {
    el.innerHTML =
      '<div class="probe-list"></div>' +
      '<p class="empty-sockets"></p>' +
      '<div class="card no-probes" hidden><p>Plug in a probe</p></div>';
    list = el.querySelector(".probe-list");
    emptyLine = el.querySelector(".empty-sockets");
    noProbes = el.querySelector(".no-probes");
    App.onStatus(render);
  }

  App.register("grill", { mount });
})();
