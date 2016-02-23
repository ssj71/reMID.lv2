#ifndef MY_LV2_AUDIO_H
#define MY_LV2_AUDIO_H

#include "midi.h"
#include "sid_chips.h" //this must appear out of order due to a circular dependency around the above typedef

int init_lv2_audio(int use_sid_volume, int max_polyphony, int debug, char** jack_connect_args, char** midi_connect_args, char* instr_file);
int process(uint32_t nsamples, void* arg);

#endif
