// Émission vers un hôte AZERTY (voir host_azerty.h).
#include "quantum.h"
#define OPT_AZ_IMPLEMENTATION
#include "optimot.h"
#include "host_azerty.h"

#define AZ_SHIFT 0x01
#define AZ_ALTGR 0x02
#define AZ_WIN_DEAD 0x04
#define AZ_DEAD_SHIFT 3

static host_os_t host_os = HOST_LINUX;

void host_os_set(host_os_t os) {
    host_os = os;
}

host_os_t host_os_get(void) {
    return host_os;
}

static const opt_az_t *az_find(uint32_t cp) {
    if (cp > 0xFFFF) return NULL;
    uint16_t lo = 0, hi = OPT_AZ_COUNT;
    while (lo < hi) {
        uint16_t mid = (lo + hi) / 2;
        uint16_t c   = pgm_read_word(&opt_az[mid].cp);
        if (c == cp) return &opt_az[mid];
        if (c < cp)
            lo = mid + 1;
        else
            hi = mid;
    }
    return NULL;
}

bool az_single_key(uint32_t cp, uint8_t *keycode, uint8_t *mods) {
    const opt_az_t *e = az_find(cp);
    if (!e) return false;
    uint8_t flags = pgm_read_byte(&e->flags);
    if (flags >> AZ_DEAD_SHIFT) return false;
    if ((flags & AZ_WIN_DEAD) && host_os == HOST_WINDOWS) return false;
    *keycode = pgm_read_byte(&e->kc);
    *mods    = ((flags & AZ_SHIFT) ? MOD_BIT(KC_LSFT) : 0) | ((flags & AZ_ALTGR) ? MOD_BIT(KC_RALT) : 0);
    return true;
}

static void tap_with_mods(uint8_t keycode, uint8_t mods) {
    if (mods) register_mods(mods);
    tap_code(keycode);
    if (mods) unregister_mods(mods);
}

// Chiffre hexadécimal tapé sur l'AZERTY : 0-9 avec Maj (ou pavé numérique), a-f en minuscule.
static void tap_hex_digit(uint8_t n, bool numpad) {
    static const uint8_t letters[6] = {KC_Q, KC_B, KC_C, KC_D, KC_E, KC_F}; // a b c d e f
    if (n >= 10) {
        tap_code(letters[n - 10]);
    } else if (numpad) {
        tap_code(n == 0 ? KC_KP_0 : KC_KP_1 + n - 1);
    } else {
        tap_with_mods(n == 0 ? KC_0 : KC_1 + n - 1, MOD_BIT(KC_LSFT));
    }
}

static void tap_hex(uint32_t cp, bool numpad) {
    bool started = false;
    for (int8_t shift = 20; shift >= 0; shift -= 4) {
        uint8_t n = (cp >> shift) & 0xF;
        if (n || started || shift < 16) {
            tap_hex_digit(n, numpad);
            started = true;
        }
    }
}

// Windows-1252 : octets 0x80-0x9F. Le reste de 0xA0-0xFF est identique à Latin-1.
static const uint16_t cp1252_high[32] PROGMEM = {
    0x20AC, 0, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, 0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0, 0x017D, 0,
    0, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0, 0x017E, 0x0178,
};

static uint8_t cp1252_byte(uint32_t cp) {
    if (cp >= 0xA0 && cp <= 0xFF) return cp;
    for (uint8_t i = 0; i < 32; i++) {
        if (pgm_read_word(&cp1252_high[i]) == cp) return 0x80 + i;
    }
    return 0;
}

static void send_unicode_linux(uint32_t cp) {
    // IBus et fcitx5 (mode « Unicode direct ») : Ctrl+Maj+U, hexadécimal, Espace.
    tap_with_mods(KC_U, MOD_BIT(KC_LCTL) | MOD_BIT(KC_LSFT));
    tap_hex(cp, false);
    tap_code(KC_SPC);
}

static void send_unicode_windows(uint32_t cp) {
    bool numlock_off = !host_keyboard_led_state().num_lock;
    if (numlock_off) tap_code(KC_NUM);

    register_code(KC_LALT);
    uint8_t b = cp1252_byte(cp);
    if (b) {
        // Code Alt natif : Alt + 0nnn (page de codes Windows-1252).
        tap_code(KC_KP_0);
        tap_code(KC_KP_1 + (b / 100) - 1);
        uint8_t t = (b / 10) % 10, u = b % 10;
        tap_code(t ? KC_KP_1 + t - 1 : KC_KP_0);
        tap_code(u ? KC_KP_1 + u - 1 : KC_KP_0);
    } else {
        // Alt + « + » du pavé + hexadécimal (clé de registre EnableHexNumpad=1).
        tap_code(KC_KP_PLUS);
        tap_hex(cp, true);
    }
    unregister_code(KC_LALT);

    if (numlock_off) tap_code(KC_NUM);
}

void az_send(uint32_t cp) {
    uint8_t saved = get_mods();
    clear_mods();
    clear_weak_mods();
    send_keyboard_report();

    const opt_az_t *e = az_find(cp);
    if (e) {
        uint8_t flags = pgm_read_byte(&e->flags);
        uint8_t dead  = flags >> AZ_DEAD_SHIFT;
        if (dead) tap_with_mods(KC_LBRC, dead == 2 ? MOD_BIT(KC_LSFT) : 0); // ^ ou ¨ morts
        tap_with_mods(pgm_read_byte(&e->kc), ((flags & AZ_SHIFT) ? MOD_BIT(KC_LSFT) : 0) | ((flags & AZ_ALTGR) ? MOD_BIT(KC_RALT) : 0));
        if ((flags & AZ_WIN_DEAD) && host_os == HOST_WINDOWS) tap_code(KC_SPC);
    } else if (host_os == HOST_WINDOWS) {
        send_unicode_windows(cp);
    } else {
        send_unicode_linux(cp);
    }

    set_mods(saved);
    send_keyboard_report();
}
