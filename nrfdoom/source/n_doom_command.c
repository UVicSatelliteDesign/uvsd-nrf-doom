#include "n_doom_command.h"

dispatch_doom_command_result_t dispatch_doom_command(const uint8_t *command, size_t command_size) {
    /* Validate command pointer and minimum size */
    if (command == NULL || command_size < 1) {
        return DOOM_COMMAND_INVALID;
    }

    /* Extract command byte */
    uint8_t command_byte = command[0];

    /* Body excludes the command byte */
    const uint8_t *body = &command[1];
    size_t body_size = command_size - 1;

    /* Validate maximum body size */
    if (body_size > DOOM_COMMAND_MAX_BODY_SIZE) {
        return DOOM_COMMAND_INVALID;
    }

    switch (command_byte) {
        /* 0x02: DOOM Command - Keyboard */
        case DOOM_COMMAND_KEYBOARD: {
            
        }
        
        /* 0x03: DOOM Command - Gamepad */
        case DOOM_COMMAND_GAMEPAD: {
            /* Currently Ignored*/
        }
        
        /* 0x04: DOOM Command - EOF */
        case DOOM_COMMAND_EOF: {

        }
        
        /* 0x07: DOOM Command - PLAY */
        case DOOM_COMMAND_PLAY: {

        }
        
        /* 0x08: DOOM Command - RESET BUFFER */
        case DOOM_COMMAND_RESET_BUFFER: {

        }
        
        /* 0x09: DOOM Command - RESET GAME */
        case DOOM_COMMAND_RESET_GAME: {

        }
    
    }
}