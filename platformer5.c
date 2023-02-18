/*	example code for cc65, for NES
 *  Scrolling Right with metatile system
 *	, basic platformer
 *	, with coins and enemies
 *	using neslib
 *	Doug Fraker 2018
 */	
 
#include "LIB/neslib.h"
#include "LIB/nesdoug.h"
#include "Sprites.h" // holds our metasprite data
#include "platformer5.h"


	
	
void main (void) {
	
	ppu_off(); // screen off
	
	song = SONG_GAME;
	music_play(song);
	
	// load the palettes
	pal_bg(palette_bg);
	pal_spr(palette_sp);
	
	// use the second set of tiles for sprites
	// both bg and sprites are set to 0 by default
	bank_spr(1);
	
	set_vram_buffer(); // do at least once
	
	load_room();
	
	ppu_on_all(); // turn on screen
	
	

	
	while (1){
		// infinite loop
		while(game_mode == MODE_GAME){
			ppu_wait_nmi();
			
			set_music_speed(8);
		
			pad1 = pad_poll(0); // read the first controller
			pad1_new = get_pad_new(0);
			
			// there is a visual delay of 1 frame, so properly you should
			// 1. move user 2.check collisions 3.allow enemy moves 4.draw sprites
			movement();
			check_spr_objects(); // see which objects are on screen
			sprite_collisions();
			enemy_moves();
			
			// set scroll
			set_scroll_x(scroll_x);
			set_scroll_y(scroll_y);
			draw_screen_R();
			draw_sprites();
			
			if(pad1_new & PAD_START){
				game_mode = MODE_PAUSE;
				song = SONG_PAUSE;
				music_play(song);
				color_emphasis(COL_EMP_DARK);
			}
		}
		while(game_mode == MODE_PAUSE){
			ppu_wait_nmi();

			pad1 = pad_poll(0); // read the first controller
			pad1_new = get_pad_new(0);
			
			draw_sprites();
			
			if(pad1_new & PAD_START){
				game_mode = MODE_GAME;
				song = SONG_GAME;
				music_play(song);
				color_emphasis(COL_EMP_NORMAL);
			}
		}
		
	}
}




void load_room(void){
	set_data_pointer(Rooms[0]);
	set_mt_pointer(metatiles1); 
	for(y=0; ;y+=0x20){
		for(x=0; ;x+=0x20){
			address = get_ppu_addr(0, x, y);
			index = (y & 0xf0) + (x >> 4);
			buffer_4_mt(address, index); // ppu_address, index to the data
			flush_vram_update2();
			if (x == 0xe0) break;
		}
		if (y == 0xe0) break;
	}
	
	
	
	// a little bit in the next room
	set_data_pointer(Rooms[1]);
	for(y=0; ;y+=0x20){
		x = 0;
		address = get_ppu_addr(1, x, y);
		index = (y & 0xf0);
		buffer_4_mt(address, index); // ppu_address, index to the data
		flush_vram_update2();
		if (y == 0xe0) break;
	}
	
	// copy the room to the collision map
	// the second one should auto-load with the scrolling code
	memcpy (c_map, Rooms[0], 240);
	
	
	sprite_obj_init();
}




void draw_sprites(void){
	// clear all sprites from sprite buffer
	oam_clear();

	
	temp_x = high_byte(BoxGuy1.x);
	if(temp_x > 0xfc) temp_x = 1;
	if(temp_x == 0) temp_x = 1;
	// draw 1 hero
	if(direction == LEFT) {
		oam_meta_spr(temp_x, high_byte(BoxGuy1.y), RoundSprL);
	}
	else{
		oam_meta_spr(temp_x, high_byte(BoxGuy1.y), RoundSprR);
	}
	
	
	for(index = 0; index < MAX_COINS; ++index){
		temp_y = coin_y[index];
		if(temp_y == TURN_OFF) continue;
		if(get_frame_count() & 8) ++temp_y; // bounce the coin
		temp1 = coin_active[index];
		temp2 = coin_x[index];
		if(temp1 && (temp_y < 0xf0)) {
			oam_meta_spr(temp2, temp_y, CoinSpr);
		}
	}
	
	
	for(index = 0; index < MAX_ENEMY; ++index){
		temp_y = enemy_y[index];
		if(temp_y == TURN_OFF) continue;
		temp1 = enemy_active[index];
		temp2 = enemy_x[index];
		if(temp2 > 0xf0) continue;
		if(temp1 && (temp_y < 0xf0)) {
			oam_meta_spr(temp2, temp_y, EnemySpr);
		}
	}
	
	
	
	// draw "coins" at the top in sprites
	oam_meta_spr(16,16, CoinsSpr);
	temp1 = coins + 0xf0;
	oam_spr(64,16,temp1,3);
}
	

	
	
