#include <curses.h>

#include "state.hpp"
#include "constants.hpp"

int main() {
	// init PDcurses
	initscr();
	noecho();
	cbreak();

	State state(SOURCE_PATH);
	state.main_loop();

	return 0;
}