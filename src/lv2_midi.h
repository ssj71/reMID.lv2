#ifndef MYLV2_MIDI_H
#define MYLV2_MIDI_H

#include "midi.h"

void* lv2_init_seq(const LV2_Feature * const* host_features)
void lv2_read_midi(void* mseq, jack_nframes_t nframes, struct midi_arrays *midi);
LV2_Atom_Sequence** lv2_get_atom_port(void* mseq);

#endif
