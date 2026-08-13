#ifndef VRAM_DECODER_H
#define VRAM_DECODER_H

#include <cstdint>
#include <vector>

// Converte un colore BGR555 GBA a 15-bit in formato RGBA8888 a 32-bit per Android
inline uint32_t bgr555_to_rgba8888(uint16_t color15) {
    uint8_t r = (color15 & 0x001F) << 3;
    uint8_t g = ((color15 >> 5) & 0x001F) << 3;
    uint8_t b = ((color15 >> 10) & 0x001F) << 3;
    return (0xFF << 24) | (b << 16) | (g << 8) | r;
}

// Renderizza una Tile 8x8 dal buffer grafico della ROM
void decode_gba_tile(const uint8_t* tile_data, uint32_t* framebuffer, int screen_x, int screen_y, const uint16_t* palette);

#endif
