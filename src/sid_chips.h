#include<sid.h>

struct SID;

struct CHIPS
{
    struct SID** sid_chips;

    int polyphony;

    short *buf;
    int buf_length;

    double clock_freq;
    double freq_mult;
    double sample_freq;
    double clocks_per_sample;

    sid_table_state_t **table_states;
};

struct SID **sid_init();

void sid_close(struct SID **chips);

void sid_set_srate(struct SID **chips, int pal, double sample_freq);

short *sid_process(struct SID **chips, int num_samples);

