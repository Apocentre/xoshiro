declare module "xoshiro" {
  type NodeArray = NodeJS.TypedArray | DataView | ArrayBuffer
  interface PrngState {
    roll(): number
    state: NodeArray
  }

  type Algorithm = "256+" | "256++" | "256**" | "512+" | "512++" | "512**"

  function create(alg: Algorithm, seed: NodeArray): PrngState
}
