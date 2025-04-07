#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <string.h>

// Definitions
#define CTRL_KEY(key) (key & 0x1f) // check if the key pressed conflicts with ctrl
#define CLEAR "\x1b[2J"            // clear terminal
#define CLEAR_LINE "\x1b[K"        // clear line
#define MV_CUR_LT "\x1b[H"         // move cursor left top
#define HIDE_CUR "\x1b[?25l"       // hide cursor
#define SHOW_CUR "\x1b[?25h"       // show cursor
#define KORA_VERSION "0.0.1"       // kora version

struct append_buffer;
struct term_settings;

void init();
void clearScreen();
void refreshScreen();
void err(const char* e);
void enableRawMode();
void disableRawMode();
int readKey();
void processInput();
void drawRows(struct append_buffer* a_buf);
int getWindowSize(int* rows, int* cols);
void moveCursor(int key);

struct term_settings {
    struct termios def; // default terminal settings

    // terminal dimensions
    int rows;
    int cols;

    int cur_x;
    int cur_y;
};

struct term_settings settings;

struct append_buffer {
    char* buf;
    int   len;
};

enum Keys {
    ARROW_LEFT = 1000, // big integer index so it doesnt conflict with other chars/keys
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN
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
    settings.cur_x = 0;
    settings.cur_y = 0;

    if(getWindowSize(&settings.rows, &settings.cols) == -1)
        err("get window size");
}

void clearScreen(){
    write(STDOUT_FILENO, CLEAR, 4); // clears the terminal
    write(STDOUT_FILENO, MV_CUR_LT, 3); // moves the cursor to top left
}

void refreshScreen() {
    struct append_buffer a_buf = {NULL, 0};

    append(&a_buf, HIDE_CUR, 6);
    append(&a_buf, MV_CUR_LT, 3); // moves the cursor to top left

    drawRows(&a_buf);

    char buf[32];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", settings.cur_y + 1, settings.cur_x + 1);
    append(&a_buf, buf, strlen(buf));

    // append(&a_buf, MV_CUR_LT, 3); // moves the cursor to top left after drawing the rowsq
    append(&a_buf, SHOW_CUR, 6);

    write(STDOUT_FILENO, a_buf.buf, a_buf.len);
    free(a_buf.buf);
}

int readKey() { // return an int instead of a char because of the key enum
    char c;

    if(read(STDIN_FILENO, &c, 1) == -1)
        err("reading input");
    
    if(c == '\x1b') {
        char arr[3];
        if(read(STDIN_FILENO, &arr[0], 1) != 1)
            return '\x1b';
        if(read(STDIN_FILENO, &arr[1], 1) != 1)
            return '\x1b';
        
        if(arr[0] == '[') {
            if(arr[1] == 'A') return ARROW_UP;
            if(arr[1] == 'B') return ARROW_DOWN;
            if(arr[1] == 'C') return ARROW_RIGHT;
            if(arr[1] == 'D') return ARROW_LEFT;
        }
        return '\x1b';
    }
    
    return c;
}

void processInput() {
    int key = readKey();
    switch(key) {
        case CTRL_KEY('q'):
            write(STDOUT_FILENO, CLEAR, 4); // clears the terminal
            write(STDOUT_FILENO, MV_CUR_LT, 3); // moves the cursor to top left
            exit(0);
            break;
        
        case ARROW_UP:
        case ARROW_LEFT:
        case ARROW_DOWN:
        case ARROW_RIGHT:
            moveCursor(key);
        break;
    }
}

void drawRows(struct append_buffer* a_buf) {
    for(int i = 0; i < settings.rows; i++) {
        if(i == settings.rows / 2) { // draws in the center line
            char msg[80];
            int len = snprintf(msg, sizeof(msg), "Kora -- version %s", KORA_VERSION); // length of the message stored in msg
            
            if(len > settings.cols)
                len = settings.cols;

            int padding = (settings.cols - len) / 2; // draws in the center
            
            if(padding) // checks if padding is not 0 (in the far left of the screen so it can draw tildes)
                append(a_buf, "~", 1);
            
                while(padding--)
                append(a_buf, " ", 1); // adds the required spaces to center the msg
            
                append(a_buf, msg, len);
        } else
            append(a_buf, "~", 1);

        append(a_buf, CLEAR_LINE, 3);
        
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

void moveCursor(int key) { // int instead of a char because of the key enum 
    switch (key) {
    case ARROW_LEFT:
        //TODO FIX ARROW LEFT
        if(settings.cur_x <= 0) {
            if(settings.cur_y <= 1) {
                settings.cur_y = 0;
                settings.cur_x = -1;
            }
            settings.cur_x = settings.cols;
            settings.cur_y--;
        }
        settings.cur_x--;
        break;
    case ARROW_RIGHT:
        if(settings.cur_x >= settings.cols - 1) {
            if(settings.cur_y >= settings.rows - 1)
                settings.cur_x = settings.cols - 2;
            else {
                settings.cur_y++;
                settings.cur_x = -1;
            }
        }
        settings.cur_x++;
        break;
    case ARROW_UP:
        if(settings.cur_y >= 1)
            settings.cur_y--;
        break;
    case ARROW_DOWN:
        if(settings.cur_y < settings.rows - 1)
            settings.cur_y++;
        break;
    }
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