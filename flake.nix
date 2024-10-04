{
  description = "ALMO is markdown parser and static site generator";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = {
    self,
    nixpkgs,
    flake-utils,
    ...
  }:
    flake-utils.lib.eachDefaultSystem (
      system: let
        pkgs = import nixpkgs {inherit system;};
        stableSrc = pkgs.fetchFromGitHub {
          owner = "abap34";
          repo = "almo";
          rev = "refs/tags/v0.9.5-alpha";
          sha256 = "sha256-Cz+XDJmdp+utzwm1c7ThTNS6kfNF6r4B16tnGQSCVMc=";
        };
        mkAlmoDerivation = {
          src,
          version,
          ...
        }:
          pkgs.stdenv.mkDerivation {
            pname = "almo";
            version = version;
            src = src;

            buildInputs = with pkgs; [
              gcc
              python312Packages.pybind11
            ];

            makeFlags = ["all"];

            # remove darwin-only linker flag on linux
            postPatch = pkgs.lib.optionalString (!pkgs.stdenv.isDarwin) ''
              substituteInPlace scripts/pybind.sh \
                --replace-fail " -undefined dynamic_lookup" ""
            '';

            installPhase = ''
              runHook preInstall
              mkdir -p $out/bin $out/lib
              cp build/almo $out/bin
              cp almo.so $out/lib
              runHook postInstall
            '';
          };
      in rec {
        devShells.default = pkgs.mkShell {
          buildInputs = with pkgs; [
            nil
            alejandra
            gcc
            gnumake
            python312
            python312Packages.pybind11
          ];
        };
        packages.stable = mkAlmoDerivation {
          src = stableSrc;
          version = "0.9.5-alpha";
        };
        packages.unstable = mkAlmoDerivation {
          src = ./.;
          version = "0.9.6-alpha";
        };
        packages.default = packages.stable;
        apps.${system}.default = {
          type = "app";
          program = "${self.packages.default}/bin/almo";
        };
      }
    );
}
