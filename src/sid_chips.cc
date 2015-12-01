
//using namespace std;//I don't think we actually need this
#include <iostream>
#include <math.h>
#include <alloca.h>
#include <cstdlib>
#include <stdio.h>
#include <resid/sid.h>

#include "sid_instr.h"
#include "midi.h"
#include "prefs.h"
#include "sid_chips.h"

extern "C"
void sid_close(struct CHIPS *chips)
{
    if(chips!=NULL)
    {
        if(chips->sid_chips!=NULL)
            for(int i=0; chips->sid_chips[i]; i++) delete chips->sid_chips[i];
        if(chips->table_states!=NULL)
        {
            for(int i=0; chips->table_states[i]; i++) free(chips->table_states[i]);
            free(chips->table_states);
        }
        free(chips);
    }
}

extern "C"
struct CHIPS* sid_init(int polyphony, int use_sid_volume, int debug)
{
    int i;

    struct CHIPS* self = (struct CHIPS*)malloc(sizeof(struct CHIPS));
    self->sid_chips = (SID**)malloc(sizeof(SID*)*(polyphony+1));
    for(i=0; i<polyphony; i++)
    {
        self->sid_chips[i] = new SID();

        self->sid_chips[i]->set_chip_model(MOS6581);
        //self->sid_chips[i]->set_chip_model(MOS8580);
        self->sid_chips[i]->reset();

        // initialise SID volume to max if we're not doing volume at the SID level
        if(!use_sid_volume) self->sid_chips[i]->write(0x18, 0x0f);
        self->use_sid_volume = use_sid_volume;

        //sid_chips[i]->write(0x04, 0x1);
        //midi_keys[i]->needs_clearing = 1;
    }
    self->sid_chips[i] = NULL;//safety net in case extra index is checked

    self->table_states = (sid_table_state_t **)calloc(polyphony+1, sizeof(sid_table_state_t *));
    for(i=0; i<polyphony; i++)
    {
        self->table_states[i] = (sid_table_state_t *)calloc(1, sizeof(sid_table_state_t));
        self->table_states[i]->stopped = 1;
    }
    self->table_states[i] = NULL;

    //cout << polyphony << " reSID chip polyphony system\n";
    printf("%i reSID chip polyphony system\n", polyphony);

    //TODO: need mechanism to set current polyphony (allows easy on the CPU)
    self->polyphony = polyphony;
    self->pt_debug = debug;

    self->buf_length = sizeof(short)*8192*self->polyphony;
    self->buf = (short *)malloc(self->buf_length);
    printf("%d bytes free in SID output buffer\n", self->buf_length);

    return self;
}

extern "C"
void sid_set_srate(struct CHIPS *chips, int pal, double srate)
{
    int i;

    chips->sample_freq = srate;

    if(pal) chips->clock_freq = 985248;
    else chips->clock_freq = 1022730;
    chips->freq_mult = chips->clock_freq/16777216.0;
    printf("%s mode: clock frequency %.2f, frequency multiplier %f\n", (pal?"PAL":"NTSC"), chips->clock_freq, chips->freq_mult);

    chips->clocks_per_sample = chips->clock_freq/chips->sample_freq;

    for(i=0; chips->sid_chips[i]; i++)
    {
        //printf("setting sid sample frequency to %f\n", sample_freq);
        chips->sid_chips[i]->set_sampling_parameters(chips->clock_freq, SAMPLE_FAST, chips->sample_freq);
        //chips[i]->set_sampling_parameters(clock_freq, SAMPLE_INTERPOLATE, sample_freq);
        //chips[i]->set_sampling_parameters(clock_freq, SAMPLE_RESAMPLE, sample_freq);
    }
}

