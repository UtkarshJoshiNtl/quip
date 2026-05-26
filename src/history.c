#include "quip.h"

static char **history = NULL;
static int history_max = 0;
static int history_len = 0;
static int history_head = 0;

static char history_file[576] = {0};

static void history_save_to_disk(void) {
    if (history_file[0] == '\0' || !history) return;

    char tmpfile[640];
    snprintf(tmpfile, sizeof(tmpfile), "%s.tmp", history_file);

    FILE *f = fopen(tmpfile, "w");
    if (!f) return;

    for (int i = 0; i < history_len; i++) {
        int idx = (history_head + i) % history_max;
        if (history[idx]) {
            fprintf(f, "%s\n", history[idx]);
        }
    }
    fclose(f);
    if (rename(tmpfile, history_file) != 0) {
        remove(tmpfile);
    }
}

void history_init(void) {
    history_max = config_get_int("history_size", 128);
    if (history_max < 16) history_max = 16;
    if (history_max > 4096) history_max = 4096;

    history = calloc((size_t)history_max, sizeof(char *));
    if (!history) {
        history_max = 0;
        return;
    }
    history_len = 0;
    history_head = 0;

    const char *data_dir = get_data_path();
    if (data_dir && data_dir[0]) {
        snprintf(history_file, sizeof(history_file), "%s/history", data_dir);

        FILE *f = fopen(history_file, "r");
        if (f) {
            char line[MAX_LINE];
            while (fgets(line, sizeof(line), f)) {
                size_t len = strlen(line);
                if (len > 0 && line[len-1] != '\n' && !feof(f)) {
                    int ch;
                    while ((ch = fgetc(f)) != EOF && ch != '\n');
                    continue;
                }
                while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r'))
                    line[--len] = '\0';
                if (len == 0) continue;

                if (history_len >= history_max) {
                    free(history[history_head]);
                    history[history_head] = NULL;
                    history_head = (history_head + 1) % history_max;
                    history_len--;
                }
                int idx = (history_head + history_len) % history_max;
                history[idx] = strdup(line);
                if (history[idx]) history_len++;
            }
            fclose(f);
        }
    }
}

void history_cleanup(void) {
    if (!history) return;
    history_save_to_disk();
    for (int i = 0; i < history_len; i++) {
        int idx = (history_head + i) % history_max;
        free(history[idx]);
        history[idx] = NULL;
    }
    free(history);
    history = NULL;
    history_len = 0;
    history_max = 0;
    history_head = 0;
}

void history_push(const char *line) {
    if (!line || line[0] == '\0' || !history) return;

    if (history_len > 0) {
        int last = (history_head + history_len - 1) % history_max;
        if (history[last] && strcmp(history[last], line) == 0)
            return;
    }

    if (history_len == history_max) {
        free(history[history_head]);
        history[history_head] = NULL;
        history_head = (history_head + 1) % history_max;
        history_len--;
    }

    int idx = (history_head + history_len) % history_max;
    history[idx] = strdup(line);
    if (history[idx]) {
        history_len++;
    }
}

const char *history_get(int index) {
    if (index < 0 || index >= history_len) {
        return NULL;
    }
    int idx = (history_head + index) % history_max;
    return history[idx];
}

int history_size(void) {
    return history_len;
}
