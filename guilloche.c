/*
 * guilloche.c - Guilloche powered by libSDL and Cairo
 *
 * Copyright (C) 2015 Jakob Flierl <jakob.flierl@gmail.com>
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#include <cairo/cairo.h>
#include <limits.h>

#include "savepng.h"

int width  = 1280;
int height = 720;

double theta_step = 0.008;
double theta_step_step = 0.00000001;
double R_step = 0.0001;
double R = 36;   // big steps
double r = 0.08; // little steps
double p = 35;   // size of the ring

double R_delta = 0.1;
double r_delta = 0.00001;

long png = 0;

void guilloche(cairo_t *cr, int width, int height) {
        /* Fill the background with white. */
        cairo_set_source_rgb (cr, 0, 0, 0);
        cairo_paint (cr);

        /* who doesn't want all those nice line settings :) */
        cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);
        cairo_set_line_width (cr, 0.4);
        cairo_set_source_rgb (cr, 0.5, 1, 0.5);

        p = height * 0.07;

        double oldx, oldy;
        int first = 0;
        double theta = 0;
        while ( theta < 2 * M_PI) {
                theta += theta_step;
                double x = (R + r) * cos(theta) + (r + p) * cos((R+r)/r * theta);
                double y = (R + r) * sin(theta) + (r + p) * sin((R+r)/r * theta);

                x = x * 4 + width / 2;
                y = y * 4 + height / 2;

                if (first == 1) {
                        double a = 1;
                        double colr = 0.4;
                        double colg = a;
                        double colb = 0.4;
                        cairo_set_source_rgb (cr, colr, colg, colb);
                        cairo_move_to (cr, oldx, oldy);
                        cairo_line_to (cr, x, y);
                        cairo_stroke (cr);
                } else {
                        first = 1;
                }

                oldx = x;
                oldy = y;
        }
        theta_step += theta_step_step;
        R += R_step;
}

SDL_Cursor *sdl_cursor;

void hide_cursor() {
    int32_t cursorData[2] = {0, 0};
    sdl_cursor = SDL_CreateCursor((Uint8 *)cursorData, (Uint8 *)cursorData, 8, 8, 4, 4);
    SDL_SetCursor(sdl_cursor);
}
                
