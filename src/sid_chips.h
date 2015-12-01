#ifndef SID_CHIPS_H
#define SID_CHIPS_H
#include "midi.h"
#include "sid_instr.h"

struct SID;

struct CHIPS
{
    struct SID** sid_chips;

    int polyphony;
    int use_sid_volume;
    int pt_debug;

    short *buf;
    int buf_length;

    double clock_freq;
    double freq_mult;
    double sample_freq;
    double clocks_per_sample;

    sid_table_state_t **table_states;
};


#ifdef  __cplusplus
extern "C" {
#endif

struct CHIPS *sid_init(int polyphony, int use_sid_volume, int debug);
void sid_close(struct CHIPS *chips);
void sid_set_srate(struct CHIPS *chips, int pal, double sample_freq);
short *sid_process(struct CHIPS *chips, struct midi_arrays* midi, sid_instrument_t **sid_instr, int num_samples);

#ifdef  __cplusplus
}
#endif

#endif
