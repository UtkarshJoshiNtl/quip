#include "quip.h"

#define MAX_JOBS 32

typedef struct {
    pid_t pid;
    char command[MAX_LINE];
    int status;
} job_t;

static job_t jobs[MAX_JOBS];
static int job_count = 0;

void jobs_init(void) {
    for (int i = 0; i < MAX_JOBS; i++) {
        jobs[i].pid = 0;
        jobs[i].command[0] = '\0';
        jobs[i].status = 0;
    }
    job_count = 0;
}

int add_job(pid_t pid, const char *command) {
    if (job_count >= MAX_JOBS) {
        return -1;
    }
    
    jobs[job_count].pid = pid;
    strncpy(jobs[job_count].command, command, MAX_LINE - 1);
    jobs[job_count].command[MAX_LINE - 1] = '\0';
    jobs[job_count].status = 1;
    
    return job_count++;
}

void remove_job(int job_id) {
    if (job_id < 0 || job_id >= job_count) {
        return;
    }
    
    for (int i = job_id; i < job_count - 1; i++) {
        jobs[i] = jobs[i + 1];
    }
    
    job_count--;
}

void cleanup_jobs(void) {
    for (int i = 0; i < job_count; i++) {
        int status;
        if (waitpid(jobs[i].pid, &status, WNOHANG) > 0) {
            remove_job(i);
            i--;
        }
    }
}

int builtin_jobs(char **argv) {
    (void)argv;
    cleanup_jobs();
    
    if (job_count == 0) {
        printf("No active jobs.\n");
        return 1;
    }
    
    for (int i = 0; i < job_count; i++) {
        printf("[%d] %d %s %s\n", 
               i + 1, 
               jobs[i].pid, 
               jobs[i].status ? "Running" : "Stopped",
               jobs[i].command);
    }
    
    return 1;
}

int builtin_fg(char **argv) {
    if (argv[1] == NULL) {
        fprintf(stderr, "fg: job ID required\n");
        return 0;
    }
    
    int job_id = atoi(argv[1]) - 1;
    if (job_id < 0 || job_id >= job_count) {
        fprintf(stderr, "fg: invalid job ID\n");
        return 0;
    }
    
    pid_t pid = jobs[job_id].pid;
    
    if (kill(pid, SIGCONT) == -1) {
        perror("kill SIGCONT failed");
        return 0;
    }
    
    int status;
    if (waitpid(pid, &status, 0) == -1) {
        perror("waitpid failed");
        return 0;
    }
    
    remove_job(job_id);
    return 1;
}

int builtin_bg(char **argv) {
    if (argv[1] == NULL) {
        fprintf(stderr, "bg: job ID required\n");
        return 0;
    }
    
    int job_id = atoi(argv[1]) - 1;
    if (job_id < 0 || job_id >= job_count) {
        fprintf(stderr, "bg: invalid job ID\n");
        return 0;
    }
    
    pid_t pid = jobs[job_id].pid;
    
    if (kill(pid, SIGCONT) == -1) {
        perror("kill SIGCONT failed");
        return 0;
    }
    
    jobs[job_id].status = 1;
    printf("[%d] %d %s &\n", job_id + 1, pid, jobs[job_id].command);
    
    return 1;
}

int is_background_command(const char *cmd) {
    if (!cmd) return 0;
    
    size_t len = strlen(cmd);
    if (len == 0) return 0;
    
    return cmd[len - 1] == '&';
}

char *strip_background_ampersand(char *cmd) {
    if (!cmd) return cmd;
    
    size_t len = strlen(cmd);
    if (len == 0) return cmd;
    
    if (cmd[len - 1] == '&') {
        cmd[len - 1] = '\0';
        
        len--;
        while (len > 0 && (cmd[len - 1] == ' ' || cmd[len - 1] == '\t')) {
            cmd[len - 1] = '\0';
            len--;
        }
    }
    
    return cmd;
}
