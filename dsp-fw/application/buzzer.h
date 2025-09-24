#ifndef BUZZER_H
#define BUZZER_H

#include <stdint.h>
#include "types.h"
#include "my_time.h"

#define BUZZER_QUEUE_SIZE 10

typedef struct {
    int freq;    //
    int time;    //  ms ou us
} note_t;

extern const note_t star_wars[];
extern const note_t mario_theme[];

extern const note_t boot_ok[];
extern const note_t gen_a_boot_ok[];
extern const note_t gen_b_boot_ok[];
extern const note_t gen_c_boot_ok[];
extern const note_t std_boot_ok[];
extern const note_t boot_fail[];
extern const note_t comm_start[];
extern const note_t comm_ok[];
extern const note_t comm_error[];
extern const note_t event_beep[];
extern const note_t warning_beep[];
extern const note_t event_led_beep[];

// Enumeration for the sound state machine
typedef enum {
    SOUND_IDLE,
    SOUND_SONG,
    SOUND_SONG_DELAY,
    SOUND_END
} sound_state_t;





// Structure for storing the buzzer state
typedef struct {
    sound_state_t current_state;
    my_time_t state_start_time;
    uint16_t sound_time;
    int16_t device;  // Device type (0 = STD, 1 = A, 2 = B, 3 = C)

    // Patterns for sounds and times for each event type
    const note_t *current_song;
    int16_t song_index;
    bool song_active;

    // Queue
    const note_t *queue[BUZZER_QUEUE_SIZE];
    int queue_head;
    int queue_tail;
    int queue_count;
} buzzer_state_t;





// Function declarations
void buzzer_init(uint8_t device);
void buzzer_enqueue(const note_t *song);
void buzzer_play(const note_t *song);
void buzzer_state_machine(my_time_t time_actual);


// Specific functions for PWM and sound
void init_epwm12_buzzer(void);
void _set_sound(int frequency);

#endif  // BUZZER_H
