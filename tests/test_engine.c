// Tests du moteur Optimot sur l'hôte : niveaux, Verr. Maj, touches mortes.
#include <stdio.h>
#include <string.h>

#include "host_keycodes.h"
#include "optimot.h"
#include "vectors.h"

static uint32_t out[64];
static int      out_len;
static int      failures;

static void collect(uint32_t cp) {
    if (out_len < 64) out[out_len++] = cp;
}

static void reset(void) {
    out_len = 0;
    opt_compose_cancel();
}

static void type(uint8_t pos, uint8_t level) {
    opt_feed(opt_keysym(pos, level), collect);
}

#define CHECK(cond, ...)                \
    do {                                \
        if (!(cond)) {                  \
            failures++;                 \
            if (failures < 30) {        \
                printf("ÉCHEC : ");     \
                printf(__VA_ARGS__);    \
                printf("\n");           \
            }                           \
        }                               \
    } while (0)

static void expect(const char *what, const uint32_t *exp, int n) {
    bool ok = out_len == n && !memcmp(out, exp, n * sizeof(uint32_t));
    CHECK(ok, "%s : %d point(s) de code obtenus, %d attendus (premier 0x%X / 0x%X)", what, out_len, n,
          out_len ? out[0] : 0, n ? exp[0] : 0);
}

static uint8_t pos_of(uint16_t kc) {
    uint8_t p = opt_pos_from_keycode(kc);
    CHECK(p != OPT_NO_POS, "keycode 0x%X sans position", kc);
    return p;
}

int main(void) {
    // 1. Chaque position et niveau donne le keysym du xkb.
    int n_levels = sizeof(level_cases) / sizeof(level_cases[0]);
    for (int i = 0; i < n_levels; i++) {
        const level_case_t *c  = &level_cases[i];
        uint8_t             ks = opt_keysym(c->pos, c->level);
        if (c->cp) CHECK(opt_keysym_unicode(ks) == c->cp, "pos %d niv %d : U+%04X au lieu de U+%04X", c->pos, c->level, opt_keysym_unicode(ks), c->cp);
    }

    // 2. Toutes les séquences du XCompose.
    int n_seq = sizeof(seq_cases) / sizeof(seq_cases[0]);
    for (int i = 0; i < n_seq; i++) {
        const seq_case_t *c = &seq_cases[i];
        reset();
        for (int k = 0; k < c->n; k++) {
            CHECK(k == c->n - 1 || out_len == 0, "séquence %d : sortie prématurée", i);
            type(c->pos[k], c->level[k]);
        }
        int n = 0;
        while (c->out[n]) n++;
        char what[32];
        snprintf(what, sizeof what, "séquence %d", i);
        expect(what, c->out, n);
        CHECK(!opt_compose_active(), "séquence %d : composition encore active", i);
    }

    // 3. Cas particuliers.
    uint8_t q = pos_of(KC_Q), one = pos_of(KC_1), s = pos_of(KC_S), r = pos_of(KC_R);
    uint8_t quot = pos_of(KC_QUOT), d = pos_of(KC_D), m = pos_of(KC_M), g = pos_of(KC_G);
    uint8_t e = pos_of(KC_E);

    // Verr. Maj : à -> À, « -> 1 (semi-alphabétique), œ -> Œ (alphabétique avec AltGr),
    // < reste < (semi-alphabétique avec AltGr), ^ ne change pas (contrôle).
    CHECK(opt_keysym_unicode(opt_keysym(q, opt_level(q, false, false, true))) == 0xC0, "Verr. Maj à");
    CHECK(opt_keysym_unicode(opt_keysym(one, opt_level(one, false, false, true))) == '1', "Verr. Maj «");
    CHECK(opt_keysym_unicode(opt_keysym(e, opt_level(e, false, true, true))) == 0x152, "Verr. Maj œ");
    CHECK(opt_keysym_unicode(opt_keysym(q, opt_level(q, false, true, true))) == '<', "Verr. Maj <");
    CHECK(opt_level(quot, false, false, true) == 0, "Verr. Maj ^");
    CHECK(opt_level(q, true, false, true) == 0, "Verr. Maj + Maj");

    // Raccourcis mixtes : Ctrl+è = Ctrl+C, Ctrl+. = Ctrl+V, Ctrl+é = Ctrl+X.
    CHECK(opt_ctrl_keycode(pos_of(KC_C)) == KC_C, "Ctrl+è");
    CHECK(opt_ctrl_keycode(pos_of(KC_V)) == KC_V, "Ctrl+.");
    CHECK(opt_ctrl_keycode(r) == KC_X, "Ctrl+é");
    CHECK(opt_ctrl_keycode(one) == KC_1, "Ctrl+«");
    // Ctrl+a (AC01) sur AZERTY : la touche A est en KC_Q.
    CHECK(opt_ctrl_keycode(pos_of(KC_A)) == KC_Q, "Ctrl+a");

    CHECK(opt_is_letter(r) && opt_is_letter(q) && !opt_is_letter(one), "lettres");

    // Touche morte ^ puis e -> ê.
    reset();
    type(quot, 0);
    CHECK(out_len == 0 && opt_compose_active(), "^ en attente");
    type(d, 0);
    expect("^e", (uint32_t[]){0xEA}, 1);

    // ^ puis Espace -> ^ seul.
    reset();
    type(quot, 0);
    type(pos_of(KC_SPC), 0);
    expect("^ espace", (uint32_t[]){'^'}, 1);

    // Séquence invalide : ^ puis g (si non défini) -> ^ puis g.
    reset();
    type(quot, 0);
    type(g, 0);
    if (out_len == 2) expect("^g invalide", (uint32_t[]){'^', 'g'}, 2);

    // Touche morte ∞ (AltGr+s) suivie d'une invalide garde la touche suivante.
    reset();
    CHECK(opt_is_dead(opt_keysym(s, 2)), "∞ morte");
    type(s, 2);
    opt_compose_cancel();
    type(m, 0);
    expect("annulation", (uint32_t[]){'c'}, 1);

    // Lettre simple.
    reset();
    type(r, 0);
    expect("é", (uint32_t[]){0xE9}, 1);

    printf("%d niveaux, %d séquences testés : %s (%d échec(s))\n", n_levels, n_seq, failures ? "ÉCHEC" : "OK", failures);
    return failures ? 1 : 0;
}
