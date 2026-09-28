// Network card: WiFi and hotspot. Changes collect here and are sent together with Apply, because the
// grill reconnects when network settings are saved.
(() => {
  // Empty in the form means DHCP, the firmware stores that as 0.0.0.0
  const DHCP_FIELDS = ["wifi_ip", "wifi_subnet", "wifi_gateway", "wifi_dns"];
  const inputs = {};
  let dirty = {};
  let note, applyButton, applyNote, scanButton, scanList;
  const clears = {};

  function field(key, label, type = "text") {
    return '<label class="field"><span>' + label + '</span><input data-net="' + key + '" type="' + type + '"' +
      (type === "password" ? ' autocomplete="new-password"' : ' autocomplete="off"') + '></label>';
  }

  function passwordField(key, label) {
    return field(key, label, "password") +
      '<label class="check"><input type="checkbox" data-clear="' + key + '"><span>Remove saved password</span></label>';
  }

  function markDirty() {
    const count = Object.keys(dirty).length;
    applyButton.disabled = count === 0;
    note.textContent = count === 0 ? "" : "Unsaved changes";
  }

  function onInput(key) {
    const input = inputs[key];
    if (key.endsWith("_password")) {
      if (input.value !== "") dirty[key] = input.value;
      else if (clears[key].checked) dirty[key] = "";
      else delete dirty[key];
    } else if (DHCP_FIELDS.includes(key)) {
      dirty[key] = input.value.trim() === "" ? "0.0.0.0" : input.value.trim();
    } else {
      dirty[key] = input.value.trim();
    }
    markDirty();
  }

  async function apply() {
    if (dirty.local_ap_password && dirty.local_ap_password.length < 8) {
      applyNote.textContent = "The hotspot password needs at least 8 characters.";
      return;
    }
    if (!window.confirm("The grill will reconnect to apply these network settings. You may need to reconnect to it at its new address. Continue?")) {
      return;
    }
    applyButton.disabled = true;
    applyNote.textContent = "Applying…";
    try {
      const result = await Api.post("/api/settings", dirty, 20000);
      dirty = {};
      Object.values(clears).forEach((box) => { box.checked = false; });
      markDirty();
      applyNote.textContent = "Applied. The grill is reconnecting, this page picks it up again when it's back.";
      Settings.fill(result);
    } catch (error) {
      applyNote.textContent = error.timeout
        ? "No answer. The grill may be reconnecting, check its new address if this page doesn't come back."
        : error.message;
      applyButton.disabled = false;
    }
  }

  function scanItem(network) {
    const item = document.createElement("li");
    const button = document.createElement("button");
    button.type = "button";
    const name = document.createElement("span");
    name.textContent = network.ssid;
    const info = document.createElement("span");
    info.className = "hint";
    info.textContent = Format.signal(network.signal_strength, true).label + (network.auth_method === "open" ? ", open" : "");
    button.append(name, info);
    button.addEventListener("click", () => {
      inputs.wifi_ssid.value = network.ssid;
      onInput("wifi_ssid");
      scanList.hidden = true;
      inputs.wifi_password.focus();
    });
    item.append(button);
    return item;
  }

  function textItem(text) {
    const item = document.createElement("li");
    item.className = "hint";
    item.textContent = text;
    return item;
  }

  async function scan() {
    scanButton.disabled = true;
    scanButton.textContent = "Scanning…";
    try {
      const networks = await Api.get("/api/wifiscan", 15000);
      const best = new Map();
      networks.forEach((network) => {
        if (network.ssid && (!best.has(network.ssid) || best.get(network.ssid).signal_strength < network.signal_strength)) {
          best.set(network.ssid, network);
        }
      });
      const sorted = [...best.values()].sort((a, b) => b.signal_strength - a.signal_strength);
      scanList.replaceChildren(...(sorted.length ? sorted.map(scanItem) : [textItem("No networks found")]));
    } catch (error) {
      scanList.replaceChildren(textItem("Scan failed: " + error.message));
    } finally {
      scanList.hidden = false;
      scanButton.disabled = false;
      scanButton.textContent = "Scan";
    }
  }

  function build() {
    const card = document.createElement("section");
    card.className = "card settings-card";
    card.innerHTML =
      '<h2 class="card-title"><span>Network</span><span class="save-note" role="status"></span></h2>' +
      '<h3 class="subheading">WiFi</h3>' +
      '<label class="field"><span>Network</span><span class="field-with-button">' +
      '<input data-net="wifi_ssid" autocomplete="off"><button type="button" class="button" data-scan>Scan</button></span></label>' +
      '<ul class="scan-results" hidden></ul>' +
      passwordField("wifi_password", "Password") +
      '<details class="advanced"><summary>Static IP (leave empty for DHCP)</summary>' +
      field("wifi_ip", "IP address") + field("wifi_subnet", "Subnet") + field("wifi_gateway", "Gateway") + field("wifi_dns", "DNS") +
      '</details>' +
      '<h3 class="subheading">Hotspot</h3>' +
      field("local_ap_ssid", "Name") +
      passwordField("local_ap_password", "Password (empty or at least 8 characters)") +
      '<details class="advanced"><summary>Hotspot address</summary>' +
      field("local_ap_ip", "IP address") + field("local_ap_subnet", "Subnet") + field("local_ap_gateway", "Gateway") +
      '</details>' +
      '<p class="warning">Applying makes the grill reconnect.</p>' +
      '<button type="button" class="button primary" data-apply disabled>Apply network changes</button>' +
      '<p class="notice" role="status"></p>';

    note = card.querySelector(".save-note");
    applyButton = card.querySelector("[data-apply]");
    applyNote = card.querySelector(".notice");
    scanButton = card.querySelector("[data-scan]");
    scanList = card.querySelector(".scan-results");
    card.querySelectorAll("[data-net]").forEach((input) => {
      inputs[input.dataset.net] = input;
      input.addEventListener("input", () => onInput(input.dataset.net));
    });
    card.querySelectorAll("[data-clear]").forEach((box) => {
      const key = box.dataset.clear;
      clears[key] = box;
      box.addEventListener("change", () => {
        if (box.checked) { inputs[key].value = ""; dirty[key] = ""; }
        else delete dirty[key];
        markDirty();
      });
    });
    applyButton.addEventListener("click", apply);
    scanButton.addEventListener("click", scan);
    return card;
  }

  function fill(settings) {
    Object.entries(inputs).forEach(([key, input]) => {
      if (key in dirty || document.activeElement === input) return;
      if (key.endsWith("_password")) {
        const isSet = !!settings[key + "_set"];
        input.value = "";
        input.placeholder = isSet ? "Saved, type to change" : "Not set";
        clears[key].parentElement.hidden = !isSet;
      } else {
        const value = settings[key] ?? "";
        input.value = DHCP_FIELDS.includes(key) && value === "0.0.0.0" ? "" : value;
      }
    });
  }

  Settings.addCard({ build, fill });
})();
