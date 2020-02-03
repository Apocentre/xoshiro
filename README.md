# Xoshiro

## What is this?

A pseudo-random-number-generator module implemented in N-API. [Here](http://prng.di.unimi.it/) is all about the algorithms.

## Requirements

[**Cmake**](https://cmake.org/download) is strongly recommended for building. Make sure it is installed first.

## Installation

Just run this command:

```shell script
npm install --save xoshiro
```

## Usage

```javascript
const xoshiro = require('xoshiro');
const crypto = require('crypto');

// create a PRNG with an algorithm and a seed
const seed = crypto.randomBytes(32);
const prng = xoshiro.create('256+', seed);

// generate a random unsigned 32-bit integer
console.log(prng.roll());
// generate a random unsigned integer in range [0, 10) (10 excluded)
console.log(prng.roll(10));

// shuffle elements in an array (or typed array)
const arr = [5, 4, 3, 2, 1];
prng.shuffle(arr);
console.log(arr);
```

### Supported algorithms

- `'256+'` __xoshiro256+__, requires the seed to be of at least 32 bytes
- `'256++'` __xoshiro256++__, requires the seed to be of at least 32 bytes
- `'256**'` __xoshiro256**__, requires the seed to be of at least 32 bytes
- `'512+'` __xoshiro512+__, requires the seed to be of at least 64 bytes
- `'512++'` __xoshiro512++__, requires the seed to be of at least 64 bytes
- `'512**'` __xoshiro512**__, requires the seed to be of at least 64 bytes

Note: In order to make it work, the seed used to initialize the PRNG should not be all 0's.