void table_clock(struct CHIPS *chips, sid_instrument_t *instr, int chip_num, int pt_debug)
{
    sid_table_state_t *tab = chips->table_states[chip_num];

    // pulse mods
    if(tab->v1_pulsemod)
    {
        tab->v1_pulse += tab->v1_pulsemod;
        chips->sid_chips[chip_num]->write(0x02, tab->v1_pulse&0xff);
        chips->sid_chips[chip_num]->write(0x03, tab->v1_pulse>>8);
    }
    if(tab->v2_pulsemod)
    {
        tab->v2_pulse += tab->v2_pulsemod;
        chips->sid_chips[chip_num]->write(0x09, tab->v2_pulse&0xff);
        chips->sid_chips[chip_num]->write(0x0a, tab->v2_pulse>>8);
    }
    if(tab->v3_pulsemod)
    {
        tab->v3_pulse += tab->v3_pulsemod;
        chips->sid_chips[chip_num]->write(0x10, tab->v3_pulse&0xff);
        chips->sid_chips[chip_num]->write(0x11, tab->v3_pulse>>8);
    }

    if(tab->wait_ticks)
    {
        tab->wait_ticks--;
        return;
    }


    sid_command_t *cmd = instr->sid_command_list;
    int state_changed = 0;
    while(cmd && !state_changed)
    {
        if(cmd->line_number!=tab->pc)
        {
            cmd = cmd->next;
            continue;
        }

        int opcode = cmd->opcode;
        int data1 = cmd->data1;
        int data2 = cmd->data2;
        void *data_ptr = cmd->data_ptr;

        int i;
        double d;
        switch(opcode)
        {
        case NOP:
            if(pt_debug) printf("nop\n");
            break;
        case STOP:
            if(pt_debug) printf("instrument program execution stopped\n");
            tab->stopped = 1;
            state_changed = 1;
            break;
        case WAIT:
            tab->wait_ticks = data1;
            if(pt_debug) printf("waiting %d\n", data1);
            break;
        case GOTO:
            if(pt_debug) printf("goto %d\n", data1);
            tab->pc = data1-1;
            state_changed = 1;
            break;
        case PRINT:
            printf("%s\n", (char *)data_ptr);
            break;
        case V1FREQ:
            tab->v1_freq = (double)data1;
            tab->v1_base_freq = tab->v1_freq;
            i = (int)round(tab->v1_freq/chips->freq_mult);
            if(pt_debug) printf("v1freq %f\n", tab->v1_freq);
            chips->sid_chips[chip_num]->write(0x00, i&0xff);
            chips->sid_chips[chip_num]->write(0x01, i>>8);
            break;
        case V2FREQ:
            tab->v2_freq = (double)data1;
            tab->v2_base_freq = tab->v2_freq;
            i = (int)round(tab->v2_freq/chips->freq_mult);
            if(pt_debug) printf("v2freq %f\n", tab->v1_freq);
            chips->sid_chips[chip_num]->write(0x07, i&0xff);
            chips->sid_chips[chip_num]->write(0x08, i>>8);
            break;
        case V3FREQ:
            tab->v3_freq = (double)data1;
            tab->v3_base_freq = tab->v3_freq;
            i = (int)round(tab->v3_freq/chips->freq_mult);
            if(pt_debug) printf("v3freq %f\n", tab->v1_freq);
            chips->sid_chips[chip_num]->write(0x0e, i&0xff);
            chips->sid_chips[chip_num]->write(0x0f, i>>8);
            break;
        case V1FREQPCT:
            d = (double)data1;
            tab->v1_freq *= (1.0+(d/100.0));
            tab->v1_base_freq = tab->v1_freq;
            i = (int)round(tab->v1_freq/chips->freq_mult);
            if(pt_debug) printf("v1freqpct %d %f\n", data1, tab->v1_freq);
            chips->sid_chips[chip_num]->write(0x00, i&0xff);
            chips->sid_chips[chip_num]->write(0x01, i>>8);
            break;
        case V2FREQPCT:
            d = (double)data1;
            tab->v2_freq *= (1.0+(d/100.0));
            tab->v2_base_freq = tab->v2_freq;
            i = (int)round(tab->v2_freq/chips->freq_mult);
            if(pt_debug) printf("v2freqpct %d %f\n", data1, tab->v2_freq);
            chips->sid_chips[chip_num]->write(0x07, i&0xff);
            chips->sid_chips[chip_num]->write(0x08, i>>8);
            break;
        case V3FREQPCT:
            d = (double)data1;
            tab->v3_freq *= (1.0+(d/100.0));
            tab->v3_base_freq = tab->v3_freq;
            i = (int)round(tab->v3_freq/chips->freq_mult);
            if(pt_debug) printf("v3freqpct %d %f\n", data1, tab->v3_freq);
            chips->sid_chips[chip_num]->write(0x0e, i&0xff);
            chips->sid_chips[chip_num]->write(0x0f, i>>8);
            break;
        case V1FREQHS:
            d = (double)data1;
            tab->v1_freq *= pow(2,(d/12.0));
            tab->v1_base_freq = tab->v1_freq;
            i = (int)round(tab->v1_freq/chips->freq_mult);
            if(pt_debug) printf("v1freqhs %d %f\n", data1, tab->v1_freq);
            chips->sid_chips[chip_num]->write(0x00, i&0xff);
            chips->sid_chips[chip_num]->write(0x01, i>>8);
            break;
        case V2FREQHS:
            d = (double)data1;
            tab->v2_freq *= pow(2,(d/12.0));
            tab->v2_base_freq = tab->v2_freq;
            i = (int)round(tab->v2_freq/chips->freq_mult);
            if(pt_debug) printf("v2freqhs %d %f\n", data1, tab->v2_freq);
            chips->sid_chips[chip_num]->write(0x07, i&0xff);
            chips->sid_chips[chip_num]->write(0x08, i>>8);
            break;
        case V3FREQHS:
            d = (double)data1;
            tab->v3_freq *= pow(2,(d/12.0));
            tab->v3_base_freq = tab->v3_freq;
            i = (int)round(tab->v3_freq/chips->freq_mult);
            if(pt_debug) printf("v3freqhs %d %f\n", data1, tab->v3_freq);
            chips->sid_chips[chip_num]->write(0x0e, i&0xff);
            chips->sid_chips[chip_num]->write(0x0f, i>>8);
            break;
        case V1PULSE:
            if(pt_debug) printf("v1pulse %d\n", data1);
            chips->sid_chips[chip_num]->write(0x02, data1&0xff);
            chips->sid_chips[chip_num]->write(0x03, data1>>8);
            break;
        case V2PULSE:
            if(pt_debug) printf("v2pulse %d\n", data1);
            chips->sid_chips[chip_num]->write(0x09, data1&0xff);
            chips->sid_chips[chip_num]->write(0x0a, data1>>8);
            break;
        case V3PULSE:
            if(pt_debug) printf("v2pulse %d\n", data1);
            chips->sid_chips[chip_num]->write(0x10, data1&0xff);
            chips->sid_chips[chip_num]->write(0x11, data1>>8);
            break;
        case V1CONTROL:
            if(pt_debug) printf("v1_control 0x%x\n", data1);
            tab->v1_control = data1;
            chips->sid_chips[chip_num]->write(0x04, (data1&0xfe)|tab->v1_gate);
            break;
        case V2CONTROL:
            if(pt_debug) printf("v2_control 0x%x\n", data1);
            tab->v3_control = data1;
            chips->sid_chips[chip_num]->write(0x0b, (data1&0xfe)|tab->v2_gate);
            break;
        case V3CONTROL:
            if(pt_debug) printf("v3_control 0x%x\n", data1);
            tab->v3_control = data1;
            chips->sid_chips[chip_num]->write(0x12, (data1&0xfe)|tab->v3_gate);
            break;
        case V1AD:
            if(pt_debug) printf("v1_ad 0x%x\n", data1);
            chips->sid_chips[chip_num]->write(0x05, data1);
            break;
        case V2AD:
            if(pt_debug) printf("v2_ad 0x%x\n", data1);
            chips->sid_chips[chip_num]->write(0x0c, data1);
            break;
        case V3AD:
            if(pt_debug) printf("v3_ad 0x%x\n", data1);
            chips->sid_chips[chip_num]->write(0x13, data1);
            break;
        case V1SR:
            if(pt_debug) printf("v1_sr 0x%x\n", data1);
            chips->sid_chips[chip_num]->write(0x06, data1);
            break;
        case V2SR:
            if(pt_debug) printf("v2_sr 0x%x\n", data1);
            chips->sid_chips[chip_num]->write(0x0d, data1);
            break;
        case V3SR:
            if(pt_debug) printf("v3_sr 0x%x\n", data1);
            chips->sid_chips[chip_num]->write(0x14, data1);
            break;
        case FILTER_CUTOFF:
            if(pt_debug) printf("filter_cutoff 0x%x\n", data1);
            chips->sid_chips[chip_num]->write(0x15, data1&0xff);
            chips->sid_chips[chip_num]->write(0x16, data1>>8);
            break;
        case FR_VIC:
            if(pt_debug) printf("fr_vic 0x%x\n", data1);
            chips->sid_chips[chip_num]->write(0x17, data1);
            break;
        case FILTER_MODE:
            if(pt_debug) printf("filter_mode 0x%x\n", data1);
            chips->sid_chips[chip_num]->write(0x18, (data1<<4)|(tab->vol&0xf));
            break;
        case V1PULSEMOD:
            if(pt_debug) printf("v1_pulsemod 0x%x\n", data1);
            tab->v1_pulsemod = data1;
            break;
        case V2PULSEMOD:
            if(pt_debug) printf("v2_pulsemod 0x%x\n", data1);
            tab->v2_pulsemod = data1;
            break;
        case V3PULSEMOD:
            if(pt_debug) printf("v3_pulsemod 0x%x\n", data1);
            tab->v3_pulsemod = data1;
            break;
        case V1GATE:
            if(pt_debug) printf("v1_gate 0x%x\n", data1);
            tab->v1_no_midi_gate = 1;
            tab->v1_gate = data1;
            chips->sid_chips[chip_num]->write(0x04, (tab->v1_control&0xfe)|tab->v1_gate);
            break;
        case V2GATE:
            if(pt_debug) printf("v2_gate 0x%x\n", data1);
            tab->v2_no_midi_gate = 1;
            tab->v2_gate = data1;
            chips->sid_chips[chip_num]->write(0x0b, (tab->v2_control&0xfe)|tab->v1_gate);
            break;
        case V3GATE:
            if(pt_debug) printf("v2_gate 0x%x\n", data1);
            tab->v3_no_midi_gate = 1;
            tab->v3_gate = data1;
            chips->sid_chips[chip_num]->write(0x12, (tab->v3_control&0xfe)|tab->v1_gate);
            break;
        }
        cmd = cmd->next;
    }
    tab->pc++;
}

