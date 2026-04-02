#include "quip.h"

static struct termios orig_termios;

void terminal_init(void) {
    if (!isatty(STDIN_FILENO)) {
        return;
    }
    
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
        perror("tcgetattr failed");
        return;
    }
    
    atexit(terminal_cleanup);
    
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        perror("tcsetattr failed");
    }
}

void terminal_cleanup(void) {
    if (!isatty(STDIN_FILENO)) {
        return;
    }
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) {
        perror("tcsetattr cleanup failed");
    }
}
