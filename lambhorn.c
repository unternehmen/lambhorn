/* lambhorn.c - a fantasy RPG
 * Copyright (c) 2020 Tofu Taco Co-op
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 * .
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 * .
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "SDL.h"

/* Mark a variable as being used. */
#define USED(x) ((void)(x))

/* The title of the game window */
#define WINDOW_TITLE "lambhorn"
/* The width of the game window in pixels */
#define WINDOW_WIDTH 640
/* The height of the game window in pixels */
#define WINDOW_HEIGHT 480

/* Images are 1-bit bitmaps which can be converted to SDL textures. */
struct image {
        int width;
        int height;
        char *pixels;
};

/* Fonts contain the data needed for drawing text */
struct font {
        int alphabet_len;
        const char *alphabet;
        struct image *images;
        SDL_Texture **textures;
        int height;
};

/* The main font's bitmaps */

static char _font_A_img_pixels[] = {
        0, 1, 1, 0,
        1, 0, 0, 1,
        1, 0, 0, 1,
        1, 1, 1, 1,
        1, 0, 0, 1,
        1, 0, 0, 1
};

static char _font_B_img_pixels[] = {
        1, 1, 1, 0,
        1, 0, 0, 1,
        1, 1, 1, 0,
        1, 0, 0, 1,
        1, 0, 0, 1,
        1, 1, 1, 0
};

static char _font_C_img_pixels[] = {
        0, 1, 1, 0,
        1, 0, 0, 1,
        1, 0, 0, 0,
        1, 0, 0, 0,
        1, 0, 0, 1,
        0, 1, 1, 0
};

static char _font_D_img_pixels[] = {
        1, 1, 1, 0,
        1, 0, 0, 1,
        1, 0, 0, 1,
        1, 0, 0, 1,
        1, 0, 0, 1,
        1, 1, 1, 0
};

static char _font_E_img_pixels[] = {
        1, 1, 1, 1,
        1, 0, 0, 0,
        1, 1, 1, 0,
        1, 0, 0, 0,
        1, 0, 0, 0,
        1, 1, 1, 1
};

static char _font_F_img_pixels[] = {
        1, 1, 1, 1,
        1, 0, 0, 0,
        1, 1, 1, 0,
        1, 0, 0, 0,
        1, 0, 0, 0,
        1, 0, 0, 0
};

static char _font_G_img_pixels[] = {
        0, 1, 1, 0,
        1, 0, 0, 1,
        1, 0, 0, 0,
        1, 0, 1, 1,
        1, 0, 0, 1,
        0, 1, 1, 0
};

static char _font_H_img_pixels[] = {
        1, 0, 0, 1,
        1, 0, 0, 1,
        1, 1, 1, 1,
        1, 0, 0, 1,
        1, 0, 0, 1,
        1, 0, 0, 1
};

static char _font_I_img_pixels[] = {
        1, 1, 1,
        0, 1, 0,
        0, 1, 0,
        0, 1, 0,
        0, 1, 0,
        1, 1, 1
};

static char _font_J_img_pixels[] = {
        1, 1, 1, 1,
        0, 0, 0, 1,
        0, 0, 0, 1,
        0, 0, 0, 1,
        1, 0, 0, 1,
        0, 1, 1, 0
};

static char _font_K_img_pixels[] = {
        1, 0, 0, 1,
        1, 0, 1, 0,
        1, 1, 0, 0,
        1, 1, 0, 0,
        1, 0, 1, 0,
        1, 0, 0, 1
};

static char _font_L_img_pixels[] = {
        1, 0, 0, 0,
        1, 0, 0, 0,
        1, 0, 0, 0,
        1, 0, 0, 0,
        1, 0, 0, 0,
        1, 1, 1, 1
};

static char _font_M_img_pixels[] = {
        1, 1, 0, 1, 0,
        1, 0, 1, 0, 1,
        1, 0, 0, 0, 1,
        1, 0, 0, 0, 1,
        1, 0, 0, 0, 1,
        1, 0, 0, 0, 1
};

