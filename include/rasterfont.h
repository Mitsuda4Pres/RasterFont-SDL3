#ifndef RASTERFONT_H
#define RASTERFONT_H
//rasterfont.c
//
//An attempt to create a font that is drawn by SDL render functions instead of the SDL_TTF format. Whether this is more or less performant, and of better or worse quality
//remains to be seen. Initial plan is to create a structure (of structures) that will hold all the data required to draw letters/characters to the screen then call a
//handful of simple functions to render strings at a location.
//
//PHASE 1: Build the library to draw strings with a line-only font, no curves, such that characters will begin to look polygonal at scale.
//	-DONE: Draw strings with a line-only font that can scale according to pixel font size.
//	-TODO: Draw filled characters (not just outline)
//	-TODO: Finish one complete font: classic.rff
//	-TODO: Incorporate shift down for lowercase letters that extend below baseline
//	-TODO: Build tool for creating fonts and writing to .rff
//PHASE 2: Add a quadratic bezier curve function to handle TTF style fonts.
//	-TODO: Build draw function that utilizes bezier curves
//	-TODO: Add kerf, non-monospace
//	-TODO: Figure out how to import glyph data from ttf format.

#include <stdio.h>
#include <stdlib.h>
#include <string.h> //remove unneeded libraries when finished
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
//RF_printString flags
#define DEFAULT 0x00
#define OUTLINE 0x01
#define BOLD	0x02
#define	DASHED	0x04

//enum to define a character's type, to allow for toUpper type functions down the road.
enum RF_CharacterType {
	UPPERCASE,
	LOWERCASE,
	NUMBER,
	SYMBOL
};

//Line structure
struct RF_Line {
	SDL_FPoint a, b;
};

//The basic information for a quadratic bezier curve. The degrees of accuracy will be determined by scale.
struct RF_Quad_Bezier {
	SDL_FPoint a, b, c;
};

//The data needed  for a character is tricky. If it's just a set of points, the function won't know where to draw the lines, how to connect them.
//It can be a set of lines, but that will be limiting for rounded characters as they scale up. I need to define an  arc of some kind but 
//I don't want to do it  using  SDL_gfxPrimitives. So each letter can be defined by a series of draw instructions that consist of a line or an arc.
//A line is simple to define as two points. The arc will need to be defined as some sort of curve, like a bezier? Time to do some research. Ultimately
//it must reduce to a set of lines, but what changes is what happens when it scales. 
//That said, scaling functions probably shouldn't happen on the fly but shouldd get stored and drawn from the base data. So in practice there is a character
//template structure then a character at scale, ready to draw structure.
struct RF_Character {
	char name;
	int offset;	//offset into the font.characters[] array
	enum RF_CharacterType type;
	float width;  //width, particularly for non-monospace fonts
	float subshift; //amount to shift the start position of the top of the character, for instance with lowercase g. Maybe this isn't the best method.
	int total_segments;
	struct RF_Line *segments;   //array of line segments to be malloced by total_segments: segments[total_segments]
};

//So a character template is assembled from lines and curves. it could be all curves but delineating them will allow to use a simple line draw without
//the bezier calculation on straight line segments.
struct RF_CharacterTemplate {
	enum RF_CharacterType type;
	int total_lines;
	struct RF_Line *lines;
	int total_quad_beziers;
	struct RF_Quad_Bezier *quad_beziers;
};

//Use SDL structures when possible. SDL_FPoint, SDL_Rect, SDL_Color, SDL_Renderer, and eventually SDL_TextEditingEvent, though maybe that's higher level than this library's scope.
struct RF_Font {
	char *name;
	char *error_msg;
	int base_width;		//smallest allowable width. In an 8x12 or 8x8 font, this is 8. Fonts can then scale up but not down.
	int base_height;	//smallest allowable height. In an 8x12 font this is 12, in an 8x8 this is 8. Can scale up but not down.
	int total_characters;   //spell out "characters' to avoid confusion with C type char.
	//struct RF_CharacterTemplate *characters; //array of characters malloced by total_characters: characters[total_characters]
	struct RF_Character *characters;
	struct RF_Character *filled_chars;
	int lookup[256];	//ASCII lookup table + 128 indices for custom characters. Could be a char?
	float top_margin;		//Space above a line of text.
	float btm_margin;		//Space below a line of text.
};

