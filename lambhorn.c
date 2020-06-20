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
        }

        if (SDL_SetPaletteColors(surf->format->palette, colors, 0, ncolors) < 0) {
                const char *error = SDL_GetError();
                SDL_FreeSurface(surf);
                print_error("SDL_SetPaletteColors: %s\n", error);
        }

        tex = SDL_CreateTextureFromSurface(renderer, surf);
        if (!tex) {
                const char *error = SDL_GetError();
                SDL_FreeSurface(surf);
                print_error("SDL_CreateTextureFromSurface: %s\n", error);
        }

        SDL_FreeSurface(surf);

        return tex;
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
                SDL_RenderCopy(_renderer, _cursor_tex, NULL, NULL);
                SDL_RenderPresent(_renderer);

                /* Delay until the next frame. */
                SDL_Delay(40);
        }

        /* The program never reaches here. */
        return 0;
}
