#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#define CTRL_KEY(key) (key & 0x1f)

struct termios orig;

void enableRawMode() {
    tcgetattr(STDIN_FILENO, &orig);
    struct termios raw = orig;
    raw.c_lflag &= ~(ICANON | ECHO | ISIG);
    raw.c_iflag &= ~(IXON | ICRNL);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig);
}

int main() {
    enableRawMode();
    char c;
    while(1) {
        read(STDIN_FILENO, &c, 1);
        if(c == CTRL_KEY('q'))
            break;
        printf("%c\r\n", c);
    }
    atexit(disableRawMode);

    return 0;
}
