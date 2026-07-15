{
  description = "A very basic flake";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
    zirco-pkgs.url = "github:zirco-lang/zrc/main";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils, zirco-pkgs }:
    flake-utils.lib.eachDefaultSystem(system:
      let
        pkgs = import nixpkgs { inherit system; };
        llvm = pkgs.llvmPackages_20;
      in {
        devShells.default = (pkgs.mkShell.override {
		stdenv = llvm.libcxxStdenv;
	}) {
          buildInputs = with pkgs; [
            llvm.clang
            gnumake
            valgrind
            gdb
            git
            lazygit
            llvm.llvm
	    llvm.libllvm
          ];
        };
        packages.zrc = pkgs.stdenv.mkDerivation {
          name = "zrc";
          buildInputs = [
            pkgs.gnumake
            pkgs.clang
	    llvm.llvm
          ];
          src = ./.;
          buildPhase = ''
	    make all
	  '';
          installPhase = ''
            mkdir -p $out/bin
            cp target/zrc $out/bin/
          '';
      };

      packages.default = self.packages.${system}.zrc;

    }
  );
}
