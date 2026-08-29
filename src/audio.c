#include "audio.h"
#include <SDL2/SDL.h>

static int audioCounter = 0;
static float last_note = -1;
static SDL_AudioDeviceID audioDev;

void initAudio(){
    SDL_AudioSpec want, have;
    want.freq = 8000;
    want.format = AUDIO_F32;
    want.channels = 1;
    want.samples = 512;
    want.callback = audioCallback;
    want.userdata = NULL;
    audioDev = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
    
}

void audioCallback(void *userdata, uint8_t *stream, int len) {
    (void)userdata;
    float *out = (float*) stream;
    int num_samples = len / sizeof(float);
    for (int i = 0 ; i < num_samples ; i++){
        audioCounter++;
        if(audioCounter % 20 == 0){
            last_note = -last_note;
        }
        out[i] = last_note;
    }
}

void changeAudioStatus(bool new_status){
    SDL_PauseAudioDevice(audioDev, new_status);
}

void killAudio(){
    SDL_CloseAudioDevice(audioDev);
}
