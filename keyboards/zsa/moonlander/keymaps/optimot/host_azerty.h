// Émission de caractères vers un hôte en AZERTY standard (Linux « fr » ou Windows).
#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum { HOST_LINUX, HOST_WINDOWS } host_os_t;

void      host_os_set(host_os_t os);
host_os_t host_os_get(void);

// Frappe unique (sans touche morte) produisant cp sur l'hôte : keycode et
// modificateurs nécessaires (MOD_BIT). Faux si cp demande une séquence.
bool az_single_key(uint32_t cp, uint8_t *keycode, uint8_t *mods);

// Tape cp sur l'hôte (frappe directe, touche morte AZERTY ou saisie Unicode).
// Les modificateurs en cours sont neutralisés le temps de l'envoi.
void az_send(uint32_t cp);
