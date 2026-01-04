#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char comline[256];
char *argv[64];
void Startup(){
    printf("Starting quip 0.1...\n");
//    system("fastfetch");
}
void prompter(){
    printf("User%-> ");
    fgets(comline, sizeof(comline), stdin);
}
int main() {
    Startup();
    while (1) {
        prompter();
            //tokenize input
            int i = 0;
            char *token = strtok(comline, " \n");
            while (token != NULL) {
            argv[i++] = token;
            token = strtok(NULL, " \n");
            }
            argv[i] = NULL;
            //Built-in commands
            if(strcmp(argv[0], "cd") == 0){
                if (argv[1] != NULL)
                {
                    if (chdir(argv[1]) != 0)
                    {
                        perror("chdir failed");
                    }
                } 
                else 
                {
                    fprintf(stderr, "cd: missing operand\n");
                    continue;
                }
            }
            if (strcmp(argv[0], "clear") == 0)
            {
                system("clear");
            }
            if (strcmp(argv[0], "exit") == 0) 
            {
                break;
            }
            if (strcmp(argv[0], "help") == 0) 
            {
                    printf("Not implemented yet.\n");
            }
    }
    return 0;
}