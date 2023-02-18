#define ACCEL 30
#define DECEL 50
#define GRAVITY 0x4c
#define MAX_SPEED 0x240
#define JUMP_VEL -0x600
#define MAX_RIGHT 0x9000



#pragma bss-name(push, "ZEROPAGE")

// GLOBAL VARIABLES
unsigned char pad1;
unsigned char pad1_new;
unsigned char collision;
unsigned char collision_L;
unsigned char collision_R;
unsigned char collision_U;
unsigned char collision_D;
unsigned char coordinates;
unsigned char temp1;
unsigned char temp2;
unsigned char temp3;
unsigned char temp4;
unsigned int temp5;
unsigned int temp6;
unsigned char eject_L; // from the left
unsigned char eject_R; // remember these from the collision sub routine
unsigned char eject_D; // from below
unsigned char eject_U; // from up
unsigned char direction; //facing left or right?
#define LEFT 0
#define RIGHT 1

int address;
unsigned char x; // room loader code
unsigned char y;
unsigned char nt;
unsigned char index;
unsigned char index2;
unsigned char room;
unsigned char map;
unsigned int scroll_x;
unsigned int scroll_y;
unsigned char scroll_count;
unsigned int pseudo_scroll_x;
//unsigned int pseudo_scroll_y;
unsigned char L_R_switch;
unsigned int old_x;
//unsigned char old_y;
unsigned char temp_x;
unsigned char temp_y;
unsigned char song;
#define MAX_SONGS 2
enum {SONG_GAME, SONG_PAUSE};
enum {SFX_JUMP, SFX_DING, SFX_NOISE};

unsigned char game_mode;
enum {MODE_GAME, MODE_PAUSE};

unsigned char coins;
const unsigned char * pointer;
unsigned char map_loaded; //only load it once

unsigned char temp_room;




#pragma bss-name(push, "BSS")

unsigned char c_map[240];
unsigned char c_map2[240];

struct Base {
	unsigned char x;
	unsigned char y;
	unsigned char width;
	unsigned char height;
};

struct Base Generic; 
struct Base Generic2; 

struct Hero {
	unsigned int x; // low byte is sub-pixel
	unsigned int y;
	signed int vel_x; // speed, signed, low byte is sub-pixel
	signed int vel_y;
};

struct Hero BoxGuy1 = {0x4000,0xc400}; // starting position
// the width and height should be 1 less than the dimensions (14x12)
// note, I'm using the top left as the 0,0 on x,y

#define HERO_WIDTH 13
#define HERO_HEIGHT 11


#define MAX_COINS 16
unsigned char coin_x[MAX_COINS];
unsigned char coin_y[MAX_COINS];
unsigned char coin_active[MAX_COINS];
unsigned char coin_room[MAX_COINS];
unsigned char coin_actual_x[MAX_COINS];

#define TURN_OFF 0xff
#define COIN_WIDTH 7
#define COIN_HEIGHT 11



#define MAX_ENEMY 16
unsigned char enemy_x[MAX_ENEMY];
unsigned char enemy_y[MAX_ENEMY];
unsigned char enemy_active[MAX_ENEMY];
unsigned char enemy_room[MAX_ENEMY];
unsigned char enemy_actual_x[MAX_ENEMY];

#define ENEMY_WIDTH 13
#define ENEMY_HEIGHT 13





const unsigned char palette_bg[]={
0x22, 0x16, 0x36, 0x0f,
0, 8, 0x18, 0x39,
0, 0, 0x10, 0x20,
0, 0x0a, 0x1a, 0x2a
}; 



const unsigned char palette_sp[]={
0x22, 0x01, 0x11, 0x10,
0x22, 0x17, 0x28, 0x38,
0x22, 0x05, 0x15, 0x35,
0x22, 0x0f, 0x00, 0x30,
}; 


// 5 bytes per metatile definition, tile TL, TR, BL, BR, palette 0-3
// T means top, B means bottom, L left,R right
// 51 maximum # of metatiles = 255 bytes
// 5th byte, 	1000 0000 = floor collision
// 				0100 0000 = all collision, solid
//				0000 0011 = palette

const unsigned char metatiles1[]={
	0, 0, 0, 0,  0,
	2, 2, 2, 2,  3,
	20, 20, 20, 20,  0,
	5, 6, 21, 22,  1,
	6, 6, 22, 22,  1,
	6, 7, 22, 23,  1,
	21, 22, 21, 22,  1,
	22, 22, 22, 22,  1,
	22, 23, 22, 23,  1,
	8, 9, 24, 25,  1,
	9, 9, 25, 25,  1,
	9, 10, 25, 26,  1
};

#define COL_DOWN 0x80
#define COL_ALL 0x40

const unsigned char is_solid[]={
	0,
	COL_DOWN,
	COL_ALL+COL_DOWN,
	COL_DOWN,
	COL_DOWN,
	COL_DOWN,
	0,
	0,
	0,
	0,
	0,
	0
};



#include "BG/Room1.c"
#include "BG/Room2.c"
#include "BG/Room3.c"
#include "BG/Room4.c"
#include "BG/Room5.c"

#define MAX_ROOMS (5-1)
#define MAX_SCROLL (MAX_ROOMS*0x100)-1
// data is exactly 240 bytes, 16 * 15
// doubles as the collision map data


const unsigned char * const Rooms[]= {
	Room1, Room2, Room3, Room4, Room5
};

//y, room, x
//y = TURN_OFF end of list
const unsigned char level_1_coins[]={
	0x61,0,0x65,
	0x81,1,0x85,
	0x41,2,0x95,
	0x61,3,0x75,
	0x41,4,0x85,
	TURN_OFF
};

//y, room, x
//y = TURN_OFF end of list
const unsigned char level_1_enemies[]={
	0xc2,0,0xb0,
	0xc2,1,0x80,
	0xc2,2,0xc0,
	0xc2,3,0xf0,
	TURN_OFF
};



// PROTOTYPES
void load_room(void);
void draw_sprites(void);
void movement(void);	
void draw_screen_R(void);
void new_cmap(void);
char bg_collision_sub(void);
char get_position(void);
void enemy_moves(void);
void sprite_collisions(void);
void check_spr_objects(void);
void sprite_obj_init(void);

char bg_coll_L(void);
char bg_coll_R(void);
char bg_coll_U(void);
char bg_coll_D(void);
char bg_coll_D2(void);
