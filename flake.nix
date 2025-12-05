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
        devShells.default = pkgs.mkShell {
          buildInputs = with pkgs; [
            gnumake
            clang
            valgrind
            gdb
            git
            lazygit
            zirco-pkgs.packages.${system}.zrc
          ];
        };
        packages.zrc = pkgs.stdenv.mkDerivation {
          name = "zrc";
          buildInputs = [
            pkgs.gnumake
            pkgs.clang
            pkgs.which
            zirco-pkgs.packages.${system}.zrc
          ];
          src = ./.;
          buildPhase = ''
            cp -r include src/
            make clean all
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
