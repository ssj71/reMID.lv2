
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#include "lv2_audio.h"
#include "lv2_midi.h"

#define REMID_URI "http://github.com/ssj71/reMID.lv2"
#ifndef MAX_POLYPHONY
#define MAX_POLYPHONY 32
#endif


#if(0)
int main(int argc, char **argv)
{
    int c, use_sid_volume=0, max_poly = MAX_POLYPHONY;
    pthread_t gui_thread;
    int use_gui;
    int pt_debug;
    char *midi_connect_args[255];
    char *jack_connect_args[255];
    char *instr_file = "instruments.conf";
    jack_connect_args[0] = NULL;
    midi_connect_args[0] = NULL;

    while((c = getopt(argc, argv, "dhj:m:np:i:s:"))!=-1)
    {
        switch (c)
        {
        case 'd':
            pt_debug = 1;
            break;
        case 'h':
            usage(argv[0]);
            break;
        case 'j':
            add_connect(jack_connect_args,optarg);
            break;
        case 'm':
            add_connect(midi_connect_args,optarg);
            break;
        case 'n':
            use_gui = 0;
            break;
        case 'p':
            max_poly = atoi(optarg);
            break;
        case 'i':
            instr_file = optarg;
            break;
        case 's':
            if(atoi(optarg))
            {
                printf("using SID volume control\n");
                use_sid_volume = 1;
            }
            else
            {
                printf("Not using SID volume control\n");
                use_sid_volume = 0;
            }
            break;
        }
    }

    if(max_poly < 1)
        max_poly = 1;
    if(max_poly > 128)
        max_poly = 128;
}
#endif

//    init_jack_audio(use_sid_volume, max_poly, pt_debug, jack_connect_args, midi_connect_args, instr_file);
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
	Remid* p = malloc(sizeof(Remid));
    p->everything = init_lv2_audio((uint32_t)sample_freq,host_features);
    p->L =
}

void connect_remid_ports(LV2_Handle handle, uint32_t port, void* data)
{
	switch(port)
	{
	case 0:
		set_lout((void*)handle,(float*)data);
		break;
	case 1:
		set_rout((void*)handle,(float*)data);
		break;
	case 2:
		set_ain((void*)handle,(float*)data);
		break;
	case 3:
		set_aout((void*)handle,(float*)data);
		break;
	}
}


static const LV2_Descriptor lv2_descriptor=
{
    REMID_URI,
    init_remid,
    connect_remid_ports,
    0,//activate
    run_remid,
    0,//deactivate
    cleanup_remid
};

LV2_SYMBOL_EXPORT
const LV2_Descriptor* lv2_descriptor(uint32_t index)
{
    switch (index)
    {
    case 0:
        return &lv2_descriptor ;