//Struct to store pre-scaled font for common usage in application. (Same data structure as above)
struct RF_FontScaled {
	char *name;
	char *error_msg;
	float scalar;
	int character_width; //"font size" definition
	int character_height;
	int total_characters;
	struct RF_Character *characters_scaled;
	int lookup[256];
	float top_margin;
	float btm_margin;
};




//Function declarations
//Scope: Obviously need the draw functions but we also need the scale functions. Underneath both of those we need a function to calculate a quadratic bezier curve. Alongside that we need
//a function to check that all the resultant points in the curve are contained within the bounds of the letter box defined by "base_width" and "base_height" * scalar.

//Draw
//Receive a string and font and print to renderer. If font == NULL, search for a set default font.
//int RF_printString(SDL_Renderer *renderer, float x, float y, char *str, struct RF_FontScaled *font);     //Go-to print function once scaling is implemented
int RF_printString(SDL_Renderer *renderer, float x, float y, int size, char *str, struct RF_Font *font, int args);

//Set a default font for print operations. Late implementation.
//int RF_setDefaultFont(struct RF_FontScaled *font);

//Private: used by printString to draw each character in the string. At the draw stage, characters consist of only line segments for faster drawing.
//int RF_drawCharacter(SDL_Renderer *renderer, struct RF_Character *c, struct RF_FontScaled *font);
int RF_drawCharacter(SDL_Renderer *renderer, struct RF_Character *c, float scalar, float x, float y);
int RF_drawCharacterFilled(SDL_Renderer *renderer, struct RF_Character *c, float scalar, float x, float y, int height);

//Scale
//Returns a pointer to an array of lines derived from the template's bezier curve. PHASE 2.
	//struct RF_Line *RF_calcQuadBezierAtScale(struct RF_CharacterTemplate c, float scalar);

//Utility
//Function to display the contents of a font file to the console
//Systematically run through the file format and print the contents in a table: UPPERCASE, LOWERCASE, NUMBERS, SYMBOLS
struct RF_Font RF_loadFontFromFile(char *filename);
//struct RF_Font *RF_loadFontFromFileP(char *filename);
int RF_buildFilledCharacterArray(struct RF_Font *font);
SDL_FPoint RF_findIntersectionWithScanline(struct RF_Line scan, struct RF_Line target);
char *RF_removeFirstChar(char *s, int slen);
struct RF_Character RF_makeCharacterStructFromFile(char *buffer);
int RF_parseLinePointFromBufferToStruct(char *buffer, int *head, int *tail);
int RF_destroyFont(struct RF_Font *f);


//Draw functions
//Make sure to setRenderDrawColor() first
int RF_printString(SDL_Renderer *renderer, float x, float y, int size, char *str, struct RF_Font *font, int args){
	//RF_printStringOutline could get conslidated with printString as a flag.
	int width = font->base_width; //monospace for starters
	float scalar = (float)size/(float)width;
	float pos = 0;				//starting position
	for(int i=0; i<strlen(str); i++){
		if(str[i] == 32){
			pos += width * scalar;
		} else {
			int index = font->lookup[(int)str[i]];
			struct RF_Character *c;
				
			//transform x/y by scale and position in the string
			float tx = x + pos;
			float ty = y;		//transform by scalar, additionally transform by any up/down shifts (lowercase j etc.)
			if(args & OUTLINE)
				c = &font->characters[index];
			else
				c = &font->filled_chars[index];
			RF_drawCharacter(renderer, c, scalar, tx, ty);
			pos += width * scalar;			//When I get to varied width fonts, this should still work
		}
	}	
	return 0;
}

//Draw a single character given scalar and top-left position
int RF_drawCharacter(SDL_Renderer *renderer, struct RF_Character *c, float scalar, float x, float y){
	for(int i=0; i<c->total_segments; i++){
		float x1 = (scalar * c->segments[i].a.x) + x;
		float y1 = (scalar * c->segments[i].a.y) + y;
		float x2 = (scalar * c->segments[i].b.x) + x;
		float y2 = (scalar * c->segments[i].b.y) + y;
		SDL_RenderLine(renderer, x1, y1, x2, y2);
	}	
	return 0;
}


