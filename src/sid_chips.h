#include<sid.h>
#include<midi.h>

struct SID;

struct CHIPS
{
    struct SID** sid_chips;

    int polyphony;
    int use_sid_volume

    short *buf;
    int buf_length;

    double clock_freq;
    double freq_mult;
    double sample_freq;
    double clocks_per_sample;

    sid_table_state_t **table_states;
};

struct CHIPS *sid_init(int polyphony, int use_sid_volume);

void sid_close(struct CHIPS *chips);

void sid_set_srate(struct SID **chips, int pal, double sample_freq);

short *sid_process(struct SID **chips, struct midi_arrays* midi, int num_samples);

