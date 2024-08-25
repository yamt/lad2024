enum block {
        _ = 0, /* floor */
        W,     /* wall */
        U,     /* light up */
        R,     /* light right */
        D,     /* light down */
        L,     /* light left */
        B,     /* bomb */
        P,     /* person */
        A,     /* robot */
        END,
};

struct stage {
	const uint8_t *data;
    const char *message;
};

extern const struct stage stages[];
extern const unsigned int nstages;
