#ifndef STATE_CLASS
#define STATE_CLASS

#include <string>
#include <curses.h>

class State {
public:
	State(const char* path);
	void deallocate();
	void main_loop();

private:
	WINDOW* window;
	std::string loaded_path;
	int width;
	int height;
	bool running = true;

	char** grid;
	char** buffer;

	void draw();
	void update();

	char** arr2d_create() const;
	void arr2d_destroy(char** source) const;
	void arr2d_copy(char** source, char** dest) const;
	void arr2d_fill_empty(char** source) const;

	char** str_to_arr2d(std::string text);
	void format_str(std::string& text) const;

	bool load_file(const char* path, std::string& dest);

	void generate_window_box();

	bool is_particle_sand(char c);
	int get_particles();
};

#endif