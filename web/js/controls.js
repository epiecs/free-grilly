// Reusable controls shared by the probe editor and the settings view.
const Controls = (() => {
  // Number stepper: big - and + buttons (hold to repeat) around a number you can also type in
  function stepper(el, { min = 0, max = 100, step = 1, label, suffix = "", onChange }) {
    el.classList.add("stepper");
    el.innerHTML =
      '<button type="button" class="step-btn" data-dir="-1"><svg class="icon"><use href="#i-minus"/></svg></button>' +
      '<span class="step-value"><input type="number" inputmode="numeric"><span class="step-suffix"></span></span>' +
      '<button type="button" class="step-btn" data-dir="1"><svg class="icon"><use href="#i-plus"/></svg></button>';

    const input = el.querySelector("input");
    const suffixEl = el.querySelector(".step-suffix");
    const [down, up] = el.querySelectorAll(".step-btn");
    input.setAttribute("aria-label", label);
    down.setAttribute("aria-label", "Lower " + label.toLowerCase());
    up.setAttribute("aria-label", "Raise " + label.toLowerCase());

    let value = min;
    let describe = typeof suffix === "function" ? suffix : () => suffix;
    let holdTimer = null;
    let repeatTimer = null;
    let held = false;

    const clampValue = (v) => Math.min(max, Math.max(min, Math.round(v / step) * step));
    function render() {
      input.value = String(value);
      suffixEl.textContent = describe(value);
    }
    function set(v) {
      value = clampValue(Number(v) || 0);
      render();
    }
    function commit(v) {
      const before = value;
      set(v);
      if (value !== before) onChange(value);
    }
    function stop() {
      clearTimeout(holdTimer);
      clearInterval(repeatTimer);
      holdTimer = null;
      repeatTimer = null;
    }

    [down, up].forEach((button) => {
      const direction = Number(button.dataset.dir);
      button.addEventListener("click", () => {
        if (held) { held = false; return; }   // the hold already changed the value
        commit(value + direction * step);
      });
      button.addEventListener("pointerdown", () => {
        stop();
        held = false;
        holdTimer = setTimeout(() => {
          held = true;
          repeatTimer = setInterval(() => commit(value + direction * step), 80);
        }, 400);
      });
      ["pointerup", "pointerleave", "pointercancel"].forEach((type) => button.addEventListener(type, stop));
    });

    input.addEventListener("change", () => {
      const typed = parseFloat(input.value);
      if (isNaN(typed)) render();
      else commit(typed);
    });
    input.addEventListener("keydown", (event) => { if (event.key === "Enter") input.blur(); });

    render();
    return {
      set,
      get: () => value,
      setLimits(newMin, newMax) { min = newMin; max = newMax; set(value); },
      setSuffix(newSuffix) { describe = typeof newSuffix === "function" ? newSuffix : () => newSuffix; render(); },
      input,
    };
  }

  // Status text next to a card or editor title: Saving… / Saved (fades) / the error
  function showNote(el, state, detail) {
    clearTimeout(el.fadeTimer);
    el.classList.remove("saved", "error");
    if (state === "idle") el.textContent = "";
    if (state === "saving") el.textContent = "Saving…";
    if (state === "saved") {
      el.textContent = "Saved";
      el.classList.add("saved");
      el.fadeTimer = setTimeout(() => { el.textContent = ""; el.classList.remove("saved"); }, 2000);
    }
    if (state === "error") {
      el.textContent = detail && detail.message ? detail.message : "Could not save";
      el.classList.add("error");
    }
  }

  function isEditing(el) {
    return el.contains(document.activeElement);
  }

  return { stepper, showNote, isEditing };
})();
