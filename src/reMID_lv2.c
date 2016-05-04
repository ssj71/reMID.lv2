
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>


#include "prefs.h"
#include "lv2_audio.h"
#include "lv2_midi.h"

#define REMID_URI "http://github.com/ssj71/reMID.lv2"
#define LV2



typedef struct arugalatastesbad
{
	float* L;
	float* R;
	LV2_Atom_Sequence* in;
	LV2_Atom_Sequence* out;

	void* everything;
}Remid;

LV2_Handle init_remid(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    return init_lv2_audio(lrint(sample_freq),host_features);
}

void connect_remid_ports(LV2_Handle handle, uint32_t port, void* data)
{
	struct super* s = (struct super*)handle;
	struct lmidi* lm = (struct lmidi*)s->midi->seq;
	switch(port)
	{
	case 0:
		s->outl = (float*)data;
		break;
	case 1:
		s->outr = (float*)data;
		break;
	case 2:
		lm->atom_in_p = (LV2_Atom_Sequence*)data;
		break;
	case 3:
		lm->atom_out_p = (LV2_Atom_Sequence*)data;
		break;
	}
}

void run_remid(LV2_Handle handle, uint32_t nframes)
{
	process(nframes,handle);
}

void cleanup_remid(LV2_Handle handle)
{
	cleanup_audio(handle);
}

