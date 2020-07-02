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
#include <GL/gl.h>
#include <GL/glu.h>

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
        GLuint texture;
        SDL_Rect *clips;
        int image_width;
        int image_height;
        int height;
};

/* Sprites are combine textures with width and height information. */
struct sprite {
        GLuint texture;
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
#define OP(glyph) glyph
        FOR_GLYPH(OP),          /* alphabet */
#undef OP
        0,                      /* texture (loaded at run-time) */
        _font_clips,            /* clips (filled at run-time) */
        0,                      /* image_width (loaded at run-time) */
        0,                      /* image_height (loaded at run-time) */
        0                       /* height (computed at run-time) */
};

/* The cursor texture */
static struct sprite _cursor_sprite = {0, 0, 0}; /* loaded at run-time */

/* The game window */
static SDL_Window *_window = NULL;
/* The GL context for drawing all graphics */
static SDL_GLContext _context = NULL;

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
        if (glIsTexture(_font.texture)) {
                glDeleteTextures(1, &_font.texture);
        }

        if (glIsTexture(_cursor_sprite.texture)) {
                glDeleteTextures(1, &_cursor_sprite.texture);
        }

        if (_context) {
                SDL_GL_DeleteContext(_context);
        }

        if (_window) {
                SDL_DestroyWindow(_window);
        }
}

/* Draw text in a certain font at a certain position. */
void draw_text(struct font *font,
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
                        pen_y -= font->height + 1;
                } else {
                        for (j = 0; font->alphabet[j] != '\0'; j++) {
                                if (font->alphabet[j] == text[i]) {
                                        SDL_Rect *clip;

                                        clip = &font->clips[j];

                                        {
                                                GLdouble clip_ratios[] = {
                                                        ((GLdouble)clip->x)
                                                          / ((GLdouble)font->image_width),
                                                        ((GLdouble)clip->y)
                                                          / ((GLdouble)font->image_height),
                                                        ((GLdouble)clip->x + (GLdouble)clip->w)
                                                          / ((GLdouble)font->image_width),
                                                        ((GLdouble)clip->y + (GLdouble)clip->h)
                                                          / ((GLdouble)font->image_height)
                                                };
                                                glBindTexture(GL_TEXTURE_2D, _font.texture);
                                                glBegin(GL_POLYGON);
                                                        glTexCoord2d(clip_ratios[0], clip_ratios[1]);
                                                        glVertex2d(pen_x, pen_y);
                                                        glTexCoord2d(clip_ratios[0], clip_ratios[3]);
                                                        glVertex2d(pen_x, pen_y - clip->h);
                                                        glTexCoord2d(clip_ratios[2], clip_ratios[3]);
                                                        glVertex2d(pen_x + clip->w, pen_y - clip->h);
                                                        glTexCoord2d(clip_ratios[2], clip_ratios[1]);
                                                        glVertex2d(pen_x + clip->w, pen_y);
                                                glEnd();
                                        }

                                        pen_x += clip->w + 1;
                                        break;
                                }
                        }
                }
        }
}

/* Load an OpenGL texture from a file. */
GLuint load_gl_texture_and_handle_surface(
                const char *path,
                void (*surface_handler)(SDL_Surface*, void*),
                void *data,
                void (*print_error)(const char*, ...)) {
        SDL_Surface *surf;
        SDL_Surface *converted;
        GLuint tex;

        /* Load the image. */
        surf = IMG_Load(path);
        if (!surf) {
                print_error("IMG_Load: %s\n", IMG_GetError());
                return 0;
        }

        /* Convert the surface to a format that OpenGL can parse. */
        converted = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_RGB24, 0);
        if (!converted) {
                SDL_FreeSurface(surf);
                print_error("SDL_ConvertSurfaceFormat: %s\n", SDL_GetError());
                return 0;
        }

        /* Turn the texture into an OpenGL texture. */
        {
                glGenTextures(1, &tex);
                glBindTexture(GL_TEXTURE_2D, tex);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexImage2D(GL_TEXTURE_2D,
                             0,
                             GL_RGB8,
                             converted->w,
                             converted->h,
                             0,
                             GL_RGB,
                             GL_UNSIGNED_BYTE,
                             converted->pixels);
        }

        /* Send the surface to the user-supplied surface handler
         * for extra processing. */
        surface_handler(surf, data);

        /* Free the surfaces. */
        SDL_FreeSurface(converted);
        SDL_FreeSurface(surf);

        /* Return the loaded texture ID. */
        return tex;
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

        font->image_width = surf->w;
        font->image_height = surf->h;

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
                                   SDL_WINDOW_OPENGL);
        if (!_window) {
                die("SDL_CreateWindow: %s\n", SDL_GetError());
        }

        _context = SDL_GL_CreateContext(_window);
        if (!_context) {
                die("SDL_GL_CreateContext: %s\n", SDL_GetError());
        }

        /* Set up OpenGL. */
        glShadeModel(GL_FLAT);
        glViewport(0, 0, (GLsizei)WINDOW_WIDTH, (GLsizei)WINDOW_HEIGHT);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluOrtho2D(0.0f, (GLdouble)WINDOW_WIDTH, 0.0f, (GLdouble)WINDOW_HEIGHT);

        /* Load the cursor texture. */
        _cursor_sprite.texture = load_gl_texture_and_handle_surface(
            "data/images/cursor.png",
            &get_sprite_dims_from_surface,
            &_cursor_sprite,
            &die);

        /* Create the renderer. */
        _font.texture = load_gl_texture_and_handle_surface(
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
                glClear(GL_COLOR_BUFFER_BIT);

                /* Enable texturing. */
                glEnable(GL_TEXTURE_2D);
                glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

                /* Draw the menu prompt. */
                draw_text(&_font, 9, WINDOW_HEIGHT - 5, current_menu->prompt);

                /* Draw the options. */
                {
                        int i;
                        for (i = 0; i < current_menu->num_options; i++) {
                                draw_text(&_font, 9, WINDOW_HEIGHT - (5 + ((_font.height + 1) * (2 + i))), current_menu->options[i]);
                        }
                }

                /* Draw the description of the selected option. */
                draw_text(&_font, 100, WINDOW_HEIGHT - (5 + ((_font.height + 1) * 2)), current_menu->descriptions[selection]);


                /* Draw the cursor. */
                {
                        int x = 0;
                        int y = WINDOW_HEIGHT - (5 - ((_cursor_sprite.height - _font.height) / 2) + ((_font.height + 1) * (2 + selection)));
                        glBindTexture(GL_TEXTURE_2D, _cursor_sprite.texture);
                        glBegin(GL_POLYGON);
                                glTexCoord2i(0, 0); glVertex2d(x, y);
                                glTexCoord2i(0, 1); glVertex2d(x, y - _cursor_sprite.height);
                                glTexCoord2i(1, 1); glVertex2d(x + _cursor_sprite.width, y - _cursor_sprite.height);
                                glTexCoord2i(1, 0); glVertex2d(x + _cursor_sprite.width, y);
                        glEnd();
                }

                glDisable(GL_TEXTURE_2D);
                glFlush();

                SDL_GL_SwapWindow(_window);

                /* Delay until the next frame. */
                SDL_Delay(40);
        }

        /* The program never reaches here. */
        return 0;
}