void movement(void){
	
// handle x

	old_x = BoxGuy1.x;
	
	if(pad1 & PAD_LEFT){
		direction = LEFT;
		
        if(BoxGuy1.vel_x >= DECEL){
            BoxGuy1.vel_x -= DECEL;
        }
        else if(BoxGuy1.vel_x > 0){
            BoxGuy1.vel_x = 0;
        }
		else {
			BoxGuy1.vel_x -= ACCEL;
			if(BoxGuy1.vel_x < -MAX_SPEED) BoxGuy1.vel_x = -MAX_SPEED;
		}
	}
	else if (pad1 & PAD_RIGHT){
		
		direction = RIGHT;

		if(BoxGuy1.vel_x <= DECEL){
            BoxGuy1.vel_x += DECEL;
        }
        else if(BoxGuy1.vel_x < 0){
            BoxGuy1.vel_x = 0;
        }
		else {
			BoxGuy1.vel_x += ACCEL;
			if(BoxGuy1.vel_x >= MAX_SPEED) BoxGuy1.vel_x = MAX_SPEED;
		}
	}
	else { // nothing pressed
		if(BoxGuy1.vel_x >= ACCEL) BoxGuy1.vel_x -= ACCEL;
		else if(BoxGuy1.vel_x < -ACCEL) BoxGuy1.vel_x += ACCEL;
		else BoxGuy1.vel_x = 0;
	}
	
	BoxGuy1.x += BoxGuy1.vel_x;
	
	if(BoxGuy1.x > 0xf000) { // too far, don't wrap around
        
        if(old_x >= 0x8000){
            BoxGuy1.x = 0xf000; // max right
        }
        else{
            BoxGuy1.x = 0x0000; // max left
        }
        
		BoxGuy1.vel_x = 0;
	} 
	
	Generic.x = high_byte(BoxGuy1.x); // this is much faster than passing a pointer to BoxGuy1
	Generic.y = high_byte(BoxGuy1.y);
	Generic.width = HERO_WIDTH;
	Generic.height = HERO_HEIGHT;
	
    if(BoxGuy1.vel_x < 0){
        if(bg_coll_L() ){ // check collision left
            high_byte(BoxGuy1.x) = high_byte(BoxGuy1.x) - eject_L;
            BoxGuy1.vel_x = 0;
            if(BoxGuy1.x > 0xf000) {
                // no wrap around
                BoxGuy1.x = 0xf000;
            }
        }
    }
    else if(BoxGuy1.vel_x > 0){
        if(bg_coll_R() ){ // check collision right
            high_byte(BoxGuy1.x) = high_byte(BoxGuy1.x) - eject_R;
            BoxGuy1.vel_x = 0;
            if(BoxGuy1.x > 0xf000) {
                // no wrap around
                BoxGuy1.x = 0x0000;
            }
        }
    }
    // skip collision if vel = 0

	
// handle y

// gravity

	// BoxGuy1.vel_y is signed
	if(BoxGuy1.vel_y < 0x300){
		BoxGuy1.vel_y += GRAVITY;
	}
	else{
		BoxGuy1.vel_y = 0x300; // consistent
	}
	BoxGuy1.y += BoxGuy1.vel_y;
	
	Generic.x = high_byte(BoxGuy1.x);
	Generic.y = high_byte(BoxGuy1.y);
	
    if(BoxGuy1.vel_y > 0){
        if(bg_coll_D() ){ // check collision below
            high_byte(BoxGuy1.y) = high_byte(BoxGuy1.y) - eject_D;
            BoxGuy1.y &= 0xff00;
            if(BoxGuy1.vel_y > 0) {
                BoxGuy1.vel_y = 0;
            }
        }
    }
    else if(BoxGuy1.vel_y < 0){
        if(bg_coll_U() ){ // check collision above
            high_byte(BoxGuy1.y) = high_byte(BoxGuy1.y) - eject_U;
            BoxGuy1.vel_y = 0;
        }
    }
    
	// check collision down a little lower than hero
	Generic.y = high_byte(BoxGuy1.y); // the rest should be the same
	
	if(pad1_new & PAD_A) {
        if(bg_coll_D2() ) {
			BoxGuy1.vel_y = JUMP_VEL; // JUMP
			sfx_play(SFX_JUMP, 0);
			//short_jump_count = 1;
		}
		
	}
	
	// do we need to load a new collision map? (scrolled into a new room)
	if((scroll_x & 0xff) < 4){
		if(!map_loaded){
			new_cmap();
			map_loaded = 1; // only do once
		}
		
	}
	else{
		map_loaded = 0;
	}
	
// scroll
	temp5 = BoxGuy1.x;
	if (BoxGuy1.x > MAX_RIGHT){
		temp1 = (BoxGuy1.x - MAX_RIGHT) >> 8;
        if (temp1 > 3) temp1 = 3; // max scroll change
		scroll_x += temp1;
		high_byte(BoxGuy1.x) = high_byte(BoxGuy1.x) - temp1;
	}

	if(scroll_x >= MAX_SCROLL) {
		scroll_x = MAX_SCROLL; // stop scrolling right, end of level
		BoxGuy1.x = temp5; // but allow the x position to go all the way right
		if(high_byte(BoxGuy1.x) >= 0xf1) {
			BoxGuy1.x = 0xf100;
		}
	}
}	




