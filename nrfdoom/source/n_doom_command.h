#ifndef N_DOOM_COMMAND_H
#define N_DOOM_COMMAND_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


#define DOOM_COMMAND_MAX_BODY_SIZE 245
#define DOOM_COMMAND_HEADER_SIZE 1

/* Keyboard command body layout */
#define DOOM_KEYBOARD_HEADER_SIZE 4
#define DOOM_KEYBOARD_ENTRY_SIZE 4
#define DOOM_KEYBOARD_MAX_ENTRIES 60

/* Gamepad command body layout */
#define DOOM_GAMEPAD_HEADER_SIZE 4
#define DOOM_GAMEPAD_ENTRY_SIZE 5
#define DOOM_GAMEPAD_MAX_ENTRIES 48

#define DOOM_COMMAND_EOF_BODY_SIZE 1
#define DOOM_COMMAND_PLAY_BODY_SIZE 1
#define DOOM_COMMAND_RESET_BUFFER_BODY_SIZE 1
#define DOOM_COMMAND_RESET_GAME_BODY_SIZE 1

/* Command bytes used by the ground-station/OBC DOOM command protocol. */
typedef enum {
    DOOM_COMMAND_KEYBOARD = 0x02,
    DOOM_COMMAND_GAMEPAD = 0x03,
    DOOM_COMMAND_EOF = 0x04,
    DOOM_COMMAND_PLAY = 0x07,
    DOOM_COMMAND_RESET_BUFFER = 0x08,
    DOOM_COMMAND_RESET_GAME = 0x09
} doom_command_byte_t;

/* Result types for dispatching DOOM commands */
typedef enum {
    DOOM_COMMAND_SUCCESS = 0,
    DOOM_COMMAND_INVALID = 1,
    DOOM_COMMAND_HANDLER_FAILED = 2,
} dispatch_doom_command_result_t;

/* Doom Command handler functions */
bool handle_keyboard_doom(uint8_t batch_num, uint8_t sequence_num, uint8_t sequence_count, uint8_t size, const uint8_t *keypresses);

bool handle_gamepad_doom(uint8_t batch_num, uint8_t sequence_num, uint8_t sequence_count, uint8_t size, const uint8_t *entries);

bool handle_eof_doom(uint8_t batch_num);

bool handle_play_doom(uint8_t batch_num);

bool reset_doom_buffer(uint8_t batch_num);

bool reset_doom_game(uint8_t batch_num);

#endif