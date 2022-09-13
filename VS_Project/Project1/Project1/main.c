#include <stdlib.h>
#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <math.h>
#pragma warning (disable: 4996)


//for exterior storage 
#define MAX_length_of_array 10000
int num_of_record_Basic = 0;
int num_of_record_Intermediate = 0;
int num_of_record_Advanced = 0;
FILE *file_in, *file_out;
typedef struct _exterior {
	double time_exterior;
}Exterior;
Exterior exterior_Basic[MAX_length_of_array], exterior_Intermediate[MAX_length_of_array], exterior_Advanced[MAX_length_of_array];
int CMP_using_time(const void* A, const void* B);
int the_level_user_played_last_time;
double the_time_the_user_consumed_last_time;//和int the_level_user_played_last_time連動
int last_time_won_or_lost = 0;//0表示輸了，1表示贏了
double the_very_first_start_time = 0;

#define LOG_ENABLED// If defined, logs will be shown on console and written to file.

/* Constants. */


const int FPS = 60;// Frame rate (frame per second)
#define frame_rate 60
const int SCREEN_W = 800;// Display (screen) width.
const int SCREEN_H = 600;// Display (screen) height.
const int RESERVE_SAMPLES = 10;// At most 4 audios can be played at a time.

// Same as:
// const int SCENE_MENU = 1;
// const int SCENE_START = 2;
enum {
	SCENE_MENU = 1,
	SCENE_START = 2,
	// [HACKATHON 3-7]完成
	// TODO: Declare a new scene id.
	SCENE_SETTINGS = 3,
	SCENE_SCOREBOARD = 4,
	SCENE_USER_WON = 5,
	SCENE_USER_LOST = 6,
};

/* Input states */


int active_scene;// The active scene id.
bool key_state[ALLEGRO_KEY_MAX];// Keyboard state, whether the key is down or not.
bool *mouse_state;// Mouse state, whether the key is down or not. 1 for left, 2 for right, 3 for middle.
int mouse_x, mouse_y;// Mouse position.
// TODO: More variables to store input states such as joysticks, ...

/* Variables for allegro basic routines. */

ALLEGRO_DISPLAY* game_display;
ALLEGRO_EVENT_QUEUE* game_event_queue;
ALLEGRO_TIMER* game_update_timer;

/* Shared resources*/

ALLEGRO_FONT* font_pirulen_32;
ALLEGRO_FONT* font_pirulen_24;
ALLEGRO_FONT* font_pirulen_60;
ALLEGRO_FONT* font_pirulen_14;
ALLEGRO_FONT* font_pirulen_20;
// TODO: More shared resources or data that needed to be accessed
// across different scenes.

/* Menu Scene resources*/
ALLEGRO_BITMAP* main_img_background;
// [HACKATHON 3-1]完成
// TODO: Declare 2 variables for storing settings images.
ALLEGRO_BITMAP* img_settings;
ALLEGRO_BITMAP* img_settings2;
ALLEGRO_BITMAP* img_scoreboard;
ALLEGRO_BITMAP* img_scoreboard2;
ALLEGRO_SAMPLE* main_bgm;
ALLEGRO_SAMPLE_ID main_bgm_id;

/* Start Scene resources*/
ALLEGRO_BITMAP* start_img_background;
ALLEGRO_BITMAP* start_img_plane;
ALLEGRO_BITMAP* start_img_enemy;
ALLEGRO_SAMPLE* start_bgm;
ALLEGRO_SAMPLE_ID start_bgm_id;


ALLEGRO_SAMPLE* sound_effect_when_on_a_button;
ALLEGRO_SAMPLE_ID sound_effect_when_on_a_button_id;

ALLEGRO_SAMPLE* sound_effect_snoring;
ALLEGRO_SAMPLE_ID sound_effect_snoring_id;

ALLEGRO_SAMPLE* sound_effect_shoot;
ALLEGRO_SAMPLE_ID sound_effect_shoot_id;
ALLEGRO_SAMPLE* sound_effect_enemy_attacked;
ALLEGRO_SAMPLE_ID sound_effect_enemy_attacked_id;
ALLEGRO_SAMPLE* sound_effect_plane_attacked;
ALLEGRO_SAMPLE_ID sound_effect_plane_attacked_id;

ALLEGRO_SAMPLE* sound_effect_user_won;
ALLEGRO_SAMPLE_ID sound_effect_user_won_id;
ALLEGRO_SAMPLE* sound_effect_user_lost;
ALLEGRO_SAMPLE_ID sound_effect_user_lost_id;

// [HACKATHON 2-1]完成
// TODO: Declare a variable to store your bullet's image.
ALLEGRO_BITMAP* img_bullet;
ALLEGRO_BITMAP* img_user_won;
ALLEGRO_BITMAP* img_user_lost;
ALLEGRO_BITMAP* img_Basic_chosen;
ALLEGRO_BITMAP* img_Basic_unchosen;
ALLEGRO_BITMAP* img_Intermediate_chosen;
ALLEGRO_BITMAP* img_Intermediate_unchosen;
ALLEGRO_BITMAP* img_Advanced_chosen;
ALLEGRO_BITMAP* img_Advanced_unchosen;
ALLEGRO_BITMAP* img_OK;
ALLEGRO_BITMAP* img_go_back;

ALLEGRO_BITMAP* img_TAIWAN;
ALLEGRO_BITMAP* img_KINA;

typedef struct {
	// The center coordinate of the image.
	float x, y;
	// The width and height of the object.
	float w, h;
	// The velocity in x, y axes.
	float vx, vy;
	// Should we draw this object on the screen.
	bool hidden;
	// The pointer to the object’s image.
	ALLEGRO_BITMAP* img;
	int HP;
} MovableObject;



typedef struct {
	int HP;
	int lives;
	float Speed_of_user;
	int num_of_bullets;
	int num_of_enemies;
	int HP_of_enemies;
	float Speed_of_enemies;
}Mode;

Mode Basic = {frame_rate * 30,5,3.5,5,7,frame_rate * 15,1.5 };
Mode Intermediate = { frame_rate * 25, 4, 3.5, 4, 7, frame_rate * 15, 1.5 };
Mode Advanced = { frame_rate * 15,3,3.5,5,8,frame_rate * 25,2 };
//Basic、Intermediate、Advanced中，num_of_bullets分別是3、5、5，故要define MAX_BULLET 5，來開啟足夠的的陣列大小
//Basic、Intermediate、Advanced中，num_of_enemies分別是8、5、7，故要define MAX_ENEMY 8，來開啟足夠的的陣列大小
#define MAX_BULLET 5
#define MAX_ENEMY 8
Mode* level[3] = { &Basic, &Intermediate, &Advanced };//level0、1、2分別是: 初、中、高級。

//初始化專區
//plane.HP = frame_rate * 10;
int lives = 5;
float Speed_of_user = 3.5;
int num_of_bullets = 5;
int num_of_enemies = 7;
int HP_of_enemies = frame_rate * 15;
float Speed_of_enemies = 1.5;
int score = 0;
bool flag_is_user_won = false;
int current_level = 0;
int now_HP = frame_rate * 30;



int num_of_enemies_remaining = 7;



bool flag_when_on_a_button_played = false;
bool flag_when_on_a_button_played_closed = false;
double start_time;
double end_time;
void draw_movable_object(MovableObject obj);

// [HACKATHON 2-2]完成
// TODO: Declare the max bullet count that will show on screen.
// You can try max 4 bullets here and see if you needed more.





MovableObject plane; 
MovableObject enemies[MAX_ENEMY];
// [HACKATHON 2-3]完成
// TODO: Declare an array to store bullets with size of max bullet count.
MovableObject bullets[MAX_BULLET];
// [HACKATHON 2-4]完成
// TODO: Set up bullet shooting cool-down variables.
// 1) Declare your shooting cool-down time as constant. (0.2f will be nice)
// 2) Declare your last shoot timestamp.
const float MAX_COOLDOWN = 0.2f;
double last_shoot_timestamp;

