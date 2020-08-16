const xoshiro = require('bindings')('xoshiro');
const crypto = require('crypto');
const assert = require('assert');

const baseTest = function (alg, len) {
  it('invalid length', function () {
    assert.throws(function () {
      xoshiro.create(alg, crypto.randomBytes(len - 1));
    });
  });

  it('seeding ArrayBuffer', function () {
    const seed = crypto.randomBytes(len).buffer;
    const prng = xoshiro.create(alg, seed);
    assert.strictEqual(prng.roll() % 1, 0);
  });

  it('seeding Dataview', function () {
    const offset = 8;
    const seed = new DataView(crypto.randomBytes(len + offset).buffer, offset);
    const prng = xoshiro.create(alg, seed);
    assert.strictEqual(prng.roll() % 1, 0);
  });

  const prng = xoshiro.create(alg, crypto.randomBytes(len));

  it('the same seeds', function () {
    const seed = crypto.randomBytes(len);
    const p0 = xoshiro.create(alg, seed);
    const p1 = xoshiro.create(alg, seed);

    assert.strictEqual(p0.roll(), p1.roll());
  });

  it('stash and restore', function () {
    const buf = prng.state;
    const x = prng.roll();
    prng.state = buf;
    const y = prng.roll();
    assert.strictEqual(x, y);
  });
};

const algs = [
  {alg: '256+', len: 32},
  {alg: '256++', len: 32},
  {alg: '256**', len: 32},
  {alg: '512+', len: 64},
  {alg: '512++', len: 64},
  {alg: '512**', len: 64},
];

algs.forEach(function ({alg, len}) {
  describe(alg, function () {
    baseTest(alg, len);
  });
});
