#include <stdint.h>

enum sprite_idx {
        SPIDX_NONE = -1,
        SPIDX_PERSON = 0,
        SPIDX_PERSON1,
        SPIDX_PERSON2,
        SPIDX_ROBOT,
        SPIDX_ROBOT1,
        SPIDX_ROBOT2,
        SPIDX_BOX,
        SPIDX_BOMB,
        SPIDX_BOMB1,
        SPIDX_BOMB2,
        SPIDX_LIGHT,
        SPIDX_WALL,
        SPIDX_END
};

struct obj {
        enum sprite_idx sprite;
        uint8_t color;
        uint8_t flags;
};

extern const struct obj objs[];
extern const uint8_t *sprites[];

void prepare_scaled_sprites(void);
