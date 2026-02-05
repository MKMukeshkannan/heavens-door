{
  description = "A Nix-flake-based C/C++ development environment";
  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-25.11";
  outputs = { self, nixpkgs }:
    let
      supportedSystems = [ "x86_64-linux" "aarch64-linux" "x86_64-darwin" "aarch64-darwin" ];
      forEachSupportedSystem = f: nixpkgs.lib.genAttrs supportedSystems (system: f {
        pkgs = import nixpkgs { inherit system; };
      });
    in
    {
      devShells = forEachSupportedSystem ({ pkgs }: {
        default = pkgs.mkShellNoCC.override
          {
            stdenv = pkgs.llvmPackages_20.stdenv;
          }
          {
            packages = with pkgs; [
              llvmPackages_20.clang-tools
              cmake
              gnumake
              raylib
              ninja
            ];
          };
      });
    };
}
