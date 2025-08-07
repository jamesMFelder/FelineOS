{ pkgs ? import <nixpkgs> {} }:
let
  # Needed because gcc15 and above (nb. this was written with gcc=14)
  # don't need one of the patches in nixpkgs, so this removes it
  # Can be removed once https://github.com/NixOS/nixpkgs/pull/422046 is merged
  patched = (import <nixpkgs> {}).applyPatches {
    name = "nixpkgs-gcc-patches";
    src = <nixpkgs>;
    patches = [ nix-patches/gcc.patch ];
  };
  # Newlib (in libgloss) has a bunch of weird errors, like how did it ever compile?
  # This fixes it until https://github.com/NixOS/nixpkgs/issues/424403 is fixed.
  nixpkgs_patched = (import patched {overlays = [(final: prev: {
    newlib = prev.newlib.overrideAttrs(nfinal: nprev: {
      patches = nprev.patches ++ [ nix-patches/newlib-fix-gloss.patch ];
    });
  })];});
in
pkgs.mkShell {
  nativeBuildInputs =
    with nixpkgs_patched; [
        # For building
        cmake
        pkgsCross.i686-embedded.buildPackages.gcc
        pkgsCross.arm-embedded.buildPackages.gcc
        # For debugging
        gdb
        # Other dev tools
        clang-tools # for LSP
        # For the i386 iso
        grub2
        xorriso
        libisoburn
    ];
}
