#pragma once
#include "SDL_audio.h"
#include "SDL.h"
#include "iostream"

class ISound {
public:
	virtual ~ISound() {};

	virtual void play_sound() = 0;
	virtual void stop_sound() = 0;

};

class Sound : public ISound{

public:
	Sound(std::string file_path);
	~Sound();

	void play_sound();
	void stop_sound();
	void setup_device();
private:
	SDL_AudioDeviceID device;
	SDL_AudioSpec audio_spec;
	uint8_t* wave_start;
	uint32_t wave_length;
};

class Sound_Manager {
public:
	Sound_Manager();
	//Sound* capture;
	//Sound* castle;
	//Sound* check;
	//Sound* checkmate;
	Sound* move;
};

