#ifndef JACK_MIDI_H
#define JACK_MIDI_H

#include "midi.h"

void* lv2_init_seq(const LV2_Feature * const* host_features)
void lv2_read_midi(void* mseq, jack_nframes_t nframes, struct midi_arrays *midi);

#endif
