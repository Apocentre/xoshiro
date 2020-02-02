declare module "xoshiro" {
  interface PrngState {
    roll(k?: number): number;
    shuffle(arr: any[]): void;
  }

  type Algorithm = "256+" | "256++" | "256**" | "512+" | "512++" | "512**";

  function create(alg: Algorithm, seed: Buffer | ArrayBuffer): PrngState;
}
