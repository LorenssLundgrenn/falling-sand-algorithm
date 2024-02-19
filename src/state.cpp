#include <iostream>
#include <curses.h>

#include <chrono>
#include <time.h> 
#include <thread>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <cstdlib>
#include <cmath>

#include "state.hpp"
#include "constants.hpp"

#define SLEEP std::this_thread::sleep_for
#define MS std::chrono::milliseconds

State::State(const char* path) {
    std::string text_str{};
    if (!load_file(path, text_str)) {

    }
    grid = str_to_arr2d(text_str);

    buffer = arr2d_create();
    arr2d_fill_empty(buffer);


    window = newwin(height + 2, width * 2 + 2, 0, 0);
}

void State::deallocate() {
    arr2d_destroy(grid);
    arr2d_destroy(buffer);
}

void State::main_loop() {
    srand((unsigned int)time(0));

    generate_window_box();

    draw();
    SLEEP(MS(MAINLOOP_START_DELAY));

    while (running) {
        update();
        //arr2d_copy(grid, buffer); planned feature to add double buffers

        draw();
        SLEEP(MS(1000 / FPS));
    }

    mvprintw(height+2, 0, "press any key to exit");

    endwin();
    deallocate();
    getch();
}

void State::draw() {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // position in window box and add
            // horizontal spacing (x * 2)
            mvwprintw(window, 
                y + 1, x * 2 + 1,
                "%c", grid[y][x]
            );
        }
    }
    wrefresh(window);
}

// this is terrible
void State::update() {
    // dont run if no change was recorded
    running = false;
    
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            // iterate particles from bottom to top
            int y = height - j - 1;

            // update from both sides to middle
            // example: width = 6
            // i 0 1 2 3 4 5 
            // x 0 5 1 4 2 3
            int x{};
            if (i % 2 == 0) {
                x = i - (int)floor(i / 2);
            }
            else {
                x = width - (int)ceil(i / 2) - 1;
            }

            // sand logic
            if (is_particle_sand(grid[y][x])) {
                
                // if sand is above ground, else do nothing
                if ( y < (height-1) ) {

                    bool max_left = x == 0;
                    bool max_right = x == width - 1;
                    bool between = !max_left && !max_right;

                    // sand stacking applies pressure
                    bool pressure = false;
                    if (y != 0 && is_particle_sand(grid[y - 1][x])) {
                        pressure = true;
                    }

                    // determine which side is free to move to:
                    // assert cant move past border (max_left/max_right);
                    // assert can move to horizontally adjacent free cells;
                    // assert can move to adjacent free cell only if the
                    // cell below it is free. this condition is ignored if
                    // sand is under pressure

                    bool left_free = false;
                    if (!max_left && grid[y][x - 1] == ' ' && 
                        (grid[y + 1][x - 1] == ' ' || pressure)
                    ) {
                        left_free = true;
                    }

                    bool right_free = false;
                    if (!max_right && grid[y][x + 1] == ' ' && 
                        (grid[y + 1][x + 1] == ' ' || pressure)
                    ) {
                        right_free = true;
                    }

                    // fall if nothing beneath
                    if (grid[y + 1][x] == ' ') {
                        grid[y + 1][x] = grid[y][x];
                        grid[y][x] = ' ';
                        running = true;
                    }

                    // something is beneath:
                    // assert either right or left side is free, on
                    // failure do nothing
                    else if ( left_free || right_free ) {
                        int dir = 0; // right(1), left(-1), do nothing(0)

                        if (max_left) dir = 1;
                        else if (max_right) dir = -1;

                        // not at left or right border:
                        // if both sides are free, randomize direction
                        else if (left_free && right_free) {
                            int rval = rand() % 2;
                            if (rval == 0) {
                                dir = 1;
                            }
                            else {
                                dir = -1;
                            }
                        }

                        // only one side is free:
                        else if (right_free) dir = 1;
                        else if (left_free) dir = -1;

                        // translate direction
                        if (dir != 0) {
                            grid[y][x + dir] = grid[y][x];
                            grid[y][x] = ' ';
                            running = true;
                        }
                    }
                }
            }
        }
    }
}

char** State::arr2d_create() const {
    char** arr2d = new char* [height] {};
    for (int i = 0; i < height; ++i) {
        arr2d[i] = new char[width] {};
    }
    return arr2d;
}

void State::arr2d_destroy(char** source) const {
    for (int i = 0; i < height; ++i) {
        delete[] source[i];
    }
    delete[] source;
}

void State::arr2d_copy(char** source, char** dest) const {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            dest[y][x] = source[y][x];
        }
    }
}

void State::arr2d_fill_empty(char** source) const {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            source[y][x] = ' ';
        }
    }
}

char** State::str_to_arr2d(std::string text) {
    format_str(text);
    char** arr2d = arr2d_create();
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            arr2d[y][x] = text[y * width + x];
        }
    }

    return arr2d;
}

// assert homogenous line length equal to width
// (maximum line length) by padding line with 
// whitespace. omit linebreaks
void State::format_str(std::string& text) const {
    std::string out{};
    std::string line{};
    std::istringstream stream(text);
    while (std::getline(stream, line)) {
        out += line;
        int diff = width - line.length();
        for (int i = 0; i < diff; i++) {
            out += ' ';
        }
    }
    text = out;
}

bool State::load_file(const char* path, std::string& dest) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return 0;
    }

    int lines = 0;
    int cols = 0;
    std::string text{};

    std::string line{};
    while (std::getline(file, line)) {
        int length = line.length();
        if (length > cols) {
            cols = length;
        }
        lines++;
        text += line + '\n';
    }

    dest = text;
    width = cols;
    height = lines;
    loaded_path = path;

    file.close();
    return 1;
}

// overengineered bullshit
void State::generate_window_box() {
    size_t max_screen_width = width * 2 - 2;
    
    std:: string dim_text = ", " + std::to_string(width) +
        'x' + std::to_string(height);
    size_t dim_text_size = dim_text.size();

    std::string box_title = loaded_path + dim_text;
    size_t box_title_size = box_title.size();
    
    if (max_screen_width <= 4) box_title = "";
    else if (box_title_size > max_screen_width) {
        box_title = loaded_path;
        box_title_size = box_title_size - dim_text_size;

        box_title.resize(max_screen_width);
        size_t new_box_title_size = box_title.size();
        if (new_box_title_size < box_title_size) {
            box_title_size = new_box_title_size;
        }
    }

    box(window, 0, 0);
    mvwprintw(window, 
        0, 2+ max_screen_width - box_title_size,
        box_title.c_str());
    refresh();
    wrefresh(window);
}

bool State::is_particle_sand(char c) {
    if (c == ' ' || c == '#') {
        return false;
    }
    return true;
}

// debug: count all sand particles.
// used to determine whether or not we lose
// any particles during simulation
int State::get_particles() {
    int n_particles = 0;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (is_particle_sand(grid[y][x])) {
                n_particles++;
            }
        }
    }
    return n_particles;
}