/* Declare function prototypes. */

// Initialize allegro5 library
void allegro5_init(void);
// Initialize variables and resources.
// Allows the game to perform any initialization it needs before
// starting to run.
void game_init(void);
// Process events inside the event queue using an infinity loop.
void game_start_event_loop(void);
// Run game logic such as updating the world, checking for collision,
// switching scenes and so on.
// This is called when the game should update its logic.
void game_update(void);
// Draw to display.
// This is called when the game should draw itself.
void game_draw(void);
// Release resources.
// Free the pointers we allocated.
void game_destroy(void);
// Function to change from one scene to another.
void game_change_scene(int next_scene);
// Load resized bitmap and check if failed.
ALLEGRO_BITMAP *load_bitmap_resized(const char *filename, int w, int h);
// [HACKATHON 3-2]完成
// TODO: Declare a function.
// Determines whether the point (px, py) is in rect (x, y, w, h).
bool pnt_in_rect(int px, int py, int x, int y, int w, int h);

/* Event callbacks. */
void on_key_down(int keycode);
void on_mouse_down(int btn, int x, int y);

/* Declare function prototypes for debugging. */

// Display error message and exit the program, used like 'printf'.
// Write formatted output to stdout and file from the format string.
// If the program crashes unexpectedly, you can inspect "log.txt" for
// further information.
void game_abort(const char* format, ...);
// Log events for later debugging, used like 'printf'.
// Write formatted output to stdout and file from the format string.
// You can inspect "log.txt" for logs in the last run.
void game_log(const char* format, ...);
// Log using va_list.
void game_vlog(const char* format, va_list arg);

int main(int argc, char** argv) {
	// Set random seed for better random outcome.
	srand(time(NULL));
	allegro5_init();
	game_log("Allegro5 initialized");
	game_log("Game begin");
	// Initialize game variables.
	game_init();
	game_log("Game initialized");
	// Draw the first frame.
	game_draw();
	game_log("Game start event loop");
	// This call blocks until the game is finished.
	game_start_event_loop();
	game_log("Game end");
	game_destroy();
	return 0;
}

void allegro5_init(void) {
	if (!al_init())
		game_abort("failed to initialize allegro");

	// Initialize add-ons.
	if (!al_init_primitives_addon())
		game_abort("failed to initialize primitives add-on");
	if (!al_init_font_addon())
		game_abort("failed to initialize font add-on");
	if (!al_init_ttf_addon())
		game_abort("failed to initialize ttf add-on");
	if (!al_init_image_addon())
		game_abort("failed to initialize image add-on");
	if (!al_install_audio())
		game_abort("failed to initialize audio add-on");
	if (!al_init_acodec_addon())
		game_abort("failed to initialize audio codec add-on");
	if (!al_reserve_samples(RESERVE_SAMPLES))
		game_abort("failed to reserve samples");
	if (!al_install_keyboard())
		game_abort("failed to install keyboard");
	if (!al_install_mouse())
		game_abort("failed to install mouse");
	// TODO: Initialize other addons such as video, ...

	// Setup game display.
	game_display = al_create_display(SCREEN_W, SCREEN_H);
	if (!game_display)
		game_abort("failed to create display");
	al_set_window_title(game_display, "I2P(I)_2019 Final Project <108062124>");

	// Setup update timer.
	game_update_timer = al_create_timer(1.0f / FPS);
	if (!game_update_timer)
		game_abort("failed to create timer");

	// Setup event queue.
	game_event_queue = al_create_event_queue();
	if (!game_event_queue)
		game_abort("failed to create event queue");

	// Malloc mouse buttons state according to button counts.
	const unsigned m_buttons = al_get_mouse_num_buttons();
	game_log("There are total %u supported mouse buttons", m_buttons);
	// mouse_state[0] will not be used.
	mouse_state = malloc((m_buttons + 1) * sizeof(bool));
	memset(mouse_state, false, (m_buttons + 1) * sizeof(bool));

	// Register display, timer, keyboard, mouse events to the event queue.
	al_register_event_source(game_event_queue, al_get_display_event_source(game_display));
	al_register_event_source(game_event_queue, al_get_timer_event_source(game_update_timer));
	al_register_event_source(game_event_queue, al_get_keyboard_event_source());
	al_register_event_source(game_event_queue, al_get_mouse_event_source());
	// TODO: Register other event sources such as timer, video, ...

	// Start the timer to update and draw the game.
	al_start_timer(game_update_timer);
}

void game_init(void) {
	/* Shared resources*/
	font_pirulen_32 = al_load_font("pirulen.ttf", 32, 0);
	if (!font_pirulen_32)
		game_abort("failed to load font: pirulen.ttf with size 32");

	font_pirulen_24 = al_load_font("pirulen.ttf", 24, 0);
	if (!font_pirulen_24)
		game_abort("failed to load font: pirulen.ttf with size 24");

	font_pirulen_60 = al_load_font("pirulen.ttf", 60, 0);
	if (!font_pirulen_60)
		game_abort("failed to load font: pirulen.ttf with size 60");

	font_pirulen_14 = al_load_font("pirulen.ttf", 14, 0);
	if (!font_pirulen_14)
		game_abort("failed to load font: pirulen.ttf with size 14");

	font_pirulen_20 = al_load_font("pirulen.ttf", 20, 0);
	if (!font_pirulen_20)
		game_abort("failed to load font: pirulen.ttf with size 20");
	/* Menu Scene resources*/
	main_img_background = load_bitmap_resized("main-bg.jpg", SCREEN_W, SCREEN_H);

	main_bgm = al_load_sample("S31-Night Prowler.ogg");
	if (!main_bgm)
		game_abort("failed to load audio: S31-Night Prowler.ogg");
	sound_effect_when_on_a_button = al_load_sample("when_on_a_button.ogg");
	if (!sound_effect_when_on_a_button)
		game_abort("failed to load aduio: when_on_a_button.ogg");

	sound_effect_snoring = al_load_sample("snoring.ogg");
	if (!sound_effect_snoring)
		game_abort("failed to load aduio: snoring.ogg");


	sound_effect_shoot = al_load_sample("shoot.ogg");
	if (!sound_effect_shoot)
		game_abort("failed to load aduio: shoot.ogg");
	sound_effect_enemy_attacked = al_load_sample("enemy_attacked.ogg");
	if (!sound_effect_enemy_attacked)
		game_abort("failed to load aduio: enemy_attacked.ogg");
	sound_effect_plane_attacked = al_load_sample("enemy_attacked.ogg");
	if (!sound_effect_plane_attacked)
		game_abort("failed to load aduio: enemy_attacked.ogg");

	sound_effect_user_won = al_load_sample("Taiwan_succesfully_defeated_KINA!_Hurray!_with_applauses.ogg");
	if (!sound_effect_user_won)
		game_abort("failed to load aduio: Taiwan_succesfully_defeated_KINA!_Hurray!_with_applauses.ogg");
	sound_effect_user_lost = al_load_sample("You_lost_GG_Hahahahaha_shame_on_you.ogg");
	if (!sound_effect_user_lost)
		game_abort("failed to load aduio: You_lost_GG_Hahahahaha_shame_on_you.ogg");
	// [HACKATHON 3-4]完成
	// TODO: Load settings images.
	// Don't forget to check their return values.
	img_settings = al_load_bitmap("settings.png");
	if (!img_settings)
		game_abort("failed to load image: settings.png");
	img_settings2 = al_load_bitmap("settings2.png");
	if (!img_settings2)
		game_abort("failed to load image: settings2.png");

	img_scoreboard = al_load_bitmap("scoreboard.png");
	if (!img_scoreboard)
		game_abort("failed to load image: scoreboard.png");
	img_scoreboard2 = al_load_bitmap("scoreboard2.png");
	if (!img_scoreboard2)
		game_abort("failed to load image: scoreboard2.png");

	/* Start Scene resources*/
	start_img_background = load_bitmap_resized("sky.jpg", SCREEN_W, SCREEN_H);

	start_img_plane = al_load_bitmap("F16.png");
	if (!start_img_plane)
		game_abort("failed to load image: F16.png");

	start_img_enemy = al_load_bitmap("EVA.png");
	if (!start_img_enemy)
		game_abort("failed to load image: EVA.png");

	start_bgm = al_load_sample("mythica.ogg");
	if (!start_bgm)
		game_abort("failed to load audio: mythica.ogg");

	// [HACKATHON 2-5]完成
	// TODO: Initialize bullets.
	// 1) Search for a bullet image online and put it in your project.
	//    You can use the image we provided.
	// 2) Load it in by 'al_load_bitmap' or 'load_bitmap_resized'.
	// 3) If you use 'al_load_bitmap', don't forget to check its return value.
	img_bullet = al_load_bitmap("silver_bullet.png");
	if (!img_bullet)
		game_abort("failed to load image: silver_bullet.png");

	img_user_won = al_load_bitmap("WIN.jpg");
	if (!img_user_won)
		game_abort("failed to load image: WIN.jpg"); 
	img_user_lost = al_load_bitmap("LOST.jpg");
	if (!img_user_lost)
		game_abort("failed to load image: LOST.jpg");

	img_Basic_chosen = al_load_bitmap("Basic_chosen.png");
	if (!img_Basic_chosen)
		game_abort("failed to load image: Basic_chosen.png");
	img_Basic_unchosen = al_load_bitmap("Basic_unchosen.png");
	if (!img_Basic_unchosen)
		game_abort("failed to load image: Basic_unchosen.png");
	img_Intermediate_chosen = al_load_bitmap("Intermediate_chosen.png");
	if (!img_Intermediate_chosen)
		game_abort("failed to load image: Intermediate_chosen.png");
	img_Intermediate_unchosen = al_load_bitmap("Intermediate_unchosen.png");
	if (!img_Intermediate_unchosen)
		game_abort("failed to load image: Intermediate_unchosen.png");
	img_Advanced_chosen = al_load_bitmap("Advanced_chosen.png");
	if (!img_Advanced_chosen)
		game_abort("failed to load image: Advanced_chosen.png");
	img_Advanced_unchosen = al_load_bitmap("Advanced_unchosen.png");
	if (!img_Advanced_unchosen)
		game_abort("failed to load image: Advanced_unchosen.png");

	img_OK = al_load_bitmap("OK.png");
	if (!img_OK)
		game_abort("failed to load image: OK.png");
	img_go_back = al_load_bitmap("go_back.png");
	if (!img_go_back)
		game_abort("failed to load image: go_back.png");

	img_TAIWAN = al_load_bitmap("TAIWAN.png");
	if (!img_TAIWAN)
		game_abort("failed to load image: TAIWAN.png");
	img_KINA = al_load_bitmap("KINA.png");
	if (!img_KINA)
		game_abort("failed to load image: KINA.png");
	
	
	// Change to first scene.
	game_change_scene(SCENE_MENU);
}

