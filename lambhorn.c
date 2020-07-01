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

/* Fonts contain the data needed for drawing text */
struct font {
        const char *alphabet;
        SDL_Texture *texture;
        SDL_Rect *clips;
        int height;
};

/* Sprites are combine textures with width and height information. */
struct sprite {
        SDL_Texture *texture;
        int width;
        int height;
};

/* The clips for each letter in the font (computed at run-time) */
#define FOR_GLYPH(X) \
        X("A")  X("B")  X("C")  X("D") \
        X("E")  X("F")  X("G")  X("H") \
        X("I")  X("J")  X("K")  X("L") \
        X("M")  X("N")  X("O")  X("P") \
        X("Q")  X("R")  X("S")  X("T") \
        X("U")  X("V")  X("W")  X("X") \
        X("Y")  X("Z")  X("?")  X(".") \
        X(",")  X(" ")  X("a")  X("b") \
        X("c")  X("d")  X("e")  X("f") \
        X("g")  X("h")  X("i")  X("j") \
        X("k")  X("l")  X("m")  X("n") \
        X("o")  X("p")  X("q")  X("r") \
        X("s")  X("t")  X("u")  X("v") \
        X("w")  X("x")  X("y")  X("z") \
        X("!")

static SDL_Rect _font_clips[] = {
#define OP(glyph) {0, 0, 0, 0},
        FOR_GLYPH(OP)
#undef OP
};

/* The main game font */
static struct font _font = {
#define OP(letter) letter
        FOR_GLYPH(OP),          /* alphabet */
#undef OP
        NULL,                   /* texture (loaded at run-time) */
        _font_clips,            /* clips (filled at run-time) */
        0                       /* height (computed at run-time) */
};

/* The cursor texture */
static struct sprite _cursor_sprite;

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

        if (_cursor_sprite.texture) {
                SDL_DestroyTexture(_cursor_sprite.texture);
        }

        if (_renderer) {
                SDL_DestroyRenderer(_renderer);
        }

        if (_window) {
                SDL_DestroyWindow(_window);
        }
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

/* Load a texture from a file. */
SDL_Texture *load_texture_and_handle_surface(
                SDL_Renderer *renderer,
                const char *path,
                void (*surface_handler)(SDL_Surface*, void*),
                void *data,
                void (*print_error)(const char*, ...)) {
        SDL_Surface *surf;
        SDL_Texture *tex;

        surf = IMG_Load(path);
        if (!surf) {
                print_error("IMG_Load: %s\n", IMG_GetError());
                return NULL;
        }

        tex = SDL_CreateTextureFromSurface(renderer, surf);
        if (!tex) {
                const char *error = SDL_GetError();
                SDL_FreeSurface(surf);
                print_error("SDL_CreateTextureFromSurface: %s\n", error);
                return NULL;
        }

        surface_handler(surf, data);
        SDL_FreeSurface(surf);

        return tex;
}

/* Get sprite dimensions from a surface. */
void get_sprite_dims_from_surface(SDL_Surface *surf, void *sprite_ptr) {
        struct sprite *sprite = sprite_ptr;

        sprite->width = surf->w;
        sprite->height = surf->h;
}

/* Get font information (clips & height) from a surface. */
void get_font_info_from_surface(SDL_Surface *surf, void *font_ptr) {
        struct font *font = font_ptr;
        int i;
        int x;

        x = 1;
        for (i = 0; font->alphabet[i] != '\0'; i++) {
                int j;

                font->clips[i].x = x;
                font->clips[i].y = 0;

                font->clips[i].w = 0;
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
                                font->clips[i].w = j - x;
                                x = j + 1;
                                break;
                        }
                }

                font->clips[i].h = surf->h - 1;
        }

        font->height = surf->h;
}

int main(int argc, char *argv[]) {
        static const char *title_options[] = {
                "New Game",
                "Quit"
        };
        static const char *title_descriptions[] = {
                "Begin a new adventure in the land of Lambhorn.",
                "Quit playing the game."
        };
        static const char *heritage_options[] = {
                "Acolith",
                "Caprons",
                "Daimyo",
                "Delvren",
                "Felith",
                "Fleurel",
                "Grimfolk",
                "Los",
                "Sunstruck",
                "Vaawie",
                "Cancel"
        };
        static const char *heritage_descriptions[] = {
                "Acolith are pointy eared immortals.",
                "Caprons are goat headed folk from Im Otra.",
                "Daimyo are ogre demons of mysterious origin.",
                "Delvren are mole headed undergrounders.",
                "Felith are mortal Acoliths.",
                "Fleurel are flower folk from Puppetwood.",
                "Grimfolk are Sunstruck who escaped from Seb.",
                "Los are anthro canines from the Smited Plains.",
                "Sunstruck are humans as we know them.",
                "Vaawie are reptilians from the Rainbow Coast.",
                "Return to the previous menu."
        };
        static const char *tradition_options[] = {
                "Birane",
                "Scevimric",
                "Veronis",
                "Cancel"
        };
        static const char *tradition_descriptions[] = {
                "The Birane cult follows an ancient code of battle.",
                "Scevimr people obey the orders of a far off emporer.",
                "The people of Veronis believe in folk myths of another world.",
                "Return to previous menu."
        };
        struct {
                const char *prompt;
                int num_options;
                const char **options;
                const char **descriptions;
        } title_menu = {"Lambhorn", 2, title_options, title_descriptions},
          heritage_menu = {"Choose thine heritage.", 11, heritage_options, heritage_descriptions},
          tradition_menu = {"Choose thy tradition.", 4, tradition_options, tradition_descriptions},
          *current_menu = &title_menu;
        int selection = 0;

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

        /* Load sprites. */
        _cursor_sprite.texture = load_texture_and_handle_surface(
            _renderer,
            "data/images/cursor.png",
            &get_sprite_dims_from_surface,
            &_cursor_sprite,
            &die);

        /* Load the font. */
        _font.texture = load_texture_and_handle_surface(
            _renderer,
            "data/images/font.png",
            &get_font_info_from_surface,
            &_font,
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

                /* Clear the screen. */
                SDL_SetRenderDrawColor(_renderer,
                                       0xff, 0xff, 0xff,
                                       SDL_ALPHA_OPAQUE);
                SDL_RenderClear(_renderer);

                /* Draw the menu prompt. */
                draw_text(_renderer, &_font, 9, 5, current_menu->prompt);

                /* Draw the options. */
                {
                        int i;
                        for (i = 0; i < current_menu->num_options; i++) {
                                draw_text(_renderer, &_font, 9, 5 + ((_font.height + 1) * (2 + i)), current_menu->options[i]);
                        }
                }

                /* Draw the description of the selected option. */
                draw_text(_renderer, &_font, 100, (_font.height + 1) * 2 + 5, current_menu->descriptions[selection]);


                /* Draw the cursor. */
                {
                        SDL_Rect rect;

                        rect.x = 0;
                        rect.y = 5 - ((_cursor_sprite.height - _font.height) / 2) + ((_font.height + 1) * (2 + selection));
                        rect.w = _cursor_sprite.width;
                        rect.h = _cursor_sprite.height;

                        SDL_RenderCopy(_renderer, _cursor_sprite.texture, NULL, &rect);
                }

                SDL_RenderPresent(_renderer);

                /* Delay until the next frame. */
                SDL_Delay(40);
        }

        /* The program never reaches here. */
        return 0;
}