int RF_drawCharacterFilled(SDL_Renderer *renderer, struct RF_Character *c, float scalar, float x, float y, int height){
	//How to  draw filled character....hmmmm.

	return 0;
}

//Utility
//
//Parse and load .rff file into memory
struct RF_Font RF_loadFontFromFile(char *filename){
	//It may serve better to handle this function with a pointer return.
	//The .rff file may contain an unknown amount of characters to put into the font, so i can either  scan the whole document and
	//add together all the line counts, or add some simple header information, which is what I'm going to do. Amount of characters per
	//section * amount of lines per character = size of each "type" section (upper/lower/number/symbol);
	struct RF_Font font;
	FILE *fp;
	char buffer[256];
	char type = 'h';			//To hold curent glyph type, determining where in the RF_Font struct to write the line information. Begin with head
								//
	char current_character;
	int segment_counter = 0;
	int total_lines = 0;		//Total number of line segments in the font. Pulled from header.
	int total_characters = 0;	//Total number of characters in the font. Pulled from header.
	int main_offset = 0;
	int line_count = 0;
	font.error_msg = "clear";
	fp = fopen(filename, "r");
	if(fp == NULL){
		printf("Error opening font file. Check filename and existence of file: %s\n", filename);
		font.error_msg = "Error opening font file. Check filename and existence of file";
		return font;
	}	
	
	fseek(fp, 0, SEEK_SET); //Just to be safe
	//First pass, malloc font.characters array with header info then populate with RF_Character structs.
	//Pass through file and collect names/line counts of all glyphs and build a table. use that table to
	//populate the font.characters array with structs, then on another pass, go in and populate each
	//font.characters.segments[] array with the line information.
	while(fgets(buffer, sizeof(buffer), fp) != NULL){
		//Ok, we should be reading our file format line by line until the end now.
		//Order of operations: 1) Look for // then delete them and all proceeding information
		//2) Read header
		//3) Look for char type
		//4) look for char itself and use proceeding number to set loop for reading line points
		size_t length = strlen(buffer);
		for(int i=0; i<length; i++){
			if(buffer[i] == '/' && buffer[i+1] == '/'){
				buffer[i] = '\0';
				break;
			}
		}
		
		if(buffer[0] != '\0'){   //skip logic if line is blank or comment-only
			switch(type){
				case 'h':{		//handle header info
					switch(buffer[0]){
						case '!':{
							break;
						}
						case 'n':{
							font.name = RF_removeFirstChar(buffer, strlen(buffer));
							break;	
						}
						case 'w':{
							//Consider adding error handling around potential "dirty" data in width line.
							char *endptr;
							char *cleanw = RF_removeFirstChar(buffer, strlen(buffer));
							long width = strtol(cleanw, &endptr, 10);   //Convert from string to long. strtol(source str, pointer to end of string (qa feat), number base (decimal));
							font.base_width = (int)width;
							free(cleanw);
							break;
						}
						case 'h':{
							//Consider adding error handling around potential "dirty" data in height line.
							char *endptr;
							char *cleanh = RF_removeFirstChar(buffer, strlen(buffer));
							long height = strtol(cleanh, &endptr, 10);   //Convert from string to long. strtol(source str, pointer to end of string (qa feat), number base (decimal));
							font.base_height = (int)height;
							free(cleanh);
							break;
						}						
						case 't':{
							char *endptr;
							char *cleant = RF_removeFirstChar(buffer, strlen(buffer));
							long total_ch = strtol(cleant, &endptr, 10);
							font.total_characters = (int)total_ch;	
							total_characters = (int)total_ch;
							free(cleant);
							break;
						}
						case 'l':{
							char *endptr;
							char *cleanl = RF_removeFirstChar(buffer, strlen(buffer));
							long total_li = strtol(cleanl, &endptr, 10);
							total_lines = (int)total_li;
							//End of Header: malloc the size of the array. This should be a pretty exact science.
							//Is it a normal practice to add a little buffer on the end to prevent overflow?
							font.characters = malloc((total_lines * sizeof(struct RF_Line)) + (font.total_characters * sizeof(int)) + (font.total_characters * sizeof(enum RF_CharacterType)));
							free(cleanl);
							break;
						}
						default:
							break;
					}	
					break;
				}
				case 'u':{		//handle uppercase letters
					//Check for new letter (A12, ...), skip line info until next pass
					if(buffer[0] == '>'){
						font.characters[main_offset] = RF_makeCharacterStructFromFile(buffer);
						font.characters[main_offset].type = UPPERCASE;
						font.characters[main_offset].offset = main_offset;
						font.lookup[font.characters[main_offset].name] = main_offset; //Add current characters offset to the ascii lookup table
						main_offset++;
					}
					break;
				}
				case 'l':{
					if(buffer[0] == '>'){
						font.characters[main_offset] = RF_makeCharacterStructFromFile(buffer);
						font.characters[main_offset].type = LOWERCASE;
						font.characters[main_offset].offset = main_offset;
						font.lookup[font.characters[main_offset].name] = main_offset; //Add current characters offset to the ascii lookup table
						main_offset++;
					}
					break;
				}
				case 'n':{
					if(buffer[0] == '>'){
						font.characters[main_offset] = RF_makeCharacterStructFromFile(buffer);
						font.characters[main_offset].type = NUMBER;
						font.characters[main_offset].offset = main_offset;
						font.lookup[font.characters[main_offset].name] = main_offset; //Add current characters offset to the ascii lookup table
						main_offset++;
					}
					break;
				}
				case 's':{
					if(buffer[0] == '>'){
						font.characters[main_offset] = RF_makeCharacterStructFromFile(buffer);
						font.characters[main_offset].type = SYMBOL;
						font.characters[main_offset].offset = main_offset;
						font.lookup[font.characters[main_offset].name] = main_offset; //Add current characters offset to the ascii lookup table
						main_offset++;
					}
					break;
				}
				default:{
					font.error_msg = "Type error while building font. Corrupted rff file may result in partial data.";
					break;
				}
			}
			if(buffer[0] == '!'){
				type = buffer[1];
			}
		}
	}