void game_start_event_loop(void) {
	bool done = false;
	ALLEGRO_EVENT event;
	int redraws = 0;
	while (!done) {
		al_wait_for_event(game_event_queue, &event);
		if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
			// Event for clicking the window close button.
			game_log("Window close button clicked");
			done = true;
		}
		else if (event.type == ALLEGRO_EVENT_TIMER) {
			// Event for redrawing the display.
			if (event.timer.source == game_update_timer)
				// The redraw timer has ticked.
				redraws++;
		}
		else if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
			// Event for keyboard key down.
			game_log("Key with keycode %d down", event.keyboard.keycode);
			key_state[event.keyboard.keycode] = true;
			on_key_down(event.keyboard.keycode);
		}
		else if (event.type == ALLEGRO_EVENT_KEY_UP) {
			// Event for keyboard key up.
			game_log("Key with keycode %d up", event.keyboard.keycode);
			key_state[event.keyboard.keycode] = false;
		}
		else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
			// Event for mouse key down.
			game_log("Mouse button %d down at (%d, %d)", event.mouse.button, event.mouse.x, event.mouse.y);
			mouse_state[event.mouse.button] = true;
			on_mouse_down(event.mouse.button, event.mouse.x, event.mouse.y);
		}
		else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
			// Event for mouse key up.
			game_log("Mouse button %d up at (%d, %d)", event.mouse.button, event.mouse.x, event.mouse.y);
			mouse_state[event.mouse.button] = false;
		}
		else if (event.type == ALLEGRO_EVENT_MOUSE_AXES) {
			if (event.mouse.dx != 0 || event.mouse.dy != 0) {
				// Event for mouse move.
				game_log("Mouse move to (%d, %d)", event.mouse.x, event.mouse.y);
				mouse_x = event.mouse.x;
				mouse_y = event.mouse.y;
			}
			else if (event.mouse.dz != 0) {
				// Event for mouse scroll.
				game_log("Mouse scroll at (%d, %d) with delta %d", event.mouse.x, event.mouse.y, event.mouse.dz);
			}
		}
		// TODO: Process more events and call callbacks by adding more
		// entries inside Scene.

		// Redraw
		if (redraws > 0 && al_is_event_queue_empty(game_event_queue)) {
			// if (redraws > 1)
			// 	game_log("%d frame(s) dropped", redraws - 1);
			// Update and draw the next frame.
			game_update();
			game_draw();
			redraws = 0;
		}
	}
}