void enemy_moves(void){
	
	temp1 = high_byte(BoxGuy1.x);
	
	if(get_frame_count() & 0x01) return; // half speed
	
	for(index = 0; index < MAX_ENEMY; ++index){
		if(enemy_active[index]){
			if(enemy_x[index] > temp1){
				if(enemy_actual_x[index] == 0) --enemy_room[index];
				--enemy_actual_x[index];
			}
			else if(enemy_x[index] < temp1){
				++enemy_actual_x[index];
				if(enemy_actual_x[index] == 0) ++enemy_room[index];
			}
		}
	}
}




char bg_coll_L(void){
    // check 2 points on the left side
    temp5 = Generic.x + scroll_x;
    temp_x = (char)temp5; // low byte
    temp_room = temp5 >> 8; // high byte
    
    eject_L = temp_x | 0xf0;
    temp_y = Generic.y + 2;
    if(bg_collision_sub() & COL_ALL) return 1;
    
    temp_y = Generic.y + Generic.height;
    temp_y -= 2;
    if(bg_collision_sub() & COL_ALL) return 1;
    
    return 0;
}

char bg_coll_R(void){
    // check 2 points on the right side
    temp5 = Generic.x + scroll_x + Generic.width;
    temp_x = (char)temp5; // low byte
    temp_room = temp5 >> 8; // high byte
    
    eject_R = (temp_x + 1) & 0x0f;
    temp_y = Generic.y + 2;
    if(bg_collision_sub() & COL_ALL) return 1;
    
    temp_y = Generic.y + Generic.height;
    temp_y -= 2;
    if(bg_collision_sub() & COL_ALL) return 1;
    
    return 0;
}

char bg_coll_U(void){
    // check 2 points on the top side
    temp5 = Generic.x + scroll_x;
    temp5 += 2;
    temp_x = (char)temp5; // low byte
    temp_room = temp5 >> 8; // high byte
    
    temp_y = Generic.y;
    eject_U = temp_y | 0xf0;
    if(bg_collision_sub() & COL_ALL) return 1;
    
    temp5 = Generic.x + scroll_x + Generic.width;
    temp5 -= 2;
    temp_x = (char)temp5; // low byte
    temp_room = temp5 >> 8; // high byte
    
    if(bg_collision_sub() & COL_ALL) return 1;
    
    return 0;
}

