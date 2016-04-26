#ifndef MY_LV2_AUDIO_H
#define MY_LV2_AUDIO_H

#include<lv2.h>
#include "midi.h"
#include "sid_chips.h" //this must appear out of order due to a circular dependency around the above typedef

//I'd really rather not put types in headers but it really simplifies the plugin version
struct super
{
    struct CHIPS* sid_bank;
    struct midi_arrays* midi;
    sid_instrument_t **sid_instr;

    float* outl; //lv2 ports
	float* outr;
};

void* init_lv2_audio(uint32_t fs, const LV2_Feature * const* host_features);
int process(uint32_t nsamples, void* arg);
void cleanup_audio(void* arg);
void set_lout(void* arg, float* lout);
void set_rout(void* arg, float* rout);
void set_ain(void* arg, void* ain);
void set_aout(void* arg, void* aout);

#endif