static char _font_N_img_pixels[] = {
        1, 1, 0, 1,
        1, 0, 1, 1,
        1, 0, 0, 1,
        1, 0, 0, 1,
        1, 0, 0, 1,
        1, 0, 0, 1
};

static char _font_O_img_pixels[] = {
        0, 1, 1, 0,
        1, 0, 0, 1,
        1, 0, 0, 1,
        1, 0, 0, 1,
        1, 0, 0, 1,
        0, 1, 1, 0
};

static char _font_P_img_pixels[] = {
        1, 1, 1, 0,
        1, 0, 0, 1,
        1, 0, 0, 1,
        1, 1, 1, 0,
        1, 0, 0, 0,
        1, 0, 0, 0
};

static char _font_Q_img_pixels[] = {
        0, 1, 1, 0,
        1, 0, 0, 1,
        1, 0, 0, 1,
        1, 0, 0, 1,
        0, 1, 1, 0,
        0, 0, 0, 1
};

static char _font_R_img_pixels[] = {
        1, 1, 1, 0,
        1, 0, 0, 1,
        1, 1, 1, 0,
        1, 0, 0, 1,
        1, 0, 0, 1,
        1, 0, 0, 1
};

static char _font_S_img_pixels[] = {
        0, 1, 1, 0,
        1, 0, 0, 1,
        0, 1, 0, 0,
        0, 0, 1, 0,
        1, 0, 0, 1,
        0, 1, 1, 0
};

static char _font_T_img_pixels[] = {
        1, 1, 1,
        0, 1, 0,
        0, 1, 0,
        0, 1, 0,
        0, 1, 0,
        0, 1, 0,
};

static char _font_U_img_pixels[] = {
        1, 0, 0, 1,
        1, 0, 0, 1,
        1, 0, 0, 1,
        1, 0, 0, 1,
        1, 0, 0, 1,
        0, 1, 1, 0
};

static char _font_V_img_pixels[] = {
        1, 0, 0, 1,
        1, 0, 0, 1,
        1, 0, 0, 1,
        0, 1, 0, 1,
        0, 0, 1, 1,
        0, 0, 0, 1
};

static char _font_W_img_pixels[] = {
        1, 0, 0, 0, 1,
        1, 0, 0, 0, 1,
        1, 0, 0, 0, 1,
        1, 0, 0, 0, 1,
        1, 0, 1, 0, 1,
        0, 1, 0, 1, 0
};

static char _font_X_img_pixels[] = {
        1, 0, 0, 1,
        1, 0, 0, 1,
        0, 1, 1, 0,
        0, 1, 1, 0,
        1, 0, 0, 1,
        1, 0, 0, 1
};

static char _font_Y_img_pixels[] = {
        1, 0, 1,
        1, 0, 1,
        1, 0, 1,
        0, 1, 0,
        0, 1, 0,
        0, 1, 0
};

static char _font_Z_img_pixels[] = {
        1, 1, 1, 1,
        0, 0, 0, 1,
        0, 0, 1, 0,
        0, 1, 0, 0,
        1, 0, 0, 0,
        1, 1, 1, 1
};

static char _font_interrog_img_pixels[] = {
        1, 1, 0,
        0, 0, 1,
        0, 1, 0,
        0, 1, 0,
        0, 0, 0,
        0, 1, 0
};

static char _font_period_img_pixels[] = {
        0,
        0,
        0,
        0,
        0,
        1
};

static char _font_comma_img_pixels[] = {
        0, 0,
        0, 0,
        0, 0,
        0, 0,
        0, 1,
        1, 0,
};

