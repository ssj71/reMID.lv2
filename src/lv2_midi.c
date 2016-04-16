//spencer jackson
#include <stdio.h>
#include<lv2.h>
#include<lv2/lv2plug.in/ns/ext/urid/urid.h>
#include<lv2/lv2plug.in/ns/ext/midi/midi.h>
#include<lv2/lv2plug.in/ns/ext/atom/util.h>
#include<lv2/lv2plug.in/ns/ext/time/time.h>
#include<lv2/lv2plug.in/ns/ext/patch/patch.h>
#include<lv2/lv2plug.in/ns/ext/atom/forge.h>
#include<lv2/lv2plug.in/ns/ext/worker/worker.h>

#include "midi.h"

#define INSTRUMENT_FILE_URI "hey"

#define SND_SEQ_EVENT_NOTEOFF 0x80
#define SND_SEQ_EVENT_NOTEON 0x90
#define SND_SEQ_EVENT_KEYPRESS 0xa0
#define SND_SEQ_EVENT_CONTROLLER 0xb0
#define SND_SEQ_EVENT_PGMCHANGE 0xc0
#define SND_SEQ_EVENT_CHANPRESS 0xd0
#define SND_SEQ_EVENT_PITCHBEND 0xe0

struct urid_t
{
    //lv2 stuff
    LV2_URID m_midi_event;
    LV2_URID a_blank;
    LV2_URID a_atom_Sequence;
    LV2_URID a_float;
    LV2_URID a_long;
    LV2_URID a_object;
    LV2_URID a_urid;
    LV2_URID t_time;
    LV2_URID t_beatsperbar;
    LV2_URID t_bpm;
    LV2_URID t_speed;
    LV2_URID t_frame;
    LV2_URID t_framespersec;
    LV2_URID patch_Set;
    LV2_URID patch_Get;
    LV2_URID patch_property;
    LV2_URID patch_value;
    LV2_URID filetype_instr;
};

struct lmidi
{
    struct urid_t urid;
    LV2_Atom_Sequence* atom_in_p;//for host to tell us what file to load
    LV2_Atom_Sequence* atom_out_p;//to notify host that what file we've loaded
    LV2_Atom_Forge forge;
    LV2_Atom_Forge_Frame atom_frame;
    LV2_Worker_Schedule* scheduler;
};

void lv2_read_midi(void* mseq, uint32_t nframes, midi_arrays_t *midi)
{
    struct lmidi* lm = (struct lmidi*)mseq;
    LV2_Atom_Event event;
    uint8_t* msg;

    //TODO: not sample accurate
    LV2_ATOM_SEQUENCE_FOREACH(lm->atom_in_p, event)
    {
    	if(event)
    	{
    		if(event->body.type == lm->urid.m_midi_event)
    		{
                msg = (uint8_t*) LV2_ATOM_BODY(&(event->body));

				uint8_t status = msg[0];
				uint8_t param = msg[1];
				uint8_t value = msg[2];
				//printf("JACK MIDI event: %x %x %x\n", status, param, value);

				uint8_t ev_type = status&0xf0;
				uint8_t channel = status&0x0f;

				switch(ev_type)
				{
				case SND_SEQ_EVENT_CONTROLLER:
					if(!midi->midi_channels[channel].in_use) break;
					if(param==64)
					{
						if(value>64) midi->midi_channels[channel].sustain = 1;
						else midi->midi_channels[channel].sustain = 0;
					}
					else if(param==1)
					{
						// modulation controlling vibrato: value=0-127
						midi->midi_channels[channel].vibrato = value;
						//printf("%d\n", value);
						midi->midi_channels[channel].vibrato_changed = 1;
					}
					break;
				// case SND_SEQ_EVENT_KEYPRESS:
				case SND_SEQ_EVENT_CHANPRESS:
					if(!midi->midi_channels[channel].in_use) break;
					midi->midi_channels[channel].chanpress = value;
					midi->midi_channels[channel].chanpress_changed = 1;
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
					// value = -8192 to +8191
					if(!midi->midi_channels[channel].in_use) break;
					//int pitchbend = (value*128)|(param&0x7f);
					int pitchbend = (((value&0x7f)<<7)|(param&0x7f))-8192;
					//printf("got pitchbend %x %x: %x %d\n", param, value, pitchbend, pitchbend);
					midi->midi_channels[channel].pitchbend = pitchbend;
					break;
				case SND_SEQ_EVENT_PGMCHANGE:
					if(midi->midi_channels[channel].program==-1) break;
					//printf("prg change %d\n", value);
					midi->midi_channels[channel].program = param;
					break;
				}//switch message type
    		}//if event is midi
    		else if(event->body.type == lm->urid.a_object)
    		{
				const LV2_Atom_Object* obj = (const LV2_Atom_Object*)&event->body;
				if (obj->body.otype == lm->urid.patch_Set)
				{
					// Get the property the set message is setting
					const LV2_Atom* property = NULL;
					lv2_atom_object_get(obj, lm->urid.patch_property, &property, 0);
					if (property && property->type == lm->urid.a_urid)
					{
						const uint32_t key = ((const LV2_Atom_URID*)property)->body;
						if (key == lm->urid.filetype_instr)
						{
							// a new file! pass the atom to the worker thread to load it
							lm->scheduler->schedule_work(lm->scheduler->handle, lv2_atom_total_size(&event->body), &event->body);
						}//property is rvb file
					}//property is URID
				}
				else if (obj->body.otype == lm->urid.patch_Get)
				{
					// Received a get message, emit our state (probably to UI)
					lv2_atom_forge_frame_time(&lm->forge, event->time.frames );//use current event's time
					LV2_Atom_Forge_Frame frame;
					lv2_atom_forge_object( &lm->forge, &frame, 0, lm->urid.patch_Set);

					lv2_atom_forge_key(&lm->forge, lm->urid.patch_property);
					lv2_atom_forge_urid(&lm->forge, lm->urid.filetype_instr);
					lv2_atom_forge_key(&lm->forge, lm->urid.patch_value);
					lv2_atom_forge_path(&lm->forge, lm->revtron->File.Filename, strlen(lm->revtron->File.Filename)+1);

					lv2_atom_forge_pop(&lm->forge, &frame);
				}
    		}
    	}//if event not null
    }//for each atom
}