	//Second pass, populate the RF_Character.segments[] arrays.
	fseek(fp, 0, SEEK_SET); //Just to be safe
	type = 'h';
	main_offset = 0;	//Offset into font.characters[]
	line_count = font.characters[main_offset].total_segments;		//loop counter for iterating through line writes
	font.characters[main_offset].segments = malloc(sizeof(struct RF_Line) * line_count);	//Must free() in loop before free()ing font.characters[].																	//
	while(fgets(buffer, sizeof(buffer), fp) != NULL){
		//First, remove comments
		size_t length = strlen(buffer);
		for(int i=0; i<length; i++){
			if(buffer[i] == '/' && buffer[i+1] == '/'){
				buffer[i] = '\0';
				break;
			}
		}
		//Next, set type. Actually this is superfluous.
		if(buffer[0] == '!'){
			type = buffer[1];
		}
		//parse a line if we are not in the header, not on an empty line, and not on a character header line
		//We will keep track of our progress using the information we grabbed on the first pass, namely
		//the .total_segments value in each RF_Character struct.
		if(buffer[0] != '\0' && buffer[0] != '!' && buffer[0] != '>' && type != 'h'){
			//Parse lines
			//A segment is one-per-line x1,y1,x2,y2,
			int head = 0;
			int tail = 0;
			char *endptr;
			char value[12];
			struct RF_Line segment;
			//Advance tail to end of first number
			segment.a.x = RF_parseLinePointFromBufferToStruct(buffer, &head, &tail);	//a line, x coord
			tail++;
			head = tail;
			segment.a.y = RF_parseLinePointFromBufferToStruct(buffer, &head, &tail);	//a line, y coord
			tail++;
			head = tail;
			segment.b.x = RF_parseLinePointFromBufferToStruct(buffer, &head, &tail);	//b line, x coord
			tail++;
			head = tail;
			segment.b.y = RF_parseLinePointFromBufferToStruct(buffer, &head, &tail);	//b line, y coord
			tail++;
			head = tail;
			font.characters[main_offset].segments[line_count-1] = segment;	//lines will be input to the struct in reverse order from the file because it shouldn't matter
			line_count--;
			if(line_count == 0){
				main_offset++;
				if(main_offset == total_characters)
					break;
				line_count = font.characters[main_offset].total_segments;
				//I was mallocing again here, but each character has already been malloced by total_segments in RF_makeCharacterStructFromFile
			}
			//If line_count is reached, advance offset and read new line count
		}	
	}
	RF_buildFilledCharacterArray(&font);