static struct image _font_imgs[] = {
        {4, 6, _font_A_img_pixels},
        {4, 6, _font_B_img_pixels},
        {4, 6, _font_C_img_pixels},
        {4, 6, _font_D_img_pixels},
        {4, 6, _font_E_img_pixels},
        {4, 6, _font_F_img_pixels},
        {4, 6, _font_G_img_pixels},
        {4, 6, _font_H_img_pixels},
        {3, 6, _font_I_img_pixels},
        {4, 6, _font_J_img_pixels},
        {4, 6, _font_K_img_pixels},
        {4, 6, _font_L_img_pixels},
        {5, 6, _font_M_img_pixels},
        {4, 6, _font_N_img_pixels},
        {4, 6, _font_O_img_pixels},
        {4, 6, _font_P_img_pixels},
        {4, 6, _font_Q_img_pixels},
        {4, 6, _font_R_img_pixels},
        {4, 6, _font_S_img_pixels},
        {3, 6, _font_T_img_pixels},
        {4, 6, _font_U_img_pixels},
        {4, 6, _font_V_img_pixels},
        {5, 6, _font_W_img_pixels},
        {4, 6, _font_X_img_pixels},
        {3, 6, _font_Y_img_pixels},
        {4, 6, _font_Z_img_pixels},
        {3, 6, _font_interrog_img_pixels},
        {1, 6, _font_period_img_pixels},
        {2, 6, _font_comma_img_pixels}
};
static SDL_Texture *_font_texs[] = {
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
        NULL, NULL, NULL, NULL, NULL
};

/* The main game font */
static struct font _font = {
        29,                              /* alphabet_len */
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ?.,", /* alphabet */
        _font_imgs,                      /* images */
        _font_texs,                      /* textures */
        0                                /* height (computed at run-time) */
};

/* The image for the menu cursor */
static char _cursor_img_pixels[] = {
        0, 0, 0, 0, 1, 0, 0, 0,
        0, 0, 0, 0, 1, 1, 0, 0,
        1, 1, 1, 1, 1, 0, 1, 0,
        1, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 1,
        1, 1, 1, 1, 1, 0, 1, 0,
        0, 0, 0, 0, 1, 1, 0, 0,
        0, 0, 0, 0, 1, 0, 0, 0
};
static struct image _cursor_img = {8, 8, _cursor_img_pixels};

/* The cursor texture */
static SDL_Texture *_cursor_tex = NULL;

/* The game window */
static SDL_Window *_window = NULL;
/* The renderer for drawing all graphics */
static SDL_Renderer *_renderer = NULL;

/* Abort the program with an error message. */
void die(const char *format, ...) {
        va_list ap;

        va_start(ap, format);
        vfprintf(stderr, format, ap);
        va_end(ap);

        exit(EXIT_FAILURE);
}

/* Clean up any used memory. */
static void _clean_up(void) {
        {
                int i;
                for (i = 0; i < _font.alphabet_len; i++) {
                        if (_font_texs[i]) {
                                SDL_DestroyTexture(_font_texs[i]);
                        }
                }
        }

        if (_cursor_tex) {
                SDL_DestroyTexture(_cursor_tex);
        }

        if (_renderer) {
                SDL_DestroyRenderer(_renderer);
        }

        if (_window) {
                SDL_DestroyWindow(_window);
        }
}

/* Create a texture from an image.  print_error may abort. */
static SDL_Texture *_create_texture_from_image(SDL_Renderer *renderer,
                                               struct image *image,
                                               SDL_Color *colors,
                                               int ncolors,
                                               void (*print_error)
                                                 (const char*, ...))
{
        SDL_Surface *surf;
        SDL_Texture *tex;

        surf = SDL_CreateRGBSurfaceWithFormatFrom(
                   image->pixels,
                   image->width,
                   image->height,
                   8,
                   image->width * (sizeof *image->pixels),
                   SDL_PIXELFORMAT_INDEX8);
        if (!surf) {
                print_error("SDL_CreateRGBSurface: %s\n", SDL_GetError());
                return NULL;
        }

        if (SDL_SetPaletteColors(surf->format->palette, colors, 0, ncolors) < 0) {
                const char *error = SDL_GetError();
                SDL_FreeSurface(surf);
                print_error("SDL_SetPaletteColors: %s\n", error);
                return NULL;
        }

        tex = SDL_CreateTextureFromSurface(renderer, surf);
        if (!tex) {
                const char *error = SDL_GetError();
                SDL_FreeSurface(surf);
                print_error("SDL_CreateTextureFromSurface: %s\n", error);
                return NULL;
        }

        SDL_FreeSurface(surf);

        return tex;
}

