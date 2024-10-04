// ISE test

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>
#include <ise.h>

typedef struct {
	ise_anim_object_t obj;
	int16_t vspeed;
	int16_t hspeed;
	int16_t min_vspeed;
	int16_t max_vspeed;
	int16_t max_hspeed;
	int hit_points;
	uint32_t restart_pos_x;
	uint32_t restart_pos_y;
	uint32_t restart_map_x;
	uint32_t restart_map_y;
} hero_t;

typedef struct {
	ise_anim_object_t obj;
	int16_t vspeed;
	int16_t hspeed;
	int16_t min_vspeed;
	int16_t max_vspeed;
	int16_t max_hspeed;
	uint32_t restart_pos_x;
	uint32_t restart_pos_y;
	int active_entry;
	int16_t restart_anim;
	bool anim_done;
	int hit_points;
} enemy_t;

void update_enemy_active(enemy_t* e, ise_gfx_t* gfx)
{
	const int ACTIVE_PAD = 128; // how far off screen to de-activate enemy
	ise_map_t* m = &(gfx->map);
	int screen_width, screen_height;
	screen_width = gfx->back->width;
	screen_height = gfx->back->height;

	int32_t x, y;
	//if(e->active_entry < 0) {
	//	x = e->restart_pos_x - m->origin_x;
	//	y = e->restart_pos_y - m->origin_y;
	//	if(x <= -ACTIVE_PAD || x >= 320+ACTIVE_PAD || y <= -ACTIVE_PAD || y >= 240+ACTIVE_PAD) {
	//		e->obj.pos_x = e->restart_pos_x;
	//		e->obj.pos_y = e->restart_pos_y;
	//	}
	//}
	x = e->obj.pos_x - m->origin_x;
	y = e->obj.pos_y - m->origin_y;
	bool new_active = (x > -ACTIVE_PAD && x < screen_width+ACTIVE_PAD && y > -ACTIVE_PAD && y < screen_height+ACTIVE_PAD);
	if(e->active_entry == -2 && new_active) {
		//objs[*num_objs] = &(e->obj);
		e->active_entry = 0;//*num_objs;
		e->obj.flags |= 0x1;
		//(*num_objs)++;
	}
	if(e->active_entry == -1 && !new_active) {
		e->active_entry = -2;  // if restart position is outside visible region, enemy i candidate to be respawned
	}
	if(e->active_entry >= 0 && !new_active) {
		//(*num_objs)--;
		//objs[e->active_entry] = objs[*num_objs];
		e->obj.flags &= ~0x1;
		e->obj.anim = e->restart_anim;
		e->active_entry = -1; // this puts enemy into restart position
		e->obj.pos_x = e->restart_pos_x;
		e->obj.pos_y = e->restart_pos_y;
		e->hit_points = 3;
	}
}

void set_assets_dir(char* exe_file)
{
    const char sub_dir[16] = "ASSETS";
    char buffer[256];
    //getcwd( buffer, sizeof(buffer) );
    //size_t buffer_len = strlen(buffer) + 1;  // add 1 for terminating null
    //size_t buffer_left = sizeof(buffer) - buffer_len;
    //size_t exe_file_len = strlen(exe_file);
    //size_t copy_len = (exe_file_len > buffer_left) ? buffer_left : exe_file_len;
    //strncat(buffer, exe_file, copy_len);

    // get full path to exe file
    _fullpath(buffer, exe_file, sizeof(buffer));

    // find and remove everything after the last 2 backslashes
    char *cp;
    const char *dp = sub_dir;
    cp = strrchr(buffer, '\\');
    if(cp) *cp = '\0';
    cp = strrchr(buffer, '\\');
    cp++;

    // concat the "assets" directory
    size_t copy_len = (size_t) (buffer + sizeof(buffer) - cp);
    int i;
    for(i=0; i<copy_len && *dp != '\0'; i++, cp++, dp++) {
        *cp = *dp;
    }
    if(i==copy_len) cp--;
    *cp = '\0';

    chdir(buffer);

    getcwd( buffer+1, sizeof(buffer)-1 );
    if(buffer[0] != buffer[1]) {
        unsigned int totals;
        _dos_setdrive( buffer[0] - 'A' + 1, &totals );
    }
}

