#include "input.h"
#include "wasm4.h"

void
read_gamepad(uint8_t *curp, uint8_t *pushedp, uint8_t *repeatedp)
{
        static uint8_t prev_gamepad = 0;
        static uint8_t holding_frames;

        uint8_t cur = *GAMEPAD1;
        uint8_t gamepad = cur;
        uint8_t gamepad_with_repeat;
        gamepad &= ~prev_gamepad;
        gamepad_with_repeat = gamepad;
        if (cur == prev_gamepad) {
                if (holding_frames < 40) {
                        holding_frames++;
                } else {
                        gamepad_with_repeat |= cur;
                }
        } else {
                holding_frames = 0;
        }
        prev_gamepad = cur;

        *curp = cur;
        *pushedp = gamepad;
        *repeatedp = gamepad_with_repeat;
}

void
read_mouse_buttons(uint8_t *curp, uint8_t *pushedp)
{
        static uint8_t prev = 0;

        uint8_t cur = *MOUSE_BUTTONS;
        uint8_t pushed = cur;
        pushed &= ~prev;
        prev = cur;

        *curp = cur;
        *pushedp = pushed;
}
