#include <curses.h>

#include "state.h"
#include "constants.h"

int main() {
	// init PDcurses
	initscr();
	noecho();
	cbreak();

	State state(SOURCE_PATH);
	state.main_loop();

	return 0;
}