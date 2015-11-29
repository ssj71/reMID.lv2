
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#ifdef GUI
#include "gui.h"
#endif

#include "jack_audio.h"
#include "prefs.h"

#ifndef MAX_POLYPHONY
#define MAX_POLYPHONY 32
#endif

void add_connect(char** connect_args, char *port) {
	int i;
	for(i=0; connect_args[i]; i++);
	strcpy(connect_args[i++], port);
	connect_args[i]=NULL;
}

void usage(char *prgname) {
	printf("usage: %s [options...]\n\n"
		"-h 			this help\n"
		"-d			debug program table execution\n"
		"-i	<path>	select instrument configuration file\n"
		"-j <client:port>	connect audio output to JACK port, may be specified multiple times\n"
		"-m <client:port>	connect MIDI input to output from client:port\n"
#ifdef GUI
		"-n			disable gui\n"
#endif
		"-p <num> 		polyphony level\n"
		"-s <0|1> 		1=use SID volume control register\n"
		"			0(default)=set SID volume to max and do volume outside\n"
		"\n", prgname);
	exit(0);
}

int main(int argc, char **argv) {
	int c, use_sid_volume, max_poly=MAX_POLYPHONY;
	pthread_t gui_thread;
	int use_gui;
	int pt_debug;
	char *midi_connect_args[255];
	char *jack_connect_args[255];
	char *instr_file = NULL;

	while((c=getopt(argc, argv, "dhj:m:np:s:"))!=-1) {
		switch (c) {
			case 'd':
				pt_debug=1;
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
				use_gui=0;
				break;
			case 'p':
				max_poly = atoi(optarg);
				break;
			case 'i':
				instr_file = optarg;
				break;
			case 's':
				if(atoi(optarg)) {
					printf("using SID volume control\n");
					use_sid_volume=1;
				} else {
					printf("Not using SID volume control\n");
					use_sid_volume=0;
				}
				break;
		}
	}

	if(max_poly < 1)
		max_poly = 1;
	if(max_poly > 128)
		max_poly = 128;

	//TODO: need to probably load instruments after super object is created

	init_jack_audio(use_sid_volume, max_poly, jack_connect_args, midi_connect_args, instr_file);

#ifdef GUI
	if(use_gui) {
		gui_args_t gargs = { argc, argv };
		pthread_create(&gui_thread, NULL, gui_start, (void *)&gargs);
	}
#endif

	while(1) sleep(1);
}

