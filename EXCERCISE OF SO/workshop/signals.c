#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>

#define MAX_LEN 64

int count=0;

void handler1(int sig);
void handler2(int sig);

void showtree();

void send_word(pid_t pid, const char *wrd);
void send_char(pid_t pid, char c);

int main(int argc, char **argv){
    if(argc < 2){
        perror("Send argument\n");
        return EXIT_FAILURE;
    }
    int n_children = 2, child_id;
    pid_t root = getpid(), children[n_children];

    signal(SIGUSR1, handler1);
    signal(SIGUSR2, handler2);

    for(child_id=0; child_id < n_children; child_id++){
        if(!(children[child_id] = fork())){
            break;
        }
    }

    if(root == getpid()){
        showtree();
        char word[MAX_LEN];
        int line = 0;
        FILE *fl = fopen(argv[1], "r");

        while(fgets(word, sizeof(word), fl)){
            word[strcspn(word, "\n")] = 0;
            if(line % 2 == 0){
                send_word(children[0], word);
            }else{
                send_word(children[n_children-1], word);
            }

            line++;
            usleep(150000);
        }

        fclose(fl);
        kill(children[0], SIGTERM);
        kill(children[n_children-1], SIGTERM);
        for(int i=0; i < n_children; i++) wait(NULL);
    }else{
        while(1) pause();
    }

    return EXIT_SUCCESS;
}

void send_word(pid_t pid, const char *wrd){
    char buff;
    for(int i=0; i < strlen(wrd); i++){
        buff = wrd[i];
        //printf("[%c] ", buff);
        send_char(pid, buff);
    }
    //printf("\n");
    kill(pid, SIGUSR2);
    usleep(30000);
}

void send_char(pid_t pid, char c){
    for(int i=0; i < c; i++){
        kill(pid, SIGUSR1);
        usleep(8000);
    }
    kill(pid, SIGUSR2);
    usleep(20000);
}

void handler2(int sig){
    if(count > 0){
        printf("%c [%d], ", (char)count, count);
        //flush(stdout);
        count = 0;
    }else{
        printf("\n");
        fflush(stdout);
    }
}

void handler1(int sig){
    count++;
}

void showtree(){
    char cmd[20] = {""};
    sprintf(cmd, "pstree -cAlp %d", getpid());
    system(cmd);	
}