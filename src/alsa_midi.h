#ifndef ALSA_MIDI_H
#define ALSA_MIDI_H

#include <midi.h>

void* alsa_init_seq(void);
void alsa_midi_connect(void* mseq, int client, int port);
void alsa_read_midi(void* mseq);
void close_alsa(void* mseq);

#endif
