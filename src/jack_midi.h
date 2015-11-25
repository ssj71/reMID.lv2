#ifndef JACK_MIDI_H
#define JACK_MIDI_H

#include <jack/jack.h>
#include "midi.h"

void* jack_init_seq(jack_client_t* client);
void jack_midi_connect(jack_client_t* client, char *port);
void jack_read_midi(void* mseq, jack_nframes_t nframes, struct midi_arrays *midi);

#endif
