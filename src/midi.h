#ifndef MIDI_H
#define MIDI_H

#include <jack/jack.h>

#define MIDI_PORTNAME "MIDI_In"

struct midi_key_state {
	int last_used;
	int needs_clearing;

	int channel;
	int note_state_changed;
	int note_on;
	int note;
	int velocity;

	//int aftertouch_state_changed;
	//int aftertouch;
};
//extern struct midi_key_state **midi_keys;

struct midi_channel_state {
	int in_use;
	int program;
	int sustain;
	int pitchbend;
	int vibrato;
	int vibrato_changed;
	int chanpress;
	int chanpress_changed;
	int last_velocity;
};

struct midi_arrays {
	struct midi_key_state **midi_keys;
	struct midi_channel_state *midi_channels;
	int *midi_programs;
	double *note_frqs;
	int *free_voices;
	int next_voice;
	int voice_use_index;
	void* seq;
};
/*
extern struct midi_channel_state midi_channels[];

extern int midi_programs[];

extern double note_frqs[];
*/

int init_midi(struct midi_arrays* midi, jack_client_t* client, int polyphony);
void read_midi(void* mseq, jack_nframes_t nframes, struct midi_channel_state* midi_channels);
void note_on(struct midi_arrays* midi, int channel, int note, int velocity);
void note_off(struct midi_arrays* midi, int channel, int note);
void silence_all(struct midi_key_state **midi_keys);
void midi_close(struct midi_arrays* midi, int polyphony);

#endif
