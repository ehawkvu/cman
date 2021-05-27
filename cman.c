#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include "game_objs.h"

#define DELAY 30000
#define NUMPOWERPELLETS 10
#define NUMWALLS 200

#ifdef LOG
#include <string.h>
#endif

#define ENEMY_T 0
#define WALL_T  1
#define PILL_T  2

const char *EMEMY_ICON = "@";
const char *WALL_ICON  = "#";
const char *PILL_ICON  = "*";

/* TODO: move the helper functions to their own header file */

int kbhit(void);
char detdir(char ch);
void update_delts(int *dx, int *dy, char direction);
void update_player_s(char **player_s, char direction);
void spawn_power_pellets(int ymax, int xmax);
void print_entity_list(entity_list_T *list, entity_T *player, int *score, int *ny, int *nx);
void init_entity_list(entity_list_T *list, int type, char *icon, int max_y, int max_x);

int main(int argc, char *argv[]) {
	int max_x   = 0, max_y   = 0;
	int delta_x = 0, delta_y = 0;
	int next_x  = 0, next_y  = 0;
	int score   = 0;
	char direction = 'l'; /*u, d, l, r, n | up down left right none */
	time_t t; /* used for srand */

	/* Create the lists for the entities */
	entity_list_T *walls      = (entity_list_T *) malloc(sizeof(entity_list_T));
	entity_list_T *enemies    = (entity_list_T *) malloc(sizeof(entity_list_T));
	entity_list_T *powerpills = (entity_list_T *) malloc(sizeof(entity_list_T));

	entity_T *player = (entity_T *) malloc(sizeof(entity_T));
	player->x = 0;
	player->y = 0;
	player->icon = "o";

	walls->head = NULL;
	enemies->head = NULL;
	powerpills->head = NULL;

	srand((unsigned) time(&t));
	initscr();
	cbreak();
	noecho();
	nodelay(stdscr, TRUE);
	curs_set(FALSE);

	getmaxyx(stdscr, max_y, max_x);

	/*
	Now that we have the max coordinates, we can now fill our
	lists of entities
	*/

	init_entity_list(powerpills, PILL_T, (char*) PILL_ICON, max_y, max_x);
	init_entity_list(walls,      WALL_T, (char*) WALL_ICON, max_y, max_x);

	while(1) {
		/* setup stuff */

		clear();
		mvprintw(player->y, player->x, player->icon);

		update_player_s(&player->icon, direction);
		update_delts(&delta_x, &delta_y, direction);
		next_x = player->x + delta_x;
		next_y = player->y + delta_y;
		if (next_x >= max_x || next_y >= max_y || next_x < 0 || next_y < 0) {
			next_x = player->x + 0;
			next_y = player->y + 0;
		}

		/* Print our walls...*/
		/* TODO: refactor; see if these two can be meaningfully merged */

		print_entity_list(walls,      player, &score, &next_y, &next_x);
		print_entity_list(powerpills, player, &score, &next_y, &next_x);

		wrefresh(stdscr);

		player->x = next_x;
		player->y = next_y;

		usleep(DELAY);
		if (kbhit()) {
			char ch = getch();
			direction = detdir(ch);
		}
	}

	endwin();
	return 0;
}

void print_entity_list(entity_list_T *list, entity_T *player, int *score, int *ny, int *nx) {
	entity_list_node_T *temp = (entity_list_node_T *) malloc(sizeof(entity_list_node_T));
	entity_list_node_T *temp2 = (entity_list_node_T *) malloc(sizeof(entity_list_node_T));
	temp = list->head;
	do {
		switch (temp->value.type) {
			case WALL_T:
				if (*nx == temp->value.x && *ny == temp->value.y) {
					*nx = player->x + 0;
					*ny = player->y + 0;
				}
				break;
			case PILL_T:
				if (*nx == temp->value.x && *ny == temp->value.y) {
					*score = *score + 10;
#ifdef LOG
				fprintf(stderr, "LOG: Score:%d\n", score);
#endif
				}
				break;
			default:
				break;
		}
		mvprintw(temp->value.y, temp->value.x, temp->value.icon);
		temp2 = temp->next;
		temp = temp2;
	} while (temp->next != NULL);
	temp = NULL;
	temp2 = NULL;
	free(temp);
	free(temp2);
}

void init_entity_list(entity_list_T *list, int type, char *icon, int max_y, int max_x){
	int i = 0;
	int number_entities = 0;
#ifdef LOG
	char desc[20];
#endif
	switch (type) {
		case PILL_T:
			number_entities = NUMPOWERPELLETS;
#ifdef LOG
			strcpy(desc, "Power Pill");
#endif
			break;
		case WALL_T:
			number_entities = NUMWALLS;
#ifdef LOG
			strcpy(desc, "Wall");
#endif
			break;
		default:
			break;
	}
	/* if the list is not empty, return */
	if (list->head != NULL)
		return;
	for (i = 0; i < number_entities; i++) {
		entity_list_node_T* new = (entity_list_node_T*) malloc(sizeof(entity_list_node_T));
		if (new == NULL)
			exit(1);
		new->value.x = (int) rand() % max_x;
		new->value.y = (int) rand() % max_y;
		new->value.icon = icon;
		new->value.type = type;
		add_entity_to_list(list, new);
#ifdef LOG
		fprintf(stderr, "LOG: Added %s with x:%d and y:%d\n", desc, new->value.x, new->value.y);
#endif
		new = NULL;
	}
}


/* TODO: merge the update_player_s and update_delts functions */

void update_player_s(char **player_s, char direction) {
	switch(direction) {
		case 'u':
			*player_s = "v";
			break;
		case 'd':
			*player_s = "^";
			break;
		case 'r':
			*player_s = "<";
			break;
		case 'l':
			*player_s = ">";
			break;
		default:
			*player_s = "o";
			break;
	}
}

void update_delts(int *dx, int *dy, char direction) {
	switch(direction) {
		case 'u':
			*dx = 0;
			*dy = -1;
			break;
		case 'd':
			*dx = 0;
			*dy = 1;
			break;
		case 'r':
			*dx = 1;
			*dy = 0;
			break;
		case 'l':
			*dx = -1;
			*dy = 0;
			break;
		default:
			break;
	}
}

/* determines the direction the player should be moving, based on a keypress */
char detdir(char ch){
	switch (ch){
		case 107: /* 'k' - up */
			return 'u';
		case 106: /* 'j' - down */
			return 'd';
		case 108: /* 'l' - right */
			return 'r';
		case 104: /* 'h' - left */
			return 'l';
		default:
			return 'n';
	}
}

/* returns 1 if a key was pressed, 0 otherwise */
int kbhit(){
	int ch = getch();
	if (ch != ERR) {
		ungetch(ch);
		return 1;
	} else {
		return 0;
	}
}
