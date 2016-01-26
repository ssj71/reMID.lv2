#ifndef PREFS_H
#define PREFS_H

#include <pthread.h>
#include "sid_instr.h"
#include "midi.h"

//extern pthread_mutex_t prefs_mutex;

//extern char *instr_file;

//extern int polyphony;
//extern int new_polyphony;

//extern int pal;

//extern int use_gui;
//extern int pt_debug;


// if 1 use the SID volume register
// if 0 set SID volume to 0xf and scale the output later
// SID volume is more authentic
// non-SID volume avoids the SID volume click bug
//  and allows all 128 velocity levels
//extern int use_sid_volume;

//extern int num_instrs;

void prefs_set_polyphony(int value);


sid_instrument_t** read_instruments(char *path, midi_arrays_t *midi);
void close_instruments(sid_instrument_t ** sid_instr);

#endif
