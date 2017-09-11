/*
 * guilloche.c - Guilloche powered by libSDL and Cairo
 *
 * Copyright (C) 2015 Jakob Flierl <jakob.flierl@gmail.com>
 *
 * Tested on Ubuntu 16.04 with:
 *
 * $ sudo apt -y install libsdl1.2-dev make gcc libcairo2-dev libpng16-dev
 *
 * for joystick access: $ sudo usermod -aG input $USER
 */

#define HAVE_JOYSTICK

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#include <cairo/cairo.h>
#include <cairo/cairo-svg.h>
#include <limits.h>

#include "savepng.h"

int width  = 1280;
int height = 720;

#ifdef HAVE_JOYSTICK
SDL_Joystick* joy = NULL;
int joy_button[20];
int joy_axis[20];
const int JOYSTICK_DEAD_ZONE = 1000;

double line_width_joy = 0.0;
#endif

int mode = 1;
int draw_mode = 0; // 0 for lines, 1 for pixels

double t_step = 0.008;
double t_step_step = 0.00000001;
double R_step = 0.0001;
double R = 36;   // big steps
double r = 0.08; // little steps
double p = 35;   // size of the ring

double Q = 30, Q_max = 150;
double m = 1,  m_max = 50, m_delta = 0.1;
double n = 6,  n_max = 50, n_delta = 0.1;

double R_delta = 0.1;
double r_delta = 0.00001;

double line_width = 0.6;

long png = 0;
long svg = 0;

/*
Epicycloid
Hypotrochoid
Epitrochoid
Spirograph

https://en.wikipedia.org/wiki/Hypocycloid
https://en.wikipedia.org/wiki/Spirograph
https://en.wikipedia.org/wiki/Harmonograph
https://en.wikipedia.org/wiki/Guilloch%C3%A9

https://en.wikipedia.org/wiki/Epitrochoid
https://en.wikipedia.org/wiki/Hypotrochoid

Trigonometric functions:
* Sine
* Cosine
* Tangent

Sinus-like functions:
* Trochoid
* Cycloid
* Clausen function

Non-smooth functions:
* Triangle wave (non-continuous first derivative)
* Sawtooth wave (non-continuous)
* Square wave (non-continuous)
* Cycloid (non-continuous first derivative)
* Tangent (non-continuous)

Vector-valued functions:
* Epitrochoid
* Epicycloid (special case of the epitrochoid)
* Limaçon (special case of the epitrochoid)
* Hypotrochoid
* Hypocycloid (special case of the hypotrochoid)
* Spirograph (special case of the hypotrochoid)
*/

void rainbow(int step, int numsteps, double *r, double *g, double *b) {
  double  h = (double) step / numsteps;
  int i = h * 6;
  double f = h * 6.0 - i;
  int q = 1 - f;

  switch (i % 6) {
  case 0:
    (*r) = 1.0;
    (*g) = f;
    (*b) = 0.0;
    break;
  case 1:
     (*r) = q;
     (*g) = 1.0;
     (*b) = 0.0;
    break;
  case 2:
     (*r) = 0.0;
     (*g) = 1.0;
     (*b) = f;
    break;
  case 3:
     (*r) = 0.0;
     (*g) = q;
     (*b) = 1.0;
    break;
  case 4:
     (*r) = f;
     (*g) = 0.0;
     (*b) = 1.0;
    break;
  case 5:
    (*r) = 1.0;
    (*g) = 0.0;
    (*b) = q;
    break;
  }
}