void game_update(void) {
	if (active_scene == SCENE_START) {
		plane.vx = plane.vy = 0;
		if (key_state[ALLEGRO_KEY_UP] || key_state[ALLEGRO_KEY_W])
			plane.vy -= 1;
		if (key_state[ALLEGRO_KEY_DOWN] || key_state[ALLEGRO_KEY_S])
			plane.vy += 1;
		if (key_state[ALLEGRO_KEY_LEFT] || key_state[ALLEGRO_KEY_A])
			plane.vx -= 1;
		if (key_state[ALLEGRO_KEY_RIGHT] || key_state[ALLEGRO_KEY_D])
			plane.vx += 1;
		// 0.71 is (1/sqrt(2)).
		plane.y += plane.vy * Speed_of_user * (plane.vx ? 0.71f : 1);;//Mode Basic = {HP = frame_rate * 10, lives = 6, MAX_Speed_of_user = 3,MAX_num_of_bullets = 3, num_of_enemies = 8, HP_of_enemies = frame_rate * 8, MAX_Speed_of_enemies = 2 };
		plane.x += plane.vx * Speed_of_user * (plane.vy ? 0.71f : 1); ;//Mode Basic = {HP = frame_rate * 10, lives = 6, MAX_Speed_of_user = 3,MAX_num_of_bullets = 3, num_of_enemies = 8, HP_of_enemies = frame_rate * 8, MAX_Speed_of_enemies = 2 };




		// [HACKATHON 1-1]完成
		// TODO: Limit the plane's collision box inside the frame.
		//       (x, y axes can be separated.)
		if (plane.x -  0.5 * plane.w < 0)
			plane.x = 0.5 * plane.w;
		else if (plane.x + 0.5 * plane.w > SCREEN_W)
			plane.x = SCREEN_W - 0.5 * plane.w;
		if (plane.y - 0.5 * plane.h < 0)
			plane.y = 0.5 * plane.h;
		else if (plane.y + 0.5 * plane.h > SCREEN_H)
			plane.y = SCREEN_H - 0.5 * plane.h;
		
		
		
		
		
		
		// [HACKATHON 2-7]完成
		// TODO: Update bullet coordinates.
		// 1) For each bullets, if it's not hidden, update x, y
		// according to vx, vy.
		// 2) If the bullet is out of the screen, hide it.
		int i;
		for (i = 0; i < num_of_bullets; i++) {
			if (bullets[i].hidden) continue;
			bullets[i].x += bullets[i].vx;
			bullets[i].y += bullets[i].vy;
			if (bullets[i].y < 0)
				bullets[i].hidden = true;
			for (int j = 0; j < num_of_enemies; j++) {
				if (!enemies[j].hidden) {
					if (bullets[i].x <= enemies[j].x + 0.5*enemies[j].w && bullets[i].x >= enemies[j].x - 0.5*enemies[j].w) {
						if (bullets[i].y <= enemies[j].y + enemies[j].h && bullets[i].y >= enemies[j].y - enemies[j].h) {
							al_stop_sample(&sound_effect_enemy_attacked_id);
							al_stop_sample(&sound_effect_plane_attacked_id);
							al_play_sample(sound_effect_enemy_attacked, 0.1, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, &sound_effect_enemy_attacked_id);
							//bullets[i].hidden = true;
							enemies[j].HP--;
							score++;
							if (enemies[j].HP == 0) {
								enemies[j].hidden = true;
								enemies[j].x = enemies[j].y = -500;
								num_of_enemies_remaining--;
							}
						}
					}
				}
			}
		}
		for (int j = 0; j < num_of_enemies; j++) {
			if (plane.x + 0.5*plane.w >= enemies[j].x - 0.5*enemies[j].w && plane.x - 0.5*plane.w <= enemies[j].x + 0.5*enemies[j].w) {
				if (plane.y - 0.5*plane.h <= enemies[j].y + 0.5*enemies[j].h&&plane.y + 0.5*plane.h >= enemies[j].y - 0.5*enemies[j].h) {
					plane.HP--;
					al_stop_sample(&sound_effect_enemy_attacked_id);
					al_stop_sample(&sound_effect_plane_attacked_id);
					al_play_sample(sound_effect_plane_attacked, 0.1, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, &sound_effect_plane_attacked_id);
					if (plane.HP == 0) {
						lives--;
						num_of_enemies_remaining = level[current_level]->num_of_enemies;
						if (lives == 0) {
							al_stop_samples();
							al_play_sample(sound_effect_user_lost, 3, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, &sound_effect_user_lost_id);
							game_change_scene(SCENE_USER_LOST);
						}
						else {
							al_rest(3.0);
							game_change_scene(SCENE_START);
						}
					} 
				}
			}
			if (!enemies[j].hidden) {
				enemies[j].vx = (enemies[j].x > plane.x) ? -Speed_of_enemies : (enemies[j].x < plane.x) ? Speed_of_enemies : 0;
				enemies[j].x += enemies[j].vx;
				enemies[j].vy = (enemies[j].y > plane.y) ? -Speed_of_enemies : (enemies[j].y < plane.y) ? Speed_of_enemies : 0;
				enemies[j].y += enemies[j].vy;


				if (enemies[j].x - 0.5 * enemies[j].w < 0)
					enemies[j].x = 0.5 * enemies[j].w;
				else if (enemies[j].x + 0.5 * enemies[j].w > SCREEN_W)
					enemies[j].x = SCREEN_W - 0.5 * enemies[j].w;
			}			
		}

		int j;
		for (j = 0; j < num_of_enemies && enemies[j].hidden; j++);
		if (j == num_of_enemies) {
			al_stop_samples();
			al_play_sample(sound_effect_user_won, 3, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, &sound_effect_user_won_id);
			game_change_scene(SCENE_USER_WON);
		} 
		// [HACKATHON 2-8]
		// TODO: Shoot if key is down and cool-down is over.
		// 1) Get the time now using 'al_get_time'.
		// 2) If Space key is down in 'key_state' and the time
		//    between now and last shoot is not less that cool
		//    down time.
		// 3) Loop through the bullet array and find one that is hidden.
		//    (This part can be optimized.)
		// 4) The bullet will be found if your array is large enough.
		// 5) Set the last shoot time to now.
		// 6) Set hidden to false (recycle the bullet) and set its x, y to the
		//    front part of your plane.
		double now = al_get_time();
		if (key_state[ALLEGRO_KEY_SPACE] && now - last_shoot_timestamp >= MAX_COOLDOWN) {
			for (i = 0; i < num_of_bullets; i++) {
		        if (bullets[i].hidden)
		            break;
		    }
		    if (i < num_of_bullets) {
		        last_shoot_timestamp = now;
		        bullets[i].hidden = false;
				bullets[i].x = plane.x;
				bullets[i].y = plane.y - 0.5 * plane.h;
				al_play_sample(sound_effect_shoot, 0.8, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, &sound_effect_shoot_id);
				
		    }
		}
	}
}

