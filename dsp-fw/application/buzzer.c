#include "buzzer.h"

#include "my_time.h"
#include "driverlib.h"
#include "F28x_Project.h"     // DSP28x Header file




// Times (in microseconds)
#define WAIT_A 0       // 0 seconds wait for A
#define WAIT_B 1000    // 1 second wait for B
#define WAIT_C 2000    // 2 seconds wait for C
#define WAIT_STD 3000  // 3 seconds wait for STD




#define c 261
#define d 294
#define e 329
#define f 349
#define g 391
#define gS 415
#define a 440
#define aS 455
#define b 466
#define cH 523
#define cSH 554
#define dH 587
#define dSH 622
#define eH 659
#define fH 698
#define fSH 740
#define gH 784
#define gSH 830
#define aH 880


#define B0   31
#define C1   33
#define CS1  35
#define D1   37
#define DS1  39
#define E1   41
#define F1   44
#define FS1  46
#define G1   49
#define GS1  52
#define A1   55
#define AS1  58
#define B1   62
#define C2   65
#define CS2  69
#define D2   73
#define DS2  78
#define E2   82
#define F2   87
#define FS2  93
#define G2   98
#define GS2  104
#define A2   110
#define AS2  117
#define B2   123
#define C3   131
#define CS3  139
#define D3   147
#define DS3  156
#define E3   165
#define F3   175
#define FS3  185
#define G3   196
#define GS3  208
#define A3   220
#define AS3  233
#define B3   247
#define C4   262
#define CS4  277
#define D4   294
#define DS4  311
#define E4   330
#define F4   349
#define FS4  370
#define G4   392
#define GS4  415
#define A4   440
#define AS4  466
#define B4   494
#define C5   523
#define CS5  554
#define D5   587
#define DS5  622
#define E5   659
#define F5   698
#define FS5  740
#define G5   784
#define GS5  831
#define A5   880
#define AS5  932
#define B5   988
#define C6   1047
#define CS6  1109
#define D6   1175
#define DS6  1245
#define E6   1319
#define F6   1397
#define FS6  1480
#define G6   1568
#define GS6  1661
#define A6   1760
#define AS6  1865
#define B6   1976
#define C7   2093
#define CS7  2217
#define D7   2349
#define DS7  2489
#define E7   2637
#define F7   2794
#define FS7  2960
#define G7   3136
#define GS7  3322
#define A7   3520
#define AS7  3729
#define B7   3951
#define C8   4186
#define CS8  4435
#define D8   4699
#define DS8  4978
#define ZERO   0



const note_t star_wars[] = {
                            {a,   500}, {0,   20}, {a,   500}, {0,   20},
                            {a,   500}, {0,   20}, {f,   350}, {0,   20},
                            {cH,  150}, {0,   20}, {a,   500}, {0,   20},
                            {f,   350}, {0,   20}, {cH,  150}, {0,   20},
                            {a,  1000}, {0,   20},

                            //    {eH,  500}, {0,   20},
                            //    {eH,  500}, {0,   20}, {eH,  500}, {0,   20},
                            //    {fH,  350}, {0,   20}, {cH,  150}, {0,   20},
                            //    {gS,  500}, {0,   20}, {f,   350}, {0,   20},
                            //    {cH,  150}, {0,   20}, {a,  1000}, {0,   20},
                            //    {aH,  500}, {0,   20}, {a,   350}, {0,   20},
                            //    {a,   150}, {0,   20}, {aH,  500}, {0,   20},
                            //    {gSH,250}, {0,   20}, {gH, 250},  {0,   20},
                            //    {fSH,125}, {0,   20}, {fH, 125},  {0,   20},
                            //    {fSH,250}, {0,  250}, {aS, 250},  {0,   20},
                            //    {dSH,500}, {0,   20}, {dH, 250},  {0,   20},
                            //    {cSH,250}, {0,   20}, {cH,125},   {0,  250},
                            //    {b,  125}, {0,   20}, {cH, 250},  {0,  250},
                            //    {f,  250}, {0,   20}, {gS, 500},  {0,   20},
                            //    {f,  375}, {0,   20}, {a,  125},  {0,   20},
                            //    {cH, 500}, {0,   20}, {a,  375},  {0,   20},
                            //    {cH, 125}, {0,   20}, {eH,1000},  {0,   20},
                            //    {aH, 500}, {0,   20}, {a,  350},  {0,   20},
                            //    {a,  150}, {0,   20}, {aH, 500},  {0,   20},
                            //    {gSH,250}, {0,   20}, {gH, 250},  {0,   20},
                            //    {fSH,125}, {0,   20}, {fH, 125},  {0,   20},
                            //    {fSH,250}, {0,  250}, {aS, 250},  {0,   20},
                            //    {dSH,500}, {0,   20}, {dH, 250},  {0,   20},
                            //    {cSH,250}, {0,   20}, {cH,125},   {0,   20},
                            //    {b,  125}, {0,   20}, {cH, 250},  {0,  250},
                            //    {f,  250}, {0,   20}, {gS, 500},  {0,   20},
                            //    {f,  375}, {0,   20}, {cH,125},   {0,   20},
                            //    {a,  500}, {0,   20}, {f,  375},  {0,   20},
                            //    {c,  125}, {0,   20}, {a, 1000},

                            {0, 0}
};

