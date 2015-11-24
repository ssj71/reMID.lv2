#ifndef GUI_H
#define GUI_H

typedef struct gui_args {
	int argc;
	char **argv;
} gui_args_t;

void *gui_start(void *args);

#endif