void game_draw(void) {
	if (active_scene == SCENE_MENU) {
		al_draw_bitmap(main_img_background, 0, 0, 0);
		al_draw_text(font_pirulen_32, al_map_rgb(255, 255, 255), SCREEN_W / 2, 30, ALLEGRO_ALIGN_CENTER, "Space Shooter");
		al_draw_textf(font_pirulen_14, al_map_rgb(255, 255, 255), 20, SCREEN_H - 76, 0, "Current level: %s.", current_level == 0 ? "basic" : (current_level == 1) ? "Intermediate" : "Advanced");
		al_draw_text(font_pirulen_14, al_map_rgb(255, 255, 255), 20, SCREEN_H - 63, 0, "To switch, please go to settings.");
		al_draw_text(font_pirulen_24, al_map_rgb(255, 255, 255), 20, SCREEN_H - 50, 0, "Press enter key to start");
		// [HACKATHON 3-5]
		// TODO: Draw settings images.
		// The settings icon should be located at (x, y, w, h) =
		// (SCREEN_W - 48, 10, 38, 38).
		// Change its image according to your mouse position.
		if (pnt_in_rect(mouse_x, mouse_y, SCREEN_W - 48, 10, 38, 38)) {
			al_draw_bitmap(img_settings2, SCREEN_W - 48, 10, 0);
			al_draw_bitmap(img_scoreboard, SCREEN_W - 120, 10, 0);
			if (!flag_when_on_a_button_played) {
				al_play_sample(sound_effect_when_on_a_button, 8, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, &sound_effect_when_on_a_button_id);
				flag_when_on_a_button_played = true;
				flag_when_on_a_button_played_closed = false;
			}
		}
		else if (pnt_in_rect(mouse_x, mouse_y, SCREEN_W - 120, 10, 56, 38)) {
			al_draw_bitmap(img_settings, SCREEN_W - 48, 10, 0);
			al_draw_bitmap(img_scoreboard2, SCREEN_W - 120, 10, 0);
			if (!flag_when_on_a_button_played) {
				al_play_sample(sound_effect_when_on_a_button, 8, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, &sound_effect_when_on_a_button_id);
				flag_when_on_a_button_played = true;
				flag_when_on_a_button_played_closed = false;
			}
		}
		else {
			al_draw_bitmap(img_settings, SCREEN_W - 48, 10, 0);
			al_draw_bitmap(img_scoreboard, SCREEN_W - 120, 10, 0);
			flag_when_on_a_button_played = false;
			if (!flag_when_on_a_button_played_closed) {
				al_stop_sample(&sound_effect_when_on_a_button_id);
				flag_when_on_a_button_played_closed = true;
			}
		}
	}
	else if (active_scene == SCENE_START) {
		al_draw_bitmap(start_img_background, 0, 0, 0);
		al_draw_bitmap(img_TAIWAN, 15, 80, 0);
		al_draw_bitmap(img_KINA, SCREEN_W-85, 45, 0);
		// [HACKATHON 2-9]完成
		// TODO: Draw all bullets in your bullet array.
		for (int i = 0; i < num_of_bullets; i++)
			draw_movable_object(bullets[i]);
		draw_movable_object(plane);
		for (int i = 0; i < num_of_enemies; i++)
			draw_movable_object(enemies[i]);

		//左上角顯示玩家狀態
		al_draw_textf(font_pirulen_14, al_map_rgb(0, 0, 0), 10, 10, 0, "Your");
		al_draw_textf(font_pirulen_14, al_map_rgb(0, 0, 0), 10, 26, 0, "Lives remaining: %d", lives);
		al_draw_textf(font_pirulen_14, al_map_rgb(0, 0, 0), 10, 42, 0, "HP remaining: %d", plane.HP);
		al_draw_textf(font_pirulen_14, al_map_rgb(0, 0, 0), 10, 58, 0, "Score: %d", score);

		//右上角顯示關卡資訊和敵人狀態
		if (current_level == 0) {
			al_draw_textf(font_pirulen_14, al_map_rgb(0, 0, 0), SCREEN_W - 235, 10, 0, "Current level: Basic");
		}
		else if (current_level == 1) {
			al_draw_textf(font_pirulen_14, al_map_rgb(0, 0, 0), SCREEN_W - 320, 10, 0, "Current level: Intermediate");
		}
		else if (current_level = 2) {
			al_draw_textf(font_pirulen_14, al_map_rgb(0, 0, 0), SCREEN_W - 287, 10, 0, "Current level: Advanced");
		}
		
		al_draw_textf(font_pirulen_14, al_map_rgb(0, 0, 0), SCREEN_W - 230, 27, 0, "Enemies remaining: %d", num_of_enemies_remaining);
	}
	// [HACKATHON 3-9]完成
	// TODO: If active_scene is SCENE_SETTINGS.
	// Draw anything you want, or simply clear the display.
	else if (active_scene == SCENE_SETTINGS) {
		al_clear_to_color(al_map_rgb(0, 0, 0));
		al_draw_text(font_pirulen_24, al_map_rgb(255, 255, 255), SCREEN_W / 2, 30, ALLEGRO_ALIGN_CENTER, "Choose the level you're in favor of: ");
		//al_play_sample(sound_effect_snoring, 0.3, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, &sound_effect_snoring_id);
		if (pnt_in_rect(mouse_x, mouse_y, SCREEN_W/2 - 260, 100, 550, 115)) {
			al_draw_bitmap(img_Basic_chosen, SCREEN_W / 2 - 260, 100, 0);
			al_draw_bitmap(img_Intermediate_unchosen, SCREEN_W / 2 - 260, 250, 0);
			al_draw_bitmap(img_Advanced_unchosen, SCREEN_W / 2 - 260, 400, 0);
			al_draw_bitmap(img_go_back, SCREEN_W / 2 - 260, 550, 0);
			if (!flag_when_on_a_button_played) {
				al_play_sample(sound_effect_when_on_a_button, 8, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, &sound_effect_when_on_a_button_id);
				flag_when_on_a_button_played = true;
			}
		}
		else if (pnt_in_rect(mouse_x, mouse_y, SCREEN_W / 2 - 260, 250, 550, 123)) {
			al_draw_bitmap(img_Basic_unchosen, SCREEN_W / 2 - 260, 100, 0);
			al_draw_bitmap(img_Intermediate_chosen, SCREEN_W / 2 - 260, 250, 0);
			al_draw_bitmap(img_Advanced_unchosen, SCREEN_W / 2 - 260, 400, 0);
			al_draw_bitmap(img_go_back, SCREEN_W / 2 - 260, 550, 0);
			if (!flag_when_on_a_button_played) {
				al_play_sample(sound_effect_when_on_a_button, 8, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, &sound_effect_when_on_a_button_id);
				flag_when_on_a_button_played = true;
			}
		}
		else if (pnt_in_rect(mouse_x, mouse_y, SCREEN_W / 2 - 260, 400, 550, 120)) {
			al_draw_bitmap(img_Basic_unchosen, SCREEN_W / 2 - 260, 100, 0);
			al_draw_bitmap(img_Intermediate_unchosen, SCREEN_W / 2 - 260, 250, 0);
			al_draw_bitmap(img_Advanced_chosen, SCREEN_W / 2 - 260, 400, 0);
			al_draw_bitmap(img_go_back, SCREEN_W / 2 - 260, 550, 0);
			if (!flag_when_on_a_button_played) {
				al_play_sample(sound_effect_when_on_a_button, 8, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, &sound_effect_when_on_a_button_id);
				flag_when_on_a_button_played = true;
			}
		}
		else if (pnt_in_rect(mouse_x, mouse_y, SCREEN_W / 2 - 260, 550, 50, 50)) {
			al_draw_bitmap(img_Basic_unchosen, SCREEN_W / 2 - 260, 100, 0);
			al_draw_bitmap(img_Intermediate_unchosen, SCREEN_W / 2 - 260, 250, 0);
			al_draw_bitmap(img_Advanced_unchosen, SCREEN_W / 2 - 260, 400, 0);
			al_draw_bitmap(img_go_back, SCREEN_W / 2 - 260, 550, 0);
			if (!flag_when_on_a_button_played) {
				al_play_sample(sound_effect_when_on_a_button, 8, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, &sound_effect_when_on_a_button_id);
				flag_when_on_a_button_played = true;
			}
		}
		else {
			al_draw_bitmap(img_Basic_unchosen, SCREEN_W / 2 - 260, 100, 0);
			al_draw_bitmap(img_Intermediate_unchosen, SCREEN_W / 2 - 260, 250, 0);
			al_draw_bitmap(img_Advanced_unchosen, SCREEN_W / 2 - 260, 400, 0);
			al_draw_bitmap(img_go_back, SCREEN_W / 2 - 260, 550, 0);
			flag_when_on_a_button_played = false;
		}
		
	}
	else if (active_scene == SCENE_SCOREBOARD) {
		file_in = fopen("record.txt", "rt");
		if (file_in == NULL)
			game_abort("failed to load record.txt (file_in)");

		fscanf(file_in, "%d %d %d", &num_of_record_Basic, &num_of_record_Intermediate, &num_of_record_Advanced);
		for (int i = 0; i < num_of_record_Basic; i++) fscanf(file_in, "%lf", &exterior_Basic[i].time_exterior);
		for (int i = 0; i < num_of_record_Intermediate; i++) fscanf(file_in, "%lf", &exterior_Intermediate[i].time_exterior);
		for (int i = 0; i < num_of_record_Advanced; i++) fscanf(file_in, "%lf", &exterior_Advanced[i].time_exterior);
		fscanf(file_in, "%d %lf %d", &the_level_user_played_last_time, &the_time_the_user_consumed_last_time, &last_time_won_or_lost);
		fclose(file_in);

		al_clear_to_color(al_map_rgb(0, 0, 0));
		al_draw_text(font_pirulen_60, al_map_rgb(255, 255, 255), SCREEN_W / 3 - 170, 0, 0, "SCORE BOARD");
		al_draw_textf(font_pirulen_20, al_map_rgb(255, 255, 255), 10, 70, 0, "YOU HAVE PASSED BASIC LEVEL %d TIMES.", num_of_record_Basic);
		int min;
		for (int i = 0, y = 100; i < 5 && i < num_of_record_Basic; i++, y += 16)
			al_draw_textf(font_pirulen_14, al_map_rgb(255, 255, 255), 10, y, 0, "%02d min  %.2f sec.", min = (int)exterior_Basic[i].time_exterior / 60, exterior_Basic[i].time_exterior - min * 60);
		for (int i = 5, y = 100; i < 10 && i < num_of_record_Basic; i++, y += 16)
			al_draw_textf(font_pirulen_14, al_map_rgb(255, 255, 255), 210, y, 0, "%02d min  %.2f sec.", min = (int)exterior_Basic[i].time_exterior / 60, exterior_Basic[i].time_exterior - min * 60);
		for (int i = 10, y = 100; i < 15 && i < num_of_record_Basic; i++, y += 16)
			al_draw_textf(font_pirulen_14, al_map_rgb(255, 255, 255), 410, y, 0, "%02d min  %.2f sec.", min = (int)exterior_Basic[i].time_exterior / 60, exterior_Basic[i].time_exterior - min * 60);
		for (int i = 15, y = 100; i < 20 && i < num_of_record_Basic; i++, y += 16)
			al_draw_textf(font_pirulen_14, al_map_rgb(255, 255, 255), 610, y, 0, "%02d min  %.2f sec.", min = (int)exterior_Basic[i].time_exterior / 60, exterior_Basic[i].time_exterior - min * 60);
		
		al_draw_textf(font_pirulen_20, al_map_rgb(255, 255, 255), 10, 190, 0, "YOU HAVE PASSED INTERMEDIATE LEVEL %d TIMES.", num_of_record_Intermediate);
		for (int i = 0, y = 220; i < 5 && i < num_of_record_Intermediate; i++, y += 16)
			al_draw_textf(font_pirulen_14, al_map_rgb(255, 255, 255), 10, y, 0, "%02d min  %.2f sec.", min = (int)exterior_Intermediate[i].time_exterior / 60, exterior_Intermediate[i].time_exterior - min * 60);
		for (int i = 5, y = 220; i < 10 && i < num_of_record_Intermediate; i++, y += 16)
			al_draw_textf(font_pirulen_14, al_map_rgb(255, 255, 255), 210, y, 0, "%02d min  %.2f sec.", min = (int)exterior_Intermediate[i].time_exterior / 60, exterior_Intermediate[i].time_exterior - min * 60);
		for (int i = 10, y = 220; i < 15 && i < num_of_record_Intermediate; i++, y += 16)
			al_draw_textf(font_pirulen_14, al_map_rgb(255, 255, 255), 410, y, 0, "%02d min  %.2f sec.", min = (int)exterior_Intermediate[i].time_exterior / 60, exterior_Intermediate[i].time_exterior - min * 60);
		for (int i = 15, y = 220; i < 20 && i < num_of_record_Intermediate; i++, y += 16)
			al_draw_textf(font_pirulen_14, al_map_rgb(255, 255, 255), 610, y, 0, "%02d min  %.2f sec.", min = (int)exterior_Intermediate[i].time_exterior / 60, exterior_Intermediate[i].time_exterior - min * 60);


		al_draw_textf(font_pirulen_20, al_map_rgb(255, 255, 255), 10, 310, 0, "YOU HAVE PASSED ADVANCED LEVEL %d TIMES.", num_of_record_Advanced);
		for (int i = 0, y = 340; i < 5 && i < num_of_record_Advanced; i++, y += 16)
			al_draw_textf(font_pirulen_14, al_map_rgb(255, 255, 255), 10, y, 0, "%02d min  %.2f sec.", min = (int)exterior_Advanced[i].time_exterior / 60, exterior_Advanced[i].time_exterior - min * 60);
		for (int i = 5, y = 340; i < 10 && i < num_of_record_Advanced; i++, y += 16)
			al_draw_textf(font_pirulen_14, al_map_rgb(255, 255, 255), 210, y, 0, "%02d min  %.2f sec.", min = (int)exterior_Advanced[i].time_exterior / 60, exterior_Advanced[i].time_exterior - min * 60);
		for (int i = 10, y = 340; i < 15 && i < num_of_record_Advanced; i++, y += 16)
			al_draw_textf(font_pirulen_14, al_map_rgb(255, 255, 255), 410, y, 0, "%02d min  %.2f sec.", min = (int)exterior_Advanced[i].time_exterior / 60, exterior_Advanced[i].time_exterior - min * 60);
		for (int i = 15, y = 340; i < 20 && i < num_of_record_Advanced; i++, y += 16)
			al_draw_textf(font_pirulen_14, al_map_rgb(255, 255, 255), 610, y, 0, "%02d min  %.2f sec.", min = (int)exterior_Advanced[i].time_exterior / 60, exterior_Advanced[i].time_exterior - min * 60);


		al_draw_textf(font_pirulen_20, al_map_rgb(255, 255, 255), 10, 480, 0, "%s", (last_time_won_or_lost) ? "LAST TIME, YOU WON IN" : "LAST TIME, YOU LOST IN");
		al_draw_textf(font_pirulen_20, al_map_rgb(255, 255, 255), 10, 505, 0, "%s", (the_level_user_played_last_time == 0) ? "BASIC LEVEL" : (the_level_user_played_last_time == 1) ? "INTERMEDIATE LEVEL" : "ADVANCED LEVEL");
		al_draw_textf(font_pirulen_20, al_map_rgb(255, 255, 255), 10, 530, 0, "USING %02d min  %.2f sec.", min = (int)the_time_the_user_consumed_last_time / 60, the_time_the_user_consumed_last_time - min * 60);
		
		al_draw_bitmap(img_go_back, SCREEN_W - 60, SCREEN_H - 60, 0);
		if (pnt_in_rect(mouse_x, mouse_y, SCREEN_W - 60, SCREEN_H - 60, 50, 50)) {
			if (!flag_when_on_a_button_played) {
				al_play_sample(sound_effect_when_on_a_button, 8, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, &sound_effect_when_on_a_button_id);
				flag_when_on_a_button_played = true;
			}
		}
		else flag_when_on_a_button_played = false;
		
	}
	else if (active_scene == SCENE_USER_WON) {
		lives = level[current_level]->lives;
		num_of_enemies_remaining = level[current_level]->num_of_enemies;
		al_draw_bitmap(img_go_back, SCREEN_W - 60, SCREEN_H - 60, 0);
		score = 0;
	}
	else if (active_scene == SCENE_USER_LOST) {
		lives = level[current_level]->lives;
		num_of_enemies_remaining = level[current_level]->num_of_enemies;
		al_draw_bitmap(img_go_back, SCREEN_W - 60, SCREEN_H - 60, 0);
		score = 0;
	}
	al_flip_display();
}

