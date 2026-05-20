#include "quip.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <stddef.h>
#include <unistd.h>
#include <signal.h>

#define PLUGIN_SOCKET "\0quip-daemon"
#define PLUGIN_SOCKET_LEN 12

static int daemon_launched = 0;

static int plugin_connect(void) {
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) return -1;

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    memcpy(addr.sun_path, PLUGIN_SOCKET, PLUGIN_SOCKET_LEN);

    socklen_t addr_len = (socklen_t)(offsetof(struct sockaddr_un, sun_path) + PLUGIN_SOCKET_LEN);
    if (connect(fd, (struct sockaddr *)&addr, addr_len) == -1) {
        close(fd);
        return -1;
    }

    return fd;
}

static int plugin_send(int fd, const char *json) {
    if (fd < 0) return -1;

    uint32_t len = htonl((uint32_t)strlen(json));
    if (write(fd, &len, 4) < 0) return -1;
    if (write(fd, json, strlen(json)) < 0) return -1;
    return 0;
}

static char *plugin_recv(int fd, size_t *out_len) {
    if (fd < 0) return NULL;

    uint32_t raw_len;
    ssize_t n = read(fd, &raw_len, 4);
    if (n < 4) return NULL;

    uint32_t msg_len = ntohl(raw_len);
    if (msg_len > 65536) return NULL;

    char *buf = malloc(msg_len + 1);
    if (!buf) return NULL;

    size_t total = 0;
    while (total < msg_len) {
        n = read(fd, buf + total, msg_len - total);
        if (n <= 0) {
            free(buf);
            return NULL;
        }
        total += (size_t)n;
    }
    buf[total] = '\0';
    if (out_len) *out_len = total;
    return buf;
}

static int plugin_launch_daemon(void) {
    pid_t pid = fork();
    if (pid < 0) return -1;

    if (pid == 0) {
        signal(SIGINT, SIG_DFL);
        signal(SIGTERM, SIG_DFL);
        signal(SIGCHLD, SIG_DFL);
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);

        const char *search_paths[] = {
            "/usr/local/share/quip/quip_daemon.py",
            "/usr/share/quip/quip_daemon.py",
            "python/quip_daemon.py",
            NULL
        };

        const char *daemon_path = NULL;
        static char buf[1024];

        ssize_t link_len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
        if (link_len > 0) {
            buf[link_len] = '\0';
            char *slash = strrchr(buf, '/');
            if (slash) {
                *slash = '\0';
                snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
                         "/../python/quip_daemon.py");
                if (access(buf, F_OK) == 0)
                    daemon_path = buf;
            }
        }

        for (int i = 0; !daemon_path && search_paths[i]; i++) {
            if (access(search_paths[i], F_OK) == 0) {
                daemon_path = search_paths[i];
            }
        }

        if (!daemon_path)
            daemon_path = "quip_daemon.py";

        execlp("python3", "python3", daemon_path, (char *)NULL);
        execlp("python", "python", daemon_path, (char *)NULL);
        _exit(1);
    }

    usleep(200000);

    for (int i = 0; i < 10; i++) {
        int fd = plugin_connect();
        if (fd >= 0) {
            close(fd);
            daemon_launched = 1;
            return 0;
        }
        usleep(100000);
    }

    int status;
    waitpid(pid, &status, WNOHANG);
    return -1;
}

void plugin_init(void) {
    daemon_launched = 0;
}

int plugin_available(void) {
    if (daemon_launched) return 1;

    const char *config_dir = get_config_path();
    if (!config_dir) return 0;

    char plugins_dir[576];
    snprintf(plugins_dir, sizeof(plugins_dir), "%s/plugins", config_dir);

    DIR *dir = opendir(plugins_dir);
    if (!dir) return 0;

    int count = 0;
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        size_t len = strlen(entry->d_name);
        if (len > 3 && strcmp(entry->d_name + len - 3, ".py") == 0 &&
            entry->d_name[0] != '_') {
            count++;
        }
    }
    closedir(dir);
    return count > 0;
}

static void write_unescaped(const char *s, size_t slen, FILE *stream) {
    for (size_t i = 0; i < slen; i++) {
        if (s[i] == '\\' && i + 1 < slen) {
            switch (s[i+1]) {
                case 'n': fputc('\n', stream); i++; break;
                case 't': fputc('\t', stream); i++; break;
                case '\\': fputc('\\', stream); i++; break;
                case '"': fputc('"', stream); i++; break;
                default: fputc(s[i], stream); break;
            }
        } else {
            fputc(s[i], stream);
        }
    }
    fflush(stream);
}

int plugin_exec(char **argv) {
    if (!argv || !argv[0]) return -1;

    int fd = plugin_connect();
    if (fd < 0) {
        if (!daemon_launched && plugin_launch_daemon() < 0)
            return -1;
        fd = plugin_connect();
        if (fd < 0)
            return -1;
    }

    char cwd[1024];
    if (!getcwd(cwd, sizeof(cwd)))
        strncpy(cwd, "/", sizeof(cwd) - 1);

    char json[8192];
    int pos = snprintf(json, sizeof(json),
        "{\"type\":\"exec\",\"plugin\":\"%s\",\"argv\":[", argv[0]);

    for (int i = 0; argv[i] != NULL && pos < (int)sizeof(json) - 64; i++) {
        if (i > 0) pos += snprintf(json + pos, sizeof(json) - (size_t)pos, ",");
        pos += snprintf(json + pos, sizeof(json) - (size_t)pos, "\"%s\"", argv[i]);
    }

    pos += snprintf(json + pos, sizeof(json) - (size_t)pos,
        "],\"cwd\":\"%s\"}", cwd);

    if (plugin_send(fd, json) < 0) {
        close(fd);
        return -1;
    }

    size_t resp_len;
    char *resp = plugin_recv(fd, &resp_len);
    close(fd);

    if (!resp)
        return -1;

    int exit_code = -1;

    if (resp_len > 0) {
        const char *p = resp;
        const char *code_start = strstr(p, "\"exit_code\": ");
        if (!code_start) code_start = strstr(p, "\"exit_code\":");

        if (code_start) {
            code_start += (code_start[12] == ' ') ? 13 : 12;
            exit_code = atoi(code_start);
        }

        if (exit_code == 127) {
            free(resp);
            return -1;
        }

        const char *out = strstr(p, "\"stdout\": ");
        if (!out) out = strstr(p, "\"stdout\":\"");
        if (out) {
            out += (out[9] == ' ') ? 10 : 9;
            const char *end = strchr(out, '"');
            if (end && end > out)
                write_unescaped(out, (size_t)(end - out), stdout);
        }

        const char *err = strstr(p, "\"stderr\": ");
        if (!err) err = strstr(p, "\"stderr\":\"");
        if (err) {
            err += (err[9] == ' ') ? 10 : 9;
            const char *end = strchr(err, '"');
            if (end && end > err)
                write_unescaped(err, (size_t)(end - err), stderr);
        }
    }

    free(resp);
    return (exit_code >= 0) ? 0 : -1;
}

void plugin_cleanup(void) {
    int fd = plugin_connect();
    if (fd >= 0) {
        plugin_send(fd, "{\"type\":\"shutdown\"}");
        close(fd);
    }
}
