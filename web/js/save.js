// Auto-save queue: collects changed fields, sends them after a short pause (or right away), and only
// ever has one request in flight. The request and the timers are passed in, so it can be unit tested.
function createSaver({
  send,
  delay = 500,
  timer = { set: (fn, ms) => setTimeout(fn, ms), clear: (id) => clearTimeout(id) },
  onStatus = () => {},
}) {
  let pending = {};
  let waiting = null;
  let chain = Promise.resolve();
  let unsent = 0;   // requests queued or in flight

  function flush() {
    if (waiting !== null) {
      timer.clear(waiting);
      waiting = null;
    }
    const fields = pending;
    pending = {};
    if (Object.keys(fields).length === 0) return chain;

    onStatus("saving");
    unsent++;
    chain = chain
      .then(() => send(fields))
      .then((result) => { unsent--; onStatus("saved", result); }, (error) => { unsent--; onStatus("error", error); });
    return chain;
  }

  function change(fields, { immediate = false } = {}) {
    Object.assign(pending, fields);
    if (waiting !== null) timer.clear(waiting);
    waiting = null;
    if (immediate) return flush();
    waiting = timer.set(flush, delay);
    return chain;
  }

  // True while there are changes the grill hasn't confirmed yet: waiting for the pause, or a request
  // queued or in flight. Inside onStatus the request it reports on no longer counts.
  function hasPending() {
    return Object.keys(pending).length > 0 || unsent > 0;
  }

  return { change, flush, hasPending };
}

if (typeof module === "object" && module.exports) module.exports = { createSaver };
