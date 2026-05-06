#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>

#define MAX_USERS 10001

typedef struct {
    int user_id;
    int video_id;
    int aspect;
    int value;
} Report;

typedef struct {
    int user_id;
    int video_id;

    int face_score;
    int audio_score;
    int sync_score;

    int has_face;
    int has_audio;
    int has_sync;
} Video;

typedef struct {
    int user_id;
    int count;
} Result;

void showtree();

int main(int argc, char **argv){
    if(argc < 2){
        perror("Send file\n");
        return EXIT_FAILURE;
    }
    pid_t root = getpid();
    int n_children = 2, child_id;
    int n_pipes = n_children + 1;
    int pipes[n_pipes][2];

    for(int i=0; i < n_pipes; i++){
        if(pipe(pipes[i]) == -1){
            perror("Fail pipe");
            exit(1);
        }
    }

    for(child_id=0; child_id < n_children; child_id++){
        if(!fork()) break;
    }

    if(root == getpid()){ //Parent Process
        showtree();
        FILE *fl = fopen(argv[1], "r");
        if(!fl){exit(1);}

        for(int i=0; i < n_pipes; i++){
            if(i != 0){
                close(pipes[i][1]); 
            }
            
            if(i != n_pipes-1){
                close(pipes[i][0]);
            }
        }
        int B, N;
        int total_ban, total_susp;
        fscanf(fl, "%d %d", &B, &N);
        printf("B %d, N: %d\n", B, N);

        total_ban = total_susp = 0; 

        write(pipes[0][1], &B, sizeof(int));

        Report resp;
        Result result;

        for(int i=0; i < N; i++){
            fscanf(fl, "%d %d %d %d", &resp.user_id, &resp.video_id, &resp.aspect, &resp.value);
            write(pipes[0][1], &resp, sizeof(Report));
        }

        resp.user_id = -1;
        write(pipes[0][1], &resp, sizeof(Report));
        close(pipes[0][1]);

        for(int i=0; i < n_children; i++) wait(NULL);

        while(read(pipes[n_pipes-1][0], &result, sizeof(Result)) > 0){
            if(result.user_id == -1){   
                total_susp = result.count;
                break;
            }
            printf("%d %d\n", result.user_id, result.count);
            total_ban++;
        }
        close(pipes[n_pipes-1][0]);

        printf("All banned: %d\n", total_ban);
        printf("All suspicious: %d\n", total_susp);

    }else{ //Child Process
        for(int i=0; i < n_pipes; i++){
            if(i != child_id){
                close(pipes[i][0]);
            }
            
            if(i != child_id + 1){
                close(pipes[i][1]); 
            }
        }
        int limit = 0;

        int B;

        read(pipes[child_id][0], &B, sizeof(int));
        if(child_id == 0){
            write(pipes[child_id+1][1], &B, sizeof(int));
        }
        //printf("Child_id: %d, B: %d\n", child_id, B);

        if(child_id == 0){
            Report rp;
            Video all_videos[MAX_USERS], vid;
            memset(all_videos, 0, sizeof(all_videos));

            while(read(pipes[child_id][0], &rp, sizeof(Report)) > 0){
                if(rp.user_id == -1) break;
                all_videos[rp.video_id].user_id = rp.user_id;
                all_videos[rp.video_id].video_id = rp.video_id;
                //printf("Video: %d - %d\n", rp.user_id, rp.video_id);
                //printf("Aspect: %d\n", rp.aspect);
                switch(rp.aspect){
                    case 1:{
                        all_videos[rp.video_id].has_face = 1;
                        all_videos[rp.video_id].face_score = rp.value;
                        break;
                    }
                    case 2:{
                        all_videos[rp.video_id].has_audio = 1;
                        all_videos[rp.video_id].audio_score = rp.value;
                        break;
                    }
                    case 3:{
                        all_videos[rp.video_id].has_sync = 1;
                        all_videos[rp.video_id].sync_score = rp.value;
                        break;
                    }
                }

                //printf("AS: %d, FS: %d, SYC: %d\n", all_videos[rp.user_id].has_audio, all_videos[rp.user_id].has_face, all_videos[rp.user_id].has_sync);

                if(all_videos[rp.video_id].has_audio && all_videos[rp.video_id].has_face && all_videos[rp.video_id].has_sync){
                    //printf("Sending Video Complete: %d\n", rp.video_id);
                    Video *v = &all_videos[rp.video_id];
                    write(pipes[child_id+1][1], v, sizeof(Video));
                    v->has_face = 0;
                    v->has_audio = 0;
                    v->has_sync = 0;
                }
            }
            vid.user_id = -1;
            write(pipes[child_id+1][1], &vid, sizeof(Video)); //-1
        }else{
            int suspicious_vid[MAX_USERS] = {0};
            Result result;
            int count, total;
            count = total = 0;
            Video vid;

            while(read(pipes[child_id][0], &vid, sizeof(vid)) > 0){
                //printf("Video: %d - %d\n", vid.user_id, vid.video_id);
                if(vid.user_id == -1) break;
                count = 0;

                if(vid.face_score >= 80) count++;
                if(vid.audio_score >= 75) count++;
                if(vid.sync_score <= 35) count++;

                if(count >= 2){
                    suspicious_vid[vid.user_id]++;
                    total++;
                }
            }

            for(int i=0; i < MAX_USERS; i++){
                if(suspicious_vid[i] > 3){
                    //printf("Sending user: %d\n", i);
                    result.user_id = i;
                    result.count = suspicious_vid[i];
                    write(pipes[child_id+1][1], &result, sizeof(Result));
                }
            }

            result.user_id = -1;
            result.count = total;
            write(pipes[child_id+1][1], &result, sizeof(Result));
        }

        close(pipes[child_id][0]);
        close(pipes[child_id+1][1]);
    }

    return EXIT_SUCCESS;
}

void showtree(){
    char cmd[20] = {""};
    sprintf(cmd, "pstree -cAlp %d", getpid());
    system(cmd);	
}
