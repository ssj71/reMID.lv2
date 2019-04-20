#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <inttypes.h>

//#define LV2
#ifndef LV2
#include <jack/jack.h>
#include "jack_audio.h"
#else
#include "lv2_audio.h"
#endif
#include "sid_instr.h"
#include "prefs.h"

#ifndef LV2
struct super
{
    struct CHIPS* sid_bank;
    struct midi_arrays* midi;
    sid_instrument_t **sid_instr;

    jack_client_t *client;
    jack_port_t *output_port_l;
    jack_port_t *output_port_r;
    char port_names[2][16];
};
#endif

int process(uint32_t nframes, void *arg)
{
    struct super* s = (struct super*)arg;

    read_midi(s->midi->seq,nframes, s->midi);

#ifndef LV2
    float *outl = (float *)jack_port_get_buffer(s->output_port_l, nframes);
    float *outr = (float *)jack_port_get_buffer(s->output_port_r, nframes);
#else
    float* outl = s->outl;
    float* outr = s->outr;
#endif

    sid_process(s->sid_bank, s->midi, s->sid_instr, (int)nframes, outr, outl);
    return 0;
}

int srate(uint32_t fs, void *arg)
{
    struct super* s = (struct super*)arg;
    printf("The sample rate is now %"PRIu32" /sec\n", fs);
    sid_set_srate(s->sid_bank, 1, fs);//1 should be pal arg
    return 0;
}

#ifndef LV2
void jack_shutdown(void *arg)
{
    struct super* s = (struct super*)arg;
    printf("JACK client shutting down\n");
    if(s->client) jack_client_close(s->client);
    midi_close(s->midi,s->sid_bank->polyphony);
    sid_close(s->sid_bank);
    free(s);
    exit(0);
}

void jack_connect_ports(jack_client_t *client, char port_names[][16], char** jack_connect_args)
{
    int i;
    char src_port[255];
    for(i=0; jack_connect_args[i]; i++)
    {
        char *port = jack_connect_args[i];
        sprintf(src_port, "%s:%s", CLIENT_NAME, port_names[i%2]);
        printf("Connecting JACK audio port to %s\n", port);
        if(jack_connect(client, src_port, port))
        {
            printf("Error connecting to JACK port %s\n", port);
        }
        else
        {
            printf("Connected to JACK port %s\n", port);
        }
        free(jack_connect_args[i]);
    }
}

int init_jack_audio( int use_sid_volume, int max_polyphony, int chiptype, int debug, char** jack_connect_args, char** midi_connect_args, char* instr_file)
{

    struct super *s = malloc(sizeof(struct super));

    s->client = jack_client_open(CLIENT_NAME,JackNullOption,NULL);
    if (!s->client)
    {
        fprintf(stderr, "Couldn't open connection to jack server\n");
        return 0;
    }


    s->midi = init_midi((void*)s->client, max_polyphony, midi_connect_args);//TODO: make sure this doesn't clobber the instrument stuff

    //load instrument //TODO: make a "find file" function
    s->sid_instr = NULL;
	s->sid_instr = read_instruments(instr_file, s->midi);
    if(!s->sid_instr)
        s->sid_instr = default_instrument();


    s->sid_bank = sid_init(max_polyphony, use_sid_volume,chiptype, debug);

    jack_set_process_callback(s->client, process, s);
    jack_set_sample_rate_callback(s->client, srate, s);
    jack_on_shutdown(s->client, jack_shutdown, s);

    strcpy(s->port_names[0],AUDIO_PORTNAME_L);
    s->output_port_l = jack_port_register(s->client, s->port_names[0], JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    strcpy(s->port_names[1],AUDIO_PORTNAME_R);
    s->output_port_r = jack_port_register(s->client, s->port_names[1], JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

    if (jack_activate(s->client))
    {
        fprintf(stderr, "Cannot activate client");
        return 0;
    }

    jack_connect_ports(s->client, s->port_names, jack_connect_args);


    printf("\nREADY.\n");


    //while(1) sleep(1);
    //jack_client_close(client);
    //exit (0);
    return 1;
}

#else
void* init_lv2_audio(uint32_t fs, char* instr_file, const LV2_Feature * const* host_features)
{

    struct super *s = malloc(sizeof(struct super));
    char* midi_connect_args[1] = {0};
    int max_polyphony = 32;
    int use_sid_volume = 0;
    int chiptype = 8580;
    int debug = 0;

    s->midi = init_midi((void*)host_features, max_polyphony, midi_connect_args);//TODO: make sure this doesn't clobber the instrument stuff

    //load instrument //TODO: make a "find file" function
    s->sid_instr = NULL;
	s->sid_instr = read_instruments(instr_file, s->midi);
    if(!s->sid_instr)
        s->sid_instr = default_instrument();//TODO: this doesn't really work

    s->oldmidi = s->newmidi = 0;
    s->old_sid_instr = s->new_sid_instr = 0;

    s->sid_bank = sid_init(max_polyphony, use_sid_volume, chiptype, debug);
    srate(fs,s);
    return (void*)s;
}

void cleanup_audio(void* arg)
{
    struct super* s = (struct super*)arg;
    midi_close(s->midi,s->sid_bank->polyphony);
    close_instruments(s->sid_instr);

    //in case there was an unfinished operation
    if(s->oldmidi)
    	free(s->oldmidi);
    if(s->newmidi)
    	free(s->newmidi);
    if(s->old_sid_instr)
    	close_instruments(s->old_sid_instr);
    if(s->new_sid_instr)
    	close_instruments(s->new_sid_instr);

    sid_close(s->sid_bank);
    free(s);
}

#endif