int main (int argc, char **argv) {
    int do_help = 0;
    int do_png  = 0;

    int videoFlags = SDL_SWSURFACE | SDL_RESIZABLE | SDL_DOUBLEBUF;
    int bpp        = 32;


    int i = 0;
    while (++i < argc) {
#define OPTION_SET(longopt,shortopt) (strcmp(argv[i], longopt)==0 || strcmp(argv[i], shortopt)==0)
#define OPTION_VALUE ((i+1 < argc)?(argv[i+1]):(NULL))
#define OPTION_VALUE_PROCESSED (i++)
        if (OPTION_SET("--fullscreen", "-f")) {
            videoFlags |= SDL_FULLSCREEN;
        } else if (OPTION_SET("--screenshot", "-s")) {
            do_png = 1;
        } else if (OPTION_SET("--help", "-h")) {
            do_help = 1;
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            do_help = 1;
        }
    }

    if (do_help) {
        fprintf(stderr, "Usage: %s [OPTIONS]\n\n"
                " Where [OPTIONS] are zero or more of the following:\n\n"
                "    [-f|--fullscreen]           Fullscreen mode\n"
                "    [-s|--screenshot]           Save screenshots\n"
                "    [-h|--help]                 Show help information\n\n"
                , argv[0]);
        return EXIT_SUCCESS;
    }
        
    if (SDL_Init(SDL_INIT_VIDEO) < 0)   { 
        fprintf(stderr, "Failed to initialize SDL");
        return -1; 
    }

    const SDL_VideoInfo* info = SDL_GetVideoInfo();
    int screenWidth = info->current_w;
    int screenHeight = info->current_h;

    SDL_WM_SetCaption("guilloche", "guilloche");

    SDL_Surface *screen; 
    screen = SDL_SetVideoMode(width, height, bpp, videoFlags); 

    /* Enable key repeat, just makes it so we don't have to worry about fancy
     * scanboard keyboard input and such */
    SDL_EnableKeyRepeat(100, 10);
    SDL_EnableUNICODE(1); 

    /* Create an SDL image surface we can hand to cairo to draw to */
    SDL_Surface *sdl_surface = SDL_CreateRGBSurface (
            videoFlags, width, height, 32,
            0x00ff0000,
            0x0000ff00,
            0x000000ff,
            0
        );

    hide_cursor();

    /* Our main event/draw loop */
    int done = 0;
    while (!done) {
        /* Clear our surface */
            // SDL_FillRect( sdl_surface, NULL, 0 );

        /* Create a cairo surface which will write directly to the sdl surface */
        cairo_surface_t *cairo_surface = cairo_image_surface_create_for_data (
                (unsigned char *)sdl_surface->pixels,
                CAIRO_FORMAT_RGB24,
                sdl_surface->w,
                sdl_surface->h,
                sdl_surface->pitch);

        cairo_t *cr = cairo_create(cairo_surface);

        guilloche(cr, width, height);

        /******************************************************************************/
        /*** Cleanup our cairo surface, copy it to the screen, deal with SDL events ***/
        /******************************************************************************/

        /* Blit our new image to our visible screen */ 
        SDL_BlitSurface(sdl_surface, NULL, screen, NULL); 
        SDL_Flip(screen); 

        /* We're now done with our cairo surface */
        cairo_surface_destroy(cairo_surface);
        cairo_destroy(cr);


        /* Handle SDL events */
        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_KEYDOWN:           
                    if (event.key.keysym.sym == SDLK_ESCAPE) {
                        done = 1;
                    } else if (event.key.keysym.sym == SDLK_RETURN) {
                            int flags = screen->flags; /* Save the current flags in case toggling fails */
                            screen = SDL_SetVideoMode(0, 0, 0, screen->flags ^ SDL_FULLSCREEN); /*Toggles FullScreen Mode */
                            if(screen == NULL) screen = SDL_SetVideoMode(screenWidth, screenHeight, 0, flags);
                            if(screen == NULL) exit(1); /* If you can't switch back for some reason, then epic fail */
                    } else if (event.key.keysym.sym == SDLK_LEFT) {
                            r -= r_delta;
                    } else if (event.key.keysym.sym == SDLK_RIGHT) {
                            r += r_delta;
                    } else if (event.key.keysym.sym == SDLK_UP) {
                            R -= R_delta;
                    } else if (event.key.keysym.sym == SDLK_DOWN) {
                            R += R_delta;
                    }
                    break;

	        case SDL_QUIT:
                    done = 1;
                    break;

                case SDL_VIDEORESIZE:
                    width = event.resize.w;
                    height = event.resize.h;
                    screen = SDL_SetVideoMode(event.resize.w, event.resize.h, bpp, videoFlags);
                    if (!screen) {
                        fprintf(stderr, "Could not get a surface after resize: %s\n", SDL_GetError( ));
                        exit(-1);
                    }
                    /* Create an SDL image surface we can hand to cairo to draw to */
                    SDL_FreeSurface(sdl_surface);
                    sdl_surface = SDL_CreateRGBSurface (
                            videoFlags, width, height, 32,
                            0x00ff0000,
                            0x0000ff00,
                            0x000000ff,
                            0
                            );
                    break;

            case SDL_MOUSEMOTION:
                    R = event.motion.x / (double)(width) * 150.0;
                    r = event.motion.y / (double)(height) * 0.15;
                    //printf("%06.2f %06.2f\n", R, r);
                    break;

            case SDL_MOUSEBUTTONDOWN:
                    //printf("mouse button %i at (%i,%i)\n", event.button.button, event.button.x, event.button.y);
                    break;
            }
        }

        if (do_png == 1) {
            char pngfile[PATH_MAX];
            snprintf(pngfile, sizeof(pngfile), "%010lu.png", png);
            SDL_SavePNG(sdl_surface, pngfile);
            png++;
        }

        SDL_Delay(1); 
    }

    /* Cleanup */
    SDL_FreeCursor(sdl_cursor);
    SDL_FreeSurface(sdl_surface);
    SDL_Quit();

    return 0;
}