	return font; //This is now a pretty big struct. I should get the pointer version working.
}

int RF_buildFilledCharacterArray(struct RF_Font *font){
    //I will need to malloc font.filled_chars some how. Do I need to do a whole first pass just to find out how many segments, then a second
    //to fill them in?
    //Or use variable array size, remallocing as i go on pass one?
    //
    //start with an array size of 2 line segments per scanline. For 16 pt base font, full ascii set, this is only 128k once malloced by sizeof(RF_Line).
    int array_size = font->base_height * font->total_characters * 4; //DEBUG: If we get a stack overflow OR out-of-bounds, check here first
	
    font->filled_chars = malloc((array_size * sizeof(struct RF_Line)) + (256 * sizeof(struct RF_Character)));

    int rows = font->base_height;
    for(int i=0; i<font->total_characters; i++){    //character loop
        //figure out how many segments then malloc then fill array with RF_Lines
        //Create scratch array the size of the letter. Then populate with edge points. Do the scanline fill. Return the edge points on each scanline as segments.
        //Ok, I just looked up what ttf does and it's similar to above. Grid fit with hinting then scanline/winding number fill.
        //I'm going to try to circumvent this with intercepts. Here goes nothing.
        struct RF_Character c = font->characters[i];
		//Malloc filled_char[i]
		struct RF_Character filled;
		filled.name = font->characters[i].name;
		filled.offset = font->characters[i].offset;
		filled.type = font->characters[i].type;
		filled.segments = calloc(c.total_segments * rows, sizeof(struct RF_Line));
		font->filled_chars[i] = filled;

        int seg_index = 0;
		int toggle = 1;
        for(int j=0; j<rows; j++){      //scanline loop
			SDL_FPoint *edges = calloc(2 * c.total_segments, sizeof(SDL_FPoint));
			if(edges == NULL){
				font->error_msg = "Could not allocate edge point array.\n";
				return 1;
			}

            struct RF_Line scan = {0, j, font->base_width, j};  //testing scanline
            int index = 0;
            for(int k=0; k<c.total_segments; k++){  //segment loop
                SDL_FPoint p = RF_findIntersectionWithScanline(scan, c.segments[k]);
                if(p.x == -1){  //colinear
                    edges[index] = c.segments[k].a;
                    index++;
                    edges[index] = c.segments[k].b;
                    index++;
                } else if(p.x != -2) {      //If there is a result
                    edges[index] = p;
                    index++;
                }
            }
            if(index > 1){      //I think there is an edge case where only one point is given, in which case, don't bother drawing, it will should get filled by the outline.
                for(int k=0; k<index-1; k++){
                    if(toggle == 1){
                        struct RF_Line l;
                        l.a = edges[k];
                        l.b = edges[k+1];
                        font->filled_chars[i].segments[seg_index] = l;
                        seg_index++;
                        toggle = 0;
                    } else
                        toggle = 1;
                }
            }
 			free(edges);
        }
        font->filled_chars[i].total_segments = seg_index;
    }
    return 0;
}

