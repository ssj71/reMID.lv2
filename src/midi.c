// a large portion of why this file exists is to try to keep all the preprocessor
// stuff for the different midi engines constrained to one file

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "midi.h"
//#define JACK_MIDI
#define LV2_MIDI

#ifdef ALSA_MIDI
#include "alsa_midi.h"
#endif

#ifdef JACK_MIDI
#include "jack_midi.h"
#endif

#ifdef LV2_MIDI
#include "lv2_midi.h"
#endif

#include "prefs.h"


void silence_all(midi_key_state_t **midi_keys)
{
    int i;
    for(i=0; midi_keys[i]; i++)
    {
        midi_keys[i]->note_on = 0;
        midi_keys[i]->needs_clearing = 1;
        midi_keys[i]->note_state_changed = 1;
    }
}

void find_next_voice(midi_key_state_t **midi_keys, int* free_voices, int* next_voice, int* voice_use_index)
{
    int i,j;
    int needs_clearing = 0;

    // find free voices
    j=0;
    for(i=0; midi_keys[i]; i++)
    {
        if(!midi_keys[i]->note_on && !midi_keys[i]->note_state_changed)
        {
            free_voices[j++] = i;
        }
    }
    if(j==0)
    {
        // all in use so all are candidates
        for(j=0; midi_keys[j]; j++) free_voices[j] = j;
        // voice will need switching off before on again
        needs_clearing = 1;
    }
    free_voices[j] = -1;
    // use the least recently used
    j = 0;
    int lru = midi_keys[free_voices[j]]->last_used;
    *next_voice = free_voices[j];
    for(j=1; (i=free_voices[j])!=-1; j++)
    {
        if(midi_keys[i]->last_used<lru)
        {
            lru = midi_keys[i]->last_used;
            *next_voice = i;
        }
    }
    midi_keys[*next_voice]->last_used = *voice_use_index++;
    midi_keys[*next_voice]->needs_clearing = needs_clearing;
}

void note_on(midi_arrays_t* midi, int channel, int note, int velocity)
{
    if(!midi->midi_channels[channel].in_use) return;
    int program = midi->midi_channels[channel].program;
    if(program==-1) return;
    int inst_num = midi->midi_programs[program];
    if(inst_num==-1) return;

    find_next_voice(midi->midi_keys, midi->free_voices, &midi->next_voice, &midi->voice_use_index);
    midi->midi_keys[midi->next_voice]->note_state_changed = 1;
    midi->midi_keys[midi->next_voice]->note_on = 1;
    midi->midi_keys[midi->next_voice]->channel = channel;
    midi->midi_keys[midi->next_voice]->note = note;
    midi->midi_keys[midi->next_voice]->velocity = velocity;

    midi->midi_channels[channel].last_velocity = velocity;
}

void note_off(midi_arrays_t* midi, int channel, int note)
{
    if(!midi->midi_channels[channel].in_use) return;
    int program = midi->midi_channels[channel].program;
    if(program==-1) return;
    int inst_num = midi->midi_programs[program];
    if(inst_num==-1) return;

    int i;
    for(i=0; midi->midi_keys[i]; i++)
    {
        if(midi->midi_keys[i]->channel==channel && midi->midi_keys[i]->note==note)
        {
            midi->midi_keys[i]->note_state_changed = 1;
            midi->midi_keys[i]->note_on = 0;
        }
    }
}

void read_midi(void* seq, uint32_t nframes, midi_arrays_t* midi)
{
#ifdef ALSA_MIDI
    alsa_read_midi(seq, midi);
#endif
#ifdef JACK_MIDI
    jack_read_midi(seq, nframes, midi);
#endif
#ifdef LV2_MIDI
    lv2_read_midi(seq, nframes, midi);
#endif
}