//this is done in a separate thread;
static LV2_Worker_Status remidwork(LV2_Handle handle, LV2_Worker_Respond_Function respond, LV2_Worker_Respond_Handle rhandle, uint32_t size, const void* data)
{
	struct super* s = (struct super*)handle;
	struct lmidi* lm = s->midi->seq;
    LV2_Atom_Object* obj = (LV2_Atom_Object*)data;
    const LV2_Atom* file_path;

    //if we're here, then we will need a new file, get rid of old ones
    while(s->newmidi){usleep(1000);}//wait if in the middle of switching files
    if(s->oldmidi)
    	free(s->oldmidi);
    if(s->old_sid_instr)
    	close_instruments(s->old_sid_instr);
    s->oldmidi = 0;
    s->old_sid_instr = 0;

    //work was scheduled to load a new file
    lv2_atom_object_get(obj, lm->urid.p_value, &file_path, 0);
    if (file_path && file_path->type == lm->urid.a_path)
    {
        // Load file.
        char* path = (char*)LV2_ATOM_BODY_CONST(file_path);
        strcpy(lm->newfilepath,path);

        //need to create new arrays based on this instrument file
        s->newmidi = new_midi_arrays(s->midi,s->sid_bank->polyphony);
        s->new_sid_instr = read_instruments(path,s->newmidi);

        respond(rhandle,0,0);//not passing the new arrays directly, using plugin newmidi etc pointers
    }//got file
    else
        return LV2_WORKER_ERR_UNKNOWN;

    return LV2_WORKER_SUCCESS;

}
//this one is run in RT thread
static LV2_Worker_Status remidwork_response(LV2_Handle handle, uint32_t size, const void* data)
{
	//just switch to the new instrument arrays
	struct super* s = (struct super*)handle;
	struct lmidi* lm = s->midi->seq;
	s->oldmidi = s->midi;
	s->old_sid_instr = s->sid_instr;
	s->midi = s->newmidi;
	s->sid_instr = s->new_sid_instr;
	s->newmidi = 0;
	s->new_sid_instr = 0;
	strcpy(lm->filepath,lm->newfilepath);
	lm->newfilepath[0] = 0;
    return LV2_WORKER_SUCCESS;
}
//this is not RT
static LV2_State_Status remidsave(LV2_Handle handle, LV2_State_Store_Function  store, LV2_State_Handle state_handle,
		uint32_t flags, const LV2_Feature* const* features)
{
	struct super* s = (struct super*)handle;
	struct lmidi* lm = s->midi->seq;

    LV2_State_Map_Path* map_path = NULL;
    for (int i = 0; features[i]; ++i)
    {
        if (!strcmp(features[i]->URI, LV2_STATE__mapPath))
        {
            map_path = (LV2_State_Map_Path*)features[i]->data;
        }
    }

    char* abstractpath = map_path->abstract_path(map_path->handle, lm->filepath);

    store(state_handle, lm->urid.filetype_instr, abstractpath, strlen(lm->filepath) + 1,
    		lm->urid.a_path, LV2_STATE_IS_POD | LV2_STATE_IS_PORTABLE);

    free(abstractpath);

    return LV2_STATE_SUCCESS;

}
//this is not RT
static LV2_State_Status remidrestore(LV2_Handle handle, LV2_State_Retrieve_Function retrieve,
		LV2_State_Handle state_handle, uint32_t flags, const LV2_Feature* const* features)
{
	struct super* s = (struct super*)handle;
	struct lmidi* lm = s->midi->seq;
    size_t   size;
    uint32_t type;
    uint32_t valflags;
    uint8_t polyphony,vol;
    uint16_t chiptype;
    char* path = 0;

    const void* value = retrieve( state_handle, lm->urid.filetype_instr, &size, &type, &valflags);
    if (value)
		path = (char*)value;

    polyphony = s->sid_bank->polyphony;
    value = retrieve( state_handle, lm->urid.polyphony, &size, &type, &valflags);
    if (value)
		polyphony = *((uint8_t*)value);

    vol = s->sid_bank->use_sid_volume;
    value = retrieve( state_handle, lm->urid.use_sid_vol, &size, &type, &valflags);
    if (value)
		vol = *((uint8_t*)value);

    chiptype = s->sid_bank->chiptype;
    value = retrieve( state_handle, lm->urid.chiptype, &size, &type, &valflags);
    if (value)
		chiptype = *((uint16_t*)value);

    if(path)
    {
		if(s->oldmidi)
			free(s->oldmidi);
		if(s->old_sid_instr)
			close_instruments(s->old_sid_instr);
		s->oldmidi = 0;
		s->old_sid_instr = 0;
		if(s->newmidi && s->newmidi != s->midi)
		{
			//was loading a file, but this will supersede
			free(s->newmidi);
			close_instruments(s->new_sid_instr);
		}
		s->newmidi = 0;
		s->new_sid_instr = 0;

		s->oldmidi = s->midi;
		s->old_sid_instr = s->sid_instr;

        s->midi = new_midi_arrays(s->oldmidi,s->sid_bank->polyphony);
        s->sid_instr = read_instruments(path,s->midi);
        free(s->oldmidi);
		close_instruments(s->old_sid_instr);

    }

    //only reinit if something has changed
    if(polyphony != s->sid_bank->polyphony
    		|| vol != s->sid_bank->use_sid_volume
			|| chiptype != s->sid_bank->chiptype)
    {
    	sid_close(s->sid_bank);
		s->sid_bank = sid_init(polyphony,vol,chiptype,0);
    }


    return LV2_STATE_SUCCESS;

}

static const void* remid_extension_data(const char* uri)
{
    static const LV2_Worker_Interface worker = { remidwork, remidwork_response, NULL };
    static const LV2_State_Interface state_iface = { remidsave, remidrestore };
    if (!strcmp(uri, LV2_STATE__interface))
    {
        return &state_iface;
    }
    else if (!strcmp(uri, LV2_WORKER__interface))
    {
        return &worker;
    }
    return NULL;
}

static const LV2_Descriptor remid_lv2_descriptor=
{
    REMID_URI,
    init_remid,
    connect_remid_ports,
    0,//activate
    run_remid,
    0,//deactivate
    cleanup_remid,
	remid_extension_data
};

LV2_SYMBOL_EXPORT
const LV2_Descriptor* lv2_descriptor(uint32_t index)
{
    switch (index)
    {
    case 0:
        return &remid_lv2_descriptor ;
    }
    return 0;
}
