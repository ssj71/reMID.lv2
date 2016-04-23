#ifndef MYLV2_MIDI_H
#define MYLV2_MIDI_H

#include "midi.h"

void* lv2_init_seq(const LV2_Feature * const* host_features)
void lv2_read_midi(void* mseq, jack_nframes_t nframes, struct midi_arrays *midi);
void lv2_set_atom_in_port(void* mseq, void* port);
void lv2_set_atom_out_port(void* mseq, void* port);
char** lv2_get_file_string(void* mseq);

#endif