midi_arrays_t* init_midi(void* o, int polyphony, char** midi_connect_args)
{
    int i;
    midi_arrays_t* midi = malloc(sizeof(midi_arrays_t));

#ifdef ALSA_MIDI
    midi->seq = alsa_init_seq();
    if (!midi->seq)
    {
        fprintf(stderr, "ALSA MIDI initialisation error.\n");
        //return 0;
    }
#endif

#ifdef JACK_MIDI
    jack_client_t* client = (jack_client_t*)o;
    midi->seq = jack_init_seq(client);
    if (!midi->seq)
    {
        fprintf(stderr, "JACK MIDI initialisation error.\n");
    }
#endif
#ifdef LV2_MIDI
    midi->seq = lv2_init_seq((const LV2_Feature * const*)o);
    if (!midi->seq)
    {
        fprintf(stderr, "MIDI initialisation error.\n");
    }

#endif

    for(i=0; midi_connect_args[i]; i++)
    {
#ifdef ALSA_MIDI
        int aclient=0;
        int port=0;
        sscanf(midi_connect_args[i], "%d:%d", &aclient, &port);
        printf("Connecting ALSA MIDI input to %d:%d\n", aclient, port);
        alsa_midi_connect(midi->seq, aclient, port);
#endif
#ifdef JACK_MIDI
        //printf("Connecting JACK MIDI input to %d:%d\n", client, port);
        jack_midi_connect(client, midi_connect_args[i]);
#endif
    }

    /*for reinit
    if(midi->midi_keys[0]!=NULL) {
    	for(i=0; midi->midi_keys[i]; i++) {
    		free(midi->midi_keys[i]);
    	}
    	free(midi->midi_keys);
    }
    if(midi->free_voices!=NULL) free(midi->free_voices);
    */

    midi->midi_keys = malloc(sizeof(midi_key_state_t *)*(polyphony+1));
    for(i=0; i<polyphony; i++)
    {
        midi->midi_keys[i] = malloc(sizeof(midi_key_state_t));
        midi->midi_keys[i]->channel = -1;
        midi->midi_keys[i]->note_on = 0;
        midi->midi_keys[i]->note_state_changed = 0;
        midi->midi_keys[i]->last_used = -1;
        midi->midi_keys[i]->needs_clearing = 0;
    }
    midi->midi_keys[i] = NULL;
    midi->free_voices = malloc(sizeof(int)*(polyphony+1));
    midi->next_voice = 0;
    midi->voice_use_index = 0;

    for(i=0; i<128; i++) midi->midi_programs[i] = -1;

    for(i=0; i<16; i++)
    {
        midi->midi_channels[i].in_use = 0;
        midi->midi_channels[i].program = -1;
        midi->midi_channels[i].sustain = 0;
        midi->midi_channels[i].pitchbend = 0;
        midi->midi_channels[i].vibrato = 0;
        midi->midi_channels[i].vibrato_changed = 0;
    }

    // calculate midi note frequencies
    for(i=0; i<128; i++) midi->note_frqs[i] = 440.0*pow(2,((double)i-69.0)/12.0);

    return midi;
}

void midi_close(midi_arrays_t* midi, int polyphony)
{
    int i;
#if ALSA_MIDI
    close_alsa(midi->seq);
#endif
    free(midi->seq);
    for(i=0; i<polyphony; i++)
    {
        free(midi->midi_keys[i]);
    }
    free(midi->midi_keys);
    free(midi->free_voices);
}


//this makes a new midi_arrays object and copies a lot of data out of the old one
midi_arrays_t* new_midi_arrays(midi_arrays_t* old_midi, int polyphony)
{
    int i;
    midi_arrays_t* midi = malloc(sizeof(midi_arrays_t));

    // copy as much as we can from old, but re-init for hygiene

    midi->midi_keys = old_midi->midi_keys;
    for(i=0; i<polyphony; i++)
    {
        midi->midi_keys[i]->channel = -1;
        midi->midi_keys[i]->note_on = 0;
        midi->midi_keys[i]->note_state_changed = 0;
        midi->midi_keys[i]->last_used = -1;
        midi->midi_keys[i]->needs_clearing = 0;
    }
    midi->midi_keys[i] = NULL;
    midi->free_voices = old_midi->free_voices;
    midi->next_voice = 0;
    midi->voice_use_index = 0;

    for(i=0; i<128; i++) midi->midi_programs[i] = -1;

    for(i=0; i<16; i++)
    {
        midi->midi_channels[i].in_use = 0;
        midi->midi_channels[i].program = -1;
        midi->midi_channels[i].sustain = 0;
        midi->midi_channels[i].pitchbend = 0;
        midi->midi_channels[i].vibrato = 0;
        midi->midi_channels[i].vibrato_changed = 0;
    }

    for(i=0; i<128; i++) midi->note_frqs[i] = old_midi->note_frqs[i];

    midi->seq = old_midi->seq;

    return midi;

}
