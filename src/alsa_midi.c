
#include <alsa/asoundlib.h>

#include "midi.h"
#include "jack_audio.h"

struct amidi{
	snd_seq_t *seq_handle;
	int in_port;
	int npfd;
	struct pollfd *pfd;
};

//void alsa_read_midi(struct midi_channel_state* midi_channels) {
void alsa_read_midi(void* mseq, struct midi_arrays* midi) {
	struct amidi* am = (struct amidi*)mseq;
	snd_seq_event_t *ev;
	int channel, param, value;

	if(!poll(am->pfd, am->npfd, 0)) return;

	do {
		snd_seq_event_input(am->seq_handle, &ev);
		switch(ev->type) {
			case SND_SEQ_EVENT_CONTROLLER:
				channel=ev->data.control.channel;
				if(!midi->midi_channels[channel].in_use) break;
				param=ev->data.control.param;
				value=ev->data.control.value;
				if(param==64) {
					if(value>64) midi->midi_channels[channel].sustain=1;
					else midi->midi_channels[channel].sustain=0;
				} else if(param==1) {
					// modulation controlling vibrato: value=0-127
					midi->midi_channels[channel].vibrato=value;
					//printf("%d\n", value);
					midi->midi_channels[channel].vibrato_changed=1;
				}
				break;
			// case SND_SEQ_EVENT_KEYPRESS:
			case SND_SEQ_EVENT_CHANPRESS:
				channel=ev->data.control.channel;
				if(!midi->midi_channels[channel].in_use) break;
				value=ev->data.control.value;
				midi->midi_channels[channel].chanpress=value;
				midi->midi_channels[channel].chanpress_changed=1;
				break;
			case SND_SEQ_EVENT_NOTEON:
				channel = ev->data.note.channel;
				if(!midi->midi_channels[channel].in_use) break;
				note_on(midi, channel, ev->data.note.note,
					ev->data.note.velocity);
				break;
			case SND_SEQ_EVENT_NOTEOFF:
				channel = ev->data.note.channel;
				if(!midi->midi_channels[channel].in_use) break;
				note_off(midi, channel, ev->data.note.note);
				break;
			case SND_SEQ_EVENT_PITCHBEND:
				// value=-8192 to +8191
				channel=ev->data.control.channel;
				if(!midi->midi_channels[channel].in_use) break;
				value=ev->data.control.value;
				midi->midi_channels[channel].pitchbend=value;
				break;
			case SND_SEQ_EVENT_PGMCHANGE:
				channel=ev->data.control.channel;
				if(midi->midi_channels[channel].program==-1) break;
				value=ev->data.control.value;
				//printf("prg change %d\n", value);
				midi->midi_channels[channel].program=value;
				break;
		}
	} while (snd_seq_event_input_pending(am->seq_handle, 0)>0);
}

void alsa_midi_connect(void* mseq, int client, int port) {
	struct amidi* am = (struct amidi*)mseq;
	snd_seq_connect_from(am->seq_handle, 0, client, port);
}

void* alsa_init_seq() {
	struct amidi* am = (struct amidi*)malloc(sizeof(struct amidi));


	if(snd_seq_open(&am->seq_handle, "default", SND_SEQ_OPEN_INPUT, 0)<0) {
		fprintf(stderr, "Error opening ALSA sequencer.\n");
		return 0;
	}

	snd_seq_set_client_name(am->seq_handle, CLIENT_NAME);

	am->in_port = snd_seq_create_simple_port(am->seq_handle, MIDI_PORTNAME,
		SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
		SND_SEQ_PORT_TYPE_APPLICATION);

	if(am->in_port<0) {
		fprintf(stderr, "Error creating sequencer port.\n");
		return 0;
	}

	am->npfd = snd_seq_poll_descriptors_count(am->seq_handle, POLLIN);
	am->pfd = (struct pollfd *)malloc(am->npfd*sizeof(struct pollfd));
	snd_seq_poll_descriptors(am->seq_handle, am->pfd, am->npfd, POLLIN);

	return (void*)am;
}

void close_alsa(void* mseq)
{
	struct amidi* am = (struct amidi*)mseq;
	free(am->pfd);
	//am is freed by calling function
}