char bg_coll_D(void){
    // check 2 points on the bottom side
    temp5 = Generic.x + scroll_x;
    temp5 += 2;
    temp_x = (char)temp5; // low byte
    temp_room = temp5 >> 8; // high byte
    
    temp_y = Generic.y + Generic.height;
    
    if((temp_y & 0x0f) > 3) return 0; // bug fix
    // so we don't snap to those platforms
    // don't fall too fast, or might miss it.
    
    eject_D = (temp_y + 1) & 0x0f;
    
    if(bg_collision_sub() ) return 1;
    
    temp5 = Generic.x + scroll_x + Generic.width;
    temp5 -= 2;
    temp_x = (char)temp5; // low byte
    temp_room = temp5 >> 8; // high byte
    
    if(bg_collision_sub() ) return 1;
    
    return 0;
}

char bg_coll_D2(void){
    // check 2 points on the bottom side
    // a little lower, for jumping
    temp5 = Generic.x + scroll_x;
    temp5 += 2;
    temp_x = (char)temp5; // low byte
    temp_room = temp5 >> 8; // high byte
    
    temp_y = Generic.y + Generic.height;
    temp_y += 2;
    if(bg_collision_sub() ) return 1;
    
    temp5 = Generic.x + scroll_x + Generic.width;
    temp5 -= 2;
    temp_x = (char)temp5; // low byte
    temp_room = temp5 >> 8; // high byte
    
    if(bg_collision_sub() ) return 1;
    
    return 0;
}




char bg_collision_sub(void){
    if(temp_y >= 0xf0) return 0;
    
	coordinates = (temp_x >> 4) + (temp_y & 0xf0);
    // we just need 4 bits each from x and y
	
	map = temp_room&1; // high byte
	if(!map){
		collision = c_map[coordinates];
	}
	else{
		collision = c_map2[coordinates];
	}
	
    return is_solid[collision];
}



void draw_screen_R(void){
	// scrolling to the right, draw metatiles as we go
	pseudo_scroll_x = scroll_x + 0x120;
	
	temp1 = pseudo_scroll_x >> 8;
	
	set_data_pointer(Rooms[temp1]);
	nt = temp1 & 1;
	x = pseudo_scroll_x & 0xff;
	
	switch(scroll_count){
		case 0:
			address = get_ppu_addr(nt, x, 0);
			index = 0 + (x >> 4);
			buffer_4_mt(address, index); // ppu_address, index to the data
			
			address = get_ppu_addr(nt, x, 0x20);
			index = 0x20 + (x >> 4);
			buffer_4_mt(address, index); // ppu_address, index to the data
			break;
			
		case 1:
			address = get_ppu_addr(nt, x, 0x40);
			index = 0x40 + (x >> 4);
			buffer_4_mt(address, index); // ppu_address, index to the data
			
			address = get_ppu_addr(nt, x, 0x60);
			index = 0x60 + (x >> 4);
			buffer_4_mt(address, index); // ppu_address, index to the data
			break;
			
		case 2:
			address = get_ppu_addr(nt, x, 0x80);
			index = 0x80 + (x >> 4);
			buffer_4_mt(address, index); // ppu_address, index to the data
			
			address = get_ppu_addr(nt, x, 0xa0);
			index = 0xa0 + (x >> 4);
			buffer_4_mt(address, index); // ppu_address, index to the data
			break;
			
		default:
			address = get_ppu_addr(nt, x, 0xc0);
			index = 0xc0 + (x >> 4);
			buffer_4_mt(address, index); // ppu_address, index to the data
			
			address = get_ppu_addr(nt, x, 0xe0);
			index = 0xe0 + (x >> 4);
			buffer_4_mt(address, index); // ppu_address, index to the data
	}


	++scroll_count;
	scroll_count &= 3; //mask off top bits, keep it 0-3
}




