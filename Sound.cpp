#include "Sound.h"


Sound::Sound(std::string file_path) {
    SDL_LoadWAV(file_path.c_str(), &audio_spec, &wave_start, &wave_length);
    setup_device();
}

Sound::~Sound() {
    SDL_FreeWAV(wave_start);
    SDL_CloseAudioDevice(device);
}


void Sound::play_sound() {
    int status = SDL_QueueAudio(device, wave_start, wave_length);
    SDL_PauseAudioDevice(device, 0);
}

void Sound::stop_sound() {
    SDL_PauseAudioDevice(device, 1);
}

void Sound::setup_device() {

    device = SDL_OpenAudioDevice(SDL_GetAudioDeviceName(2, 0), 0, &audio_spec, nullptr, 0);
    if (0 == device) {
        std::cerr << "sound device error: " << SDL_GetError() << std::endl;
    }
}

Sound_Manager::Sound_Manager() {
    //capture = new Sound("./capture.wav");
    //castle = new Sound("./castle.wav");
    //check = new Sound("./check.wav");
    //checkmate = new Sound("./checkmate.wav");
    move = new Sound("./move.wav");
}