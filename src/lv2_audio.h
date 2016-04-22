#ifndef MY_LV2_AUDIO_H
#define MY_LV2_AUDIO_H

#include "midi.h"
#include "sid_chips.h" //this must appear out of order due to a circular dependency around the above typedef

void* init_lv2_audio(uint32_t fs, const LV2_Feature * const* host_features);
int process(uint32_t nsamples, void* arg);
void set_lout(void* arg, float* lout);
void set_rout(void* arg, float* rout);

#endif
