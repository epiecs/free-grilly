// Pure formatting and status helpers. No DOM access, so the unit tests can run them in Node.
const Format = (() => {
  const clamp = (value, min = 0, max = 1) => Math.min(max, Math.max(min, value));
  const isNumber = (value) => typeof value === "number" && isFinite(value);

  function unitSymbol(unit) {
    return unit === "fahrenheit" ? "°F" : "°C";
  }

  function number(value) {
    return isNumber(value) ? value.toFixed(1) : "–";
  }

  function temperature(value, unit) {
    return isNumber(value) ? value.toFixed(1) + unitSymbol(unit) : "–";
  }

  function signal(rssi, connected) {
    if (!connected) return { bars: 0, label: "Hotspot", hotspot: true };
    let bars = 0;
    if (rssi >= -55) bars = 4;
    else if (rssi >= -67) bars = 3;
    else if (rssi >= -75) bars = 2;
    else if (rssi >= -85) bars = 1;
    const label = bars >= 3 ? "Good" : bars === 2 ? "Fair" : "Weak";
    return { bars, label, hotspot: false };
  }

  function battery(percentage) {
    const pct = Math.round(clamp(Number(percentage) || 0, 0, 100));
    const level = pct > 50 ? "good" : pct >= 20 ? "warn" : "bad";
    return { pct, level };
  }

  // A minimum above 0 means range mode, a target above 0 without a minimum means target mode
  function alarmMode(probe) {
    if (!(probe.target_temperature > 0)) return "off";
    return probe.minimum_temperature > 0 ? "range" : "target";
  }

  function probeStatus(probe) {
    const t = probe.temperature;
    const target = probe.target_temperature;
    const mode = alarmMode(probe);

    if (mode === "off") return { kind: "none", text: "", progress: null };
    if (mode === "range") {
      if (t < probe.minimum_temperature) return { kind: "low", text: "Too low", progress: clamp(t / target) };
      if (t > target) return { kind: "high", text: "Too high", progress: 1 };
      return { kind: "in-range", text: "In range", progress: 1 };
    }
    if (t >= target) return { kind: "ready", text: "Ready", progress: 1 };
    return { kind: "to-go", text: "+" + (target - t).toFixed(1) + "° to go", progress: clamp(t / target) };
  }

  function alarmLabel(probe) {
    const mode = alarmMode(probe);
    if (mode === "target") return "→ " + Math.round(probe.target_temperature) + "°";
    if (mode === "range") return Math.round(probe.minimum_temperature) + "–" + Math.round(probe.target_temperature) + "°";
    return "";
  }

  function duration(seconds) {
    if (!(seconds > 0)) return "";
    if (seconds < 60) return "<1m";
    const hours = Math.floor(seconds / 3600);
    const minutes = Math.floor((seconds % 3600) / 60);
    return hours > 0 ? hours + "h " + minutes + "m" : minutes + "m";
  }

  function emptySockets(ids) {
    if (ids.length === 0) return "";
    const sorted = [...ids].sort((a, b) => a - b);
    const parts = [];
    let start = sorted[0];
    let previous = sorted[0];
    for (const id of sorted.slice(1).concat([null])) {
      if (id === previous + 1) { previous = id; continue; }
      parts.push(start === previous ? String(start) : start + "–" + previous);
      start = previous = id;
    }
    return (ids.length === 1 ? "Socket " : "Sockets ") + parts.join(", ") + " empty";
  }

  function changedFields(original, current) {
    const changed = {};
    for (const key of Object.keys(current)) {
      if (original[key] !== current[key]) changed[key] = current[key];
    }
    return changed;
  }

  return { unitSymbol, number, temperature, signal, battery, alarmMode, probeStatus, alarmLabel,
           duration, emptySockets, changedFields };
})();

if (typeof module === "object" && module.exports) module.exports = Format;