void* lv2_init_seq(const LV2_Feature * const* host_features)
{
    struct lmidi* lm = (struct lmidi*)malloc(sizeof(struct lmidi));
    for (int i = 0; host_features[i]; i++)
    {
        if (strcmp(host_features[i]->URI, LV2_URID__map) == 0)
        {
            LV2_URID_Map *urid_map = (LV2_URID_Map *) host_features[i]->data;
            if (urid_map)
            {
                lm->urid.m_midi_event = urid_map->map(urid_map->handle, LV2_MIDI__MidiEvent);
                lm->urid.a_blank = urid_map->map(urid_map->handle, LV2_ATOM__Blank);
                lm->urid.a_long = urid_map->map(urid_map->handle, LV2_ATOM__Long);
                lm->urid.a_float = urid_map->map(urid_map->handle, LV2_ATOM__Float);
                lm->urid.a_object = urid_map->map(urid_map->handle, LV2_ATOM__Object);
                lm->urid.a_urid = urid_map->map(urid_map->handle, LV2_ATOM__URID);
                lm->urid.t_time = urid_map->map(urid_map->handle, LV2_TIME__Position);
                lm->urid.t_beatsperbar = urid_map->map(urid_map->handle, LV2_TIME__barBeat);
                lm->urid.t_bpm = urid_map->map(urid_map->handle, LV2_TIME__beatsPerMinute);
                lm->urid.t_speed = urid_map->map(urid_map->handle, LV2_TIME__speed);
                lm->urid.t_frame = urid_map->map(urid_map->handle, LV2_TIME__frame);
                lm->urid.t_framespersec = urid_map->map(urid_map->handle, LV2_TIME__framesPerSecond);
                lm->urid.patch_Set = urid_map->map(urid_map->handle,LV2_PATCH__Set);
                lm->urid.patch_Get = urid_map->map(urid_map->handle,LV2_PATCH__Get);
                lm->urid.patch_property = urid_map->map(urid_map->handle,LV2_PATCH__property);
                lm->urid.patch_value = urid_map->map(urid_map->handle,LV2_PATCH__value);
                lm->urid.filetype_instr = urid_map->map(urid_map->handle,INSTRUMENT_FILE_URI);
                break;
            }
        }
        else if(!strcmp(host_features[i]->URI,LV2_WORKER__schedule))
        {
            lm->scheduler = (LV2_Worker_Schedule*)host_features[i]->data;
        }
    }
    return (void*)lm;
}

void lv2_close_seq(void* mseq)
{
    struct lmidi* lm = (struct lmidi*)mseq;
    free(lm);
}

LV2_Atom_Sequence** lv2_get_atom_port(void* mseq)
{
    struct lmidi* lm = (struct lmidi*)mseq;
    return &(lm->atom_in_p);
}
