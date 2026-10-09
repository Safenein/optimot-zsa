// Moteur Optimot (voir optimot.h).
#ifdef OPT_HOST_TEST
#    define PROGMEM
#    include "host_keycodes.h"
#else
#    include "quantum.h"
#endif

#define OPT_TABLES_IMPLEMENTATION
#include "optimot.h"

#ifndef pgm_read_byte
#    define pgm_read_byte(p) (*(const uint8_t *)(p))
#    define pgm_read_word(p) (*(const uint16_t *)(p))
#    define pgm_read_dword(p) (*(const uint32_t *)(p))
#endif

uint8_t opt_pos_from_keycode(uint16_t keycode) {
    // <BKSL> correspond aux deux codes HID (touche ANSI \ et touche ISO #).
    if (keycode == KC_NUHS) keycode = KC_BSLS;
    for (uint8_t i = 0; i < OPT_NPOS; i++) {
        if (opt_pos_keycode[i] == keycode) return i;
    }
    return OPT_NO_POS;
}

uint8_t opt_level(uint8_t pos, bool shift, bool altgr, bool caps) {
    switch (opt_pos_type[pos]) {
        case OPT_T_ALPHA:
            shift ^= caps;
            break;
        case OPT_T_SEMIALPHA:
            if (!altgr) shift ^= caps;
            break;
        default:
            break;
    }
    return (altgr ? 2 : 0) + (shift ? 1 : 0);
}

uint8_t opt_keysym(uint8_t pos, uint8_t level) {
    return pgm_read_byte(&opt_pos_levels[pos][level]);
}

uint32_t opt_keysym_unicode(uint8_t keysym) {
    return pgm_read_dword(&opt_ks_unicode[keysym]);
}

uint8_t opt_ctrl_keycode(uint8_t pos) {
    return pgm_read_byte(&opt_pos_ctrl[pos]);
}

bool opt_is_letter(uint8_t pos) {
    return pgm_read_byte(&opt_pos_letter[pos]);
}

// --- Trie ------------------------------------------------------------------------

static uint16_t trie_child(uint16_t node, uint8_t keysym) {
    uint8_t n = pgm_read_byte(&opt_trie_nchild[node]);
    if (!n) return 0;
    uint16_t lo = pgm_read_word(&opt_trie_val[node]);
    uint16_t hi = lo + n;
    while (lo < hi) {
        uint16_t mid = (lo + hi) / 2;
        uint8_t  k   = pgm_read_byte(&opt_trie_ks[mid]);
        if (k == keysym) return mid;
        if (k < keysym)
            lo = mid + 1;
        else
            hi = mid;
    }
    return 0;
}

static bool trie_is_leaf(uint16_t node) {
    return pgm_read_byte(&opt_trie_nchild[node]) == 0;
}

static void emit_output(uint16_t leaf, opt_emit_fn emit) {
    uint16_t off = pgm_read_word(&opt_trie_val[leaf]);
    uint8_t  len = pgm_read_word(&opt_out_pool[off]);
    for (uint8_t i = 1; i <= len; i++) {
        uint32_t u = pgm_read_word(&opt_out_pool[off + i]);
        if (u >= 0xD800 && u < 0xDC00 && i < len) {
            uint32_t lo = pgm_read_word(&opt_out_pool[off + ++i]);
            u           = 0x10000 + ((u - 0xD800) << 10) + (lo - 0xDC00);
        }
        emit(u);
    }
}

bool opt_is_dead(uint8_t keysym) {
    uint16_t n = trie_child(0, keysym);
    return n && !trie_is_leaf(n);
}

uint32_t opt_standalone(uint8_t keysym) {
    uint16_t n = trie_child(0, keysym);
    if (n) {
        uint16_t sp = trie_child(n, OPT_KS_SPACE);
        if (sp && trie_is_leaf(sp)) {
            uint16_t off = pgm_read_word(&opt_trie_val[sp]);
            if (pgm_read_word(&opt_out_pool[off]) == 1) return pgm_read_word(&opt_out_pool[off + 1]);
        }
    }
    return opt_keysym_unicode(keysym);
}

// --- Composition -----------------------------------------------------------------

static uint16_t cur_node;
static uint8_t  hist[OPT_MAX_SEQ];
static uint8_t  hist_len;

bool opt_compose_active(void) {
    return cur_node != 0;
}

void opt_compose_cancel(void) {
    cur_node = 0;
    hist_len = 0;
}

static void emit_cp(uint32_t cp, opt_emit_fn emit) {
    if (cp) emit(cp);
}

void opt_feed(uint8_t keysym, opt_emit_fn emit) {
    uint16_t next = trie_child(cur_node, keysym);

    if (!next) {
        if (cur_node) {
            // Séquence invalide : formes isolées de ce qui a été tapé, puis la touche
            // elle-même (qui peut ouvrir une nouvelle séquence).
            for (uint8_t i = 0; i < hist_len; i++) emit_cp(opt_standalone(hist[i]), emit);
            opt_compose_cancel();
            opt_feed(keysym, emit);
            return;
        }
        emit_cp(opt_keysym_unicode(keysym), emit);
        return;
    }

    if (trie_is_leaf(next)) {
        emit_output(next, emit);
        opt_compose_cancel();
        return;
    }

    cur_node = next;
    if (hist_len < OPT_MAX_SEQ) hist[hist_len++] = keysym;
}
