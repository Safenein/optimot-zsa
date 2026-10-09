SRC += optimot.c host_azerty.c

OS_DETECTION_ENABLE = yes
CAPS_WORD_ENABLE = yes
MOUSEKEY_ENABLE = yes
EXTRAKEY_ENABLE = yes

# Place pour les tables Optimot (~46 Ko) dans les 128 Ko de flash.
LTO_ENABLE = yes
AUDIO_ENABLE = no
CONSOLE_ENABLE = no
COMMAND_ENABLE = no
SWAP_HANDS_ENABLE = no