const note_t mario_theme[] =
{
 {E7, 120}, {E7, 120}, {0, 120}, {E7, 120},
 {0, 120}, {C7, 120}, {E7, 120}, {0, 120},
 {G7, 120}, {0, 120}, {0, 120}, {0, 120},
 {G6, 120}, {0, 120}, {0, 120}, {0, 120},

 // {C7, 120}, {0, 120}, {0, 120}, {G6, 120},
 // {0, 120}, {0, 120}, {E6, 120}, {0, 120},
 // {0, 120}, {A6, 120}, {0, 120}, {B6, 120},
 // {0, 120}, {AS6, 120}, {A6, 120}, {0, 120},
 //
 // {G6, 90}, {E7, 90}, {G7, 90},
 // {A7, 120}, {0, 120}, {F7, 120}, {G7, 120},
 // {0, 120}, {E7, 120}, {0, 120}, {C7, 120},
 // {D7, 120}, {B6, 120}, {0, 120}, {0, 120},
 //
 // {C7, 120}, {0, 120}, {0, 120}, {G6, 120},
 // {0, 120}, {0, 120}, {E6, 120}, {0, 120},
 // {0, 120}, {A6, 120}, {0, 120}, {B6, 120},
 // {0, 120}, {AS6, 120}, {A6, 120}, {0, 120},
 //
 // {G6, 90}, {E7, 90}, {G7, 90},
 // {A7, 120}, {0, 120}, {F7, 120}, {G7, 120},
 // {0, 120}, {E7, 120}, {0, 120}, {C7, 120},
 // {D7, 120}, {B6, 120}, {0, 120}, {0, 120},

 {0, 0}
};

const note_t boot_ok[] =
{
 {C5, 100}, //{E5, 100}, {G5, 100}, {C6, 200},
 {0, 0}
};

const note_t gen_a_boot_ok[] =
{
 {C2, 100},
 {0, 0}
};

const note_t gen_b_boot_ok[] =
{
 {0, 200},{C3, 100},
 {0, 0}
};

const note_t gen_c_boot_ok[] =
{
 {0, 400},{C4, 100},
 {0, 0}
};

const note_t std_boot_ok[] =
{
 {0, 600},{C5, 100},
 {0, 0}
};


const note_t boot_fail[] =
{
 {C5, 150}, {G4, 150}, {E4, 300},
 {0, 0}
};



const note_t comm_start[] =
{
 {D5, 80}, {G5, 80},
 {0, 0}
};

const note_t comm_ok[] =
{
 {E5, 80}, {G5, 80}, {E6, 100},
 {0, 0}
};

const note_t comm_error[] =
{
 {G3, 80}, {0, 80}, {G3, 200},
 {0, 0}
};

const note_t warning_beep[] =
{
 {C6, 100}, {0, 50}, {C6, 100}, {0, 50}, {C6, 100},
 {0, 0}
};

const note_t event_beep[] =
{
 {A5, 50},
 {0, 0}
};

const note_t event_led_beep[] =
{
 {GS2, 15},
 {0, 0}
};

#if defined(CPU2)

/*static*/ buzzer_state_t buzzer;

