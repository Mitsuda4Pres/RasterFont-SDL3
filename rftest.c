//rftest.c
//Showcase/sandbox for rasterfont.h

#define SDL_MAIN_USE_CALLBACKS 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_gfx/SDL3_gfxPrimitives.h>
//android build
//#include <SDL3_gfxPrimitives.h>
#include "include/rasterfont.h"

#define SCREEN_WIDTH 1200
#define SCREEN_HEIGHT 600
#define RED_OPAQUE 0xFF0000FF
#define GREEN_OPAQUE 0xFF00FF00
#define BLUE_OPAQUE 0xFFFF0000
#define YELLOW_OPAQUE 0xFF00FFFF
#define MAGENTA_OPAQUE 0xFFFF00FF
#define CYAN_OPAQUE 0xFFFFFF00
#define WHITE_OPAQUE 0xFFFFFFFF

//mouse flags
#define NONE 0x00
#define MOUSE_LEFT_BTN_DOWN 0x01
#define MOUSE_RIGHT_BTN_DOWN 0x02
#define MOUSE_WHEEL_CLICK_DOWN 0x04

typedef struct{
	/*Contains state data about entire program*/
	//
	//Structural
	SDL_Window *window;
	SDL_Renderer *renderer;
	struct RF_Font font;
	int app_is_running;
	int debug_text;    //toggle for debug text display
	//
	//Timing
	Uint64 prev_time;
	Uint64 current_time;
	double dt;
	double dt_accumulator;
	int phy_cycles_per_frame;
	//
	//Mouse context
	int mouse_x;
	int mouse_y;
	char button_down_state; //flag with LEFT_CLICK_DOWN, RIGHT_CLICK_DOWN, other state reserved for more buttons/wheel
	char btn_released_state; //same
} AppState;

//Function declarations
void SDL_AppQuit(void *appstate, SDL_AppResult result);
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]);
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event);
SDL_AppResult SDL_AppIterate(void *appstate);

int draw(AppState *app_state);

