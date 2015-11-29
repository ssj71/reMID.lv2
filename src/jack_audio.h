#ifndef MY_JACK_AUDIO_H
#define MY_JACK_AUDIO_H

#include <jack/jack.h>
#include "midi.h"
#include "sid_chips.h" //this must appear out of order due to a circular dependency around the above typedef

#define CLIENT_NAME "reMID"
#define AUDIO_PORTNAME_L "audio_out_left"
#define AUDIO_PORTNAME_R "audio_out_right"

int init_jack_audio(int use_sid_volume, int max_polyphony, char** jack_connect_args, char** midi_connect_args, char* instr_file);

#endif
