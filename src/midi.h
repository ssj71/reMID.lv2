#ifndef MIDI_H
#define MIDI_H

#include<stdint.h>

#define MIDI_PORTNAME "MIDI_In"

typedef struct midi_key_state
{
    int last_used;
    int needs_clearing;

    int channel;
    int note_state_changed;
    int note_on;
    int note;
    int velocity;

    //int aftertouch_state_changed;
    //int aftertouch;
} midi_key_state_t;
//extern struct midi_key_state **midi_keys;

typedef struct midi_channel_state
{
    int in_use;
    int program;
    int sustain;
    int pitchbend;
    int vibrato;
    int vibrato_changed;
    int chanpress;
    int chanpress_changed;
    int last_velocity;
} midi_channel_state_t;

typedef struct midi_arrays
{
    struct midi_key_state **midi_keys;
    struct midi_channel_state midi_channels[16];
    int midi_programs[128];
    double note_frqs[128];
    int *free_voices;
    int next_voice;
    int voice_use_index;
    void* seq;
} midi_arrays_t;
/*
extern struct midi_channel_state midi_channels[];

extern int midi_programs[];

extern double note_frqs[];
*/

midi_arrays_t* init_midi(void* o, int polyphony, char** midi_connect_args);
void read_midi(void* mseq, uint32_t nframes, midi_arrays_t* midi);
void note_on(midi_arrays_t* midi, int channel, int note, int velocity);
void note_off(midi_arrays_t* midi, int channel, int note);
void silence_all(midi_key_state_t **midi_keys);
void midi_close(midi_arrays_t* midi, int polyphony);

#endif
