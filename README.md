# OpenZRC

This is a community-driven Zirco compiler

This implementation is `partial` and more **unstable** than the offical [zrc](https://github.com/zirco-lang/zrc)

The ultimate goal for this compiler is for it to be self-hosting in Zirco, with a smaller implementation used for bootstrapping written in C.

There will be multiple stages to this compiler, each one bringing in more features.

Currently we are developing stage 1, which should be a partial implementation (see docs for more)

This should only provide enough code to support the compilation of a compiler written in Zirco itself.

The second stage should actually implement most features of Zirco, which will compile into `stage3`, the actual zirco compiler.

## A Note on the current implementation

Currently, while stage 1 is being developed, very little is functional.
Code in this `main` branch will compile, but it will only compile some features as
they get implemented. This is intentional, so we are cautious to make this function in
stages. The file `example.zr` is the latest test that works.


