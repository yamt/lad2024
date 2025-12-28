struct score;

/*
 * change the score to play.
 *
 * if another score is being played right now, fade it out first
 * (which takes 100 frames) and then start the new score.
 *
 * specify NULL to stop playing.
 */
void music_change(const struct score *score);

/*
 * call this on each frames. ie. update()
 */
void music_update(void);
