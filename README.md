# Xoshiro

![Node.js CI](https://github.com/0x10001/xoshiro/workflows/Node.js%20CI/badge.svg)
[![npm version](https://badge.fury.io/js/xoshiro.svg)](https://badge.fury.io/js/xoshiro)

## Apocentre Fork

This fork makes this package compatible with the [BC-UR](https://github.com/BlockchainCommons/Research/blob/master/papers/bcr-2020-005-ur.md) specification for use with the 256** algorithm.

## What is this?

A pseudo-random-number-generator module implemented in N-API. [Here](http://prng.di.unimi.it/) is all about the algorithms.

## Requirements

- [CMake](http://www.cmake.org/download/)
- A proper C/C++ compiler toolchain of the given platform
  - **Windows**:
        - [Visual C++ Build Tools](https://visualstudio.microsoft.com/visual-cpp-build-tools/)
        or a recent version of Visual C++ will do ([the free Community](https://www.visualstudio.com/products/visual-studio-community-vs) version works well)
  - **Unix/Posix**:
        - Clang or GCC
        - Make

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

// store the current state
const s = prng.state
// generate a random number
const x = prng.roll()
// restore the state
prng.state = s
// generate a random number from the previous state
const y = prng.roll()
// and they should be equal
console.log(x === y)  // -> true
```

### Supported algorithms

- `'256+'` __xoshiro256+__, requires the seed to be of at least 32 bytes
- `'256++'` __xoshiro256++__, requires the seed to be of at least 32 bytes
- `'256**'` __xoshiro256**__, requires the seed to be of at least 32 bytes
- `'512+'` __xoshiro512+__, requires the seed to be of at least 64 bytes
- `'512++'` __xoshiro512++__, requires the seed to be of at least 64 bytes
- `'512**'` __xoshiro512**__, requires the seed to be of at least 64 bytes

Note: In order to make it work, the seed used to initialize the PRNG should not be all 0's.