void guilloche(cairo_t *cr, int width, int height) {
        /* who doesn't want all those nice line settings :) */
        cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);
        cairo_set_line_width (cr, line_width);
        cairo_set_source_rgba (cr, 0.5, 1, 0.5, 1.0);

        p = height * 0.07;

        double oldx, oldy;
        int first = 0;
        double t = 0;
	int i = 0;
	
        while ( t < 2 * M_PI) {
                t += t_step;
                double x = (R+r)*cos(t)+(r+p)*cos((R+r)/r*t);
                double y = (R+r)*sin(t)+(r+p)*sin((R+r)/r*t);

                x = x * 4 + width / 2;
                y = y * 4 + height / 2;

		double a = 1;
		double colr = 0.4;
		double colg = a;
		double colb = 0.4;

		// printf("%d %d\n", i, (int)(2 * M_PI / t_step));
		rainbow(i, (int)(2 * M_PI / t_step), &colr, &colg, &colb);
		cairo_set_source_rgb (cr, colr, colg, colb);
		
                if (first == 1) {
		  if (draw_mode == 0) {
		    cairo_move_to (cr, oldx, oldy);
		    cairo_line_to (cr, x, y);
		  } else {
		    cairo_arc(cr, x, y, line_width, 0, 2 * M_PI);
		  }
		  cairo_stroke (cr);
                } else {
                        first = 1;
                }

                oldx = x;
                oldy = y;
		i++;
        }
        t_step += t_step_step;
        R += R_step;
}

void guilloche2(cairo_t *cr, int width, int height) {
    //R = 50;
    //r = -0.25;
    //p = 25;
    p = height * 0.03;
    
    double t_step = 0.001;
    double t_step_step = 0.000000001;
    
    /* who doesn't want all those nice line settings :) */
    cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_width (cr, line_width);
    cairo_set_source_rgba (cr, 0.5, 1, 0.5, 1.0);

    double oldx, oldy;
    int first = 0;
    double t = 0;
    int i = 0;
    while ( t < 2 * M_PI) {
        t += t_step;
        double x =(R+r)*cos(m*t)+(r+p)*cos(m*t*(R+r)/r)+Q*cos(n*t);
        double y =(R+r)*sin(m*t)+(r+p)*sin(m*t*(R+r)/r)+Q*sin(n*t);
        
        x = x * 4 + width / 2;
        y = y * 4 + height / 2;
        
	double a = 1;
	double colr = 0.4;
	double colg = a;
	double colb = 0.4;
	// printf("%d %d\n", i, (int)(2 * M_PI / t_step));
	rainbow(i, (int)(2 * M_PI / t_step), &colr, &colg, &colb);
	cairo_set_source_rgb (cr, colr, colg, colb);
	    
        if (first == 1) {
	    if (draw_mode == 0) {
	      cairo_move_to (cr, oldx, oldy);
	      cairo_line_to (cr, x, y);
	    } else {
	      cairo_arc(cr, x, y, line_width, 0, 2 * M_PI);
	    }

            cairo_stroke (cr);
        } else {
            first = 1;
        }
        
        oldx = x;
        oldy = y;
	i++;
    }
    t_step += t_step_step;
    R += R_step;
}

void draw(cairo_t *cr, int width, int height) {
    /* Fill the background with black. */
    cairo_set_source_rgb (cr, 0, 0, 0);
    cairo_paint (cr);

    if (mode == 0) {
        guilloche(cr, width, height);
    } else if (mode == 1) {
        guilloche2(cr, width, height);
    }
}

