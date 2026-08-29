#ifndef AUDIO_H
#define AUDIO_H
#include <stdint.h>
#include <stdbool.h>

void initAudio();

void audioCallback(void *userdata, uint8_t *stream, int len);

void changeAudioStatus(bool new_status);

void killAudio();

#endif
