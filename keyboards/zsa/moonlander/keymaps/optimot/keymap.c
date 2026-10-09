// Moonlander — disposition Optimot Ergo native, pour un hôte en AZERTY standard.
//
// Les couches reprennent telles quelles la configuration Oryx « Optimot VIM Emacs »
// (7Ozzp, révision Mad3Ab, vendor/oryx/oryx.json), qui envoyait du QWERTY à un pilote
// Optimot installé sur l'OS. Ici, chaque keycode de position (KC_Q, KC_SCLN...) est
// intercepté et traduit comme l'aurait fait ce pilote : niveaux, AltGr, touches mortes,
// raccourcis Ctrl « mixtes ». La couche Gaming n'est pas traduite.
//
// Optimot (c) Patrick Jamet, https://optimot.fr/ — CC BY-NC-SA 4.0. Adapté en firmware QMK.
#include QMK_KEYBOARD_H
#include "optimot.h"
#include "host_azerty.h"

enum layers { MAIN, SPECIAL, MOUSE, GAMING };

enum custom_keycodes {
    OS_AUTO = SAFE_RANGE, // OS hôte : détection automatique
    OS_LNX,               // OS hôte forcé : Linux (saisie Unicode Ctrl+Maj+U)
    OS_WIN,               // OS hôte forcé : Windows (codes Alt)
    ORX_SLD,              // Oryx : couleur unie
    ORX_RED,              // Oryx : tout en #f50909
    ORX_BLUE,             // Oryx : tout en #0075FF
};

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [MAIN] = LAYOUT(
        KC_EQL  , KC_1    , KC_2    , KC_3    , KC_4    , KC_5    , KC_GRV  , AU_TOGG , KC_6    , KC_7    , KC_8    , KC_9    , KC_0    , KC_MINS,
        KC_TAB  , KC_Q    , KC_W    , KC_E    , KC_R    , KC_T    , KC_CAPS , MU_TOGG , KC_Y    , KC_U    , KC_I    , KC_O    , KC_P    , KC_BSLS,
        KC_CAPS , KC_A    , KC_S    , KC_D    , KC_F    , KC_G    , KC_DEL  , _______ , KC_H    , KC_J    , KC_K    , KC_L    , KC_SCLN , KC_QUOT,
        KC_LSFT , KC_Z    , KC_X    , KC_C    , KC_V    , KC_B    ,                     KC_N    , KC_M    , KC_COMM , KC_DOT  , KC_SLSH , KC_RSFT,
        KC_LCTL , KC_LALT , CW_TOGG , MO(SPECIAL), OSL(MOUSE), TG(GAMING),        _______ , KC_RALT , _______ , KC_LBRC , KC_RBRC , MO(SPECIAL),
                                      KC_SPC  , KC_BSPC , KC_LGUI ,                     KC_LALT , KC_TAB  , KC_ENT
    ),
    [SPECIAL] = LAYOUT(
        KC_ESC  , KC_F1   , KC_F2   , KC_F3   , KC_F4   , KC_F5   , _______ , _______ , KC_F6   , KC_F7   , KC_F8   , KC_F9   , KC_F10  , KC_F11,
        _______ , KC_EXLM , KC_AT   , KC_LCBR , KC_RCBR , KC_PIPE , _______ , _______ , KC_UP   , KC_P7   , KC_P8   , KC_P9   , KC_PAST , KC_F12,
        _______ , KC_HASH , KC_DLR  , KC_LPRN , KC_RPRN , KC_GRV  , _______ , _______ , KC_DOWN , KC_P4   , KC_P5   , KC_P6   , KC_PPLS , KC_NUM,
        _______ , KC_PERC , KC_CIRC , KC_LBRC , KC_RBRC , KC_TILD ,                     KC_AMPR , KC_P1   , KC_P2   , KC_P3   , KC_BSLS , _______,
        _______ , KC_COMM , ORX_RED , _______ , ORX_BLUE, _______ ,                     RM_TOGG , KC_P0   , KC_P0   , KC_PDOT , KC_PEQL , _______,
                                      RM_VALD , RM_VALU , TOGGLE_LAYER_COLOR,           ORX_SLD , RM_HUED , RM_HUEU
    ),
    [MOUSE] = LAYOUT(
        OS_AUTO , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , QK_BOOT,
        OS_LNX  , _______ , _______ , KC_UP   , _______ , _______ , _______ , _______ , KC_MUTE , _______ , MS_UP   , _______ , _______ , _______,
        OS_WIN  , _______ , KC_LEFT , KC_DOWN , KC_RGHT , _______ , _______ , _______ , KC_VOLU , MS_LEFT , MS_DOWN , MS_RGHT , _______ , KC_MPLY,
        _______ , _______ , _______ , _______ , _______ , _______ ,                     KC_VOLD , _______ , KC_MPRV , KC_MNXT , _______ , _______,
        _______ , _______ , _______ , _______ , _______ , _______ ,                     _______ , MS_BTN1 , MS_BTN2 , _______ , _______ , _______,
                                      _______ , _______ , _______ ,                     _______ , _______ , _______
    ),
    [GAMING] = LAYOUT(
        KC_ESC  , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______,
        KC_TAB  , KC_TAB  , KC_Q    , KC_W    , KC_E    , KC_R    , KC_T    , _______ , _______ , _______ , _______ , _______ , _______ , _______,
        KC_CAPS , KC_CAPS , KC_A    , KC_S    , KC_D    , KC_F    , KC_G    , _______ , _______ , _______ , _______ , _______ , _______ , _______,
        KC_LSFT , KC_LSFT , KC_Z    , KC_X    , KC_C    , KC_V    ,                     _______ , _______ , _______ , _______ , _______ , KC_RSFT,
        KC_LCTL , KC_LCTL , _______ , LGUI_T(KC_HOME), KC_B, _______,                   _______ , _______ , _______ , _______ , _______ , KC_RCTL,
                                      KC_SPC  , KC_SPC  , KC_LALT ,                     _______ , _______ , _______
    ),
};
// clang-format on

