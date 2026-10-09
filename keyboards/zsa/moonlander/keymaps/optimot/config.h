#pragma once

// Les séquences Unicode / codes Alt enchaînent beaucoup de frappes : un léger délai
// évite que l'hôte en perde.
#define TAP_CODE_DELAY 2

// Caps Word : double Maj l'active aussi.
#define DOUBLE_TAP_SHIFT_TURNS_ON_CAPS_WORD
