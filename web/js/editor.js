// Probe editor: a bottom sheet on phones, a side panel on wide screens. Every change saves itself.
const Editor = (() => {
  const TYPES = {
    grilleye_iris: "Grilleye Iris",
    ikea_fantast: "Ikea Fantast",
    maverick_et733: "Maverick ET733",
    weber_igrill: "Weber iGrill",
    custom: "Custom",
  };

  let backdrop, sheet, form, title, live, note, nameInput, modeInputs;
  let targetGroup, targetLabel, minGroup, targetStepper, minStepper;
  let typeSelect, typeName, customBox;
  const refInputs = {};
  let saver;
  let probeId = null;
  let returnFocus = null;
  let closeTimer = null;
  let swipeStart = null;

  function unit() {
    const status = App.getStatus();
    return status ? status.temperature_unit : "celcius";
  }

  function defaults() {
    return unit() === "fahrenheit"
      ? { max: 572, target: 160, rangeMin: 212, rangeMax: 230 }
      : { max: 300, target: 70, rangeMin: 100, rangeMax: 110 };
  }

  function save(fields, options) {
    saver.change({ probe_id: probeId, ...fields }, options);
  }

  function mode() {
    return [...modeInputs].find((input) => input.checked).value;
  }

  function showMode(current) {
    targetGroup.hidden = current === "off";
    minGroup.hidden = current !== "range";
    targetLabel.textContent = current === "range" ? "Maximum temperature" : "Target temperature";
  }

  function onModeChange() {
    const current = mode();
    showMode(current);
    if (current === "off") {
      save({ target_temperature: 0, minimum_temperature: 0 });
    } else if (current === "target") {
      save({ target_temperature: targetStepper.get(), minimum_temperature: 0 });
    } else {
      if (minStepper.get() >= targetStepper.get()) minStepper.set(targetStepper.get() - 5);
      save({ target_temperature: targetStepper.get(), minimum_temperature: minStepper.get() });
    }
  }

  function onTargetChange(value) {
    if (mode() === "range") {
      if (minStepper.get() >= value) minStepper.set(value - 1);
      save({ target_temperature: value, minimum_temperature: minStepper.get() });
    } else {
      save({ target_temperature: value });
    }
  }

  function onMinChange(value) {
    if (value >= targetStepper.get()) targetStepper.set(value + 1);
    save({ target_temperature: targetStepper.get(), minimum_temperature: value });
  }

  function references() {
    return {
      reference_kohm: Number(refInputs.reference_kohm.value) || 0,
      reference_celcius: Number(refInputs.reference_celcius.value) || 0,
      reference_beta: Number(refInputs.reference_beta.value) || 0,
    };
  }

  function onTypeChange() {
    const type = typeSelect.value;
    customBox.hidden = type !== "custom";
    typeName.textContent = TYPES[type];
    if (type === "custom") save({ probe_type: "custom", ...references() }, { immediate: true });
    else save({ probe_type: type }, { immediate: true });
  }

  // Fills the controls from a probe, but leaves the ones the user is working in alone
  function fill(probe) {
    if (!probe) return;
    const d = defaults();
    targetStepper.setLimits(1, d.max);
    minStepper.setLimits(1, d.max - 1);
    targetStepper.setSuffix(Format.unitSymbol(unit()));
    minStepper.setSuffix(Format.unitSymbol(unit()));

    if (document.activeElement !== nameInput) nameInput.value = probe.name;
    const current = Format.alarmMode(probe);
    if (!Controls.isEditing(form.querySelector(".segmented"))) {
      modeInputs.forEach((input) => { input.checked = input.value === current; });
    }
    if (!Controls.isEditing(targetStepper.input.parentElement.parentElement)) {
      targetStepper.set(probe.target_temperature > 0 ? probe.target_temperature : (current === "range" ? d.rangeMax : d.target));
    }
    if (!Controls.isEditing(minStepper.input.parentElement.parentElement)) {
      minStepper.set(probe.minimum_temperature > 0 ? probe.minimum_temperature : Math.min(d.rangeMin, targetStepper.get() - 1));
    }
    showMode(current);

    if (!Controls.isEditing(typeSelect)) typeSelect.value = TYPES[probe.probe_type] ? probe.probe_type : "custom";
    typeName.textContent = TYPES[typeSelect.value];
    customBox.hidden = typeSelect.value !== "custom";
    Object.keys(refInputs).forEach((key) => {
      if (document.activeElement !== refInputs[key]) refInputs[key].value = probe[key];
    });
    form.disabled = false;
  }

  function updateLive(status) {
    if (probeId === null || !status) return;
    const probe = status.probes.find((p) => p.probe_id === probeId);
    live.textContent = probe && probe.connected
      ? "Now " + Format.temperature(probe.temperature, status.temperature_unit)
      : "Not connected";
  }

  async function open(id, trigger) {
    if (probeId !== null && probeId !== id) saver.flush();
    probeId = id;
    returnFocus = trigger || null;
    title.textContent = "Probe " + id;
    Controls.showNote(note, "idle");
    form.disabled = true;
    updateLive(App.getStatus());
    show();
    try {
      const probes = await Api.get("/api/probes", 5000);
      if (probeId === id) fill(probes.find((p) => p.probe_id === id));
    } catch (error) {
      Controls.showNote(note, "error", error);
    }
  }

  function show() {
    clearTimeout(closeTimer);
    backdrop.hidden = false;
    sheet.hidden = false;
    sheet.getBoundingClientRect();   // apply the hidden state first so the slide-in animates
    document.body.classList.add("sheet-open");
    sheet.querySelector("[data-close]").focus();
  }

  function close() {
    if (probeId === null) return;
    saver.flush();
    probeId = null;
    document.body.classList.remove("sheet-open");
    closeTimer = setTimeout(() => { backdrop.hidden = true; sheet.hidden = true; }, 260);
    if (returnFocus && document.contains(returnFocus)) returnFocus.focus();
  }

  // Keeps Tab inside the sheet while it's open
  function trapFocus(event) {
    if (event.key === "Escape") { close(); return; }
    if (event.key !== "Tab") return;
    const focusable = [...sheet.querySelectorAll("button, input, select, summary")]
      .filter((el) => !el.disabled && el.offsetParent !== null);
    const first = focusable[0];
    const last = focusable[focusable.length - 1];
    if (event.shiftKey && document.activeElement === first) { last.focus(); event.preventDefault(); }
    else if (!event.shiftKey && document.activeElement === last) { first.focus(); event.preventDefault(); }
  }

  function mount() {
    backdrop = document.createElement("div");
    backdrop.className = "sheet-backdrop";
    backdrop.hidden = true;

    sheet = document.createElement("aside");
    sheet.className = "sheet";
    sheet.hidden = true;
    sheet.setAttribute("role", "dialog");
    sheet.setAttribute("aria-modal", "true");
    sheet.setAttribute("aria-labelledby", "editor-title");
    sheet.innerHTML =
      '<div class="sheet-grab" aria-hidden="true"></div>' +
      '<div class="sheet-head">' +
      '  <h2 id="editor-title"></h2><span class="save-note" role="status"></span>' +
      '  <button type="button" class="icon-button" data-close aria-label="Close"><svg class="icon"><use href="#i-close"/></svg></button>' +
      '</div>' +
      '<p class="sheet-live live"></p>' +
      '<fieldset class="editor-form">' +
      '  <label class="field"><span>Name</span><input id="editor-name" maxlength="32" autocomplete="off"></label>' +
      '  <fieldset class="segmented"><legend>Alarm</legend><div class="options">' +
      '    <label><input type="radio" name="editor-mode" value="off"><span>Off</span></label>' +
      '    <label><input type="radio" name="editor-mode" value="target"><span>Target</span></label>' +
      '    <label><input type="radio" name="editor-mode" value="range"><span>Range</span></label>' +
      '  </div></fieldset>' +
      '  <div class="field" id="editor-min-group"><span class="field-label">Minimum temperature</span><div id="editor-min"></div></div>' +
      '  <div class="field" id="editor-target-group"><span class="field-label" id="editor-target-label"></span><div id="editor-target"></div></div>' +
      '  <details class="probe-type"><summary><span>Probe type</span><span class="summary-value"><span id="editor-type-name"></span> <svg class="icon"><use href="#i-chevron"/></svg></span></summary>' +
      '    <label class="field"><span>Type</span><select id="editor-type"></select></label>' +
      '    <div id="editor-custom">' +
      '      <label class="field"><span>Reference resistance (kOhm)</span><input type="number" inputmode="numeric" data-ref="reference_kohm"></label>' +
      '      <label class="field"><span>Reference temperature (°C)</span><input type="number" inputmode="numeric" data-ref="reference_celcius"></label>' +
      '      <label class="field"><span>Beta</span><input type="number" inputmode="numeric" data-ref="reference_beta"></label>' +
      '    </div>' +
      '  </details>' +
      '</fieldset>';
    document.body.append(backdrop, sheet);

    form = sheet.querySelector(".editor-form");
    title = sheet.querySelector("#editor-title");
    live = sheet.querySelector(".sheet-live");
    note = sheet.querySelector(".save-note");
    nameInput = sheet.querySelector("#editor-name");
    modeInputs = sheet.querySelectorAll('input[name="editor-mode"]');
    targetGroup = sheet.querySelector("#editor-target-group");
    targetLabel = sheet.querySelector("#editor-target-label");
    minGroup = sheet.querySelector("#editor-min-group");
    typeSelect = sheet.querySelector("#editor-type");
    typeName = sheet.querySelector("#editor-type-name");
    customBox = sheet.querySelector("#editor-custom");
    sheet.querySelectorAll("[data-ref]").forEach((input) => { refInputs[input.dataset.ref] = input; });

    Object.entries(TYPES).forEach(([value, label]) => typeSelect.add(new Option(label, value)));

    targetStepper = Controls.stepper(sheet.querySelector("#editor-target"), {
      min: 1, max: 300, label: "Target temperature", onChange: onTargetChange,
    });
    minStepper = Controls.stepper(sheet.querySelector("#editor-min"), {
      min: 1, max: 299, label: "Minimum temperature", onChange: onMinChange,
    });

    saver = createSaver({
      send: (fields) => Api.post("/api/probes", [fields]),
      onStatus: (state, detail) => {
        Controls.showNote(note, state, detail);
        if (state === "saved" && Array.isArray(detail) && probeId !== null) {
          fill(detail.find((p) => p.probe_id === probeId));
        }
      },
    });

    nameInput.addEventListener("change", () => save({ name: nameInput.value.trim() }, { immediate: true }));
    nameInput.addEventListener("keydown", (event) => { if (event.key === "Enter") nameInput.blur(); });
    modeInputs.forEach((input) => input.addEventListener("change", onModeChange));
    typeSelect.addEventListener("change", onTypeChange);
    Object.entries(refInputs).forEach(([key, input]) => {
      input.addEventListener("change", () => save({ probe_type: "custom", ...references() }, { immediate: true }));
    });

    sheet.querySelector("[data-close]").addEventListener("click", close);
    backdrop.addEventListener("click", close);
    sheet.addEventListener("keydown", trapFocus);

    // Swipe down on the grab handle or the title row to close
    sheet.addEventListener("pointerdown", (event) => {
      if (event.target.closest(".sheet-grab, .sheet-head")) swipeStart = event.clientY;
    });
    sheet.addEventListener("pointerup", (event) => {
      if (swipeStart !== null && event.clientY - swipeStart > 80) close();
      swipeStart = null;
    });

    App.onStatus(updateLive);
  }

  // The editor has no view of its own, it mounts once when the app starts
  App.register("editor", { mount });

  return { open, close };
})();
