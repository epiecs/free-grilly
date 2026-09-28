const test = require("node:test");
const assert = require("node:assert/strict");
const Controls = require("../js/controls.js");

test("roundToStep rounds to the step's own decimals, avoiding float noise", () => {
  assert.equal(Controls.roundToStep(1.5, 0.1), 1.5);
  assert.equal(Controls.roundToStep(0.1 + 0.2, 0.1), 0.3);
  assert.equal(Controls.roundToStep(-10.05, 0.1), -10);
  assert.equal(Controls.roundToStep(1.24, 0.1), 1.2);
  assert.equal(Controls.roundToStep(1.26, 0.1), 1.3);
  assert.equal(Controls.roundToStep(7, 1), 7);
  assert.equal(Controls.roundToStep(7.4, 1), 7);
});