// --- Choix de l'OS hôte -----------------------------------------------------------

typedef enum { OS_MODE_AUTO, OS_MODE_LINUX, OS_MODE_WINDOWS } os_mode_t;

static os_mode_t    os_mode;
static os_variant_t detected_os = OS_UNSURE;

static void apply_os_mode(void) {
    switch (os_mode) {
        case OS_MODE_LINUX:
            host_os_set(HOST_LINUX);
            break;
        case OS_MODE_WINDOWS:
            host_os_set(HOST_WINDOWS);
            break;
        default:
            host_os_set(detected_os == OS_WINDOWS ? HOST_WINDOWS : HOST_LINUX);
            break;
    }
}

void keyboard_post_init_user(void) {
    os_mode = eeconfig_read_user() & 0x3;
    if (os_mode > OS_MODE_WINDOWS) os_mode = OS_MODE_AUTO;
    apply_os_mode();
}

bool process_detected_host_os_user(os_variant_t os) {
    detected_os = os;
    apply_os_mode();
    return true;
}

static void set_os_mode(os_mode_t mode) {
    os_mode = mode;
    eeconfig_update_user(mode);
    apply_os_mode();
}

// --- Optimot -----------------------------------------------------------------------

static bool altgr; // AltGr Optimot (KC_RALT), jamais transmis tel quel à l'hôte
static bool caps;  // Verr. Maj Optimot (Maj+KC_CAPS), suit les types xkb

// Ce qui est maintenu par position pour garder la répétition automatique de l'hôte.
static uint8_t held_kc[OPT_NPOS];
static uint8_t held_mods[OPT_NPOS];

static void hold(uint8_t pos, uint8_t keycode, uint8_t mods) {
    if (mods) register_mods(mods);
    register_code(keycode);
    held_kc[pos]   = keycode;
    held_mods[pos] = mods;
}