void game_destroy(void) {
	// Destroy everything you have created.
	// Free the memories allocated by malloc or allegro functions.
	// Destroy shared resources.
	al_destroy_font(font_pirulen_32);
	al_destroy_font(font_pirulen_24);
	al_destroy_font(font_pirulen_60);
	al_destroy_font(font_pirulen_14);
	al_destroy_font(font_pirulen_20);

	/* Menu Scene resources*/
	al_destroy_bitmap(main_img_background);
	al_destroy_sample(main_bgm);
	al_destroy_sample(sound_effect_when_on_a_button);
	al_destroy_sample(sound_effect_snoring);
	al_destroy_sample(sound_effect_shoot);
	al_destroy_sample(sound_effect_enemy_attacked);
	al_destroy_sample(sound_effect_plane_attacked);

	al_destroy_sample(sound_effect_user_won);
	al_destroy_sample(sound_effect_user_lost);

	// [HACKATHON 3-6]完成
	// TODO: Destroy the 2 settings images.
	// Uncomment and fill in the code below.
	al_destroy_bitmap(img_settings);
	al_destroy_bitmap(img_settings2);

	/* Start Scene resources*/
	al_destroy_bitmap(start_img_background);
	al_destroy_bitmap(start_img_plane);
	al_destroy_bitmap(start_img_enemy);
	al_destroy_sample(start_bgm);
	// [HACKATHON 2-10]完成
	// TODO: Destroy your bullet image.
	al_destroy_bitmap(img_bullet);
	al_destroy_bitmap(img_user_won);
	al_destroy_bitmap(img_user_lost);

	al_destroy_bitmap(img_Basic_chosen);
	al_destroy_bitmap(img_Basic_unchosen);
	al_destroy_bitmap(img_Intermediate_chosen);
	al_destroy_bitmap(img_Intermediate_unchosen);
	al_destroy_bitmap(img_Advanced_chosen);
	al_destroy_bitmap(img_Advanced_unchosen);
	al_destroy_bitmap(img_OK);
	al_destroy_bitmap(img_go_back);

	al_destroy_bitmap(img_KINA);
	al_destroy_bitmap(img_TAIWAN);

	al_destroy_timer(game_update_timer);
	al_destroy_event_queue(game_event_queue);
	al_destroy_display(game_display);
	free(mouse_state);
}