void clear_key(midi_key_state** midi_keys, int key)
{
    midi_keys[key]->note_on = 0;
    midi_keys[key]->note_state_changed = 0;
}

extern "C"
short *sid_process(struct CHIPS *chips, midi_arrays_t* midi, sid_instrument_t** sid_instr, int num_samples)
{

    jack_time_t time_now = jack_get_time();
    int i;
    for(i=0; i<chips->polyphony; i++)
    {
        SID *sid = chips->sid_chips[i];
        sid_table_state_t *tab = chips->table_states[i];
        int channel = midi->midi_keys[i]->channel;
        if(channel==-1)
        {
            clear_key(midi->midi_keys,i);
            continue;
        }
        int program = midi->midi_channels[channel].program;
        if(program==-1)
        {
            clear_key(midi->midi_keys,i);
            continue;
        }
        int inst_num = midi->midi_programs[program];
        if(inst_num==-1)
        {
            clear_key(midi->midi_keys,i);
            continue;
        }
        //printf("%d: %d - %d\n", channel, program, inst_num);
        sid_instrument_t *instr = sid_instr[inst_num];//TODO: global instrument thing

        // percussion map
        if(instr->type==PERCUSSION_MAP)
        {
            int note = midi->midi_keys[i]->note;
            inst_num = instr->percussion_map[note];
            if(inst_num==-1)
            {
                clear_key(midi->midi_keys,i);
                continue;
            }
            instr = sid_instr[inst_num];
        }

        // process new midi events
        if(midi->midi_keys[i]->note_state_changed)
        {
            //printf("using channel %d\n", channel);
            //printf("using program %d\n", program);
            //printf("using instrument %d\n", inst_num);

            if(midi->midi_keys[i]->needs_clearing)
            {
                sid->write(0x04, 0x0); // release voices
                sid->write(0x0b, 0x0);
                sid->write(0x12, 0x0);
                midi->midi_keys[i]->needs_clearing = 0;
            }
            else if(midi->midi_keys[i]->note_on)
            {
                double freq = midi->note_frqs[midi->midi_keys[i]->note];
                int freqi = (int)round(freq/chips->freq_mult);
                //int freq = note_frqs[midi_keys[i]->note];
                //printf("%d, %d, %d\n", freq, freq&0xff, freq>>8);

                if(instr->v1_freq)
                {
                    double v1_freq = instr->v1_freq;
                    int v1_freqi = (int)round(v1_freq/chips->freq_mult);
                    sid->write(0x00, v1_freqi&0xff);	// v1 freq lo
                    sid->write(0x01, v1_freqi>>8);		// v1 freq hi
                    tab->v1_freq = v1_freq;
                    tab->v1_base_freq = v1_freq;
                }
                else
                {
                    sid->write(0x00, freqi&0xff);	// v1 freq lo
                    sid->write(0x01, freqi>>8);	// v1 freq hi
                    tab->v1_freq = freq;
                    tab->v1_base_freq = freq;
                }

                if(instr->v2_freq)
                {
                    double v2_freq = instr->v2_freq;
                    int v2_freqi = (int)round(v2_freq/chips->freq_mult);
                    sid->write(0x07, v2_freqi&0xff);	// v1 freq lo
                    sid->write(0x08, v2_freqi>>8);		// v1 freq hi
                    tab->v2_freq = v2_freq;
                    tab->v2_base_freq = v2_freq;
                }
                else
                {
                    sid->write(0x07, freqi&0xff);	// v2 freq lo
                    sid->write(0x08, freqi>>8);	// v2 freq hi
                    tab->v2_freq = freq;
                    tab->v2_base_freq = freq;
                }

                if(instr->v3_freq)
                {
                    double v3_freq = instr->v2_freq;
                    int v3_freqi = (int)round(v3_freq/chips->freq_mult);
                    sid->write(0x0e, v3_freqi&0xff);	// v1 freq lo
                    sid->write(0x0f, v3_freqi>>8);		// v1 freq hi
                    tab->v3_freq = v3_freq;
                    tab->v3_base_freq = v3_freq;
                }
                else
                {
                    sid->write(0x0e, freqi&0xff);	// v3 freq lo
                    sid->write(0x0f, freqi>>8);	// v3 freq hi
                    tab->v3_freq = freq;
                    tab->v3_base_freq = freq;
                }

                sid->write(0x02, instr->v1_pulse&0xff);
                sid->write(0x03, instr->v1_pulse>>8);
                sid->write(0x05, instr->v1_ad);
                sid->write(0x06, instr->v1_sr);

                sid->write(0x09, instr->v2_pulse&0xff);
                sid->write(0x0a, instr->v2_pulse>>8);
                sid->write(0x0c, instr->v2_ad);
                sid->write(0x0d, instr->v2_sr);

                sid->write(0x10, instr->v3_pulse&0xff);
                sid->write(0x11, instr->v3_pulse>>8);
                sid->write(0x13, instr->v3_ad);
                sid->write(0x14, instr->v3_sr);

                sid->write(0x15, instr->filter_cutoff&0xff);
                sid->write(0x16, instr->filter_cutoff>>8);
                sid->write(0x17, instr->fr_vic);

                //now get rest of table ready for commands
                if(chips->use_sid_volume)
                {
                    int vol = midi->midi_keys[i]->velocity/8;
                    tab->vol = vol;
                }
                else
                {
                    tab->vol = 0xf;
                }
                tab->filter_mode = instr->filter_mode;
                sid->write(0x18, (tab->filter_mode<<4)|(tab->vol&0xf));

                sid->write(0x04, instr->v1_control|0x01);
                tab->v1_control = instr->v1_control;
                tab->v1_gate = 1;

                sid->write(0x0b, instr->v2_control|0x01);
                tab->v2_control = instr->v2_control;
                tab->v2_gate = 1;

                sid->write(0x12, instr->v3_control|0x01);
                tab->v3_control = instr->v3_control;
                tab->v3_gate = 1;

                midi->midi_keys[i]->note_state_changed = 0;

                tab->v1_pulse = instr->v1_pulse;
                tab->v1_pulsemod = 0;
                tab->v1_no_midi_gate = 0;
                tab->v2_pulse = instr->v2_pulse;
                tab->v2_pulsemod = 0;
                tab->v2_no_midi_gate = 0;
                tab->v3_pulse = instr->v3_pulse;
                tab->v3_pulsemod = 0;
                tab->v3_no_midi_gate = 0;
                tab->inst_num = inst_num;
                tab->next_tick = time_now;
                tab->wait_ticks = 0;
                tab->stopped = 0;
                tab->pc = 0;
            }
            else if(!midi->midi_channels[channel].sustain && !midi->midi_keys[i]->note_on)
            {
                if(!tab->v1_no_midi_gate)
                {
                    sid->write(0x04, tab->v1_control&0xfe); // voice 1 waveform, start R
                    tab->v1_gate = 0;
                }
                if(!tab->v2_no_midi_gate)
                {
                    sid->write(0x0b, tab->v2_control&0xfe); // voice 2 waveform, start R
                    tab->v2_gate = 0;
                }
                if(!tab->v3_no_midi_gate)
                {
                    sid->write(0x12, tab->v3_control&0xfe); // voice 3 waveform, start R
                    tab->v3_gate = 0;
                }
                midi->midi_keys[i]->note_state_changed = 0;
            }
        }

        // TODO chanpress - per channel aftertouch
        /*if(midi_channels[channel].chanpress_changed) {
        	int press = midi_channels[channel].chanpress;
        	printf("chanpress %d\n", press);
        	midi_channels[channel].chanpress_changed = 0;
        	sid->write(0x16, press>>1);
        	sid->write(0x17, 0x3f);
        	sid->write(0x18, 0x6f);
        }*/

        // pitchbend
        if(midi->midi_channels[channel].pitchbend != tab->pitchbend)
        {
            double d = ((double)midi->midi_channels[channel].pitchbend)/8192;
            //printf("%f\n", d);
            // TODO configurable range
            double range = 2.0; // pitchbend range in half steps
            tab->v1_freq = tab->v1_base_freq*(pow(2,(range/12.0)*d));
            tab->v2_freq = tab->v2_base_freq*(pow(2,(range/12.0)*d));
            tab->v3_freq = tab->v3_base_freq*(pow(2,(range/12.0)*d));

            int ipb;
            ipb = (int)round(tab->v1_freq/chips->freq_mult);
            sid->write(0x00, ipb&0xff);
            sid->write(0x01, ipb>>8);

            ipb = (int)round(tab->v2_freq/chips->freq_mult);
            sid->write(0x07, ipb&0xff);
            sid->write(0x08, ipb>>8);

            ipb = (int)round(tab->v3_freq/chips->freq_mult);
            sid->write(0x0e, ipb&0xff);
            sid->write(0x0f, ipb>>8);

            tab->pitchbend = midi->midi_channels[channel].pitchbend;
        }

        // vibrato control 0-127
        // TODO fix return to zero
        if(midi->midi_channels[channel].vibrato_changed)
        {
            int vibrato = midi->midi_channels[channel].vibrato;
            if(!vibrato) midi->midi_channels[channel].vibrato_changed = 0;

            // TODO - configurable range and speed
            double range = 2.0; // range of vibration in half steps
            double speed = 10.0; // speed of vibration in cycles per second

            double amplitude = ((double)vibrato)/127.0; // 0.0 - 1.0
            double mod = amplitude*range;

            int time_mod = (int)time_now%(int)(1000000.0/speed);
            double period = (double)time_mod/(1000000.0/speed); // 0.0 - 1.0

            double sinval = sin(2*M_PI*period);
            double modx = pow(2.0, (mod*sinval)/12.0);

            tab->v1_freq = tab->v1_base_freq*modx;
            tab->v2_freq = tab->v2_base_freq*modx;
            tab->v3_freq = tab->v3_base_freq*modx;

            int ifr;
            ifr = (int)round(tab->v1_freq/chips->freq_mult);
            sid->write(0x00, ifr&0xff);
            sid->write(0x01, ifr>>8);
            ifr = (int)round(tab->v2_freq/chips->freq_mult);
            sid->write(0x07, ifr&0xff);
            sid->write(0x08, ifr>>8);
            ifr = (int)round(tab->v3_freq/chips->freq_mult);
            sid->write(0x0e, ifr&0xff);
            sid->write(0x0f, ifr>>8);
        }

        // process table
        while((!tab->stopped) && time_now>=tab->next_tick)
        {
            table_clock(chips, instr, i, chips->pt_debug);//TODO: get pt_debug from prefs
            int speed = instr->program_speed;
            tab->next_tick += (1000000/speed);
        }
    }

    for(i=0; i<chips->polyphony; i++)
    {
        // clock and get output
        //printf("getting output\n");
        int samples_received = 0;
        int j;
        while(samples_received<num_samples)
        {
            cycle_count cycles = (cycle_count)chips->clocks_per_sample*(cycle_count)(num_samples-samples_received);
            j = chips->sid_chips[i]->clock(cycles, chips->buf+(i*num_samples)+samples_received, num_samples-samples_received);
            samples_received += j;
            //printf("samples_received: %d / num_samples: %d\n", samples_received, num_samples);
        }
        //printf("got output\n");
    }

    return chips->buf;
}