static void release(uint8_t pos) {
    unregister_code(held_kc[pos]);
    if (held_mods[pos]) unregister_mods(held_mods[pos]);
    held_kc[pos]   = 0;
    held_mods[pos] = 0;
}

// Touches qui ne doivent pas interrompre une séquence de touche morte.
static bool is_passive(uint16_t keycode) {
    return IS_MODIFIER_KEYCODE(keycode) || IS_QK_MOMENTARY(keycode) || IS_QK_ONE_SHOT_LAYER(keycode) || IS_QK_TOGGLE_LAYER(keycode) || keycode == CW_TOGG;
}

// Keycode basique et Maj intégrée (KC_EXLM = S(KC_1)...). Faux si autre modificateur.
static bool basic_keycode(uint16_t keycode, uint16_t *basic, bool *shift) {
    *shift = false;
    if (IS_QK_MODS(keycode)) {
        uint8_t mods = QK_MODS_GET_MODS(keycode);
        if ((mods & 0x0F) != MOD_LSFT) return false;
        *shift  = true;
        keycode = QK_MODS_GET_BASIC_KEYCODE(keycode);
    }
    *basic = keycode;
    return keycode <= 0xFF;
}

static bool process_optimot(uint8_t pos, bool key_shift) {
    uint8_t real_mods = get_mods() | get_oneshot_mods();
    uint8_t host_mods = real_mods | get_weak_mods();
    bool    shift     = key_shift || (host_mods & MOD_MASK_SHIFT);
    bool    host_shift = host_mods & MOD_MASK_SHIFT;
    if (get_oneshot_mods()) clear_oneshot_mods();

    // Ctrl : raccourci « mixte » du niveau 5/6, modificateurs conservés.
    if (real_mods & MOD_MASK_CTRL) {
        opt_compose_cancel();
        hold(pos, opt_ctrl_keycode(pos), 0);
        return false;
    }

    uint8_t  ks = opt_keysym(pos, opt_level(pos, shift, altgr, caps));
    uint32_t cp = opt_keysym_unicode(ks);
    uint8_t  kc, mods;

    // Alt ou GUI : le caractère s'il tient en une frappe, sinon la lettre du raccourci.
    if (real_mods & (MOD_MASK_ALT | MOD_MASK_GUI)) {
        opt_compose_cancel();
        if (az_single_key(cp, &kc, &mods) && !(mods & MOD_BIT(KC_RALT)) && !!(mods & MOD_MASK_SHIFT) == host_shift) {
            hold(pos, kc, 0);
        } else {
            hold(pos, opt_ctrl_keycode(pos), 0);
        }
        return false;
    }

    if (opt_compose_active() || opt_is_dead(ks)) {
        opt_feed(ks, az_send);
        return false;
    }

    // Frappe simple maintenue (répétition native) quand l'état Maj de l'hôte s'y prête.
    if (az_single_key(cp, &kc, &mods)) {
        bool need_shift = mods & MOD_MASK_SHIFT;
        if (need_shift == host_shift) {
            hold(pos, kc, mods & MOD_BIT(KC_RALT));
            return false;
        }
        if (need_shift && !(real_mods & MOD_MASK_SHIFT)) {
            hold(pos, kc, mods);
            return false;
        }
    }
    az_send(cp);
    return false;
}

