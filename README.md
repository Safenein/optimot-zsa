# Optimot natif pour ZSA Moonlander

Firmware QMK qui rend la disposition [Optimot](https://optimot.fr/) Ergo 1.8.0 **native au clavier**.
L'ordinateur reste réglé en **AZERTY standard (France)**, sans pilote Optimot à installer.

Les couches sont celles de la configuration Oryx « Optimot VIM Emacs » (`7Ozzp`, révision `Mad3Ab`) :
le clavier envoie toujours `KC_Q`, `KC_SCLN`…, mais le firmware les traduit comme le faisait le pilote
Optimot de l'OS.

- **4 niveaux** : base, Maj, AltGr (touche `KC_RALT`), AltGr+Maj.
- **Touche Verr. Maj** (`KC_CAPS`) : elle fonctionne comme l'option xkb `caps:escape_shifted_capslock`,
  sur toutes les couches. Seule, elle donne Échap. Avec Maj, elle active ou coupe le verrouillage
  majuscules. Ce verrouillage est géré dans le clavier et respecte les types de touches d'Optimot :
  par exemple, la rangée du haut donne alors les chiffres. Une LED blanche l'indique. En Gaming,
  c'est le verrouillage de l'hôte qui bascule.
- **Caps Word** (`CW_TOGG`, ou double Maj) : il met en majuscules toutes les lettres Optimot, é, à, ç… compris.
- **Toutes les touches mortes** du pilote Linux officiel : ^ ¨ ´ ` ~ ¸ ˚ ˇ ¯ ˘ ˙ ̛ ¤, plus les touches
  mortes ∞ (sciences), µ (grec), ж (cyrillique), ø, Ľ, ↑ ↓ →. Cela fait 8 102 séquences.
  Une séquence invalide donne le signe de la touche morte suivi de la touche tapée.
- **Raccourcis mixtes** : avec Ctrl, on obtient la lettre du niveau Ctrl d'Optimot.
  Par exemple, Ctrl+è donne Ctrl+C et Ctrl+. donne Ctrl+V.
- **Couche Gaming** : elle n'est **pas** traduite. Les jeux reçoivent les positions physiques.

## Comment les caractères arrivent à l'hôte

1. Les caractères qui existent sur l'AZERTY sont tapés directement, ce qui garde la répétition
   automatique. Les voyelles accentuées â ê ë… passent par les touches mortes ^ et ¨ de l'AZERTY.
2. Pour les autres (« » ’ œ É … → ≠…), le firmware utilise la saisie Unicode de l'OS :
   - **Linux** : il tape Ctrl+Maj+U, le code hexadécimal, puis Espace. Ça marche avec IBus, et avec
     fcitx5 (mode « Unicode direct », actif par défaut), dans les applications où la méthode de
     saisie est active.
   - **Windows** : il utilise les codes Alt du pavé numérique (Alt+0171 = «). C'est natif pour tous
     les caractères de Windows-1252 (« » ’ “ ” œ Œ É … – — • © ® ™ ± × ÷ ¼ ½ ¾ ¿ ¡ ß æ ø…).
     Le firmware active Verr. Num le temps de la saisie si nécessaire. Les autres caractères (→ ≠ ≈
     ∞, grec…) demandent le réglage de registre suivant (une fois, puis redémarrer la session) :
     ```
     reg add "HKCU\Control Panel\Input Method" /v EnableHexNumpad /t REG_SZ /d 1
     ```

**OS hôte** : il est détecté automatiquement au branchement. On peut le forcer depuis la couche
Mouse (`OSL`) :

| Touche (colonne de gauche) | Effet |
|---|---|
| ligne 1 | détection automatique |
| ligne 2 | Linux |
| ligne 3 | Windows |

Le mode actif s'allume en vert quand la couche Mouse est affichée, et le choix est gardé en mémoire.

## Réglage de l'hôte

- **Linux / Hyprland** : `input { kb_layout = fr }`, sans variante, et retirer la disposition Optimot.
- **Windows** : « Français (France) – AZERTY ».

## Compiler et flasher

L'environnement est fourni par [devenv](https://devenv.sh). Le dossier `qmk_firmware/` est un sous-module
qui pointe vers le fork ZSA (`firmware25`, la version utilisée par Oryx).

```sh
git submodule update --init --recursive   # première fois
devenv shell run-tests   # tests du moteur (8 102 séquences) et de la table AZERTY
devenv shell build       # -> build/zsa_moonlander_reva_optimot.bin
devenv shell flash       # clavier en mode bootloader (touche QK_BOOT, couche Mouse)
```

On peut aussi flasher `build/zsa_moonlander_reva_optimot.bin` avec Keymapp. Sous NixOS, le flash
direct demande les règles udev : `hardware.keyboard.zsa.enable = true;`.

Le firmware Oryx d'origine est gardé dans `vendor/oryx/`, pour revenir en arrière.

## Organisation

| Chemin | Rôle |
|---|---|
| `vendor/optimot/` | pilote Linux officiel Optimot Ergo 1.8.0 (`.xkb`, `.XCompose`, licence) |
| `vendor/oryx/` | export de la configuration Oryx et firmware d'origine |
| `tools/gen_optimot.py` | génère `optimot_tables.h` (niveaux, trie des touches mortes, table AZERTY) |
| `keyboards/zsa/moonlander/keymaps/optimot/` | keymap QMK (userspace) |
| `  optimot.c` | moteur pur : niveaux, raccourcis, touches mortes (testé sur l'hôte) |
| `  host_azerty.c` | émission vers l'hôte AZERTY, saisie Unicode Linux / Windows |
| `  keymap.c` | couches Oryx, interception des touches, Caps Word, témoins lumineux |
| `tests/` | tests sur l'hôte (gcc, et `xkbcli` pour l'AZERTY) |

## Licence

La disposition Optimot, ses pilotes et sa documentation sont © Patrick Jamet — <https://optimot.fr/>,
sous licence [CC BY-NC-SA 4.0](https://creativecommons.org/licenses/by-nc-sa/4.0/deed.fr).
Ce dépôt en est une adaptation (portage en firmware), distribuée sous la même licence, sans usage
commercial. Le fork QMK de ZSA (`qmk_firmware/`) reste sous GPL-2.0.
