#include "Text.h"

Text::Text(uint8_t size, uint8_t r, uint8_t g, uint8_t b) {
    if (TTF_Init() == -1) {
        std::cout << "Error with TTF_Init...\n";
        return;
    }

    m_font = TTF_OpenFont(font_path, size);
    if (m_font == nullptr) {
        std::cout << "Error with font...\n";
        return;
    }

   font_color_r = r;
   font_color_g = g;
   font_color_b = b;
}

Text::Text(const uint8_t size) {
    if (TTF_Init() == -1) {
        std::cout << "Error with TTF_Init...\n";
        return;
    }

    m_font = TTF_OpenFont(font_path, size);
    if (m_font == nullptr) {
        std::cout << "Error with font...\n";
        return;
    }
}

Text::~Text() {
    TTF_CloseFont(m_font);
}

void Text::draw_text(std::string text, SDL_Surface* surface, uint16_t x, uint16_t y) {
    m_draw_surface = TTF_RenderText_Solid(m_font, text.c_str(), { font_color_r,font_color_g,font_color_b });
    SDL_Rect rect = { x, y, 0, 0 };
    SDL_BlitSurface(m_draw_surface, NULL, surface, &rect);
    SDL_FreeSurface(m_draw_surface);
}

void Text::draw_text(std::string text, SDL_Surface* surface, uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b) {
    m_draw_surface = TTF_RenderText_Solid(m_font, text.c_str(), { r,g,b });
    SDL_Rect rect = { x, y, 0, 0 };
    SDL_BlitSurface(m_draw_surface, NULL, surface, &rect);
    SDL_FreeSurface(m_draw_surface);
}
