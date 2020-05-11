declare module "xoshiro" {
  interface PrngState {
    roll(k?: number): number
    shuffle(arr: any[] | NodeJS.TypedArray): void
    stash(): void
    restore(): void
    count: number
  }

  type Algorithm = "256+" | "256++" | "256**" | "512+" | "512++" | "512**"

  function create(alg: Algorithm, seed: NodeJS.TypedArray | DataView | ArrayBuffer): PrngState
}
