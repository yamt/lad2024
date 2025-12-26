/*
 * the wasm4 font has some unmapped codepoints.
 * cf. https://wasm4.org/docs/guides/text
 *
 * we use some of them (0x90-0x99) for our special symbols.
 * see draw_message().
 */

#define C_W "\x91"
#define C_B "\x92"
#define C_X "\x93"

#define C_R "\x96"

#define C_P "\x98"
#define C_A "\x99"

/*
 * non-ascii characters
 */
#define C_COPYRIGHT "\xa9"

struct stage {
        const uint8_t *data;
        const char *message;
};

extern const struct stage stages[];
