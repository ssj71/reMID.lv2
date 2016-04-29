#ifndef MYLV2_MIDI_H
#define MYLV2_MIDI_H

#include<lv2.h>
#include<lv2/lv2plug.in/ns/ext/urid/urid.h>
#include<lv2/lv2plug.in/ns/ext/midi/midi.h>
#include<lv2/lv2plug.in/ns/ext/atom/util.h>
#include<lv2/lv2plug.in/ns/ext/time/time.h>
#include<lv2/lv2plug.in/ns/ext/patch/patch.h>
#include<lv2/lv2plug.in/ns/ext/atom/forge.h>
#include<lv2/lv2plug.in/ns/ext/worker/worker.h>
#include<lv2/lv2plug.in/ns/ext/state/state.h>
#include "midi.h"

//again I'd rather not have this in the header, but it does simplify things a lot
struct urid_t
{
    //lv2 stuff
    LV2_URID m_midi_event;
    LV2_URID a_blank;
    LV2_URID a_atom_Sequence;
    LV2_URID a_float;
    LV2_URID a_long;
    LV2_URID a_object;
    LV2_URID a_path;
    LV2_URID a_urid;
    LV2_URID t_time;
    LV2_URID t_beatsperbar;
    LV2_URID t_bpm;
    LV2_URID t_speed;
    LV2_URID t_frame;
    LV2_URID t_framespersec;
    LV2_URID p_Set;
    LV2_URID p_Get;
    LV2_URID p_property;
    LV2_URID p_value;
    LV2_URID filetype_instr;
    LV2_URID polyphony;
    LV2_URID chiptype;
    LV2_URID use_sid_vol;
};

struct lmidi
{
    struct urid_t urid;
    LV2_Atom_Sequence* atom_in_p;//for host to tell us what file to load
    LV2_Atom_Sequence* atom_out_p;//to notify host that what file we've loaded
    LV2_Atom_Forge forge;
    LV2_Atom_Forge_Frame atom_frame;
    LV2_Worker_Schedule* scheduler;
    char filepath[255];
    char newfilepath[255];
};

void* lv2_init_seq(const LV2_Feature * const* host_features);
void lv2_read_midi(void* mseq, uint32_t nframes, struct midi_arrays *midi);

#endif
