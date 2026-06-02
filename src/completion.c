#include "quip.h"
#include <dirent.h>

static const char *builtin_commands[] = {
    "cd", "pwd", "help", "history", "clear", "echo", "env",
    "jobs", "fg", "bg", "exit", NULL
};

static int builtin_commands_lookup(const char *name) {
    for (int i = 0; builtin_commands[i]; i++) {
        if (strcmp(builtin_commands[i], name) == 0)
            return 1;
    }
    return 0;
}

static int count_matches(const char *prefix, char **matches, int max_matches) {
    int count = 0;
    size_t plen = strlen(prefix);

    for (int i = 0; builtin_commands[i]; i++) {
        if (strncmp(builtin_commands[i], prefix, plen) == 0) {
            if (matches && count < max_matches)
                matches[count] = strdup(builtin_commands[i]);
            if (matches && matches[count] == NULL)
                return count;
            count++;
        }
    }

    DIR *dir = opendir(".");
    if (dir) {
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (strncmp(entry->d_name, prefix, plen) == 0) {
                if (matches && count < max_matches) {
                    matches[count] = strdup(entry->d_name);
                    if (matches[count] == NULL) {
                        closedir(dir);
                        return count;
                    }
                }
                count++;
            }
        }
        closedir(dir);
    }

    return count;
}

static const char *find_longest_common_prefix(char **matches, int count) {
    if (count == 0) return NULL;
    if (count == 1) return matches[0];

    size_t min_len = strlen(matches[0]);
    for (int i = 1; i < count; i++) {
        size_t len = strlen(matches[i]);
        if (len < min_len) min_len = len;
    }

    static char prefix[MAX_LINE];
    size_t prefix_len = 0;

    for (size_t i = 0; i < min_len; i++) {
        char c = matches[0][i];
        int all_match = 1;
        for (int j = 1; j < count; j++) {
            if (matches[j][i] != c) {
                all_match = 0;
                break;
            }
        }
        if (all_match)
            prefix[prefix_len++] = c;
        else
            break;
    }

    prefix[prefix_len] = '\0';
    return prefix;
}

void handle_completion(char *buf, int *pos, int *len) {
    if (*pos == 0) return;

    int copy_len = *pos < MAX_LINE ? *pos : MAX_LINE - 1;
    char prefix[MAX_LINE];
    strncpy(prefix, buf, copy_len);
    prefix[copy_len] = '\0';

    char *matches[MAX_ARGS];
    int raw_count = count_matches(prefix, matches, MAX_ARGS);
    int match_count = raw_count < MAX_ARGS ? raw_count : MAX_ARGS;

    if (match_count == 0) return;

    if (match_count == 1) {
        size_t match_len = strlen(matches[0]);
        size_t prefix_len = strlen(prefix);

        if (match_len > prefix_len) {
            for (size_t i = prefix_len; i < match_len && *len < MAX_LINE - 1; i++) {
                memmove(buf + *pos + 1, buf + *pos, *len - *pos);
                buf[*pos] = matches[0][i];
                (*pos)++;
                (*len)++;
                buf[*len] = '\0';
            }
            write(STDOUT_FILENO, buf + *pos - (match_len - prefix_len),
                  match_len - prefix_len);
        }

        for (int i = 0; i < match_count; i++)
            free(matches[i]);

    } else {
        const char *common_prefix = find_longest_common_prefix(matches, match_count);
        if (common_prefix && strlen(common_prefix) > strlen(prefix)) {
            size_t common_len = strlen(common_prefix);
            size_t prefix_len = strlen(prefix);

            for (size_t i = prefix_len; i < common_len && *len < MAX_LINE - 1; i++) {
                memmove(buf + *pos + 1, buf + *pos, *len - *pos);
                buf[*pos] = common_prefix[i];
                (*pos)++;
                (*len)++;
                buf[*len] = '\0';
            }
            write(STDOUT_FILENO, buf + *pos - (common_len - prefix_len),
                  common_len - prefix_len);
        } else {
            write(STDOUT_FILENO, "\n", 1);

            for (int i = 0; i < match_count; i++) {
                size_t plen = strlen(prefix);
                if (strncmp(matches[i], prefix, plen) == 0 &&
                    builtin_commands_lookup(matches[i]))
                    printf(ANSI_ACCENT "  %s\n" ANSI_RESET, matches[i]);
                else
                    printf(ANSI_GRAY "  %s\n" ANSI_RESET, matches[i]);
            }

            write(STDOUT_FILENO, "\r", 1);
            print_prompt();
            printf("%s", buf);
            fflush(stdout);
        }

        for (int i = 0; i < match_count; i++)
            free(matches[i]);
    }
}
