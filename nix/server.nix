{ stdenv, buildInputs }:

stdenv.mkDerivation {
  inherit buildInputs;
  name = "learning";
  version = "0-unstable-2024-10-07";

  src = ../.;

  buildPhase = ''
    runHook preBuild

    make

    runHook postBuild
  '';

  installPhase = ''
    runHook preInstall

    mkdir -p $out/bin
    cp bin/server $out/bin

    runHook postInstall
  '';

  meta = {
    mainProgram = "server";
  };
}
