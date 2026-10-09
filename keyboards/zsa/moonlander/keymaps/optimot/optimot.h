// Moteur Optimot : niveaux, raccourcis Ctrl et touches mortes.
// Aucune dépendance à QMK, pour pouvoir être testé sur l'hôte (tests/test_engine.c).
//
// Optimot (c) Patrick Jamet, https://optimot.fr/ — CC BY-NC-SA 4.0.
#pragma once

#include <stdbool.h>
#include <stdint.h>

enum { OPT_T_CONTROL, OPT_T_ALPHA, OPT_T_SEMIALPHA };

#include "optimot_tables.h"

#define OPT_NO_POS 0xFF

// Reçoit les points de code produits (sortie de composition ou caractère simple).
typedef void (*opt_emit_fn)(uint32_t cp);

// Position Optimot d'un keycode basique (KC_Q...), ou OPT_NO_POS.
uint8_t opt_pos_from_keycode(uint16_t keycode);

// Niveau 0..3 (base, Maj, AltGr, AltGr+Maj) selon le type xkb de la position.
uint8_t opt_level(uint8_t pos, bool shift, bool altgr, bool caps);

uint8_t  opt_keysym(uint8_t pos, uint8_t level);
uint32_t opt_keysym_unicode(uint8_t keysym);
// Keycode AZERTY (HID) envoyé avec Ctrl : niveaux 5/6, « raccourcis mixtes ».
uint8_t opt_ctrl_keycode(uint8_t pos);
// Le niveau 1 est une lettre (pour Caps Word).
bool opt_is_letter(uint8_t pos);

// Point de code d'un keysym tapé seul, hors composition. Pour une touche morte,
// c'est sa forme isolée (séquence <dead> <space>) ; 0 si aucune.
uint32_t opt_standalone(uint8_t keysym);
// Le keysym ouvre une séquence de composition (touche morte).
bool opt_is_dead(uint8_t keysym);

// Composition : alimente l'automate avec un keysym. Émet les points de code
// produits : rien tant qu'une séquence est en cours, la sortie à la fin, ou
// pour une séquence invalide les formes isolées des touches déjà tapées puis la touche.
void opt_feed(uint8_t keysym, opt_emit_fn emit);
bool opt_compose_active(void);
// Abandonne la séquence en cours sans rien émettre.
void opt_compose_cancel(void);