static void set_solid_color(uint8_t h, uint8_t s) {
    rgb_matrix_mode(RGB_MATRIX_SOLID_COLOR);
    rgb_matrix_sethsv(h, s, rgb_matrix_get_val());
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    bool pressed = record->event.pressed;

    switch (keycode) {
        case OS_AUTO:
            if (pressed) set_os_mode(OS_MODE_AUTO);
            return false;
        case OS_LNX:
            if (pressed) set_os_mode(OS_MODE_LINUX);
            return false;
        case OS_WIN:
            if (pressed) set_os_mode(OS_MODE_WINDOWS);
            return false;
        case ORX_SLD:
            if (pressed) rgb_matrix_mode(RGB_MATRIX_SOLID_COLOR);
            return false;
        case ORX_RED:
            if (pressed) set_solid_color(0, 246);
            return false;
        case ORX_BLUE:
            if (pressed) set_solid_color(150, 255);
            return false;
    }

    uint16_t basic;
    bool     key_shift;
    uint8_t  pos = OPT_NO_POS;
    if (basic_keycode(keycode, &basic, &key_shift)) pos = opt_pos_from_keycode(basic);

    // Relâchement d'une touche Optimot traitée à l'appui (quelle que soit la couche).
    if (!pressed && pos != OPT_NO_POS && held_kc[pos]) {
        release(pos);
        return false;
    }

    // Équivalent de l'option xkb caps:escape_shifted_capslock, sur toutes les couches :
    // Verr. Maj seule = Échap, Maj+Verr. Maj = verrouillage (celui de l'hôte en Gaming).
    if (keycode == KC_CAPS) {
        static bool esc_held;
        if (pressed) {
            if ((get_mods() | get_oneshot_mods()) & MOD_MASK_SHIFT) {
                if (IS_LAYER_ON(GAMING))
                    tap_code(KC_CAPS);
                else
                    caps = !caps;
            } else {
                opt_compose_cancel();
                register_code(KC_ESC);
                esc_held = true;
            }
        } else if (esc_held) {
            unregister_code(KC_ESC);
            esc_held = false;
        }
        return false;
    }

    if (IS_LAYER_ON(GAMING)) return true;

    if (keycode == KC_RALT) {
        altgr = pressed;
        return false;
    }

    if (pos == OPT_NO_POS) {
        if (pressed && opt_compose_active() && !is_passive(keycode)) opt_compose_cancel();
        return true;
    }
    if (!pressed) return false;
    return process_optimot(pos, key_shift);
}

// Caps Word : majuscule sur les lettres Optimot (é, à, ç... compris), continue sur
// les touches mortes, les chiffres, - et _.
bool caps_word_press_user(uint16_t keycode) {
    uint16_t basic;
    bool     key_shift;
    uint8_t  pos = OPT_NO_POS;
    if (basic_keycode(keycode, &basic, &key_shift)) pos = opt_pos_from_keycode(basic);

    if (pos != OPT_NO_POS && !IS_LAYER_ON(GAMING)) {
        if (opt_is_letter(pos)) {
            add_weak_mods(MOD_BIT(KC_LSFT));
            return true;
        }
        uint32_t base = opt_keysym_unicode(opt_keysym(pos, 0));
        uint32_t up   = opt_keysym_unicode(opt_keysym(pos, 1));
        return opt_is_dead(opt_keysym(pos, 0)) || base == '-' || up == '_' || (up >= '0' && up <= '9');
    }
    switch (keycode) {
        case KC_BSPC:
        case KC_DEL:
            return true;
        default:
            return false;
    }
}

// --- Témoins lumineux ---------------------------------------------------------------

static void light_keycode(uint8_t layer, uint16_t keycode, uint8_t r, uint8_t g, uint8_t b) {
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            if (keymap_key_to_keycode(layer, (keypos_t){.row = row, .col = col}) != keycode) continue;
            uint8_t led = g_led_config.matrix_co[row][col];
            if (led != NO_LED) rgb_matrix_set_color(led, r, g, b);
        }
    }
}

bool rgb_matrix_indicators_user(void) {
    if (caps) light_keycode(MAIN, KC_CAPS, RGB_WHITE);
    if (IS_LAYER_ON(GAMING)) light_keycode(MAIN, TG(GAMING), 0xF5, 0x09, 0x09);
    if (IS_LAYER_ON(MOUSE)) light_keycode(MOUSE, os_mode == OS_MODE_LINUX ? OS_LNX : os_mode == OS_MODE_WINDOWS ? OS_WIN : OS_AUTO, RGB_GREEN);
    return true;
}