void cleanup()
{
    ise_dsp_uninstall();
    ise_opl_uninstall();
#ifdef DOS32
    ise_usb_uninstall();
#endif
    ise_keybd_uninstall();
    ise_time_uninstall();
    chdir("..\\bin");
    //exit(0);
}

int main(int argc, char *argv[])
{
    ise_keybd_t keybd;
    ise_gfx_t gfx;

    if(argc > 0) set_assets_dir(argv[0]);

    ise_time_install();
    keybd.enable_default_isr = false;
    ise_keybd_install(&keybd);
	
	//ise_vga_timing_t vga_timing = {0};
	//vga_timing.width = 640;
	//vga_timing.height = 480;
	//vga_timing.refresh = 60;
	//ise_vga_calc_timing(&vga_timing);
	//printf("pix_clk: %d\n", vga_timing.pixel_clk);
	//printf("h: fporch: %d sync: %d bporch: %d\n", vga_timing.h_front_porch, vga_timing.h_sync_pulse, vga_timing.h_back_porch);
	//printf("v: fporch: %d sync: %d bporch: %d\n", vga_timing.v_front_porch, vga_timing.v_sync_pulse, vga_timing.v_back_porch);
	//cleanup();
	//exit(0);

	
	ise_vmode_t vmode;
	vmode.width = 640;
	vmode.height = 480;
	vmode.bpp = 8;
	ise_gfx_get_mode(&vmode);

	if(vmode.mode == (int16_t) 0x7FFF) {
		printf("Could not find mode\n");
		cleanup();
		exit(-1);
	}

#ifdef DOS32
    if(ise_pci_detect() == 0) {
        //int s, f;
        //uint32_t bar, cmd_sts, irq;
        printf("PCI bus found\n");
        ise_pci_scan();
        //for(s=0; s<ISE_PCI_MAX_SLOTS; s++) {
        //    for(f=0; f<ISE_PCI_MAX_FUNC; f++) {
        //        if(ise_pci.slot[s][f].vendor_id != 0xffff) {
        //            cmd_sts = ise_pci_read_config(0, s, f, 0x04);
        //            bar = ise_pci_read_config(0, s, f, 0x10);
        //            irq = ise_pci_read_config(0, s, f, 0x3C);
        //            printf("s/f 0x%x/0x%x class 0x%x bar 0x%x  cmd_sts 0x%x irq 0x%x\n",
        //            s, f, ise_pci.slot[s][f].class_code, bar, cmd_sts, irq);
        //            printf("\tvendor 0x%x device 0x%x ss_vendor 0x%x ss_device 0x%x\n",
        //            ise_pci.slot[s][f].vendor_id, ise_pci.slot[s][f].device_id,
        //            ise_pci.slot[s][f].subsystem_vendor_id, ise_pci.slot[s][f].subsystem_id);
        //        }
        //    }
        //}
        ise_usb_install();
    } else {
        printf("PCI bus not found\n");
    }
	    
    int opl = ise_opl_install();
    switch(opl) {
        case ISE_OPL_DEVICE_NOT_FOUND: printf("No OPL device found\n"); break;
        case ISE_OPL_DEVICE_OPL2:      printf("OPL2 device found\n"); break;
        case ISE_OPL_DEVICE_OPL3:      printf("OPL3 device found\n"); break;
    }
    ise_dsp_install();

    ise_midi_event_t* track = ise_midi_load_track("mario1.mid");
    ise_dsp_pcm_t wav_jump, wav_swoosh, wav_punch;
    ise_dsp_load_wav_file("jump.wav", &wav_jump);
	ise_dsp_load_wav_file("swoosh.wav", &wav_swoosh);
	ise_dsp_load_wav_file("jab.wav", &wav_punch);

	//printf("paddr: 0x%x\n", ise_mem_va_to_pa((void*) &track));

	//ise_vga_get_timing();

    //ise_gfx_init(&gfx, &vmode, 15, 0);
    //ise_gfx_clear(gfx.front, 0x07);
	//*((uint16_t*) ise_vram_base) = 0xFFFF;
	//*((uint16_t*) ise_vram_base + 641) = 0xF800;
	//*((uint16_t*) ise_vram_base + 1023) = 0xFFFF;
	//*((uint16_t*) ise_vram_base + 1025) = 0x07E0;
	//*((uint16_t*) ise_vram_base + (599*1024)) = 0xF800;
	//*((uint16_t*) ise_vram_base + (599*1024) + 799) = 0x07E0;
	//ise_opl_play_track(track);
    //ise_dsp_play_track(track);
    //ise_dsp_play_pcm(&pcm);
	//bool first_time = true;
    //while(keybd.key[0x01] == 0x0) {
	//	if(first_time) { while(keybd.key[0x01] == 0x0) { ise_dsp_tick(); ise_gfx_swap_buffers(&gfx); } keybd.key[0x01] = 0x0; first_time = false; }
    //    ise_usb_tick();  // rescan ports
    //    ise_dsp_tick();
    //    if(track) ise_opl_tick();
    //    delay(16);
    //    ise_time_wait(16);//delay(1); // wait a little
    //    if(keybd.key[0x11]) ise_dsp_play_pcm(&pcm);
    //}
    //keybd.key[0x01] = 0;

	//vmode.mode = ISE_VGA_MODE_80X25;
    //ise_gfx_init(&gfx, &vmode, 0, 0);

	//ise_shader_t vs;
	//ise_load_shader_from_file("vs.fxo", &vs);
	//ise_mem_dump(NULL, vs.instructions, vs.num_instructions);
	//ise_free_shader(&vs);

	//printf("vmode - w: %d h: %d bpp: %d a: 0x%x\n", vmode.width, vmode.height, vmode.bpp, vmode.attributes);
	//printf("addr: 0x%x  pitch: %d ctrl: 0x%x\n", vmode.framebuffer, gfx.front->pitch, ise_svga_ctrl_base);
	//printf("vram size: 0x%x\n", ise_vram_size_64k);
    //cleanup();
    //return 0;

#endif

    ise_gfx_init(&gfx, &vmode, 15, 0);
    gfx.bg_color = 0xf8;

    ise_gfx_load_map(&gfx, "test.lvl");

	ise_anim_set_t anim_knight, anim_zombie1, anim_zombie2;
	chdir("knight");
	ise_load_anim_set(&gfx, &anim_knight, "knight.ani");
	chdir("..\\zombie1");
	ise_load_anim_set(&gfx, &anim_zombie1, "zombie1.ani");
	chdir("..\\zombie2");
	ise_load_anim_set(&gfx, &anim_zombie2, "zombie2.ani");
	
	const int NUM_ZOMBIES = 8;
	hero_t knight;
	enemy_t zombie[NUM_ZOMBIES];
	uint32_t num_objs;
	ise_anim_object_t* objs[NUM_ZOMBIES+1] = {NULL};


	knight.obj.anim = 0;
	knight.obj.frame = 0;
    knight.obj.frame_time = 0;
	knight.obj.flags = 1;
    knight.restart_pos_x = knight.obj.pos_x = 100;
    knight.restart_pos_y = knight.obj.pos_y = 580;
	knight.restart_map_x = gfx.map.origin_x;
	knight.restart_map_y = gfx.map.origin_y;
    knight.obj.anim_sprite = anim_knight.anim_sprite;
	knight.obj.frac_pos_x = 0;
	knight.obj.frac_pos_y = 0;
	knight.vspeed = 0;
	knight.hspeed = 0;
	knight.min_vspeed = -8400;
	knight.max_vspeed = 8400;
	knight.max_hspeed = 6300;
	knight.hit_points = 5;
	
	zombie[0].obj.frame = 0;
    zombie[0].obj.frame_time = 0;
	zombie[0].obj.flags = 0;
    zombie[0].obj.anim_sprite = anim_zombie1.anim_sprite;
	zombie[0].obj.frac_pos_x = 0;
	zombie[0].obj.frac_pos_y = 0;
	zombie[0].restart_anim = 1;
    zombie[0].restart_pos_x = 500;
    zombie[0].restart_pos_y = 581;
	zombie[0].vspeed = 0;
	zombie[0].hspeed = 0;
	zombie[0].min_vspeed = -8400;
	zombie[0].max_vspeed = 8400;
	zombie[0].max_hspeed = 7200;
	zombie[0].active_entry = -1;
	zombie[0].anim_done = false;
	zombie[0].hit_points = 3;
	
	int i, j;
	for(i=0; i<NUM_ZOMBIES; i++) zombie[i] = zombie[0];

	zombie[0].restart_anim = 1;
    zombie[1].restart_pos_x = 600;
    zombie[1].restart_pos_y = 160;
	zombie[2].restart_anim = 1;
    zombie[2].restart_pos_x = 700;
    zombie[2].restart_pos_y = 628;
    zombie[2].obj.anim_sprite = anim_zombie2.anim_sprite;
	zombie[3].restart_anim = 1;
    zombie[3].restart_pos_x = 800;
    zombie[3].restart_pos_y = 160;
	zombie[4].restart_anim = 1;
    zombie[4].restart_pos_x = 900;
    zombie[4].restart_pos_y = 160;
	zombie[5].restart_anim = 1;
    zombie[5].restart_pos_x = 1000;
    zombie[5].restart_pos_y = 160;
    zombie[5].obj.anim_sprite = anim_zombie2.anim_sprite;
	zombie[6].restart_anim = 0;
    zombie[6].restart_pos_x = 800;
    zombie[6].restart_pos_y = 612;
    zombie[6].obj.anim_sprite = anim_zombie2.anim_sprite;
	zombie[7].restart_anim = 0;
    zombie[7].restart_pos_x = 900;
    zombie[7].restart_pos_y = 612;
	
	num_objs = 9;
	objs[0] = &knight.obj;
	for(i=0; i<NUM_ZOMBIES; i++) {
		zombie[i].obj.anim = zombie[i].restart_anim;
		zombie[i].obj.pos_x = zombie[i].restart_pos_x;
		zombie[i].obj.pos_y = zombie[i].restart_pos_y;
		objs[i+1] = &(zombie[i].obj);
	}
	
    ise_fast_sprite_t im;
    ise_load_sprite(&im.sprite, "Idle1.bmp");
	chdir("..");

    ise_gfx_upload_fast_sprite(&gfx, &im);
    ise_vfree_all(&gfx.assets);

	ise_gfx_set_map_origin(&gfx, knight.restart_map_x, knight.restart_map_y);
    //ise_draw_map(&gfx.map, gfx.front, 0, 0, gfx.bg_color, ISE_MASK_NONE);
    //ise_draw_map(&gfx.map, gfx.back,  0, 0, gfx.bg_color, ISE_MASK_NONE);

    //while(keybd.key[0x01] == 0x0) ;
    //keybd.key[0x01] = 0;

    //ise_gfx_update_display(&gfx);

    uint32_t last_time = ise_time_get_time();
    int32_t delta_time;
    int32_t accel_x, accel_y;
	uint32_t collision = 0, last_collision = 0;
	bool on_ground = false;
	int scroll_x, scroll_y;
    //ise_opl_play_track(track);
    ise_dsp_play_track(track);
	//int plane;
	//for(plane=0; plane<4; plane++) {
	//	ise_set_plane(plane);
	//	ise_draw_sprite(&im.sprite, gfx.back,  100, 100, 0, -1, plane);
	//	ise_draw_sprite(&im.sprite, gfx.front, 100, 100, 0, -1, plane);
	//}
	bool jump_held = false;
	bool anim_done = false, attacking = false, attack_start = false, hit = false;
	int unhittable_time = 0;
	int flash_time = 0;
	const int FLASH_DELAY = 75;
	const int KEY_QUIT = 0x01; // Escape
	const int KEY_JUMP = 0x39; // Space
	const int KEY_ATTACK = 0x1d; // L Ctrl
	const int KEY_LEFT = 0xcb; // left arrow
	const int KEY_RIGHT = 0xcd; // right arrow
	uint32_t action = 0x0;
	const uint32_t ACTION_QUIT = 0x1;
	const uint32_t ACTION_JUMP = 0x2;
	const uint32_t ACTION_ATTACK = 0x4;
	const uint32_t ACTION_LEFT = 0x8;
	const uint32_t ACTION_RIGHT = 0x10;
    while((action & ACTION_QUIT) == 0x0) {  // Escape
		action = 0x0;
		if(keybd.key[KEY_QUIT]) action |= ACTION_QUIT;
		if(keybd.key[KEY_JUMP]) action |= ACTION_JUMP;
		if(keybd.key[KEY_ATTACK]) action |= ACTION_ATTACK;
		if(keybd.key[KEY_LEFT]) action |= ACTION_LEFT;
		if(keybd.key[KEY_RIGHT]) action |= ACTION_RIGHT;
		if(ise_usb_num_hid_interfaces) {
			ise_usb_hid_interface_t* hid_interface = ise_usb_hid_interface[0];
			int x = ise_usb_hid_get_control(hid_interface, ISE_USB_HID_USAGE_DESKTOP_X);
			uint32_t buttons = ise_usb_hid_get_buttons(hid_interface);
			if(x >= 192) action |= ACTION_RIGHT;
			if(x < 64) action |= ACTION_LEFT;
			if(buttons & 0x2) action |= ACTION_JUMP;
			if(buttons & 0x1) action |= ACTION_ATTACK;
			if(buttons & 0x200) action |= ACTION_QUIT;
		}

		//ise_clear_anim_objects(&gfx, num_objs, objs);
		for(i=0; i<NUM_ZOMBIES; i++) {
			update_enemy_active(&(zombie[i]), &gfx);
		}

		accel_x = 0; accel_y = 0;
		on_ground = (collision & 0x4) || (last_collision & 0x4);

        delta_time = ise_time_get_delta(&last_time);
        if(delta_time < 0) delta_time = 0;
		if(unhittable_time > 0) {
			unhittable_time -= delta_time;
			flash_time += delta_time;
			if(unhittable_time <= 0) {
				unhittable_time = 0;
				flash_time = 0;
				knight.obj.flags |= 0x1;
				if(knight.hit_points == 0) {
					knight.hit_points = 5;
					knight.obj.pos_x = knight.restart_pos_x;
					knight.obj.pos_y = knight.restart_pos_y;
					ise_gfx_set_map_origin(&gfx, knight.restart_map_x, knight.restart_map_y);
				}
			} else {
				if(flash_time >= FLASH_DELAY) {
					flash_time -= FLASH_DELAY;
					if(knight.hit_points > 0 && knight.obj.flags & 1) knight.obj.flags &= ~0x1; else knight.obj.flags |= 0x1;
				}
			}
		}

		knight.max_vspeed = 12600 + abs(knight.hspeed)/2;
		knight.min_vspeed = -knight.max_vspeed;
        if((action & ACTION_JUMP) && !jump_held && on_ground && !hit) {
			knight.vspeed = -knight.max_vspeed;//(-150 * delta_time) >> 8;
			ise_dsp_play_pcm(&wav_jump);
		} else {
			if(!(action & ACTION_JUMP)) {
				//ise_dsp_stop_pcm();
				if(knight.vspeed < -1000) knight.vspeed = -1000;
			}
			accel_y = (7200 * delta_time) >> 8;
		}
		jump_held = ((action & ACTION_JUMP) != 0x0);
        //else if(keybd.key[0x1f]) accel_y = (150 * delta_time) >> 8;
		//else accel_y = -knight.vspeed;
		attack_start = false;
		if(attacking && anim_done) attacking = false;
		if(hit && anim_done && knight.hit_points > 0) hit = false;
		if((action & ACTION_ATTACK) && !attacking && !hit) {
			attacking = true;
			attack_start = true;
			ise_dsp_play_pcm(&wav_swoosh);
		}
        if((action & ACTION_LEFT) && !attacking && !hit) accel_x = ((on_ground ? -3600 : -1200) * delta_time) >> 8;
        else if((action & ACTION_RIGHT) && !attacking && !hit) accel_x = ((on_ground ? 3600 : 1200) * delta_time) >> 8;
		else if(on_ground) {
			accel_x = (2400 * delta_time) >> 8;
			if(accel_x > abs(knight.hspeed)) accel_x = abs(knight.hspeed);
			if(knight.hspeed > 0) accel_x = -accel_x;
		}

		for(i=0; i<NUM_ZOMBIES; i++) {
			if(zombie[i].active_entry >= 0) {
				if((zombie[i].obj.anim & ~1) != 2 && zombie[i].anim_done && zombie[i].hit_points > 0) {
					ise_set_animation(&(zombie[i].obj), 2 | !(zombie[i].obj.anim & 1), true);
					zombie[i].hspeed = (zombie[i].obj.anim & 1) ? -3200 : 3200;
				}
				zombie[i].vspeed += (7200 * delta_time) >> 8;
				if(zombie[i].vspeed < zombie[i].min_vspeed) zombie[i].vspeed = zombie[i].min_vspeed;
				if(zombie[i].vspeed > zombie[i].max_vspeed) zombie[i].vspeed = zombie[i].max_vspeed;
				zombie[i].obj.vec_x = zombie[i].hspeed * delta_time;
				zombie[i].obj.vec_y = zombie[i].vspeed * delta_time;
			}
		}
		for(i=0; i<NUM_ZOMBIES; i++) {
			if(zombie[i].active_entry >= 0) {
				uint32_t zombie_collision = 0;
				uint32_t knight_collision = ise_check_obj_collision(&(knight.obj), &(zombie[i].obj));
				uint16_t ledge_depth = 2;
				if(knight_collision && zombie[i].hit_points > 0) {
					if((knight_collision & 3) == (1 << (knight.obj.anim & 1)) && attacking && unhittable_time == 0) {
						if((zombie[i].obj.anim & ~0x1) != 4) {
							zombie[i].hit_points--;
							if(zombie[i].hit_points == 0) {
								ise_set_animation(&(zombie[i].obj), 6 | (zombie[i].obj.anim & 1), true);
								zombie[i].hspeed = 0;
								zombie[i].vspeed = 0;
							} else {
								ise_set_animation(&(zombie[i].obj), 4 | !(knight.obj.anim & 1), true);
								zombie[i].hspeed = (knight.obj.anim & 1) ? -7200 : 7200;
								zombie[i].vspeed = -7000;
							}
						}
					} else if(!hit && unhittable_time == 0) {
						knight.hit_points--;
						hit = true;
						unhittable_time = 5000;
						if(knight.hit_points <= 0) {
							knight.hit_points = 0;
							ise_set_animation(&(knight.obj), 14 | !(knight_collision & 1), true);
						} else {
							ise_set_animation(&(knight.obj), 12 | !(knight_collision & 1), true);
							ise_dsp_play_pcm(&wav_punch);
							knight.hspeed = (knight_collision & 1) ? -7200 : 7200;
							knight.vspeed = -7000;
						}
					}
				}
				if((zombie[i].obj.anim & ~0x1) == 4) ledge_depth = 0;
				zombie_collision = ise_check_map_collision(&gfx.map, &(zombie[i].obj), ledge_depth);
				for(j=0; j<NUM_ZOMBIES; j++) {
					if((i!=j) && zombie[j].active_entry >= 0) {
						zombie_collision |= ise_check_obj_collision(&(zombie[i].obj), &(zombie[j].obj));
					}
				}
				ise_move_obj(&(zombie[i].obj));
				if((zombie_collision & 0x3) == (0x1 << (zombie[i].obj.anim & 1)) || (zombie_collision & 0x40)) {
					if(zombie[i].hit_points > 0) {
						ise_set_animation(&(zombie[i].obj), zombie[i].obj.anim & 1, true);
						zombie[i].hspeed = 0;
					}
				}
				zombie[i].anim_done = ise_animate_object(&(zombie[i].obj), delta_time);
			}
		}
		knight.hspeed += accel_x;
		knight.vspeed += accel_y;
		if(abs(knight.hspeed) > knight.max_hspeed) knight.hspeed = (knight.hspeed < 0) ? -knight.max_hspeed : knight.max_hspeed;
		if(knight.vspeed < knight.min_vspeed) knight.vspeed = knight.min_vspeed;
		if(knight.vspeed > knight.max_vspeed) knight.vspeed = knight.max_vspeed;
		knight.obj.vec_x = knight.hspeed * delta_time;
		knight.obj.vec_y = knight.vspeed * delta_time;

		last_collision = collision;
		collision = ise_check_map_collision(&gfx.map, &(knight.obj), 0);

		ise_move_obj(&(knight.obj));
		if(collision & 0x3) knight.hspeed = 0;
		if(collision & 0xc) knight.vspeed = 0;
		if(!hit) {
			int knight_move_x = knight.hspeed;
			int knight_move_y = knight.vspeed;
			uint16_t x_dir_anim = (knight_move_x == 0) ? (knight.obj.anim & 1) : (knight_move_x < 0);
			if(knight_move_y == 0) {
				if(attacking) ise_set_animation(&(knight.obj), 8 | x_dir_anim, attack_start);
				else ise_set_animation(&(knight.obj), ((knight_move_x == 0) ? 0 : 2) | x_dir_anim, true);
			} else {
				if(attacking) ise_set_animation(&(knight.obj), 10 | x_dir_anim, attack_start);
				else ise_set_animation(&(knight.obj), ((knight_move_y < 0) ? 4 : 6) | x_dir_anim, true);
			}
		}
		const int SCROLL_PAD = 64;  // how close to edge of screen to scroll
		scroll_x = (knight.obj.pos_x - gfx.map.origin_x) - SCROLL_PAD;
		if(scroll_x >= 0) {
			scroll_x = SCROLL_PAD+64 - (gfx.back->width - (knight.obj.pos_x - gfx.map.origin_x));
			if(scroll_x < 0) scroll_x = 0;
		}
		scroll_y = (knight.obj.pos_y - gfx.map.origin_y) - SCROLL_PAD;
		if(scroll_y >= 0) {
			scroll_y = SCROLL_PAD+64 - (gfx.back->height - (knight.obj.pos_y - gfx.map.origin_y));
			if(scroll_y < 0) scroll_y = 0;
		}
		anim_done = ise_animate_object(&(knight.obj), delta_time);
        
		ise_gfx_scroll(&gfx, scroll_x, scroll_y);
		ise_draw_anim_objects(&gfx, num_objs, objs);
		gfx.bg_color = 0x0;
		ise_draw_text(&gfx, 10, 10, "Knightmare", ISE_XFORM_SHADOW);
		gfx.bg_color = 0xf8;
		//for(plane=0; plane<4; plane++) {
		//	ise_set_plane(plane);
		//	//ise_draw_sprite(&im.sprite, gfx.back,  100, 100, 0, -1, plane);
		//	for(i=0; i<NUM_ZOMBIES; i++)
		//		if(zombie[i].active) ise_draw_anim_object(&(zombie[i].obj), gfx.back, &gfx.map, -1, plane);
		//	ise_draw_anim_object(&knight, gfx.back, &gfx.map, -1, plane);
		//}
        ise_usb_tick();
        ise_dsp_tick();
        ise_opl_tick();
        //if(keybd.key[0x10]) ise_dsp_play_pcm(&pcm);
        ise_gfx_swap_buffers(&gfx);
        /*key = kbhit();
        if(key) {
            key = getch();
            if(key == 0 || key == 224) key = 256 + getch();
            switch(key) {
            case 0x148: // Up
                ise_gfx_scroll(&gfx, 0, -1);
                break;
            case 0x150: // Down
                ise_gfx_scroll(&gfx, 0, 1);
                break;
            case 0x14b: // Left
                ise_gfx_scroll(&gfx, -1, 0);
                break;
            case 0x14d: // Right
                ise_gfx_scroll(&gfx, 1, 0);
                break;
            }
            ise_gfx_swap_buffers(&gfx);
            ise_gfx_update_display(&gfx);
        }*/
        //sleep(1);
    }
    
	cleanup();

	ise_dsp_free_pcm(&wav_jump);
	ise_dsp_free_pcm(&wav_swoosh);
	ise_dsp_free_pcm(&wav_punch);
	ise_free_anim_set(&anim_zombie2);
	ise_free_anim_set(&anim_zombie1);
	ise_free_anim_set(&anim_knight);
    ise_free_sprite(&im.sprite);
    ise_gfx_free_map(&gfx);

	vmode.mode = ISE_VGA_MODE_80X25;
    ise_gfx_init(&gfx, &vmode, 0, 0);

 	printf("vram_size: 0x%x\n", ise_vram_size_64k);
    return 0;
}