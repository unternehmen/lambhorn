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
#include "SDL_image.h"

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
        const char *alphabet;
        SDL_Texture *texture;
        SDL_Rect *clips;
        int height;
};

/* The clips for each letter in the font (computed at run-time) */
static SDL_Rect _font_clips[] = {
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
};

/* The main game font */
static struct font _font = {
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ?., "
        "abcdefghijklmnopqrstuvwxyz!",    /* alphabet */
        NULL,                             /* texture (loaded at run-time) */
        _font_clips,                      /* clips */
        0                                 /* height (computed at run-time) */
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
        if (_font.texture) {
                SDL_DestroyTexture(_font.texture);
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

                if (text[i] == '\n') {
                        pen_x = start_x;
                        pen_y += font->height + 1;
                } else {
                        for (j = 0; font->alphabet[j] != '\0'; j++) {
                                if (font->alphabet[j] == text[i]) {
                                        SDL_Rect rect;
                                        SDL_Rect *clip;

                                        clip = &font->clips[j];

                                        rect.x = pen_x;
                                        rect.y = pen_y;
                                        rect.w = clip->w;
                                        rect.h = clip->h;

                                        SDL_RenderCopy(renderer, font->texture, clip, &rect);
                                        pen_x += clip->w + 1;
                                        break;
                                }
                        }
                }
        }
}

int main(int argc, char *argv[]) {
        struct {
                int num_options;
        } title_menu = {2},
          heritage_menu = {11},
          tradition_menu = {4},
          *current_menu = &title_menu;
        int selection = 0;
        SDL_Color colors[2];

        USED(argc);
        USED(argv);

        /* Initialize SDL. */
        if (SDL_Init(SDL_INIT_VIDEO) == -1) {
                die("SDL_Init: %s\n", SDL_GetError());
        }
        atexit(&SDL_Quit);

        /* Initialize the image loader. */
        if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != IMG_INIT_PNG) {
                die("IMG_Init: %s\n", IMG_GetError());
        }
        atexit(&IMG_Quit);

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

        /* Load the font texture. */
        {
                SDL_Surface *surf;
                SDL_Texture *tex;

                surf = IMG_Load("data/images/font.png");
                if (!surf) {
                        die("IMG_Load: %s\n", IMG_GetError());
                }

                tex = SDL_CreateTextureFromSurface(_renderer, surf);
                if (!tex) {
                        const char *error = SDL_GetError();
                        SDL_FreeSurface(surf);
                        die("SDL_CreateTextureFromSurface: %s\n", error);
                }

                /* Determine the clips for each letter. */
                {
                        int i;
                        int x;

                        x = 1;
                        for (i = 0; _font.alphabet[i] != '\0'; i++) {
                                int j;

                                _font.clips[i].x = x;
                                _font.clips[i].y = 0;

                                _font.clips[i].w = 0;
                                for (j = x; j < surf->w; j++) {
                                        char *pixels_bytes;
                                        int locked = 0;
                                        int is_divider = 0;

                                        if (SDL_MUSTLOCK(surf)) {
                                                SDL_LockSurface(surf);
                                                locked = 1;
                                        }

                                        pixels_bytes = surf->pixels;
                                        is_divider = (pixels_bytes[surf->pitch * (surf->h - 1) + j]
                                                      == pixels_bytes[surf->pitch * (surf->h - 1)]);

                                        if (locked) {
                                                SDL_UnlockSurface(surf);
                                        }

                                        if (is_divider) {
                                                _font.clips[i].w = j - x;
                                                x = j + 1;
                                                break;
                                        }
                                }

                                _font.clips[i].h = surf->h - 1;
                        }
                }

                /* Store the font's height. */
                _font.height = surf->h;
                SDL_FreeSurface(surf);

                /* Store the font's texture. */
                _font.texture = tex;
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
                                        case SDL_SCANCODE_DOWN:
                                                if (selection < current_menu->num_options - 1) {
                                                        selection++;
                                                }
                                                break;
                                        case SDL_SCANCODE_UP:
                                                if (selection > 0) {
                                                        selection--;
                                                }
                                                break;
                                        case SDL_SCANCODE_SPACE:
                                                if (current_menu == &title_menu) {
                                                        if (selection == 0) {
                                                                /* New Game */
                                                                current_menu = &heritage_menu;
                                                                selection = 0;
                                                        } else if (selection == 1) {
                                                                exit(EXIT_SUCCESS);
                                                        }
                                                } else if (current_menu == &heritage_menu) {
                                                        if (selection == 10) {
                                                                current_menu = &title_menu;
                                                                selection = 0;
                                                        } else {
                                                                current_menu = &tradition_menu;
                                                                selection = 0;
                                                        }
                                                } else if (current_menu == &tradition_menu) {
                                                        if (selection == 3) {
                                                                current_menu = &heritage_menu;
                                                                selection = 0;
                                                        }
                                                }
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
                if (current_menu == &title_menu) {
                        draw_text(_renderer, &_font, 9, 5, "Lambhorn\n\nNew Game\nQuit");
                } else if (current_menu == &heritage_menu) {
                        draw_text(_renderer, &_font, 9, 5, "Choose your heritage.\n\nAcolith\nCaprons\nDaimyo\nDelvren\nFelith\nFleurel\nGrimfol\nLos\nSunstruck\nVaawie\nCancel");
                } else if (current_menu == &tradition_menu) {
                        draw_text(_renderer, &_font, 9, 5, "Choose your tradition.\n\nBirane\nScevimric\nVeronis\nCancel");
                }

                {
                        SDL_Rect rect;

                        rect.x = 0;
                        rect.y = 5 - ((8 - _font.height) / 2) + ((_font.height + 1) * (2 + selection));
                        rect.w = 8;
                        rect.h = 8;

                        SDL_RenderCopy(_renderer, _cursor_tex, NULL, &rect);
                }

                /* Show heritage descriptions in the heritage menu. */
                if (current_menu == &heritage_menu) {
                        const char *heritage_descs[] = {
                                "Acolith are pointy eared immortals.",
                                "Caprons are goat headed folk from Im Otra.",
                                "Daimyo are ogre demons of mysterious origin.",
                                "Delvren are mole headed undergrounders.",
                                "Felith are mortal Acoliths.",
                                "Fleurel are flower folk from Puppetwood.",
                                "Grimfolk are Sunstruck who escaped from Seb.",
                                "Los are anthro canines from the Smited Plains.",
                                "Sunstruck are humans as we know them.",
                                "Vaawie are reptilians from the Rainbow Coast."
                        };

                        if (selection >= 0 && selection < 10) {
                                draw_text(_renderer, &_font, 100, (_font.height + 1) * 2 + 5, heritage_descs[selection]);
                        }
                } else if (current_menu == &tradition_menu) {
                        const char *tradition_descs[] = {
                                "The Birane cult follows an ancient code of battle.",
                                "Scevimr people obey the orders of a far off emporer.",
                                "The people of Veronis believe in folk myths of another world."
                        };

                        if (selection >= 0 && selection < 3) {
                                draw_text(_renderer, &_font, 100, (_font.height + 1) * 2 + 5, tradition_descs[selection]);
                        }
                }

                SDL_RenderPresent(_renderer);

                /* Delay until the next frame. */
                SDL_Delay(40);
        }

        /* The program never reaches here. */
        return 0;
}
