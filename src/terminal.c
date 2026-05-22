#include "quip.h"

static struct termios orig_termios;
static int terminal_initialized = 0;

void terminal_init(void) {
    if (!isatty(STDIN_FILENO)) {
        return;
    }

    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
        perror("tcgetattr failed");
        return;
    }

    struct termios raw = orig_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= CS8;
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        perror("tcsetattr failed");
        return;
    }

    terminal_initialized = 1;
}

void terminal_cleanup(void) {
    if (!terminal_initialized) {
        return;
    }
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) {
        perror("tcsetattr cleanup failed");
    }
    terminal_initialized = 0;
}