double sgn(double x) {
    if (x > 0) return 1.0;
    if (x < 0) return -1.0;
    return 0.0;
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

#ifdef HAVE_JOYSTICK
    double R_joy = 0.0, r_joy = 0.0;
#endif
    
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

#ifdef HAVE_JOYSTICK
    //SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
    if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0)   {
#else
    if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO) < 0)   {
#endif
        fprintf(stderr, "Unable to initialise SDL: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

#ifdef HAVE_JOYSTICK
    if (SDL_NumJoysticks() < 1) {
        printf( "Warning: No joysticks connected!\n" );
    } else {
        joy = SDL_JoystickOpen( 0 );
        if (joy == NULL) {
            printf( "Warning: Unable to open game controller! SDL Error: %s\n", SDL_GetError() );
        }
    }
#endif

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

#ifdef HAVE_JOYSTICK
        R += R_joy;
        r += r_joy;
        line_width += line_width_joy;
#endif
        
        draw(cr, width, height);

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
                    } else if (event.key.keysym.sym == SDLK_F2) {
                        char svgfile[PATH_MAX];
                        snprintf(svgfile, sizeof(svgfile), "%010lu.svg", svg);
                        printf("saving to %s.. ", svgfile);
                        cairo_surface_t *surface = cairo_svg_surface_create(svgfile, width, height);
                        //cairo_svg_surface_restrict_to_version (surface, CAIRO_SVG_VERSION_1_1);
                        cairo_t* cr_svg = cairo_create (surface);                        
                        draw(cr_svg, width, height);
                        cairo_destroy(cr_svg);
                        cairo_surface_finish(surface);
                        cairo_surface_destroy(surface);
                        
                        printf("ok\n");
                        svg++;
                    } else if (event.key.keysym.sym == SDLK_LEFT) {
                            r -= r_delta;
                    } else if (event.key.keysym.sym == SDLK_RIGHT) {
                            r += r_delta;
                    } else if (event.key.keysym.sym == SDLK_UP) {
                            R -= R_delta;
                    } else if (event.key.keysym.sym == SDLK_DOWN) {
                            R += R_delta;
                    } else if (event.key.keysym.sym == SDLK_F1) {
                            mode++;
                            if (mode > 1) mode = 0;
                    } else if (event.key.keysym.sym == SDLK_1) {
                            line_width-=0.1;
                    } else if (event.key.keysym.sym == SDLK_2) {
                            line_width+=0.1;
                    } else if (event.key.keysym.sym == SDLK_q) {
                            Q++;
                            if (Q > Q_max) Q = -Q_max;
                    } else if (event.key.keysym.sym == SDLK_a) {
                            Q--;
                            if (Q < -Q_max) Q = Q_max;
                    } else if (event.key.keysym.sym == SDLK_w) {
		      m+=m_delta;
                            if (m > m_max) m = -m_max;
                    } else if (event.key.keysym.sym == SDLK_s) {
 		      m-=m_delta;
                            if (m < -m_max) m = m_max;
                    } else if (event.key.keysym.sym == SDLK_e) {
		      n+=n_delta;
                            if (n > n_max) n = -n_max;
                    } else if (event.key.keysym.sym == SDLK_d) {
		      n-=n_delta;
                            if (n < -n_max) n = n_max;
                    } else if (event.key.keysym.sym == SDLK_m) {
		      if (draw_mode == 0) {
			draw_mode = 1;
		      } else {
			draw_mode = 0;
		      }
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

#ifdef HAVE_JOYSTICK
        if (event.type == SDL_JOYAXISMOTION) {
#define JOY_SCALE (1.0 / (32768.0 * 4.0))
            if (event.jaxis.value < -JOYSTICK_DEAD_ZONE || event.jaxis.value > JOYSTICK_DEAD_ZONE) {
                joy_axis[event.jaxis.axis] = event.jaxis.value;
            } else {
                joy_axis[event.jaxis.axis] = 0.0;
            }
        } else if (event.type == SDL_JOYBUTTONDOWN) {
            joy_button[event.jbutton.button] = (event.jbutton.state == SDL_PRESSED) ? 1 : 0;
            printf("joy button: %d: %d\n", event.jbutton.button, event.jbutton.state);
        } else if (event.type == SDL_JOYBUTTONUP) {
            joy_button[event.jbutton.button] = (event.jbutton.state == SDL_PRESSED) ? 1 : 0;
            // printf("joy button: %d: %d\n", event.jbutton.button, event.jbutton.state);
        }

        R_joy = joy_axis[0] * JOY_SCALE * 1.5 + joy_axis[2] * JOY_SCALE * 0.1 + (joy_button[4] * 0.5 * sgn(joy_axis[2]));
        r_joy = joy_axis[1] * JOY_SCALE * 0.1 + joy_axis[3] * JOY_SCALE * 0.00001 + (joy_button[6] * 0.01 * sgn(joy_axis[3]));
        line_width_joy = - joy_button[5] * 0.02 + joy_button[7] * 0.02;
#endif

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

#ifdef HAVE_JOYSTICK
    if (joy) {
        SDL_JoystickClose(joy);
        joy = NULL;
    }
#endif
    
    SDL_Quit();

    return 0;
}
