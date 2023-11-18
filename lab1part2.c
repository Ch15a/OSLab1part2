#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int wordCount(char* msg, int bufLen) {
    int count = 0;
    for (int i = 0; i < bufLen; i++) {
        if (msg[i] == ' ' || msg[i] == '\n') {
            msg++;
        }
    }
    return count;
}

int main() {
    mqd_t msgq = mq_open("/msgq", O_RDWR | O_CREAT);
    struct mq_attr mqattr;
    if (mq_getattr(msgq, &mqattr) == -1) {
        exit(1);
    }
    pid_t pid = fork();
    if (pid == -1) {
        exit(1);
    }
    else if (pid == 0) {
        int numRead;
        char* rBuffer = malloc(mqattr.mq_msgsize);
        numRead = mq_receive(msgq, rBuffer, mqattr.mq_msgsize, NULL);
        printf("Number of words: %d\n", wordCount(rBuffer, numRead));
        fflush(stdout);
        free(rBuffer);
        mq_close(msgq);
        mq_unlink("/msgq");
        exit(0);
    }
    else {
        FILE *file;
        file = fopen("test.txt", "r");
        if (file == NULL) {
            fclose(file);
            exit(1);
        }
        char* wBuffer = malloc(mqattr.mq_msgsize);
        char ch;
        int offset = 0;
        do {
            ch = fgetc(file);
            if (ch != EOF) {
                wBuffer[offset++] = ch;;
                if (offset == mqattr.mq_msgsize) {
                    mq_send(msgq, wBuffer, mqattr.mq_msgsize, 1);
                }
            }
            else {
                for(int i = offset; i < mqattr.mq_msgsize; i++) {
                    wBuffer[i] = 0;;
                }
                mq_send(msgq, wBuffer, mqattr.mq_msgsize, 1); 
            }
        } while (ch != EOF);
        fclose(file);
        mq_close(msgq);
        mq_unlink("/msgq");
        free(wBuffer);
        exit(0);
    }
    return 0;
}