void new_cmap(void){
	// copy a new collision map to one of the 2 c_map arrays
	room = ((scroll_x >> 8) +1); //high byte = room, one to the right
	
	map = room & 1; //even or odd?
	if(!map){
		memcpy (c_map, Rooms[room], 240);
	}
	else{
		memcpy (c_map2, Rooms[room], 240);
	}
}




void sprite_collisions(void){

	Generic.x = high_byte(BoxGuy1.x);
	Generic.y = high_byte(BoxGuy1.y);
	Generic.width = HERO_WIDTH;
	Generic.height = HERO_HEIGHT;
	
	Generic2.width = COIN_WIDTH;
	Generic2.height = COIN_HEIGHT;
	
	for(index = 0; index < MAX_COINS; ++index){
		if(coin_active[index]){
			Generic2.x = coin_x[index];
			Generic2.y = coin_y[index];
			if(check_collision(&Generic, &Generic2)){
				coin_y[index] = TURN_OFF;
				sfx_play(SFX_DING, 0);
				++coins;
			}
		}
	}

	Generic2.width = ENEMY_WIDTH;
	Generic2.height = ENEMY_HEIGHT;
	
	for(index = 0; index < MAX_ENEMY; ++index){
		if(enemy_active[index]){
			Generic2.x = enemy_x[index];
			Generic2.y = enemy_y[index];
			if(check_collision(&Generic, &Generic2)){
				enemy_y[index] = TURN_OFF;
				sfx_play(SFX_NOISE, 0);
				if(coins) --coins;
			}
		}
	}
}




void check_spr_objects(void){
	// mark each object "active" if they are, and get the screen x
	
	for(index = 0; index < MAX_COINS; ++index){
		coin_active[index] = 0; //default to zero
		if(coin_y[index] != TURN_OFF){
			temp5 = (coin_room[index] << 8) + coin_actual_x[index];
			coin_active[index] = get_position();
			coin_x[index] = temp_x; // screen x coords
		}

	}
	

	for(index = 0; index < MAX_ENEMY; ++index){
		enemy_active[index] = 0; //default to zero
		if(enemy_y[index] != TURN_OFF){
			temp5 = (enemy_room[index] << 8) + enemy_actual_x[index];
			enemy_active[index] = get_position();
			enemy_x[index] = temp_x; // screen x coords
		}

	}

	
}



char get_position(void){
	// is it in range ? return 1 if yes
	
	temp5 -= scroll_x;
	temp_x = temp5 & 0xff;
	if(high_byte(temp5)) return 0;
	return 1;
}



// cc65 is very slow if 2 arrays/pointers are on the same line, so I
// broke them into 2 separate lines with temp1 as a passing variable
void sprite_obj_init(void){

	pointer = level_1_coins;
	for(index = 0,index2 = 0;index < MAX_COINS; ++index){
		
		coin_x[index] = 0;

		temp1 = pointer[index2]; // get a byte of data
		coin_y[index] = temp1;
		
		if(temp1 == TURN_OFF) break;

		++index2;
		
		coin_active[index] = 0;

		
		temp1 = pointer[index2]; // get a byte of data
		coin_room[index] = temp1;
		
		++index2;
		
		temp1 = pointer[index2]; // get a byte of data
		coin_actual_x[index] = temp1;
		
		++index2;
	}
	
	for(++index;index < MAX_COINS; ++index){
		coin_y[index] = TURN_OFF;
	}
	
	
	

	pointer = level_1_enemies;
	for(index = 0,index2 = 0;index < MAX_ENEMY; ++index){
		
		enemy_x[index] = 0;

		temp1 = pointer[index2]; // get a byte of data
		enemy_y[index] = temp1;
		
		if(temp1 == TURN_OFF) break;

		++index2;
		
		enemy_active[index] = 0;
		
		temp1 = pointer[index2]; // get a byte of data
		enemy_room[index] = temp1;
		
		++index2;
		
		temp1 = pointer[index2]; // get a byte of data
		enemy_actual_x[index] = temp1;
		
		++index2;
	}
	
	for(++index;index < MAX_ENEMY; ++index){
		enemy_y[index] = TURN_OFF;
	}
}






