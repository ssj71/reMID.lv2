
#include <jack/jack.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "jack_audio.h"
#include "sid_instr.h"
#include "prefs.h"

struct super{
	struct CHIPS* sid_bank;
	struct midi_arrays* midi;
	sid_instrument_t **sid_instr;

	jack_client_t *client;
	jack_port_t *output_port_l;
	jack_port_t *output_port_r;
	char port_names[2][16];
};

int process(jack_nframes_t nframes, void *arg) {
	int i,j;

	short x;
	static short high=0;
	static short low=0;

	struct super* s = (struct super*)arg;

	pthread_mutex_lock(&prefs_mutex);//TODO: no mutexes in RT

	read_midi(s->midi->seq,nframes, s->midi->midi_channels);

	sample_t *outl = (sample_t *)jack_port_get_buffer(s->output_port_l, nframes);
	for(i=0; i<nframes; i++) outl[i]=0.0;
	sample_t *outr=(sample_t *)jack_port_get_buffer(s->output_port_r, nframes);
	for(i=0; i<nframes; i++) outr[i]=0.0;

	short *sid_buf = sid_process(s->sid_bank, s->midi, s->sid_instr, (int)nframes);
	for(i=0; i<s->sid_bank->polyphony; i++) {
		int channel = s->midi->midi_keys[i]->channel;
		if(channel==-1) continue;
		int program = s->midi->midi_channels[channel].program;
		if(program==-1) continue;
		//int inst_num=midi_programs[channel];
		int inst_num = s->midi->midi_programs[program];
		if(inst_num==-1) continue;
		sid_instrument_t *instr = s->sid_instr[inst_num];
		sample_t volume = ((sample_t)s->midi->midi_keys[i]->velocity)/128.0;
		for(j=0; j<nframes; j++) {
			x = sid_buf[(i*nframes)+j];
			sample_t a = ((sample_t)x)/32768.0;

			if(!s->sid_bank->use_sid_volume) a*=volume;

			sample_t al=a*instr->vol_left;
			sample_t ar=a*instr->vol_right;

			if(instr->panning) { 
				// 0-127 -> 0.0-2.0
				sample_t note=((sample_t)s->midi->midi_keys[i]->note)/64.0;
				al*=2.0-note;
				ar*=note;
			}

			outl[j]+=al;
			outr[j]+=ar;

			if(x>high) high=x;
			if(x<low) low=x;
		}
	}
	//printf("low: %d, high: %d\n", low, high);
	pthread_mutex_unlock(&prefs_mutex);//TODO: big no-no here
	return 0;      
}

int srate(jack_nframes_t nframes, void *arg) {
	struct super* s = (struct super*)arg;
	printf("The sample rate is now %" PRIu32 "/sec\n", nframes);
	sid_set_srate(s->sid_bank, 1, nframes);//1 should be pal arg
	return 0;
}

void jack_shutdown(void *arg) {
	struct super* s = (struct super*)arg;
	printf("JACK client shutting down\n");
	if(s->client) jack_client_close(s->client);
	midi_close(s->midi,s->sid_bank->polyphony);
	sid_close(s->sid_bank);
	free(s);
	exit(0);
}

void jack_connect_ports(jack_client_t *client, char port_names[][16]) {
	int i;
	char src_port[255];
	for(i=0; jack_connect_args[i]; i++) {
		char *port=jack_connect_args[i];
		snprintf(src_port, 255, "%s:%s", CLIENT_NAME, port_names[i%2]);
		printf("Connecting JACK audio port to %s\n", port);
		if(jack_connect(client, src_port, port)) {
			printf("Error connecting to JACK port %s\n", port);
		} else {
			printf("Connected to JACK port %s\n", port);
		}
	}
}

int init_jack_audio( int use_sid_volume, int max_polyphony, sid_instrument_t** sid_instr) {

	pthread_mutex_lock(&prefs_mutex);
	struct super *s = malloc(sizeof(struct super));

	s->client = jack_client_open(CLIENT_NAME,JackNullOption,NULL);
	if (!s->client) {
		fprintf(stderr, "Couldn't open connection to jack server\n");
		pthread_mutex_unlock(&prefs_mutex);
		return 0;
	}

	//TODO: need mechanism to set current polyphony
	s->sid_bank->polyphony=max_polyphony;

	init_midi(s->midi, s->client, s->sid_bank->polyphony);
	s->sid_instr = sid_instr;
	s->sid_bank = sid_init(s->sid_bank->polyphony, use_sid_volume);
	
	jack_set_process_callback(s->client, process, s);
	jack_set_sample_rate_callback(s->client, srate, s);
	jack_on_shutdown(s->client, jack_shutdown, s);

	strcpy(s->port_names[0],AUDIO_PORTNAME_L);
	s->output_port_l=jack_port_register(s->client, s->port_names[0], JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
	strcpy(s->port_names[1],AUDIO_PORTNAME_R);
	s->output_port_r=jack_port_register(s->client, s->port_names[1], JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

	if (jack_activate(s->client)) {
		fprintf(stderr, "Cannot activate client");
		pthread_mutex_unlock(&prefs_mutex);
		return 0;
	}

	jack_connect_ports(s->client, s->port_names);

	pthread_mutex_unlock(&prefs_mutex);

	printf("\nREADY.\n");


	//while(1) sleep(1);
	//jack_client_close(client);
	//exit (0);
	return 1;
}

