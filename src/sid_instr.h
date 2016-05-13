#ifndef SID_INSTRUMENT_H
#define SID_INSTRUMENT_H
//#include <jack/jack.h>
#include <stdint.h>

//keep max 2^N
#define MAXNLOOPS 16

//TODO: add env3toFilter and osc3toFitler
enum OPCODES
{
    NOP=0,
    STOP,
    WAIT,
    GOTO,
    PRINT,
    V1FREQ, V1FREQPCT, V1FREQHS, V1PULSE, V1CONTROL, V1AD, V1SR,
    V2FREQ, V2FREQPCT, V2FREQHS, V2PULSE, V2CONTROL, V2AD, V2SR,
    V3FREQ, V3FREQPCT, V3FREQHS, V3PULSE, V3CONTROL, V3AD, V3SR,
    FILTER_CUTOFF,
	FILTER_CUTPCT,
	FILTER_CUTMOD,
    FR_VIC,
    FILTER_MODE,
    V1PULSEMOD, V2PULSEMOD, V3PULSEMOD,
    V1GATE, V2GATE, V3GATE,
	V1DETUNE, V2DETUNE, V3DETUNE,
	ENV32FILTER,
	OSC32FILTER,
    INVALID,
    NUM_OPCODES
};

typedef struct sid_command
{
    int line_number;
    enum OPCODES opcode;
    int data1, data2;
    int reg;
    void *data_ptr;
    struct sid_command *next;
} sid_command_t;

enum INSTR_TYPE
{
    NORMAL=0,
    PERCUSSION_MAP
};

typedef struct sid_instrument
{
    char *name;
    char *description;

    int v1_freq;		// d400, d401
    int v1_pulse;		// d402, d403
    int v1_control;		// d404
    int v1_ad;		// d405
    int v1_sr;		// d406
    int v1_detune;

    int v2_freq;		// d407, d408
    int v2_pulse;		// d409, d40a
    int v2_control;		// d40b
    int v2_ad;		// d40c
    int v2_sr;		// d40d
    int v2_detune;

    int v3_freq;		// d40e, d40f
    int v3_pulse;		// d410, d411
    int v3_control;		// d412
    int v3_ad;		// d413
    int v3_sr;		// d414
    int v3_detune;

    int filter_cutoff;	// d415, d416
    int fr_vic;		// d417
    int filter_mode;	// d418-hi

    //jack_time_t program_speed;
    uint32_t program_speed;

    //sample_t vol_left;
    //sample_t vol_right;
    float vol_left;
    float vol_right;

    int panning;

    //sid_command_t sid_commands[256];
    sid_command_t *sid_command_list;

    enum INSTR_TYPE type;
    int percussion_map[128];	// percussion note to instrument array index
    char *percussion_mapn[128];	// names of percussion instruments

} sid_instrument_t;

typedef struct sid_table_state
{
    //jack_time_t next_tick;
	uint32_t next_tick;

    int pc;
    int counter[MAXNLOOPS];//we only allow 8 loops though they can be nested
    int stopped;
    int wait_ticks;
    int inst_num;

    double v1_base_freq;
    double v1_freq;
    int v1_control;
    int v1_gate;
    int v1_no_midi_gate;
    int v1_pulse;
    int v1_pulsemod;

    double v2_base_freq;
    double v2_freq;
    int v2_control;
    int v2_gate;
    int v2_no_midi_gate;
    int v2_pulse;
    int v2_pulsemod;

    double v3_base_freq;
    double v3_freq;
    int v3_control;
    int v3_gate;
    int v3_no_midi_gate;
    int v3_pulse;
    int v3_pulsemod;

    int fc;
    float fmod;
    int fr_vic;
    int filter_mode;
    int env32filter;
    int osc32filter;
    int vol;

    int pitchbend;
} sid_table_state_t;

#endif