//Specifically to find intercept with a horizontal scanline, not  for intercept of two lines.
//Returns x coordinate of intersection with that  segment, otherwise -2,-2 for no intersection or -1,-1 for colinear.
SDL_FPoint RF_findIntersectionWithScanline(struct RF_Line scan, struct RF_Line target){
    //Three possible returns: SDL_Point of intercept, no intercept, colinear.
    //y = mx + b ---> b = y - mx
    float ms = 0; //slope of scanline
    float bs = scan.a.y;    //y-intercept of horizontal line is any of its y-values
    float mt;
    float bt;
    float x_ret;
    SDL_FPoint result;
	if(target.a.x - target.b.x != 0){
		mt = (target.a.y - target.b.y) / (target.a.x - target.b.x); //if not vertical, grab slope
		bt = target.a.y - (mt * target.a.x);	//plug in values of a point on line to find y-intercept
	}
	else if((target.a.y < scan.a.y && target.b.y > scan.a.y) || (target.a.y > scan.a.y && target.b.y < scan.a.y)){   //Find out if the vertical segment crosses the scanline
		result.x = target.a.x;										//if vertical, intersection is at target.x, scan.y
		result.y = scan.a.y;
		return result;
	} else {
		result.x = -2;
		result.y = -2;
		return result;
	}
    if(ms == mt && bs == bt){
        result.x = -1;
        result.y = -1;
        return result; //colinear
    }
    if(ms == mt && bs != bt){
        result.x = -2;
        result.y = -2;
        return result; //parallel
    }
    //ms * (x) + bs = mt * (x) + bt --> ms(x) - mt(x) = bt - bs --> (bt - bs) / (ms - mt)
    x_ret = (bt - bs) / (ms - mt);
    //is x_ret within the bounds of the target line segment?
    int small_tx = target.a.x;
    int big_tx = target.b.x;
    if(small_tx > big_tx){
        int holder = big_tx;
        big_tx = small_tx;
        small_tx = holder;
    }
    if(x_ret < small_tx || x_ret > big_tx){
        result.x = -2;
        result.y = -2;
        return result;  //intersection is outside line segment
    }
    else{
        result.x = x_ret;
        result.y = (mt * x_ret) + bt;
        return result;
    }

}


//Internal function to RF_loadFontFromFile() specifically to load one line of text into an RF_Line struct and pass
//it back to the font.characters[n].segments[] array.
int RF_parseLinePointFromBufferToStruct(char *buffer, int *head, int *tail){
	//TODO: Right now this function is passing a correct first value, then only zeros after that
	//Something is wrong when it is called a second time, likely the head <-> tail relationship
	//isn't advancing properly.
	char value[12];
	char *endptr;
	while(buffer[*tail] != ',' && buffer[*tail] != '\0')
		(*tail)++;
	strncpy(value, buffer + *head, (*tail - *head));
	value[(*tail-*head)] = '\0';
	float p = strtof(value, &endptr);
	int point = (int)p;
	return point;
}

struct RF_Character RF_makeCharacterStructFromFile(char *buffer){
	struct RF_Character ch;
	ch.name = buffer[1];	//The name is always a single character because that's what each of these represents
	char *endptr;
	char *clean = RF_removeFirstChar(buffer, strlen(buffer));
	char *cleaner = RF_removeFirstChar(clean, strlen(clean));
	long tl = strtol(cleaner, &endptr, 10);
	ch.total_segments = (int)tl;
	printf("Total segments: %d\n", ch.total_segments);
	ch.segments = malloc(sizeof(struct RF_Line) * ch.total_segments);
	free(clean);
	free(cleaner);
	return ch;
}


//call to destroy RF_Font structure. Should free any mallocs within, including the character array.
int RF_destroyFont(struct RF_Font *f){
	//Order to destroy, 
	for(int i=0; i<f->total_characters; i++){
		free(f->characters[i].segments);
		free(f->filled_chars[i].segments);
	}	
	free(f->characters);
	free(f->filled_chars);
	free(f->name);
	return 0;
}

//Return a string without the first character, mainly for parsing formatting signals (! or >)
char *RF_removeFirstChar(char *s, int slen){
	char *result = malloc(slen * sizeof(char)); //I ought to be able to go -1 here, but at the cost of 1 byte, I kind of like the psychological wiggle room.
	for(int i=1; i<slen; i++){
		result[i-1] = s[i];
		result[i] = '\0';
	}
	return result; 				//Remember to free() results, usually in place when called if using a temp var.
}


#endif
