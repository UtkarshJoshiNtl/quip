#include "quip.h"
#include <sys/stat.h>

typedef struct {
    char key[CONFIG_KEY_LEN];
    char val[CONFIG_VAL_LEN];
} config_entry_t;

static config_entry_t config_entries[CONFIG_MAX_ENTRIES];
static int config_count = 0;

static char data_path[512] = {0};
static char config_path[512] = {0};

static void ensure_dir(const char *path) {
    struct stat st;
    if (stat(path, &st) == -1 && errno == ENOENT) {
        if (mkdir(path, 0755) == -1) {
            fprintf(stderr, ANSI_DIM "quip: " ANSI_RED "mkdir(%s): %s\n" ANSI_RESET, path, strerror(errno));
        }
    }
}

static void build_xdg_paths(void) {
    const char *xdg_config = getenv("XDG_CONFIG_HOME");
    const char *xdg_data = getenv("XDG_DATA_HOME");
    const char *home = getenv("HOME");

    if (xdg_config && xdg_config[0]) {
        snprintf(config_path, sizeof(config_path), "%s/quip", xdg_config);
    } else if (home && home[0]) {
        snprintf(config_path, sizeof(config_path), "%s/.config/quip", home);
    } else {
        snprintf(config_path, sizeof(config_path), "/tmp/quip");
    }

    if (xdg_data && xdg_data[0]) {
        snprintf(data_path, sizeof(data_path), "%s/quip", xdg_data);
    } else if (home && home[0]) {
        snprintf(data_path, sizeof(data_path), "%s/.local/share/quip", home);
    } else {
        snprintf(data_path, sizeof(data_path), "/tmp/quip");
    }
}

const char *get_config_path(void) {
    if (config_path[0] == '\0')
        build_xdg_paths();
    return config_path;
}

const char *get_data_path(void) {
    if (data_path[0] == '\0')
        build_xdg_paths();
    return data_path;
}

void config_init(void) {
    build_xdg_paths();

    ensure_dir(config_path);
    ensure_dir(data_path);

    char filepath[576];
    snprintf(filepath, sizeof(filepath), "%s/config", config_path);

    FILE *f = fopen(filepath, "r");
    if (!f) return;

    char line[512];
    while (fgets(line, sizeof(line), f) && config_count < CONFIG_MAX_ENTRIES) {
        char *p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '#' || *p == ';' || *p == '\n' || *p == '\0') continue;

        char *eq = strchr(p, '=');
        if (!eq) continue;

        size_t key_len = eq - p;
        if (key_len == 0 || key_len >= CONFIG_KEY_LEN) continue;

        strncpy(config_entries[config_count].key, p, key_len);
        config_entries[config_count].key[key_len] = '\0';

        char *v = eq + 1;
        while (*v == ' ' || *v == '\t') v++;

        size_t vlen = strlen(v);
        while (vlen > 0 && (v[vlen-1] == '\n' || v[vlen-1] == '\r' || v[vlen-1] == ' ' || v[vlen-1] == '\t'))
            vlen--;
        if (vlen >= CONFIG_VAL_LEN) vlen = CONFIG_VAL_LEN - 1;

        strncpy(config_entries[config_count].val, v, vlen);
        config_entries[config_count].val[vlen] = '\0';

        config_count++;
    }
    fclose(f);
}

const char *config_get(const char *key) {
    for (int i = 0; i < config_count; i++) {
        if (strcmp(config_entries[i].key, key) == 0)
            return config_entries[i].val;
    }
    return NULL;
}

int config_get_int(const char *key, int default_val) {
    const char *val = config_get(key);
    if (!val) return default_val;
    char *end;
    errno = 0;
    long result = strtol(val, &end, 10);
    if (errno == ERANGE || *end != '\0' || result < 0) return default_val;
    return (int)result;
}
