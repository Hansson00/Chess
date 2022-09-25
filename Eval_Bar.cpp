#include "Eval_Bar.h"

Eval_Bar::Eval_Bar(uint16_t x, uint16_t height) {
	
	text = new Text(18);
	this->height = height;
	bar_rect = { x, 0, width, height };
	w_rect = { 0, 0, width, 0 };
	b_rect = { 0, 0, width, 0 };

	m_eval_surface = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
}

Eval_Bar::~Eval_Bar(){
	delete(m_eval_surface);
	delete(text);
}

SDL_Surface* Eval_Bar::create_eval_surface(double_t score) {

	double_t abs_score= abs(score);
	abs_score = abs_score > 15 ? 15.0 : abs_score; // save guard

	const uint16_t whole_number = (int)abs_score;
	std::string score_ = std::to_string(whole_number);

	if (abs_score < 10) {
		score_.append(".");
		score_.append(std::to_string((int)((abs_score - whole_number) * 10)));
	}
	w_rect.y = score < 0 ? (height >> 1) + abs_score * 30 :(height >> 1) - abs_score * 30;

	w_rect.h = height - w_rect.y;
	b_rect.h = height - w_rect.h;
	SDL_FillRect(m_eval_surface, &w_rect, 0xFFFFFF); // white
	SDL_FillRect(m_eval_surface, &b_rect, 0x3F3E39); // black

	if (score < 0) { // black
		text->draw_text(score_, m_eval_surface, 1, 0, 255, 255, 255);
	}
	else { // white 
		text->draw_text(score_, m_eval_surface, 1, bar_rect.h-22, 63, 62, 57);
	}
	return m_eval_surface;
}

