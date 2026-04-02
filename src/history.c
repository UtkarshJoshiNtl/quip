#include "quip.h"

static char *history[MAX_HISTORY];
static int history_len = 0;

void history_init(void) {
    for (int i = 0; i < MAX_HISTORY; i++) {
        history[i] = NULL;
    }
    history_len = 0;
}

void history_cleanup(void) {
    for (int i = 0; i < history_len; i++) {
        free(history[i]);
        history[i] = NULL;
    }
    history_len = 0;
}

void history_push(const char *line) {
    if (!line || line[0] == '\0') return;
    
    if (history_len > 0 && strcmp(history[history_len - 1], line) == 0)
        return;
    
    if (history_len == MAX_HISTORY) {
        free(history[0]);
        memmove(history, history + 1, (MAX_HISTORY - 1) * sizeof(char *));
        history_len--;
    }
    
    history[history_len] = strdup(line);
    if (history[history_len]) {
        history_len++;
    }
}

const char *history_get(int index) {
    if (index < 0 || index >= history_len) {
        return NULL;
    }
    return history[index];
}

int history_size(void) {
    return history_len;
}
