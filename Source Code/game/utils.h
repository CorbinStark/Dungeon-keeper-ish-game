#ifndef UTILS_H
#define UTILS_H

#include "bahamut.h"
#include <random>

const i32 ANIM_INTERVAL = 50;

static inline
i32 random_int(i32 min, i32 max) {
    static std::random_device rd;
    static std::mt19937 mt(rd());
    std::uniform_int_distribution<> dist(0, max-min);
    return dist(mt) + min;
}

//
//   GUI
//

struct BitmapFont {
    Texture chars[SCHAR_MAX];
};

static inline
Texture get_sub_image(unsigned char* pixels, int pixels_width, int x, int y, int width, int height, int texparam) {
    Texture subimage;
    glPixelStorei(GL_UNPACK_ROW_LENGTH, pixels_width);
    unsigned char* subimage_pixels = pixels + (x + y * pixels_width) * 4;
    subimage = load_texture(subimage_pixels, width, height, texparam);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    return subimage;
}

static inline
BitmapFont load_neighbors_font(u8 scale = 1) {
    BitmapFont font = { 0 };
    i32 h;
    i32 w;
    unsigned char* image = SOIL_load_image("data/art/good_neighbors.png", &w, &h, 0, SOIL_LOAD_RGBA);
    font.chars['!'] = get_sub_image(image, w, 1, 0, 6, h, GL_NEAREST);
    font.chars['"'] = get_sub_image(image, w, 8, 0, 7, h, GL_NEAREST);
    font.chars['#'] = get_sub_image(image, w, 16, 0, 10, h, GL_NEAREST);
    font.chars['$'] = get_sub_image(image, w, 27, 0, 10, h, GL_NEAREST);
    font.chars['%'] = get_sub_image(image, w, 38, 0, 11, h, GL_NEAREST);
    font.chars['&'] = get_sub_image(image, w, 50, 0, 11, h, GL_NEAREST);
    font.chars['\''] = get_sub_image(image, w, 62, 0, 4, h, GL_NEAREST);
    font.chars['('] = get_sub_image(image, w, 67, 0, 6, h, GL_NEAREST);
    font.chars[')'] = get_sub_image(image, w, 74, 0, 6, h, GL_NEAREST);
    font.chars['*'] = get_sub_image(image, w, 81, 0, 10, h, GL_NEAREST);
    font.chars['+'] = get_sub_image(image, w, 92, 0, 8, h, GL_NEAREST);
    font.chars[','] = get_sub_image(image, w, 101, 0, 4, h, GL_NEAREST);
    font.chars['-'] = get_sub_image(image, w, 106, 0, 9, h, GL_NEAREST);
    font.chars['.'] = get_sub_image(image, w, 116, 0, 4, h, GL_NEAREST);
    font.chars['/'] = get_sub_image(image, w, 121, 0, 8, h, GL_NEAREST);
    font.chars['0'] = get_sub_image(image, w, 130, 0, 8, h, GL_NEAREST);
    font.chars['1'] = get_sub_image(image, w, 139, 0, 6, h, GL_NEAREST);
    font.chars['2'] = get_sub_image(image, w, 146, 0, 8, h, GL_NEAREST);
    font.chars['3'] = get_sub_image(image, w, 155, 0, 8, h, GL_NEAREST);
    font.chars['4'] = get_sub_image(image, w, 164, 0, 9, h, GL_NEAREST);
    font.chars['5'] = get_sub_image(image, w, 174, 0, 8, h, GL_NEAREST);
    font.chars['6'] = get_sub_image(image, w, 183, 0, 8, h, GL_NEAREST);
    font.chars['7'] = get_sub_image(image, w, 192, 0, 8, h, GL_NEAREST);
    font.chars['8'] = get_sub_image(image, w, 201, 0, 8, h, GL_NEAREST);
    font.chars['9'] = get_sub_image(image, w, 210, 0, 8, h, GL_NEAREST);
    font.chars[':'] = get_sub_image(image, w, 219, 0, 4, h, GL_NEAREST);
    font.chars[';'] = get_sub_image(image, w, 224, 0, 4, h, GL_NEAREST);
    font.chars['<'] = get_sub_image(image, w, 229, 0, 9, h, GL_NEAREST);
    font.chars['='] = get_sub_image(image, w, 239, 0, 7, h, GL_NEAREST);
    font.chars['>'] = get_sub_image(image, w, 247, 0, 9, h, GL_NEAREST);
    font.chars['?'] = get_sub_image(image, w, 257, 0, 8, h, GL_NEAREST);
    font.chars['@'] = get_sub_image(image, w, 266, 0, 10, h, GL_NEAREST);
    font.chars['A'] = get_sub_image(image, w, 277, 0, 8, h, GL_NEAREST);
    font.chars['B'] = get_sub_image(image, w, 286, 0, 8, h, GL_NEAREST);
    font.chars['C'] = get_sub_image(image, w, 295, 0, 8, h, GL_NEAREST);
    font.chars['D'] = get_sub_image(image, w, 304, 0, 9, h, GL_NEAREST);
    font.chars['E'] = get_sub_image(image, w, 314, 0, 8, h, GL_NEAREST);
    font.chars['F'] = get_sub_image(image, w, 323, 0, 8, h, GL_NEAREST);
    font.chars['G'] = get_sub_image(image, w, 332, 0, 8, h, GL_NEAREST);
    font.chars['H'] = get_sub_image(image, w, 341, 0, 8, h, GL_NEAREST);
    font.chars['I'] = get_sub_image(image, w, 350, 0, 6, h, GL_NEAREST);
    font.chars['J'] = get_sub_image(image, w, 357, 0, 9, h, GL_NEAREST);
    font.chars['K'] = get_sub_image(image, w, 367, 0, 8, h, GL_NEAREST);
    font.chars['L'] = get_sub_image(image, w, 376, 0, 8, h, GL_NEAREST);
    font.chars['M'] = get_sub_image(image, w, 385, 0, 10, h, GL_NEAREST);
    font.chars['N'] = get_sub_image(image, w, 396, 0, 9, h, GL_NEAREST);
    font.chars['O'] = get_sub_image(image, w, 406, 0, 8, h, GL_NEAREST);
    font.chars['P'] = get_sub_image(image, w, 415, 0, 8, h, GL_NEAREST);
    font.chars['Q'] = get_sub_image(image, w, 424, 0, 9, h, GL_NEAREST);
    font.chars['R'] = get_sub_image(image, w, 434, 0, 9, h, GL_NEAREST);
    font.chars['S'] = get_sub_image(image, w, 444, 0, 8, h, GL_NEAREST);
    font.chars['T'] = get_sub_image(image, w, 453, 0, 8, h, GL_NEAREST);
    font.chars['U'] = get_sub_image(image, w, 462, 0, 8, h, GL_NEAREST);
    font.chars['V'] = get_sub_image(image, w, 471, 0, 8, h, GL_NEAREST);
    font.chars['W'] = get_sub_image(image, w, 480, 0, 10, h, GL_NEAREST);
    font.chars['X'] = get_sub_image(image, w, 491, 0, 9, h, GL_NEAREST);
    font.chars['Y'] = get_sub_image(image, w, 501, 0, 8, h, GL_NEAREST);
    font.chars['Z'] = get_sub_image(image, w, 510, 0, 8, h, GL_NEAREST);
    font.chars['['] = get_sub_image(image, w, 519, 0, 6, h, GL_NEAREST);
    font.chars['\\'] = get_sub_image(image, w, 526, 0, 8, h, GL_NEAREST);
    font.chars[']'] = get_sub_image(image, w, 535, 0, 6, h, GL_NEAREST);
    font.chars['^'] = get_sub_image(image, w, 542, 0, 11, h, GL_NEAREST);
    font.chars['_'] = get_sub_image(image, w, 554, 0, 8, h, GL_NEAREST);
    font.chars['`'] = get_sub_image(image, w, 563, 0, 6, h, GL_NEAREST);
    font.chars['a'] = get_sub_image(image, w, 570, 0, 8, h, GL_NEAREST);
    font.chars['b'] = get_sub_image(image, w, 579, 0, 8, h, GL_NEAREST);
    font.chars['c'] = get_sub_image(image, w, 588, 0, 8, h, GL_NEAREST);
    font.chars['d'] = get_sub_image(image, w, 597, 0, 8, h, GL_NEAREST);
    font.chars['e'] = get_sub_image(image, w, 606, 0, 8, h, GL_NEAREST);
    font.chars['f'] = get_sub_image(image, w, 615, 0, 7, h, GL_NEAREST);
    font.chars['g'] = get_sub_image(image, w, 623, 0, 8, h, GL_NEAREST);
    font.chars['h'] = get_sub_image(image, w, 632, 0, 8, h, GL_NEAREST);
    font.chars['i'] = get_sub_image(image, w, 641, 0, 6, h, GL_NEAREST);
    font.chars['j'] = get_sub_image(image, w, 648, 0, 6, h, GL_NEAREST);
    font.chars['k'] = get_sub_image(image, w, 655, 0, 8, h, GL_NEAREST);
    font.chars['l'] = get_sub_image(image, w, 664, 0, 5, h, GL_NEAREST);
    font.chars['m'] = get_sub_image(image, w, 670, 0, 10, h, GL_NEAREST);
    font.chars['n'] = get_sub_image(image, w, 681, 0, 8, h, GL_NEAREST);
    font.chars['o'] = get_sub_image(image, w, 690, 0, 8, h, GL_NEAREST);
    font.chars['p'] = get_sub_image(image, w, 699, 0, 8, h, GL_NEAREST);
    font.chars['q'] = get_sub_image(image, w, 708, 0, 9, h, GL_NEAREST);
    font.chars['r'] = get_sub_image(image, w, 718, 0, 8, h, GL_NEAREST);
    font.chars['s'] = get_sub_image(image, w, 727, 0, 8, h, GL_NEAREST);
    font.chars['t'] = get_sub_image(image, w, 736, 0, 8, h, GL_NEAREST);
    font.chars['u'] = get_sub_image(image, w, 745, 0, 8, h, GL_NEAREST);
    font.chars['v'] = get_sub_image(image, w, 754, 0, 8, h, GL_NEAREST);
    font.chars['w'] = get_sub_image(image, w, 763, 0, 10, h, GL_NEAREST);
    font.chars['x'] = get_sub_image(image, w, 774, 0, 8, h, GL_NEAREST);    
    font.chars['y'] = get_sub_image(image, w, 783, 0, 8, h, GL_NEAREST);
    font.chars['z'] = get_sub_image(image, w, 792, 0, 8, h, GL_NEAREST);
    font.chars['{'] = get_sub_image(image, w, 801, 0, 7, h, GL_NEAREST);
    font.chars['|'] = get_sub_image(image, w, 809, 0, 4, h, GL_NEAREST);
    font.chars['}'] = get_sub_image(image, w, 814, 0, 7, h, GL_NEAREST);
    font.chars['~'] = get_sub_image(image, w, 822, 0, 9, h, GL_NEAREST);
    font.chars[' '] = get_sub_image(image, w, 831, 0, 6, h, GL_NEAREST);
    font.chars[1] = get_sub_image(image, w, 831, 0, 1, h, GL_NEAREST);

    for (u8 i = 0; i < SCHAR_MAX; ++i) {
        font.chars[i].width *= scale;
        font.chars[i].height *= scale;
    }
    free(image);

    return font;
}

