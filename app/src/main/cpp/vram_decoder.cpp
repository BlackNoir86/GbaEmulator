#include "vram_decoder.h"

void decode_gba_tile(const uint8_t* tile_data, uint32_t* framebuffer, int screen_x, int screen_y, const uint16_t* palette) {
    for (int py = 0; py < 8; py++) {
        for (int px = 0; px < 8; px++) {
            int out_x = screen_x + px;
            int out_y = screen_y + py;

            if (out_x >= 240 || out_y >= 160) continue;

            // In modalità 4-bit per pixel (16 colori per tile)
            int byte_idx = (py * 8 + px) / 2;
            uint8_t palette_idx = 0;
            if (px % 2 == 0) {
                palette_idx = tile_data[byte_idx] & 0x0F;
            } else {
                palette_idx = (tile_data[byte_idx] >> 4) & 0x0F;
            }

            if (palette_idx != 0) { // Indice 0 = Trasparente
                uint16_t color15 = palette[palette_idx];
                framebuffer[out_y * 240 + out_x] = bgr555_to_rgba8888(color15);
            }
        }
    }
}
