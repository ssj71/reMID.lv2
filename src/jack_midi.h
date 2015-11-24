#ifndef JACK_MIDI_H
#define JACK_MIDI_H

#include <jack/jack.h>
#include "midi.h"

int jack_init_seq(void);
void jack_midi_connect(char *port);
void jack_read_midi(jack_nframes_t nframes, struct midi_arrays *midi);

#endif