/* Draw text in a certain font at a certain position. */
void draw_text(SDL_Renderer *renderer,
               struct font *font,
               int start_x,
               int start_y,
               const char *text)
{
        int i;
        int pen_x = start_x;
        int pen_y = start_y;
        
        for (i = 0; text[i] != '\0'; i++) {
                int j;

                if (text[i] == ' ') {
                        pen_x += 4;
                } else if (text[i] == '\n') {
                        pen_x = start_x;
                        pen_y += font->height + 1;
                } else {
                        for (j = 0; font->alphabet[j] != '\0'; j++) {
                                if (font->alphabet[j] == text[i]) {
                                        SDL_Rect rect;
                                        struct image *image;

                                        image = &font->images[j];

                                        rect.x = pen_x;
                                        rect.y = pen_y;
                                        rect.w = image->width;
                                        rect.h = image->height;

                                        SDL_RenderCopy(renderer, font->textures[j], NULL, &rect);
                                        pen_x += image->width + 1;
                                        break;
                                }
                        }
                }
        }
}

int main(int argc, char *argv[]) {
        SDL_Color colors[2];

        USED(argc);
        USED(argv);

        /* Initialize SDL. */
        if (SDL_Init(SDL_INIT_VIDEO) == -1) {
                die("SDL_Init: %s\n", SDL_GetError());
        }
        atexit(&SDL_Quit);

        /* Set up an exit handler to free any used memory. */
        atexit(&_clean_up);

        /* Open the window. */
        _window = SDL_CreateWindow(WINDOW_TITLE,
                                   SDL_WINDOWPOS_UNDEFINED,
                                   SDL_WINDOWPOS_UNDEFINED,
                                   WINDOW_WIDTH,
                                   WINDOW_HEIGHT,
                                   0);
        if (!_window) {
                die("SDL_CreateWindow: %s\n", SDL_GetError());
        }

        /* Create the renderer. */
        _renderer = SDL_CreateRenderer(_window, -1, 0);
        if (!_renderer) {
                die("SDL_CreateRenderer: %s\n", SDL_GetError());
        }

        /* Set up the master palette. */
        colors[0].r = 0xff;
        colors[0].g = 0xff;
        colors[0].b = 0xff;
        colors[0].a = 0xff;
        colors[1].r = 0x00;
        colors[1].g = 0x00;
        colors[1].b = 0x00;
        colors[1].a = 0xff;

        /* Create the cursor texture. */
        _cursor_tex = _create_texture_from_image(_renderer,
                                                 &_cursor_img,
                                                 colors,
                                                 2,
                                                 &die);

        /* Create the font textures. */
        {
                int i;
                for (i = 0; i < _font.alphabet_len; i++) {
                        struct image *image = &_font_imgs[i];

                        if (image->height > _font.height) {
                                _font.height = image->height;
                        }

                        _font_texs[i] = _create_texture_from_image(_renderer, image, colors, 2, &die);
                }
        }

        /* Loop until the game ends. */
        while (1) {
                SDL_Event event;

                /* Handle user input. */
                while (SDL_PollEvent(&event)) {
                        if (event.type == SDL_QUIT) {
                                exit(EXIT_SUCCESS);
                        } else if (event.type == SDL_KEYDOWN) {
                                switch (event.key.keysym.scancode) {
                                        case SDL_SCANCODE_ESCAPE:
                                                exit(EXIT_SUCCESS);
                                                break;
                                        default:
                                                break;
                                }
                        }
                }

                /* Draw the scene. */
                SDL_SetRenderDrawColor(_renderer,
                                       0xff, 0xff, 0xff,
                                       SDL_ALPHA_OPAQUE);
                SDL_RenderClear(_renderer);
                draw_text(_renderer, &_font, 5, 5, "THE QUICK, BROWN FOX\nJUMPS OVER THE LAZY DOG.");
                SDL_RenderPresent(_renderer);

                /* Delay until the next frame. */
                SDL_Delay(40);
        }

        /* The program never reaches here. */
        return 0;
}
