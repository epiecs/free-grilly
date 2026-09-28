// Firmware updates card: admin password, and installing an -ota.bin firmware file.
(() => {
  let version, adminInput, adminRemove, warning, note, fileInput, authField, authInput, updateButton, progress, notice;
  let saver;
  let retryAdmin = null;   // an admin password change waiting for the right current password

  const sleep = (ms) => new Promise((resolve) => setTimeout(resolve, ms));

  // After the restart the grill takes a few seconds to come back, poll until it answers
  async function waitForGrill() {
    await sleep(4000);
    for (let attempt = 0; attempt < 45; attempt++) {
      try {
        const status = await Api.get("/api/grill", 3000);
        return status.firmware_version;
      } catch (error) {
        await sleep(2000);
      }
    }
    return null;
  }

  // Changing the admin password and installing firmware both need the current admin password, when one is set
  function currentPasswordMissing(show) {
    const settings = Settings.current();
    if (!settings || !settings.admin_password_set || authInput.value !== "") return false;
    show("Enter the current admin password first.");
    authInput.focus();
    return true;
  }

  function changeAdmin(password) {
    retryAdmin = password;
    if (currentPasswordMissing((text) => Controls.showNote(note, "error", new Error(text)))) return;
    saver.change({ admin_password: password }, { immediate: true });
  }

  function setProgress(fraction) {
    progress.firstElementChild.style.width = Math.round(fraction * 100) + "%";
  }

  async function runUpdate() {
    const file = fileInput.files[0];
    if (!file) return;
    if (currentPasswordMissing((text) => { notice.textContent = text; })) return;
    updateButton.disabled = true;
    fileInput.disabled = true;
    progress.hidden = false;
    setProgress(0);
    notice.textContent = "Uploading…";
    try {
      await Api.upload("/api/update", file, authInput.value, setProgress);
      notice.textContent = "Installing… the grill restarts.";
      const newVersion = await waitForGrill();
      if (newVersion) {
        notice.textContent = "Updated. The grill now runs " + newVersion + ".";
        setTimeout(() => location.reload(), 1500);   // load the web app of the new firmware
      } else {
        notice.textContent = "The grill hasn't come back yet. Reload this page in a minute.";
      }
    } catch (error) {
      notice.textContent = error.message;
      progress.hidden = true;
    } finally {
      fileInput.disabled = false;
      updateButton.disabled = !fileInput.files[0];
    }
  }

  function build() {
    const card = document.createElement("section");
    card.className = "card settings-card";
    card.innerHTML =
      '<h2 class="card-title"><span>Firmware updates</span><span class="save-note" role="status"></span></h2>' +
      '<p>Installed version: <strong data-version></strong></p>' +
      '<label class="field" data-auth-field><span>Current admin password</span><input type="password" autocomplete="current-password" data-auth></label>' +
      '<label class="field"><span>Admin password</span><input type="password" autocomplete="new-password" data-admin></label>' +
      '<button type="button" class="link-button" data-admin-remove>Remove saved password</button>' +
      '<p class="warning" data-warning>No admin password is set. Anyone on your network can install firmware.</p>' +
      '<h3 class="subheading">Install an update</h3>' +
      '<label class="field"><span>Firmware file</span><input type="file" accept=".bin" data-file></label>' +
      '<p class="hint">Use the grilly-plus-…-ota.bin file from the releases page.</p>' +
      '<button type="button" class="button primary" data-update disabled>Update</button>' +
      '<div class="progress upload" hidden><i></i></div>' +
      '<p class="notice" role="status"></p>';

    version = card.querySelector("[data-version]");
    adminInput = card.querySelector("[data-admin]");
    adminRemove = card.querySelector("[data-admin-remove]");
    warning = card.querySelector("[data-warning]");
    note = card.querySelector(".save-note");
    fileInput = card.querySelector("[data-file]");
    authField = card.querySelector("[data-auth-field]");
    authInput = card.querySelector("[data-auth]");
    updateButton = card.querySelector("[data-update]");
    progress = card.querySelector(".progress");
    notice = card.querySelector(".notice");

    saver = createSaver({
      send: async (fields) => {
        const result = await Api.post("/api/settings", fields, 5000, authInput.value);
        authInput.value = fields.admin_password;   // the new password is the current one from now on
        return result;
      },
      onStatus: (state, detail) => {
        Controls.showNote(note, state, detail);
        if (state === "saved") retryAdmin = null;
        // Only a wrong current password is worth retrying, other errors need a new change
        if (state === "error" && !(detail && detail.status === 401)) retryAdmin = null;
        if (state === "saved" && detail) Settings.fill(detail);
      },
    });
    adminInput.addEventListener("change", () => {
      if (adminInput.value !== "") changeAdmin(adminInput.value);
    });
    adminRemove.addEventListener("click", () => changeAdmin(""));
    // Correcting the current password retries a refused change, without typing the new one again
    authInput.addEventListener("change", () => {
      if (retryAdmin !== null && authInput.value !== "") changeAdmin(retryAdmin);
    });
    fileInput.addEventListener("change", () => { updateButton.disabled = !fileInput.files[0]; notice.textContent = ""; });
    updateButton.addEventListener("click", runUpdate);
    return card;
  }

  function fill(settings) {
    const isSet = !!settings.admin_password_set;
    version.textContent = settings.firmware_version;
    adminInput.placeholder = isSet ? "Saved, type to change" : "Not set";
    if (document.activeElement !== adminInput && !saver.hasPending()) adminInput.value = "";
    adminRemove.hidden = !isSet;
    warning.hidden = isSet;
    authField.hidden = !isSet;
  }

  Settings.addCard({ build, fill });
})();
