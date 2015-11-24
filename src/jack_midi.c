
#include <stdio.h>
#include <jack/midiport.h>

#include "jack_audio.h"
#include "midi.h"

#define SND_SEQ_EVENT_NOTEOFF 0x80
#define SND_SEQ_EVENT_NOTEON 0x90
#define SND_SEQ_EVENT_KEYPRESS 0xa0
#define SND_SEQ_EVENT_CONTROLLER 0xb0
#define SND_SEQ_EVENT_PGMCHANGE 0xc0
#define SND_SEQ_EVENT_CHANPRESS 0xd0
#define SND_SEQ_EVENT_PITCHBEND 0xe0

char *jmidi_portname="MIDI_In";

jack_port_t *midi_port;
void *midi_buf;
jack_midi_event_t midi_event;

void jack_read_midi(jack_nframes_t nframes, struct midi_arrays *midi) {
	jack_nframes_t i;

	midi_buf=jack_port_get_buffer(midi_port, nframes);
	jack_nframes_t  num_events=jack_midi_get_event_count(midi_buf);
	for(i=0; i<num_events; i++) {
		if(jack_midi_event_get(&midi_event, midi_buf, i)) return;
		int status=midi_event.buffer[0];
		jack_midi_data_t param=midi_event.buffer[1];
		jack_midi_data_t value=midi_event.buffer[2];
		//printf("JACK MIDI event: %x %x %x\n", status, param, value);

		int ev_type=status&0xf0;
		int channel=status&0x0f;

		switch(ev_type) {
			case SND_SEQ_EVENT_CONTROLLER:
				if(!midi->midi_channels[channel].in_use) break;
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
				if(!midi->midi_channels[channel].in_use) break;
				midi->midi_channels[channel].chanpress=value;
				midi->midi_channels[channel].chanpress_changed=1;
				break;
			case SND_SEQ_EVENT_NOTEON:
				if(!midi->midi_channels[channel].in_use) break;
				note_on(midi, channel, param, value);
				break;
			case SND_SEQ_EVENT_NOTEOFF:
				if(!midi->midi_channels[channel].in_use) break;
				note_off(midi, channel, param);
				break;
			case SND_SEQ_EVENT_PITCHBEND:
				// value=-8192 to +8191
				if(!midi->midi_channels[channel].in_use) break;
				//int pitchbend=(value*128)|(param&0x7f);
				int pitchbend=(((value&0x7f)<<7)|(param&0x7f))-8192;
				//printf("got pitchbend %x %x: %x %d\n", param, value, pitchbend, pitchbend);
				midi->midi_channels[channel].pitchbend=pitchbend;
				break;
			case SND_SEQ_EVENT_PGMCHANGE:
				if(midi->midi_channels[channel].program==-1) break;
				//printf("prg change %d\n", value);
				midi->midi_channels[channel].program=param;
				break;
		}
	}
}

void jack_midi_connect(char *port) {
	char dst_port[255];
	snprintf(dst_port, 255, "%s:%s", jclientname, jmidi_portname);
	printf("Connecting JACK MIDI port %s to %s\n", port, dst_port);
	// FIXME always fails
	if(jack_connect(client, port, dst_port)) {
		printf("Error connecting JACK MIDI port %s to %s\n", port, dst_port);
	}
}

int jack_init_seq() {
	fprintf(stderr, "Opening JACK MIDI port\n");
	midi_port=jack_port_register(client, jmidi_portname, JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);
	return 0;
}

