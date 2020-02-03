const xoshiro = require('bindings')('xoshiro');
const crypto = require('crypto');

const baseTest = function (alg, len) {
  test('invalid length', function () {
    expect(function () {
      xoshiro.create(alg, crypto.randomBytes(len - 1));
    }).toThrow();
  });

  test('seeding ArrayBuffer', function () {
    const prng = xoshiro.create(alg, crypto.randomBytes(len).buffer);
    expect(prng.roll(1)).toBe(0);
  });

  const prng = xoshiro.create(alg, crypto.randomBytes(len));

  test('rolls in range', function () {
    const t = 2048;
    const rolls = Array.from({length: t}, () => prng.roll(t));
    expect(rolls.every((value) => value > -1 && value < t)).toBe(true);
  });

  test('no param', function () {
    expect(prng.roll() % 1 === 0).toBe(true);
  });

  test('the same seeds', function () {
    const seed = crypto.randomBytes(len);
    const p0 = xoshiro.create(alg, seed);
    const p1 = xoshiro.create(alg, seed);
    const upper = prng.roll();

    // test that passing 0 or nothing gives the same result
    expect(p0.roll(0) - p1.roll()).toBe(0);
    expect(p0.roll(upper) - p1.roll(upper)).toBe(0);
  });

  test('shuffling', function () {
    const deck = Array.from({length: 100}, (_, index) => index);
    const copy = deck.slice();
    prng.shuffle(deck);
    expect(deck.sort((a, b) => a - b)).toEqual(copy);
  });

  test('shuffling with the same seeds', function () {
    const seed = crypto.randomBytes(len);
    const p0 = xoshiro.create(alg, seed);
    const p1 = xoshiro.create(alg, seed);
    const d0 = Array.from({length: 100}, (_, index) => index);
    const d1 = d0.slice();

    p0.shuffle(d0);
    p1.shuffle(d1);
    expect(d0).toEqual(d1);
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
