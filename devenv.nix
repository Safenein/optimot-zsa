{
  pkgs,
  config,
  ...
}: let
  kb = "zsa/moonlander/reva";
  km = "optimot";
in {
  # qmk embarque gcc-arm-embedded, dfu-util et ses dépendances Python.
  packages = with pkgs; [
    qmk
    python3
    gcc
    git
    libxkbcommon # xkbcli, pour vérifier la table AZERTY dans les tests
    xkeyboard_config
  ];

  env = {
    QMK_HOME = "${config.devenv.root}/qmk_firmware";
    QMK_USERSPACE = config.devenv.root;
    KEYSYMDEF = "${pkgs.xorg.xorgproto}/include/X11/keysymdef.h";
    XKB_CONFIG_ROOT = "${pkgs.xkeyboard_config}/share/X11/xkb";
  };

  scripts = {
    gen.exec = ''
      python3 "$DEVENV_ROOT/tools/gen_optimot.py"
    '';
    run-tests.exec = ''
      set -e
      gen
      python3 "$DEVENV_ROOT/tests/gen_vectors.py"
      cc -std=c11 -Wall -Wextra -Werror -O1 -DOPT_HOST_TEST \
        -I "$DEVENV_ROOT/keyboards/zsa/moonlander/keymaps/${km}" \
        -I "$DEVENV_ROOT/build" -I "$DEVENV_ROOT/tests" \
        -o "$DEVENV_ROOT/build/test_engine" \
        "$DEVENV_ROOT/tests/test_engine.c" \
        "$DEVENV_ROOT/keyboards/zsa/moonlander/keymaps/${km}/optimot.c"
      "$DEVENV_ROOT/build/test_engine"
      [ -f "$DEVENV_ROOT/tests/test_azerty.py" ] && python3 "$DEVENV_ROOT/tests/test_azerty.py"
    '';
    build.exec = ''
      set -e
      gen
      git -C "$DEVENV_ROOT" submodule update --init qmk_firmware
      qmk compile -kb ${kb} -km ${km} "$@"
      mkdir -p "$DEVENV_ROOT/build"
      cp "$QMK_HOME"/zsa_moonlander_reva_${km}.bin "$DEVENV_ROOT/build/" 2>/dev/null \
        || cp "$DEVENV_ROOT"/zsa_moonlander_reva_${km}.bin "$DEVENV_ROOT/build/"
      ls -l "$DEVENV_ROOT/build/"*.bin
    '';
    # Clavier en mode bootloader (touche QK_BOOT ou bouton reset), puis :
    flash.exec = ''
      set -e
      build
      qmk flash -kb ${kb} -km ${km}
    '';
  };

  enterShell = ''
    echo "Optimot Moonlander : gen | run-tests | build | flash"
  '';
}