//SDL Initializer funtion
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
	SDL_SetAppMetadata("RasterFont Test", "1.0", "null");

	if(!SDL_Init(SDL_INIT_VIDEO  | SDL_INIT_EVENTS)){
		SDL_Log("Error initializing SDL. %s\n", SDL_GetError());
		return SDL_APP_FAILURE;
	}
	AppState *app_state = NULL;
	app_state = (AppState *)malloc(sizeof(AppState));
	memset(app_state, 0, sizeof(AppState));

	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");

	if(!SDL_CreateWindowAndRenderer("RasterFont Test", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE, &app_state->window, &app_state->renderer)) {
		SDL_Log("Couldn't create window and renderer: %s\n", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	SDL_SetWindowRelativeMouseMode(app_state->window, false);
	SDL_SetRenderLogicalPresentation(app_state->renderer, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_LOGICAL_PRESENTATION_LETTERBOX);
	SDL_SetRenderViewport(app_state->renderer, NULL);
	//
	//
	//Initialize debug state
	app_state->debug_text = 0;
	app_state->button_down_state = 0;
	//
	//load raster font
	app_state->font = RF_loadFontFromFile("resources/classic.rff");
	
	if(strcmp(app_state->font.error_msg, "clear") != 0)
		SDL_Log("RF_loadfontFromFile() produced error: %s\n", app_state->font.error_msg);
	//Test display app_state->font struct elements
	printf("app_state->font name: %s\n", app_state->font.name);
	printf("app_state->font width: %d\n", app_state->font.base_width);
	printf("app_state->font height: %d\n", app_state->font.base_height);
	printf("1st character: %c, lines: %d, offset: %d\n", app_state->font.characters[0].name, app_state->font.characters[0].total_segments, app_state->font.characters[0].offset);
	printf("First line: %d, %d, %d, %d\n", app_state->font.characters[0].segments[0].a.x, app_state->font.characters[0].segments[0].a.y,app_state->font.characters[0].segments[0].b.x,app_state->font.characters[0].segments[0].b.y);
	printf("Fifth line: %d, %d, %d, %d\n", app_state->font.characters[0].segments[4].a.x, app_state->font.characters[0].segments[4].a.y,app_state->font.characters[0].segments[4].b.x,app_state->font.characters[0].segments[4].b.y);
	printf("2nd character: %c, lines: %d, offset: %d\n", app_state->font.characters[1].name, app_state->font.characters[1].total_segments, app_state->font.characters[1].offset);
	printf("First line: %d, %d, %d, %d\n", app_state->font.characters[1].segments[0].a.x, app_state->font.characters[1].segments[0].a.y,app_state->font.characters[1].segments[0].b.x,app_state->font.characters[1].segments[0].b.y);
	printf("Fifth line: %d, %d, %d, %d\n", app_state->font.characters[1].segments[4].a.x, app_state->font.characters[1].segments[4].a.y,app_state->font.characters[1].segments[4].b.x,app_state->font.characters[1].segments[4].b.y);
	printf("3rd character: %c, lines: %d, offset: %d\n", app_state->font.characters[2].name, app_state->font.characters[2].total_segments, app_state->font.characters[2].offset);
	printf("First line: %d, %d, %d, %d\n", app_state->font.characters[2].segments[0].a.x, app_state->font.characters[2].segments[0].a.y,app_state->font.characters[2].segments[0].b.x,app_state->font.characters[2].segments[0].b.y);
	printf("Fifth line: %d, %d, %d, %d\n", app_state->font.characters[2].segments[4].a.x, app_state->font.characters[2].segments[4].a.y,app_state->font.characters[2].segments[4].b.x,app_state->font.characters[2].segments[4].b.y);

	*appstate = app_state;
	return SDL_APP_CONTINUE;
}

//Handle all input events
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event){
	AppState *app_state = (AppState *)appstate;

	switch(event->type){
		case SDL_EVENT_QUIT: {
			//run cleanups?
			return SDL_APP_SUCCESS;
		}
		case SDL_EVENT_KEY_DOWN: {
			switch(event->key.key){
				case SDLK_ESCAPE:{
					break;
				}
				case SDLK_F1:{
					app_state->debug_text = !app_state->debug_text;
					if(app_state->debug_text)
						printf("Debug text on.\n");
					if(!app_state->debug_text)
						printf("Debug text off.\n");
					break;	
				}
			//Other keyboard actions here
			}
			break;
		}
		case SDL_EVENT_MOUSE_MOTION:{
			app_state->mouse_x = event->motion.x;
			app_state->mouse_y = event->motion.y;
			break;
		}
		case SDL_EVENT_MOUSE_BUTTON_DOWN:{
			switch(event->button.button){
				case SDL_BUTTON_LEFT:{
					app_state->button_down_state |= MOUSE_LEFT_BTN_DOWN; 
					app_state->mouse_x = event->motion.x;
					app_state->mouse_y = event->motion.y;
	
					break;
				}
			}
			break;
		}
		case SDL_EVENT_MOUSE_BUTTON_UP:{
			switch(event->button.button){
				case SDL_BUTTON_LEFT:{
					app_state->button_down_state &= ~MOUSE_LEFT_BTN_DOWN; //Operation to clear bit flag (AND equals NOT flag)
					break;
				}
			}
			break;
		}
	}
	return SDL_APP_CONTINUE;
}


SDL_AppResult SDL_AppIterate(void *appstate){
	AppState *app_state = (AppState *)appstate;

	draw(app_state);
	return SDL_APP_CONTINUE;
}

//Draw objects based on data-oriented architecture.
int draw(AppState *app_state){
	SDL_SetRenderDrawColor(app_state->renderer,0,0,0,255);
	SDL_RenderClear(app_state->renderer);
	SDL_SetRenderDrawColor(app_state->renderer, 255,255,255,255);
	//FC_DrawAlign(app_state->chrono_font, app_state->renderer, (SCREEN_WIDTH/2), 8, FC_ALIGN_CENTER, "%d Bubbles!", app_state->obj.bubble_count);
	RF_printString(app_state->renderer, 10, 10, 12, "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG", &app_state->font, 0);
	RF_printString(app_state->renderer, 20,100, 48, "THE QUICK BROWN FOX", &app_state->font, 0);
	RF_printString(app_state->renderer, 40,250, 96, "JUMPS OVER", &app_state->font, 0);
	RF_printString(app_state->renderer, 600,50, 16, "THE LAZY DOG", &app_state->font, 0);
	//debug text display
	if(app_state->debug_text){

	}
	//
	int mx, my;
		
	SDL_RenderPresent(app_state->renderer);
	return 0;	
}

//Destroy SDL objects and free memory.
void SDL_AppQuit(void *appstate, SDL_AppResult result){
	AppState *app_state = (AppState *)appstate;
	//Free structural elements
	if(app_state){
		SDL_DestroyRenderer(app_state->renderer);
		app_state->renderer = NULL;
		SDL_DestroyWindow(app_state->window);
		app_state->window = NULL;
		RF_destroyFont(&app_state->font);
	}
	SDL_Quit();
}
//End of life-cycle functions
