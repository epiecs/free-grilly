// App shell: registers views, switches them on the url hash, and polls /api/grill for the header
// and every view that wants live data.
const App = (() => {
  const views = {};
  const listeners = [];
  let current = null;
  let status = null;
  let failures = 0;
  let pollTimer = null;
  let polling = false;

  function register(name, view) { views[name] = view; }
  function onStatus(listener) { listeners.push(listener); }
  function getStatus() { return status; }

  function route() {
    const requested = (location.hash || "#grill").slice(1);
    const name = document.getElementById("view-" + requested) ? requested : "grill";

    document.querySelectorAll("main > .view").forEach((el) => { el.hidden = el.dataset.view !== name; });
    document.querySelectorAll(".tabs a").forEach((link) => {
      if (link.dataset.view === name) link.setAttribute("aria-current", "page");
      else link.removeAttribute("aria-current");
    });

    if (current !== name) {
      if (current && views[current] && views[current].hide) views[current].hide();
      current = name;
      if (views[name] && views[name].show) views[name].show();
    }
  }

  function renderHeader(s) {
    document.getElementById("grill-subtitle").textContent = s.name + " · " + Format.unitSymbol(s.temperature_unit);

    const battery = Format.battery(s.battery_percentage);
    const batteryPill = document.getElementById("battery-pill");
    batteryPill.hidden = false;
    batteryPill.className = "pill level-" + battery.level;
    document.getElementById("battery-fill").setAttribute("width", String(18 * battery.pct / 100));
    document.getElementById("battery-bolt").style.display = s.battery_charging ? "" : "none";
    document.getElementById("battery-text").textContent = battery.pct + "%";
    batteryPill.setAttribute("aria-label", "Battery " + battery.pct + "%" + (s.battery_charging ? ", charging" : ""));

    const signal = Format.signal(s.wifi_signal, s.wifi_connected);
    const signalPill = document.getElementById("signal-pill");
    signalPill.hidden = false;
    signalPill.className = "pill " + (signal.hotspot ? "" : "level-" + (signal.bars >= 3 ? "good" : signal.bars === 2 ? "warn" : "bad"));
    document.getElementById("signal-bars").style.display = signal.hotspot ? "none" : "";
    document.getElementById("signal-hotspot").hidden = !signal.hotspot;
    document.querySelectorAll("#signal-bars .signal-bar").forEach((bar, index) => {
      bar.classList.toggle("on", index < signal.bars);
    });
    document.getElementById("signal-text").textContent = signal.label;
    signalPill.setAttribute("aria-label", signal.hotspot ? "Using the grill's hotspot" : "WiFi signal " + signal.label.toLowerCase());
  }

  async function poll() {
    pollTimer = null;
    polling = true;
    try {
      status = await Api.get("/api/grill", 3000);
      failures = 0;
      document.body.classList.remove("offline");
      document.getElementById("offline-banner").hidden = true;
      renderHeader(status);
      listeners.forEach((listener) => listener(status));
    } catch (error) {
      failures += 1;
      if (failures >= 3) {
        document.body.classList.add("offline");
        document.getElementById("offline-banner").hidden = false;
      }
    } finally {
      polling = false;
      schedule(1000);
    }
  }

  // One request at a time: the next poll is only planned after the previous one finished
  function schedule(delay) {
    if (document.hidden || pollTimer !== null || polling) return;
    pollTimer = setTimeout(poll, delay);
  }

  function start() {
    Object.entries(views).forEach(([name, view]) => {
      if (view.mount) view.mount(document.getElementById("view-" + name));
    });
    window.addEventListener("hashchange", route);
    document.addEventListener("visibilitychange", () => {
      if (document.hidden) {
        clearTimeout(pollTimer);
        pollTimer = null;
      } else {
        schedule(0);
      }
    });
    route();
    schedule(0);
  }

  return { register, onStatus, getStatus, start };
})();
