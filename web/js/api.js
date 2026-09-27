// Talks to the grill's JSON API. Paths are relative, so the dev server can proxy or mock them.
const Api = (() => {
  async function request(path, options, timeoutMs) {
    const controller = new AbortController();
    const timer = setTimeout(() => controller.abort(), timeoutMs);
    try {
      const response = await fetch(path, { ...options, signal: controller.signal, cache: "no-store" });
      const text = await response.text();
      let body = null;
      if (text) {
        try { body = JSON.parse(text); } catch (error) { body = null; }
      }
      if (!response.ok) {
        const error = new Error(body && body.error ? body.error : "The grill answered " + response.status);
        error.status = response.status;
        throw error;
      }
      return body;
    } catch (error) {
      if (error.name === "AbortError") {
        const timeout = new Error("The grill didn't answer in time");
        timeout.timeout = true;
        throw timeout;
      }
      if (error instanceof TypeError) throw new Error("Can't reach the grill");
      throw error;
    } finally {
      clearTimeout(timer);
    }
  }

  function get(path, timeoutMs = 3000) {
    return request(path, {}, timeoutMs);
  }

  function post(path, data, timeoutMs = 5000) {
    return request(path, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(data),
    }, timeoutMs);
  }

  // Firmware upload with progress. XMLHttpRequest because fetch can't report upload progress.
  function upload(path, file, password, onProgress) {
    return new Promise((resolve, reject) => {
      const xhr = new XMLHttpRequest();
      xhr.open("POST", path);
      xhr.timeout = 180000;
      xhr.setRequestHeader("X-Grilly-Update", "1");
      if (password) {
        const credentials = new TextEncoder().encode("admin:" + password);
        xhr.setRequestHeader("Authorization", "Basic " + btoa(String.fromCharCode(...credentials)));
      }
      xhr.upload.onprogress = (event) => { if (event.lengthComputable) onProgress(event.loaded / event.total); };
      xhr.onload = () => {
        let body = null;
        try { body = JSON.parse(xhr.responseText); } catch (error) { body = null; }
        if (xhr.status === 200) resolve(body);
        else reject(new Error(body && body.error ? body.error : "The update failed (" + xhr.status + ")"));
      };
      xhr.onerror = () => reject(new Error("The upload was interrupted"));
      xhr.ontimeout = () => reject(new Error("The upload took too long"));
      const form = new FormData();
      form.append("firmware", file, file.name);
      xhr.send(form);
    });
  }

  return { get, post, upload };
})();
