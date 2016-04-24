
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#include "lv2_audio.h"
#include "lv2_midi.h"
#include<lv2/lv2plug.in/ns/ext/atom/util.h>
#include<lv2/lv2plug.in/ns/ext/state/state.h>
#include<lv2/lv2plug.in/ns/ext/worker/worker.h>

#define REMID_URI "http://github.com/ssj71/reMID.lv2"
#ifndef MAX_POLYPHONY
#define MAX_POLYPHONY 32
#endif
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
    return init_lv2_audio((uint32_t)sample_freq,host_features);
}

void connect_remid_ports(LV2_Handle handle, uint32_t port, void* data)
{
	//I don't really love the function calls here, but refactoring will be painful
	struct super* s = (struct super*)handle;
	struct lmidi* lm = (struct lmidi*)s->midi->seq;
	switch(port)
	{
	case 0:
		//set_lout((void*)handle,(float*)data);
		s->outl = (float*)data;
		break;
	case 1:
		//set_rout((void*)handle,(float*)data);
		s->outr = (float*)data;
		break;
	case 2:
		//set_ain((void*)handle,(float*)data);
		lm->atom_in_p = (LV2_Atom_Sequence*)data;
		break;
	case 3:
		//set_aout((void*)handle,(float*)data);
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
	//trouble is all the atom stuff we need is out of scope...
	//may need to put all structs in header files so I can access them :(

}
//this one is run in RT thread
static LV2_Worker_Status remidwork_response(LV2_Handle handle, uint32_t size, const void* data)
{

}
//this is not RT
static LV2_State_Status remidsave(LV2_Handle handle, LV2_State_Store_Function  store, LV2_State_Handle state_handle,
		uint32_t flags, const LV2_Feature* const* features)
{

}
//this is not RT
static LV2_State_Status remidrestore(LV2_Handle handle, LV2_State_Retrieve_Function retrieve,
		LV2_State_Handle state_handle, uint32_t flags, const LV2_Feature* const* features)
{

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

static const LV2_Descriptor lv2_descriptor=
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
        return &lv2_descriptor ;
