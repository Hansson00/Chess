#include "Sound.h"

Sound_Manager::Sound_Manager() {

    int audio_rate = 22050;
    Uint16 audio_format = AUDIO_S16SYS;
    int audio_channels = 2;
    int audio_buffers = 4096;

    if(Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers) != 0){
        std::cout << "Sound init error: " << SDL_GetError() << std::endl;
    }

   capture = load_sound("./audio_files/capture.wav");
   castle = load_sound("./audio_files/castle.wav");
   check = load_sound("./audio_files/check.wav");
   checkmate = load_sound("./audio_files/checkmate.wav");
   move = load_sound("./audio_files/move.wav");
}

Sound_Manager::~Sound_Manager() {
}

void Sound_Manager::play_sound(Mix_Chunk* mc) {

    Mix_PlayChannel(-1, mc, 0);

}

Mix_Chunk* Sound_Manager::load_sound(const char* file_path) {

    Mix_Chunk* mc = Mix_LoadWAV(file_path);
    if (mc == nullptr) {
        std::cout << "Wav init error: " << SDL_GetError() << std::endl;
    }
    return mc;
}


