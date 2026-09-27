// Settings view: grouped cards that save themselves. Network and firmware updates are separate
// cards (network.js, updater.js) that register through Settings.addCard.
const Settings = (() => {
  const CARDS = [
    { id: "grill", title: "Grill", fields: [
      { key: "name", label: "Name", type: "text", maxlength: 32 },
      { key: "temperature_unit", label: "Unit", type: "segmented", options: [["celcius", "°C"], ["fahrenheit", "°F"]] },
    ] },
    { id: "alarms", title: "Alarms", fields: [
      { key: "beep_enabled", label: "Beeps", type: "toggle" },
      { key: "beep_volume", label: "Volume", type: "stepper", min: 0, max: 5 },
      { key: "beep_on_ready", label: "Beep when ready", type: "toggle" },
      { key: "beep_outside_target", label: "Beep outside range", type: "toggle" },
      { key: "beep_degrees_before", label: "Early warning", type: "stepper", min: 0, max: 100,
        suffix: (v) => (v === 0 ? "off" : "° before") },
      { key: "cucaracha_enabled", label: "Cucaracha tune", type: "toggle" },
    ] },
    { id: "display", title: "Display", fields: [
      { key: "backlight_brightness", label: "Brightness", type: "stepper", min: 0, max: 5, suffix: (v) => (v === 0 ? "off" : "") },
      { key: "screen_timeout_minutes", label: "Screen off after", type: "stepper", min: 0, max: 10000,
        suffix: (v) => (v === 0 ? "never" : "min") },
      { key: "backlight_timeout_minutes", label: "Backlight off after", type: "stepper", min: 0, max: 10000,
        suffix: (v) => (v === 0 ? "never" : "min") },
    ] },
    { id: "mqtt", title: "MQTT", fields: [
      { key: "mqtt_broker", label: "Broker", type: "text", placeholder: "Not used when empty" },
      { key: "mqtt_port", label: "Port", type: "number", min: 1, max: 65535 },
      { key: "mqtt_topic", label: "Topic", type: "text" },
      { key: "mqtt_user", label: "User", type: "text" },
      { key: "mqtt_password", label: "Password", type: "password" },
    ] },
    { id: "opengrill", title: "Opengrill", fields: [
      { key: "opengrill_server", label: "Server", type: "text", placeholder: "Not used when empty" },
    ] },
  ];

  const controls = [];      // { set(settings) } for every auto-saved field
  const extraCards = [];    // cards added by other scripts
  const savers = [];
  let settings = null;
  let loadError;

  function addCard(card) { extraCards.push(card); }
  function current() { return settings; }

  function fill(newSettings) {
    settings = newSettings;
    controls.forEach((control) => control.set(settings));
    extraCards.forEach((card) => card.fill(settings));
  }

  function labelled(field, input) {
    const wrapper = document.createElement("label");
    wrapper.className = "field";
    const text = document.createElement("span");
    text.textContent = field.label;
    wrapper.append(text, input);
    return wrapper;
  }

  function row(field, control) {
    const wrapper = document.createElement("div");
    wrapper.className = "setting-row";
    const text = document.createElement(control.tagName === "INPUT" ? "label" : "span");
    text.className = "field-label";
    text.textContent = field.label;
    if (control.tagName === "INPUT") {
      control.id = "setting-" + field.key;
      text.htmlFor = control.id;
    }
    wrapper.append(text, control);
    return wrapper;
  }

  function buildField(field, saver) {
    const send = (value, options) => saver.change({ [field.key]: value }, options);

    if (field.type === "text" || field.type === "number") {
      const input = document.createElement("input");
      input.type = field.type === "number" ? "number" : "text";
      if (field.maxlength) input.maxLength = field.maxlength;
      if (field.placeholder) input.placeholder = field.placeholder;
      if (field.type === "number") { input.min = field.min; input.max = field.max; input.inputMode = "numeric"; }
      input.addEventListener("change", () => {
        if (field.type === "number") {
          const value = parseInt(input.value, 10);
          if (isNaN(value) || value < field.min || value > field.max) { input.value = settings[field.key]; return; }
          send(value, { immediate: true });
        } else {
          send(input.value.trim(), { immediate: true });
        }
      });
      input.addEventListener("keydown", (event) => { if (event.key === "Enter") input.blur(); });
      const wrapper = labelled(field, input);
      let hint = null;
      if (field.key === "mqtt_topic") {
        hint = document.createElement("p");
        hint.className = "hint";
        wrapper.append(hint);
      }
      controls.push({ set: (s) => {
        if (document.activeElement !== input) input.value = s[field.key] ?? "";
        if (hint) hint.textContent = "Topics: " + (s.mqtt_topic || "grilly-plus") + "/" + s.uuid + "/…";
      } });
      return wrapper;
    }

    if (field.type === "password") {
      const input = document.createElement("input");
      input.type = "password";
      input.autocomplete = "new-password";
      input.addEventListener("change", () => {
        if (input.value !== "") send(input.value, { immediate: true });
      });
      const wrapper = labelled(field, input);
      const remove = document.createElement("button");
      remove.type = "button";
      remove.className = "link-button";
      remove.textContent = "Remove saved password";
      remove.addEventListener("click", () => send("", { immediate: true }));
      wrapper.append(remove);
      controls.push({ set: (s) => {
        const isSet = !!s[field.key + "_set"];
        input.placeholder = isSet ? "Saved, type to change" : "Not set";
        if (document.activeElement !== input) input.value = "";
        remove.hidden = !isSet;
      } });
      return wrapper;
    }

    if (field.type === "toggle") {
      const input = document.createElement("input");
      input.type = "checkbox";
      input.className = "switch";
      input.setAttribute("role", "switch");
      input.addEventListener("change", () => send(input.checked));
      controls.push({ set: (s) => { if (document.activeElement !== input) input.checked = !!s[field.key]; } });
      return row(field, input);
    }

    if (field.type === "stepper") {
      const holder = document.createElement("div");
      const stepper = Controls.stepper(holder, {
        min: field.min, max: field.max, label: field.label, suffix: field.suffix || "", onChange: (value) => send(value),
      });
      controls.push({ set: (s) => { if (!Controls.isEditing(holder)) stepper.set(s[field.key]); } });
      return row(field, holder);
    }

    // segmented
    const group = document.createElement("fieldset");
    group.className = "segmented";
    const legend = document.createElement("legend");
    legend.textContent = field.label;
    const options = document.createElement("div");
    options.className = "options";
    field.options.forEach(([value, text]) => {
      const label = document.createElement("label");
      const input = document.createElement("input");
      input.type = "radio";
      input.name = "setting-" + field.key;
      input.value = value;
      input.addEventListener("change", () => send(value));
      const span = document.createElement("span");
      span.textContent = text;
      label.append(input, span);
      options.append(label);
    });
    group.append(legend, options);
    controls.push({ set: (s) => {
      if (Controls.isEditing(group)) return;
      group.querySelectorAll("input").forEach((input) => { input.checked = input.value === s[field.key]; });
    } });
    return group;
  }

  function buildCard(def) {
    const card = document.createElement("section");
    card.className = "card settings-card";
    card.innerHTML = '<h2 class="card-title"><span></span><span class="save-note" role="status"></span></h2>';
    card.querySelector(".card-title span").textContent = def.title;
    const note = card.querySelector(".save-note");
    const saver = createSaver({
      send: (fields) => Api.post("/api/settings", fields),
      onStatus: (state, detail) => {
        Controls.showNote(note, state, detail);
        if (state === "saved" && detail) fill(detail);
      },
    });
    savers.push(saver);
    def.fields.forEach((field) => card.append(buildField(field, saver)));
    return card;
  }

  async function show() {
    try {
      fill(await Api.get("/api/settings", 5000));
      loadError.hidden = true;
    } catch (error) {
      loadError.textContent = "Couldn't load the settings: " + error.message;
      loadError.hidden = false;
    }
  }

  function hide() {
    savers.forEach((saver) => saver.flush());
  }

  function mount(el) {
    el.innerHTML = '<h1 class="view-title">Settings</h1><p class="banner" hidden></p><div class="settings-grid"></div>';
    loadError = el.querySelector(".banner");
    const grid = el.querySelector(".settings-grid");
    CARDS.forEach((def) => grid.append(buildCard(def)));
    extraCards.forEach((card) => grid.append(card.build()));
  }

  App.register("settings", { mount, show, hide });

  return { addCard, fill, current };
})();
