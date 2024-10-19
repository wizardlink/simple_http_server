{
  inputs = {
    utils.url = "github:numtide/flake-utils";
  };
  outputs =
    {
      self,
      nixpkgs,
      utils,
    }:
    utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
        buildInputs = with pkgs; [
          curlFull
          gcc
          gnumake
        ];
      in
      {
        devShell = pkgs.mkShell {
          buildInputs =
            buildInputs
            ++ (with pkgs; [
              man-pages
              man-pages-posix
            ]);
        };

        packages.default = pkgs.callPackage ./nix/server.nix { inherit buildInputs; };

        formatter.${system} = nixpkgs.legacyPackages.${system}.nixfmt;
      }
    );
}
