#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <string.h>

// Definitions
#define CTRL_KEY(key) (key & 0x1f) // checks if the key pressed conflicts with ctrl
#define CLEAR "\x1b[2J" // clear terminal
#define MV_CUR_LT "\x1b[H" // move cursor left top

struct append_buffer;
struct term_settings;

void init();
void clearScreen();
void refreshScreen();
void err(const char* e);
void enableRawMode();
void disableRawMode();
char readKey();
void processInput();
void drawRows(struct append_buffer* a_buf);
int getWindowSize(int* rows, int* cols);

struct term_settings {
    struct termios def; // default terminal settings

    // terminal dimensions
    int rows;
    int cols;
};

struct term_settings settings;

struct append_buffer {
    char* buf;
    int len;
};

/*
This is a function that appends a string to a dynamic buffer.

Parameters:
    - a_buf: a pointer to the buffer struct which contains the actual buffer and its length
    - s: the string which is going to be appended to the buffer
    - len: the length of the string which is going to be appended to the buffer

Returns:
    - Nothing (void)

Notes:
    - This function resizes the buffer dynamically and adds the string to the end.
*/
void append(struct append_buffer* a_buf, char* s, int len) {
    char* temp = realloc(a_buf->buf, a_buf->len + len);
    if(temp == NULL)
        return;
    memcpy(&temp[a_buf->len], s, len);
    a_buf->buf = temp;
    a_buf->len += len;
}

void err(const char* e) {
    write(STDOUT_FILENO, CLEAR, 4); // clears the terminal
    write(STDOUT_FILENO, MV_CUR_LT, 3); // moves the cursor to top left
    perror(e);
    exit(1);
}

void enableRawMode() {
    // get default terminal settings
    if(tcgetattr(STDIN_FILENO, &settings.def) == -1)
        err("store attributes from default terminal");
    
    atexit(disableRawMode);
    
    // copy settings to a new struct
    struct termios raw = settings.def;

    // set the flags
    raw.c_lflag &= ~(ICANON | ECHO | ISIG);
    raw.c_iflag &= ~(IXON | ICRNL);

    // sets the settings from the raw struct to the terminal
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
        err("enable raw mode");
}

void disableRawMode() {
    // returns the terminal to its default settings
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &settings.def) == -1)
        err("disable raw mode");
}

void init() {
    if(getWindowSize(&settings.rows, &settings.cols) == -1)
        err("get window size");
}

void clearScreen(){
    write(STDOUT_FILENO, CLEAR, 4); // clears the terminal
    write(STDOUT_FILENO, MV_CUR_LT, 3); // moves the cursor to top left
}

void refreshScreen() {
    struct append_buffer a_buf = {NULL, 0};

    append(&a_buf, CLEAR, 4); // clears the terminal
    append(&a_buf, MV_CUR_LT, 3); // moves the cursor to top left

    drawRows(&a_buf);

    append(&a_buf, MV_CUR_LT, 3); // moves the cursor to top left after drawing the rows

    write(STDOUT_FILENO, a_buf.buf, a_buf.len);
    free(a_buf.buf);
}

char readKey() {
    char c;

    if(read(STDIN_FILENO, &c, 1) == -1)
        err("reading input");
    
    return c;
}

void processInput() {
    char key = readKey();
    if(key == CTRL_KEY('q')) {
        write(STDOUT_FILENO, CLEAR, 4); // clears the terminal
        write(STDOUT_FILENO, MV_CUR_LT, 3); // moves the cursor to top left
        exit(1);
    }
}

void drawRows(struct append_buffer* a_buf) {
    for(int i = 0; i < settings.rows; i++) {
        append(a_buf, "~", 1);

        if(i < settings.rows - 1)
            append(a_buf, "\r\n", 2);
    }
}

int getWindowSize(int* rows, int* cols) {
    struct winsize win_size;
    if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &win_size) == -1)
        return -1;
    *rows = win_size.ws_row;
    *cols = win_size.ws_col;
    return 0;
}

int main() {
    enableRawMode();
    init();
    while(1) {
        refreshScreen();
        processInput();
    }

    return 0;
}