static inline
void draw_text(RenderBatch* batch, BitmapFont* font, const char* str, f32 x, f32 y, f32 r=255, f32 g=255, f32 b=255, f32 a=255) {
    i32 len = strlen(str);
    for (u16 i = 0; i < len; ++i) {
        draw_texture(batch, font->chars[str[i]], x, y, { r/255,g/255,b/255,a/255 });
        x += font->chars[str[i]].width - font->chars[1].width;
    }
}

static inline
i32 get_string_width(BitmapFont* font, const char* str) {
    i32 width = 0;
    i32 len = strlen(str);
    for (u16 i = 0; i < len; ++i) {
        width += font->chars[str[i]].width - font->chars[1].width;
    }
    return width;
}

static inline
bool button(RenderBatch* batch, Texture btn, Texture btnDown, i32 x, i32 y, vec2 mouse) {
    Rect button = { x, y, btn.width, btn.height };
    bool collided = colliding(button, mouse.x, mouse.y);
    bool buttonReleased = is_button_released(MOUSE_BUTTON_LEFT);

    if (collided && is_button_down(MOUSE_BUTTON_LEFT)) {
        draw_texture(batch, btnDown, x, y + 5);
    }
    else {
        draw_texture(batch, btn, x, y);
    }

    return collided & buttonReleased;
}

#endif