void game_change_scene(int next_scene) {
	game_log("Change scene from %d to %d", active_scene, next_scene);
	// TODO: Destroy resources initialized when creating scene.
	if (active_scene == SCENE_MENU) {
		al_stop_sample(&main_bgm_id);
		game_log("stop audio (bgm)");
		flag_when_on_a_button_played = false;
	}
	else if (active_scene == SCENE_START) {
		//以下file_in開始到"if (next_scene == SCENE_START) {"的"前面"，都是在預處理外部紀錄。
		file_in = fopen("record.txt", "rt");
		if (file_in == NULL)
			game_abort("failed to load record.txt (file_in)");

 		fscanf(file_in, "%d %d %d", &num_of_record_Basic, &num_of_record_Intermediate, &num_of_record_Advanced);
		for (int i = 0; i < num_of_record_Basic; i++) fscanf(file_in, "%lf", &exterior_Basic[i].time_exterior);
		for (int i = 0; i < num_of_record_Intermediate; i++) fscanf(file_in, "%lf", &exterior_Intermediate[i].time_exterior);
		for (int i = 0; i < num_of_record_Advanced; i++) fscanf(file_in, "%lf", &exterior_Advanced[i].time_exterior);
		fscanf(file_in, "%d %lf %d", &the_level_user_played_last_time, &the_time_the_user_consumed_last_time, &last_time_won_or_lost);

		file_out = fopen("record.txt", "w+t");
		if (file_out == NULL)
			game_abort("failed to load record.txt (file_out)");


		if (next_scene == SCENE_START) {
			al_stop_sample(&start_bgm_id);
			game_log("stop audio (bgm)");
		}
		else {
			al_stop_sample(&sound_effect_shoot_id);
			al_stop_sample(&sound_effect_enemy_attacked_id);
			al_stop_sample(&sound_effect_plane_attacked_id);
			al_stop_sample(&start_bgm_id);
			game_log("stop audio (bgm)");
		}
	}
	else if (active_scene == SCENE_SETTINGS) {
		flag_when_on_a_button_played = false;
		al_stop_samples(&sound_effect_snoring_id);
	}
	else if (active_scene == SCENE_USER_LOST) {
		al_stop_sample(&sound_effect_user_lost_id);
	}
	else if (active_scene == SCENE_USER_WON) {
		al_stop_sample(&sound_effect_user_won_id);
	}



	active_scene = next_scene;
	// TODO: Allocate resources before entering scene.
	if (active_scene == SCENE_MENU) {
		al_play_sample(main_bgm, 0.6, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, &main_bgm_id);
	}
	else if (active_scene == SCENE_START) {
		start_time = al_get_time();
		if (lives == level[current_level]->lives) the_very_first_start_time = start_time;
		al_flip_display();
		int i;
		plane.img = start_img_plane;
		plane.x = 400;
		plane.y = 500;
		plane.w = al_get_bitmap_width(plane.img);
		plane.h = al_get_bitmap_height(plane.img);
		plane.HP = now_HP;//Mode Basic = {HP = frame_rate * 10, lives = 6, MAX_Speed_of_user = 3,MAX_num_of_bullets = 3, num_of_enemies = 8, HP_of_enemies = frame_rate * 8, MAX_Speed_of_enemies = 2 };
		for (i = 0; i < num_of_enemies; i++) {
			enemies[i].img = start_img_enemy;
			enemies[i].w = al_get_bitmap_width(start_img_enemy);
			enemies[i].h = al_get_bitmap_height(start_img_enemy);
			enemies[i].x = enemies[i].w / 2 + (float)rand() / RAND_MAX * (SCREEN_W - enemies[i].w);
			enemies[i].y = 80;
			enemies[i].vx = (float)rand() / RAND_MAX;
			enemies[i].HP = 300;//HP和fps有關，因為是30fps，所以一秒會就扣了30HP!!所以設計HP的時候要注意
			enemies[i].hidden = false;
		}
		// [HACKATHON 2-6]完成
		// TODO: Initialize bullets.
		// For each bullets in array, set their w and h to the size of
		// the image, and set their img to bullet image, hidden to true,
		// (vx, vy) to (0, -3).
		for (i = 0; i < num_of_bullets; i++) {
			bullets[i].w = al_get_bitmap_width(img_bullet);
			bullets[i].h = al_get_bitmap_height(img_bullet);
			bullets[i].img = img_bullet;
			bullets[i].vx = 0;
			bullets[i].vy = -3;
			bullets[i].hidden = true;
		}
		if (!al_play_sample(start_bgm, 0.6, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, &start_bgm_id))
			game_abort("failed to play audio (bgm)");
	}
	else if (active_scene == SCENE_SETTINGS) {
		//待補
	}
	else if (active_scene == SCENE_SCOREBOARD) {
		//好像啥都不用幹，因為都已經在void game_draw(void)裡用好了
	}
	else if (active_scene == SCENE_USER_WON) {
       	end_time = al_get_time();
		al_draw_bitmap(img_user_won, 0, 0, 0);
		al_draw_text(font_pirulen_60, al_map_rgb(255, 255, 255), SCREEN_W / 2, 10, ALLEGRO_ALIGN_CENTER, "!TAIWAN WINS!");
		al_draw_textf(font_pirulen_14, al_map_rgb(255, 255, 255), SCREEN_W / 2, 70, ALLEGRO_ALIGN_CENTER, "Score You Earned: %d, Time You Consumed: %.2f sec", score, end_time - the_very_first_start_time);
		al_draw_text(font_pirulen_32, al_map_rgb(255, 255, 255), SCREEN_W / 2,SCREEN_H- 60, ALLEGRO_ALIGN_CENTER, "SO GIVE ME HIGH SCORE");

		//for exterior record use
		if (current_level == 0) exterior_Basic[num_of_record_Basic++].time_exterior += end_time - the_very_first_start_time;
		else if (current_level == 1) exterior_Intermediate[num_of_record_Intermediate++].time_exterior += end_time - the_very_first_start_time;
		else if (current_level == 2) exterior_Advanced[num_of_record_Advanced++].time_exterior += end_time - the_very_first_start_time;


		qsort(exterior_Basic, num_of_record_Basic, sizeof(Exterior), CMP_using_time);
		qsort(exterior_Intermediate, num_of_record_Intermediate, sizeof(Exterior), CMP_using_time);
		qsort(exterior_Advanced, num_of_record_Advanced, sizeof(Exterior), CMP_using_time);

		fprintf(file_out, "%d %d %d\n\n\n", num_of_record_Basic, num_of_record_Intermediate, num_of_record_Advanced);
		for (int i = 0; i < num_of_record_Basic; i++) fprintf(file_out, "%.3f\n", exterior_Basic[i].time_exterior);
		fprintf(file_out, "\n\n\n");
		for (int i = 0; i < num_of_record_Intermediate; i++) fprintf(file_out, "%.3f\n", exterior_Intermediate[i].time_exterior);
		fprintf(file_out, "\n\n\n");
		for (int i = 0; i < num_of_record_Advanced; i++) fprintf(file_out, "%.3f\n", exterior_Advanced[i].time_exterior);		
		fprintf(file_out, "\n\n\n");
		fprintf(file_out, "%d %.3f %d\n", current_level, end_time - the_very_first_start_time, 1);

		fclose(file_in);
		fclose(file_out);
	}
	else if (active_scene == SCENE_USER_LOST) {
		end_time = al_get_time();
		al_draw_bitmap(img_user_lost, 0, 0, 0);
		al_draw_text(font_pirulen_60, al_map_rgb(255, 255, 255), SCREEN_W / 2, 30, ALLEGRO_ALIGN_CENTER, "!YOU LOST!");
		al_draw_text(font_pirulen_14, al_map_rgb(255, 255, 255), SCREEN_W / 2, SCREEN_H - 60, ALLEGRO_ALIGN_CENTER, "SO YOU OWE ME HIGH SCORE");
		al_draw_text(font_pirulen_14, al_map_rgb(255, 255, 255), SCREEN_W / 2, SCREEN_H - 30, ALLEGRO_ALIGN_CENTER, "OR YOU WOULD BE ACCUSED OF COLLABORATION");


		//for exterior record use
		fprintf(file_out, "%d %d %d\n\n\n", num_of_record_Basic, num_of_record_Intermediate, num_of_record_Advanced);
		for (int i = 0; i < num_of_record_Basic; i++) fprintf(file_out, "%.3f\n", exterior_Basic[i].time_exterior);
		fprintf(file_out, "\n\n\n");
		for (int i = 0; i < num_of_record_Intermediate; i++) fprintf(file_out, "%.3f\n", exterior_Intermediate[i].time_exterior);
		fprintf(file_out, "\n\n\n");
		for (int i = 0; i < num_of_record_Advanced; i++) fprintf(file_out, "%.3f\n", exterior_Advanced[i].time_exterior);
		fprintf(file_out, "\n\n\n");
		fprintf(file_out, "%d %.3f %d\n", current_level, end_time - the_very_first_start_time, 0);

		fclose(file_in);
		fclose(file_out);
	}
}

