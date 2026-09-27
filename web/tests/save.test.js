const test = require("node:test");
const assert = require("node:assert/strict");
const { createSaver } = require("../js/save.js");

function fakeTimer() {
  let next = 1;
  const jobs = new Map();
  return {
    set(fn) { const id = next++; jobs.set(id, fn); return id; },
    clear(id) { jobs.delete(id); },
    run() { const fns = [...jobs.values()]; jobs.clear(); fns.forEach((fn) => fn()); },
    get size() { return jobs.size; },
  };
}

test("changes within the delay are merged into one send", async () => {
  const timer = fakeTimer();
  const sent = [];
  const saver = createSaver({ send: async (fields) => { sent.push(fields); return "ok"; }, timer });

  saver.change({ a: 1 });
  saver.change({ b: 2 });
  saver.change({ a: 3 });
  assert.equal(timer.size, 1);
  timer.run();
  await saver.flush();

  assert.deepEqual(sent, [{ a: 3, b: 2 }]);
});

test("immediate sends pending fields right away", async () => {
  const timer = fakeTimer();
  const sent = [];
  const saver = createSaver({ send: async (fields) => { sent.push(fields); }, timer });

  saver.change({ a: 1 });
  await saver.change({ name: "x" }, { immediate: true });

  assert.deepEqual(sent, [{ a: 1, name: "x" }]);
  assert.equal(timer.size, 0);
  assert.equal(saver.hasPending(), false);
});

test("sends run one at a time", async () => {
  const timer = fakeTimer();
  const calls = [];
  let release;
  const saver = createSaver({
    timer,
    send: (fields) => {
      calls.push(fields);
      if (calls.length === 1) return new Promise((resolve) => { release = resolve; });
      return Promise.resolve();
    },
  });

  saver.change({ a: 1 }, { immediate: true });
  const second = saver.change({ b: 2 }, { immediate: true });
  await Promise.resolve();
  assert.deepEqual(calls, [{ a: 1 }]);

  release();
  await second;
  assert.deepEqual(calls, [{ a: 1 }, { b: 2 }]);
});

test("reports saving, saved and error, and keeps working after an error", async () => {
  const timer = fakeTimer();
  const states = [];
  let fail = true;
  const saver = createSaver({
    timer,
    send: async () => { if (fail) throw new Error("nope"); return { saved: true }; },
    onStatus: (state, detail) => states.push([state, detail && (detail.message || detail.saved)]),
  });

  await saver.change({ a: 1 }, { immediate: true });
  fail = false;
  await saver.change({ a: 2 }, { immediate: true });

  assert.deepEqual(states, [["saving", undefined], ["error", "nope"], ["saving", undefined], ["saved", true]]);
});

test("flush without changes does not send", async () => {
  let sends = 0;
  const saver = createSaver({ send: async () => { sends++; }, timer: fakeTimer() });
  await saver.flush();
  assert.equal(sends, 0);
});

test("hasPending covers waiting changes and queued requests, but not the one being reported", async () => {
  const timer = fakeTimer();
  const releases = [];
  const seen = [];
  const saver = createSaver({
    timer,
    send: (fields) => new Promise((resolve) => { releases.push(() => resolve(fields)); }),
    onStatus: (state, detail) => { if (state === "saved") seen.push([detail.v, saver.hasPending()]); },
  });

  assert.equal(saver.hasPending(), false);
  saver.change({ v: 75 });
  assert.equal(saver.hasPending(), true);         // waiting for the pause
  timer.run();
  await Promise.resolve();
  assert.equal(saver.hasPending(), true);         // 75 in flight
  saver.change({ v: 77 });
  releases[0]();
  await new Promise((resolve) => setImmediate(resolve));
  assert.deepEqual(seen, [[75, true]]);           // 77 still waiting, so the 75 answer must not refill

  timer.run();
  await new Promise((resolve) => setImmediate(resolve));
  releases[1]();
  await saver.flush();
  assert.deepEqual(seen, [[75, true], [77, false]]);
  assert.equal(saver.hasPending(), false);
});
