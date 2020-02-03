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
    assert.equal(prng.roll(1), 0);
  });

  it('seeding Dataview', function () {
    const offset = 8;
    const seed = new DataView(crypto.randomBytes(len + offset).buffer, offset);
    const prng = xoshiro.create(alg, seed);
    assert.equal(prng.roll(1), 0);
  });

  const prng = xoshiro.create(alg, crypto.randomBytes(len));

  it('rolls in range', function () {
    const t = 2048;
    const rolls = Array.from({length: t}, () => prng.roll(t));
    assert.equal(rolls.every((value) => value > -1 && value < t), true);
  });

  it('no param', function () {
    assert.equal(prng.roll() % 1, 0);
  });

  it('the same seeds', function () {
    const seed = crypto.randomBytes(len);
    const p0 = xoshiro.create(alg, seed);
    const p1 = xoshiro.create(alg, seed);
    const upper = prng.roll();

    // test that passing 0 or nothing should give the same result
    assert.equal(p0.roll(0), p1.roll());
    assert.equal(p0.roll(upper), p1.roll(upper));
  });

  it('shuffling arrays', function () {
    const deck = Array.from({length: 8}, (_, index) => index);
    const copy = deck.slice();
    prng.shuffle(deck);
    assert.deepStrictEqual(deck.sort((a, b) => a - b), copy);
  });

  it('shuffling typed arrays', function () {
    const deck = new Uint16Array(Array.from({length: 8}, (_, index) => index));
    const copy = deck.slice();
    prng.shuffle(deck);
    assert.deepStrictEqual(deck.sort((a, b) => a - b), copy);
  });

  it('shuffling with the same seeds', function () {
    const seed = crypto.randomBytes(len);
    const p0 = xoshiro.create(alg, seed);
    const p1 = xoshiro.create(alg, seed);
    const d0 = Array.from({length: 8}, (_, index) => index);
    const d1 = d0.slice();

    p0.shuffle(d0);
    p1.shuffle(d1);
    assert.deepStrictEqual(d0, d1);
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
