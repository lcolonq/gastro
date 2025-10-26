{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    flake-utils.url = "github:numtide/flake-utils";
    qoi.url = "github:lcolonq/qoi";
  };

  outputs = { self, nixpkgs, ... }@inputs:
    inputs.flake-utils.lib.eachDefaultSystem
      (system:
        let
          pkgs = nixpkgs.legacyPackages.${system};
          gastro = pkgs.stdenv.mkDerivation {
            pname = "gastro";
            version = "git";
            src = ./.;
            hardeningDisable = ["all"];
            installPhase = ''
              make prefix=$out install
            '';
          };
        in {
          packages = {
            inherit gastro;
            default = gastro;
          };
          devShells.default = pkgs.mkShell {
            hardeningDisable = ["all"];
            buildInputs = [
              pkgs.clang
              pkgs.raylib
              pkgs.valgrind
              inputs.qoi.packages.x86_64-linux.default
            ];
          };
        }
      );
}
