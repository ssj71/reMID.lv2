
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#include "prefs.h"

#ifndef MAX_POLYPHONY
#define MAX_POLYPHONY 32
#endif


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

//    init_jack_audio(use_sid_volume, max_poly, pt_debug, jack_connect_args, midi_connect_args, instr_file);
    init_lv2_audio();

    while(1) sleep(1);
}