// Initialize EPwm12 for controlling the buzzer
void init_epwm12_buzzer(void)
{

    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_EPWM12);

    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0;
    EDIS;

    EPwm12Regs.TBCTL.bit.CTRMODE = TB_FREEZE; // Count up and down mode
    EPwm12Regs.TBPRD = 100000;                      // Timer period
    EPwm12Regs.TBCTL.bit.PHSEN = TB_DISABLE;        // Disable phase loading
    EPwm12Regs.TBPHS.bit.TBPHS = 0x0000;            // Set phase to 0
    EPwm12Regs.TBCTR = 0x0000;                      // Clear counter
    EPwm12Regs.TBCTL.bit.HSPCLKDIV = TB_DIV4;       // Clock divider
    EPwm12Regs.TBCTL.bit.CLKDIV = TB_DIV4;
    EPwm12Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    EPwm12Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
    EPwm12Regs.AQCTLB.bit.ZRO = AQ_TOGGLE;          // Toggle PWM12A at zero

    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;
}

// Set the sound frequency for the buzzer
void _set_sound(int frequency)
{
    if (frequency < 25) {
        EPwm12Regs.TBCTL.bit.CTRMODE = TB_FREEZE; // Stop the counter
    } else {
        EPwm12Regs.TBPRD = (65535 * 23.84) / frequency; // Set the timer period
        EPwm12Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN; // Enable counting
    }
}

void buzzer_play_song(const note_t *song)
{
    if(buzzer.current_state == SOUND_IDLE)
    {
        buzzer.current_state = SOUND_SONG;
        buzzer.current_song = song;
        buzzer.song_active = true;
        buzzer.song_index = 0;
    }
}

// Initialize the buzzer state machine with the device type and setup sound/time patterns
void buzzer_init(uint8_t device)
{
    buzzer.current_state = SOUND_IDLE;
    buzzer.state_start_time = my_time(NULL);;
    buzzer.device = device;


    buzzer.queue_head = 0;
    buzzer.queue_tail = 0;
    buzzer.queue_count = 0;

    init_epwm12_buzzer();
}


void buzzer_enqueue(const note_t *song)
{
    if (buzzer.queue_count < BUZZER_QUEUE_SIZE) {
        buzzer.queue[buzzer.queue_tail] = song;
        buzzer.queue_tail = (buzzer.queue_tail + 1) % BUZZER_QUEUE_SIZE;
        buzzer.queue_count++;

        if (buzzer.current_state == SOUND_IDLE) {
            buzzer.current_song = buzzer.queue[buzzer.queue_head];
            buzzer.queue_head = (buzzer.queue_head + 1) % BUZZER_QUEUE_SIZE;
            buzzer.queue_count--;

            buzzer.current_state = SOUND_SONG;
            buzzer.song_active = true;
            buzzer.song_index = 0;
        }
    }
}


// Run the buzzer state machine based on the current time and state
void buzzer_state_machine(my_time_t time_actual)
{

    switch (buzzer.current_state)
    {
    case SOUND_IDLE:
        break;

    case SOUND_SONG:
        if (!buzzer.song_active || !buzzer.current_song)
            break;
        note_t note = buzzer.current_song[buzzer.song_index];
        if (note.time == 0 && note.freq == 0) {
            _set_sound(0);
            buzzer.song_active = false;
            buzzer.song_index = 0;

            if (buzzer.queue_count > 0) {
                buzzer.current_song = buzzer.queue[buzzer.queue_head];
                buzzer.queue_head = (buzzer.queue_head + 1) % BUZZER_QUEUE_SIZE;
                buzzer.queue_count--;

                buzzer.song_active = true;
                buzzer.song_index = 0;
                buzzer.current_state = SOUND_SONG;
            } else {
                buzzer.current_state = SOUND_IDLE;
            }

            break;
        }
        _set_sound(note.freq);
        buzzer.sound_time = note.time;
        buzzer.current_state = SOUND_SONG_DELAY;
        buzzer.state_start_time = time_actual;
        break;

    case SOUND_SONG_DELAY:
        if (time_actual - buzzer.state_start_time >= buzzer.sound_time)
        {
            buzzer.song_index++;
            buzzer.current_state = SOUND_SONG;
            buzzer.state_start_time = time_actual;
        }
        break;

    case SOUND_END:
    default:
        buzzer.current_state = SOUND_IDLE;
        break;
    }
}






#elif defined(CPU1)

#include "ipc_simple.h"

void buzzer_enqueue_pattern(uint8_t* pattern_id)
{
    (void)ipc_buzzer_play(pattern_id);
}


#else
#  error "Macro CPU1 or CPU2 undefined!"
#endif
