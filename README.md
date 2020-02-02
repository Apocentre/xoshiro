# Xoshiro

## What is this?

A pseudo-random-number-generator module implemented in N-API.

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

// generate a random integer
console.log(prng.roll());
console.log(prng.roll(10));

// shuffle elements in an array
const arr = [5, 4, 3, 2, 1];
prng.shuffle(arr);
console.log(arr);
```

## Todo

1. Add tests
2. Add API Docs in Readme
