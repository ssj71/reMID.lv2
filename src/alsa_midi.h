#ifndef ALSA_MIDI_H
#define ALSA_MIDI_H

#include <midi.h>

int alsa_init_seq(void);
void alsa_midi_connect(int client, int port);
void alsa_read_midi(void);

#endif