void on_key_down(int keycode) {
	if (active_scene == SCENE_MENU) {
		if (keycode == ALLEGRO_KEY_ENTER) {
			game_change_scene(SCENE_START);
		}
			
	}
}

void on_mouse_down(int btn, int x, int y) {
	// [HACKATHON 3-8]
	// TODO: When settings clicked, switch to settings scene.
	if (active_scene == SCENE_MENU) {
		if (btn == 1) {
			if (pnt_in_rect(x, y, SCREEN_W - 48, 10, 38, 38))
				game_change_scene(SCENE_SETTINGS);
			else if(pnt_in_rect(x, y, SCREEN_W - 120, 10, 56, 38))
				game_change_scene(SCENE_SCOREBOARD);
		}
	}
	else if (active_scene == SCENE_SETTINGS) {//
		if (btn == 1) {//
			if (pnt_in_rect(x, y, SCREEN_W / 2 - 260, 100, 550, 115)) {
				now_HP = plane.HP = level[0]->HP;
				lives = level[0]->lives;
				Speed_of_user = level[0]->Speed_of_user;
				num_of_bullets = level[0]->num_of_bullets;
				num_of_enemies = level[0]->num_of_enemies;
				HP_of_enemies = level[0]->HP_of_enemies;
				Speed_of_enemies = level[0]->Speed_of_enemies;

				num_of_enemies_remaining = num_of_enemies;

				current_level = 0;
				al_draw_bitmap(img_OK, SCREEN_W / 4+165, 120, 0);
				al_flip_display();
				al_rest(0.3);
			}
			else if (pnt_in_rect(x, y, SCREEN_W / 2 - 260, 250, 550, 123)) {
				now_HP = plane.HP = level[1]->HP;
				lives = level[1]->lives;
				Speed_of_user = level[1]->Speed_of_user;
				num_of_bullets = level[1]->num_of_bullets;
				num_of_enemies = level[1]->num_of_enemies;
				HP_of_enemies = level[1]->HP_of_enemies;
				Speed_of_enemies = level[1]->Speed_of_enemies;

				num_of_enemies_remaining = num_of_enemies;

				current_level = 1;
				al_draw_bitmap(img_OK, SCREEN_W / 4 + 165, 275, 0);
				al_flip_display();
				al_rest(0.3);
			}
			else if (pnt_in_rect(x, y, SCREEN_W / 2 - 260, 400, 550, 120)) {
					now_HP = plane.HP = level[2]->HP;
					lives = level[2]->lives;
					Speed_of_user = level[2]->Speed_of_user;
					num_of_bullets = level[2]->num_of_bullets;
					num_of_enemies = level[2]->num_of_enemies;
					HP_of_enemies = level[2]->HP_of_enemies;
					Speed_of_enemies = level[2]->Speed_of_enemies;

					num_of_enemies_remaining = num_of_enemies;

					current_level = 2;
					al_draw_bitmap(img_OK, SCREEN_W / 4 + 165, 420, 0);
					al_flip_display();
					al_rest(0.3);
			}
			else if (pnt_in_rect(x, y, SCREEN_W / 2 - 260, 550, 50, 50)) {
				game_change_scene(SCENE_MENU);
			}
		}
	}
	else if (active_scene == SCENE_SCOREBOARD) {
		if (pnt_in_rect(x, y, SCREEN_W - 60, SCREEN_H - 60, 50, 50))
			game_change_scene(SCENE_MENU);
	}
	else if (active_scene == SCENE_START) {
		if (btn == 1) {
			if (pnt_in_rect(x, y, SCREEN_W - 48, 10, 38, 38))
				game_change_scene(SCENE_SETTINGS);
			else if (pnt_in_rect(x, y, SCREEN_W - 120, 10, 56, 38))
				game_change_scene(SCENE_SCOREBOARD);
		}
	}
	else if (active_scene == SCENE_USER_WON) {
		if (pnt_in_rect(x, y, SCREEN_W - 60, SCREEN_H - 60, 50, 50))
			game_change_scene(SCENE_MENU);
	}
	else if (active_scene == SCENE_USER_LOST) {
		if (pnt_in_rect(x, y, SCREEN_W - 60, SCREEN_H - 60, 50, 50))
			game_change_scene(SCENE_MENU);
	}
}

void draw_movable_object(MovableObject obj) {
	if (obj.hidden)
		return;
	al_draw_bitmap(obj.img, round(obj.x - obj.w / 2), round(obj.y - obj.h / 2), 0);
}

ALLEGRO_BITMAP *load_bitmap_resized(const char *filename, int w, int h) {
	ALLEGRO_BITMAP* loaded_bmp = al_load_bitmap(filename);
	if (!loaded_bmp)
		game_abort("failed to load image: %s", filename);
	ALLEGRO_BITMAP *resized_bmp = al_create_bitmap(w, h);
	ALLEGRO_BITMAP *prev_target = al_get_target_bitmap();

	if (!resized_bmp)
		game_abort("failed to create bitmap when creating resized image: %s", filename);
	al_set_target_bitmap(resized_bmp);
	al_draw_scaled_bitmap(loaded_bmp, 0, 0,
		al_get_bitmap_width(loaded_bmp),
		al_get_bitmap_height(loaded_bmp),
		0, 0, w, h, 0);
	al_set_target_bitmap(prev_target);
	al_destroy_bitmap(loaded_bmp);

	game_log("resized image: %s", filename);

	return resized_bmp;
}

// [HACKATHON 3-3]
// TODO: Define bool pnt_in_rect(int px, int py, int x, int y, int w, int h)
bool pnt_in_rect(int px, int py, int x, int y, int w, int h) {
	return (px >= x && px <= (x + w) && py >= y && (py) <= (y + h)) ? 1 : 0;
}


// +=================================================================+
// | Code below is for debugging purpose, it's fine to remove it.    |
// | Deleting the code below and removing all calls to the functions |
// | doesn't affect the game.                                        |
// +=================================================================+

void game_abort(const char* format, ...) {
	va_list arg;
	va_start(arg, format);
	game_vlog(format, arg);
	va_end(arg);
	fprintf(stderr, "error occured, exiting after 2 secs");
	// Wait 2 secs before exiting.
	al_rest(2);
	// Force exit program.
	exit(1);
}

void game_log(const char* format, ...) {
#ifdef LOG_ENABLED
	va_list arg;
	va_start(arg, format);
	game_vlog(format, arg);
	va_end(arg);
#endif
}

void game_vlog(const char* format, va_list arg) {
#ifdef LOG_ENABLED
	static bool clear_file = true;
	vprintf(format, arg);
	printf("\n");
	// Write log to file for later debugging.
	FILE* pFile = fopen("log.txt", clear_file ? "w" : "a");
	if (pFile) {
		vfprintf(pFile, format, arg);
		fprintf(pFile, "\n");
		fclose(pFile);
	}
	clear_file = false;
#endif
}


int CMP_using_time(const void* A, const void* B) {
	Exterior x = *(Exterior *)A;
	Exterior y = *(Exterior *)B;
	return (x.time_exterior > y.time_exterior) ? 1 : -1;
}
