// About view: firmware and connection details, the repository, and credits.
(() => {
  const fields = {};

  function fill(status) {
    fields.name.textContent = status.name;
    fields.version.textContent = status.firmware_version;
    fields.wifi.textContent = status.wifi_connected
      ? "Connected to " + status.wifi_ssid
      : "Not connected to WiFi, reachable through the grill's hotspot";
    fields.ip.textContent = status.wifi_connected && status.wifi_ip ? status.wifi_ip : "Not on WiFi";
    // The hotspot is always on, so the grill can always be reached at this address
    fields.hotspot.textContent = status.local_ap_ssid + ", " + status.local_ap_ip;
    fields.uuid.textContent = status.unique_id;
  }

  function mount(el) {
    el.innerHTML =
      '<div class="about">' +
      '  <div class="wordmark about-wordmark">Grilly<b>+</b></div>' +
      '  <dl class="card about-list">' +
      '    <dt>Grill</dt><dd data-about="name"></dd>' +
      '    <dt>Firmware</dt><dd data-about="version"></dd>' +
      '    <dt>WiFi</dt><dd data-about="wifi"></dd>' +
      '    <dt>IP address</dt><dd data-about="ip"></dd>' +
      '    <dt>Hotspot</dt><dd data-about="hotspot"></dd>' +
      '    <dt>UUID</dt><dd data-about="uuid"></dd>' +
      '  </dl>' +
      '  <p><a class="button" href="https://github.com/bardesss/grilly-plus" target="_blank" rel="noopener">Grilly+ on GitHub</a></p>' +
      '  <p class="hint">Grilly+ is a fork of <a href="https://github.com/epiecs/free-grilly" target="_blank" rel="noopener">Free-Grilly</a> ' +
      'by <a href="https://github.com/epiecs" target="_blank" rel="noopener">Epiecs</a> and ' +
      '<a href="https://questum.be" target="_blank" rel="noopener">Questum</a>.</p>' +
      '</div>';
    el.querySelectorAll("[data-about]").forEach((dd) => { fields[dd.dataset.about] = dd; });
    App.onStatus(fill);
  }

  App.register("about", { mount });
})();
