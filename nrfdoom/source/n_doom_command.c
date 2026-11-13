#include "n_doom_command.h"

dispatch_doom_command_result_t dispatch_doom_command(const uint8_t *doom_command, size_t doom_command_size) {
    /* Validate command pointer and minimum size */
    if (doom_command == NULL || doom_command_size < DOOM_COMMAND_HEADER_SIZE) {
        return DOOM_COMMAND_INVALID;
    }

    /* Extract command byte */
    uint8_t command_byte = doom_command[0];

    /* Body excludes the command byte */
    const uint8_t *body = &doom_command[1];
    size_t body_size = doom_command_size - DOOM_COMMAND_HEADER_SIZE;

    /* Validate maximum body size */
    if (body_size > DOOM_COMMAND_MAX_BODY_SIZE) {
        return DOOM_COMMAND_INVALID;
    }

    switch (command_byte) {
        /* 0x02: DOOM Command - Keyboard */
        case DOOM_COMMAND_KEYBOARD: {
            if (body_size < DOOM_KEYBOARD_HEADER_SIZE) {
                return DOOM_COMMAND_INVALID;
            }
            
            /* Extract keyboard command parameters */
            uint8_t batch = body[0];
            uint8_t sequence = body[1];
            uint8_t sequence_count = body[2];
            uint8_t size = body[3];
            const uint8_t *keypresses = &body[4];
            
            /* Validate keyboard command parameters */
            if(size > DOOM_KEYBOARD_MAX_ENTRIES) {
                return DOOM_COMMAND_INVALID;
            }

            if (!handle_keyboard_doom(batch, sequence, sequence_count, size, keypresses)) {
                return DOOM_COMMAND_HANDLER_FAILED;
            }
            return DOOM_COMMAND_SUCCESS;
        }

        /* 0x03: DOOM Command - Gamepad */
        case DOOM_COMMAND_GAMEPAD: {
            if (body_size < DOOM_GAMEPAD_HEADER_SIZE) {
                return DOOM_COMMAND_INVALID;
            }
            
            /* Extract gamepad command parameters */
            uint8_t batch = body[0];
            uint8_t sequence = body[1];
            uint8_t sequence_count = body[2];
            uint8_t size = body[3];
            const uint8_t *entries = &body[4];

            /* Validate gamepad command parameters */
            if(size > DOOM_GAMEPAD_MAX_ENTRIES) {
                return DOOM_COMMAND_INVALID;
            }

            if (!handle_gamepad_doom(batch, sequence, sequence_count, size, entries)) {
                return DOOM_COMMAND_HANDLER_FAILED;
            }
            return DOOM_COMMAND_SUCCESS;
        }
        
        /* 0x04: DOOM Command - EOF */
        case DOOM_COMMAND_EOF: {
            if (body_size != DOOM_COMMAND_EOF_BODY_SIZE) {
                return DOOM_COMMAND_INVALID;
            }
            
            uint8_t batch = body[0];

            if (!handle_eof_doom(batch)) {
                return DOOM_COMMAND_HANDLER_FAILED;
            }
            return DOOM_COMMAND_SUCCESS;
        }
        
        /* 0x07: DOOM Command - PLAY */
        case DOOM_COMMAND_PLAY: {
            if (body_size != DOOM_COMMAND_PLAY_BODY_SIZE) {
                return DOOM_COMMAND_INVALID;
            }

            uint8_t batch = body[0];

            if (!handle_play_doom(batch)) {
                return DOOM_COMMAND_HANDLER_FAILED;
            }
            return DOOM_COMMAND_SUCCESS;
        }
        
        /* 0x08: DOOM Command - RESET BUFFER */
        case DOOM_COMMAND_RESET_BUFFER: {
            if (body_size != DOOM_COMMAND_RESET_BUFFER_BODY_SIZE) {
                return DOOM_COMMAND_INVALID;
            }

            uint8_t batch = body[0];

            if (!reset_doom_buffer(batch)) {
                return DOOM_COMMAND_HANDLER_FAILED;
            }
            return DOOM_COMMAND_SUCCESS;
        }
        
        /* 0x09: DOOM Command - RESET GAME */
        case DOOM_COMMAND_RESET_GAME: {
            if (body_size != DOOM_COMMAND_RESET_GAME_BODY_SIZE) {
                return DOOM_COMMAND_INVALID;
            }

            if (!reset_doom_game()) {
                return DOOM_COMMAND_HANDLER_FAILED;
            }
            return DOOM_COMMAND_SUCCESS;
        }
        default:
            return DOOM_COMMAND_INVALID;

    }
}

bool handle_keyboard_doom(uint8_t batch_num, uint8_t sequence_num, uint8_t sequence_count, uint8_t size, const uint8_t *keypresses) {
    /* TODO: Implement keyboard command handling */
    return true;
}

bool handle_gamepad_doom(uint8_t batch_num, uint8_t sequence_num, uint8_t sequence_count, uint8_t size, const uint8_t *entries) {
    /* Stub Function - Gamepad Inputs Currently Ignored */
    return true;
}

bool handle_eof_doom(uint8_t batch_num) {
    /* TODO: Implement EOF command handling */
    return true;
}

bool handle_play_doom(uint8_t batch_num) {
    /* TODO: Implement PLAY command handling */
    return true;
}

bool reset_doom_buffer(uint8_t batch_num) {
    /* TODO: Implement reset buffer command handling */
    return true;
}

bool reset_doom_game() {
    /* TODO: Implement reset game command handling */
    return true;
}