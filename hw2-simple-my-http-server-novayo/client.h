#ifndef CLIENT_H
#define CLIENT_H
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>

#define BUFFER 9999
#define TMPBUFFER 150

char *sendM(char *sendMesg, char *gMesg);
void find_all(char *sendMesg, char *gMesg);
void *createDir(void *arg);
char LOCALHOST[TMPBUFFER];
char PORT[TMPBUFFER];
int port;
int level;

char *sendM(char *sendMesg, char *gMesg)
{
    char *bu = malloc(BUFFER);
    memset(bu, '\0', BUFFER);
    int sock = 0;
    struct sockaddr_in serv_addr;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    inet_pton(AF_INET, LOCALHOST, &serv_addr.sin_addr);
    connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    //send
    send(sock, sendMesg, strlen(sendMesg), 0);
    recv(sock, bu, BUFFER, MSG_WAITALL);
    printf("%s\n",bu);
    return bu;
}

void find_all(char *sendMesg, char *gMesg)
{
    char buffer[BUFFER] = {'\0'};
    char *x = sendM(sendMesg, gMesg);
    strcpy(buffer, x);
    free(x);
    char tmp_buffer[BUFFER] = {'\0'};
    strcpy(tmp_buffer, buffer);
    char *remain = strstr(tmp_buffer, "directory");
    // if is directory
    if (remain) {
        char str[BUFFER] = {'\0'};
        strcpy(str, remain);
        char *pch;
        pch = strstr(str, "\r\n\r\n");
        int i=0;
        for (i=0; i<strlen(pch); i++) pch[i] = pch[i+4];

        char tmp_remain[TMPBUFFER] = {'\0'};
        strcpy(tmp_remain, pch);

        char *substr = strtok(tmp_remain, " ");
        int c=0;
        char child[TMPBUFFER][TMPBUFFER] = {'\0'};
        while(substr) {
            strcpy(child[c], substr);
            char y[TMPBUFFER] = {'\0'};
            strcpy(y, "/");
            strcat(y, child[c]);
            strcpy(child[c], y);
            substr = strtok(NULL, " ");
            c++;
        }

        for (i=0; i<c; i++) {
            char u[TMPBUFFER] = {'\0'};
            strcpy(u, gMesg);
            strcat(u, child[i]);
            strcpy(child[i], u);

            char tmp_gainMesg[TMPBUFFER] = {'\0'};
            strcpy(tmp_gainMesg, child[i]);
            char *type = strstr(tmp_gainMesg, ".");
            if(type) {
                char ttmp[TMPBUFFER] = {'\0'};
                sprintf(ttmp, "GET %s HTTP/1.x\\r\\nHOST: %s:%s\\r\\n\\r\\n", child[i], LOCALHOST, PORT);
                char *x = sendM(ttmp, child[i]);

                // create file
                char sstr[BUFFER] = {'\0'};
                strcpy(sstr, x);
                char *pch;
                pch = strstr(sstr, "\r\n\r\n");
                int m=0;
                for (m=0; m<strlen(pch); m++) pch[m] = pch[m+4];

                char tmp_pch[BUFFER] = {'\0'};
                strcpy(tmp_pch, pch);

                pthread_t ttid;
                pthread_create(&ttid, NULL, createDir, (void *)child[i]);
                pthread_join(ttid, NULL);

                char tmp_root[TMPBUFFER] = {'\0'};
                strcpy(tmp_root, "./Output");
                strcat(tmp_root, child[i]);
                FILE *fp;
                fp = fopen(tmp_root, "w");
                if (!fp) {
                    printf("-------------------\nSomething wrong in creating file......Client Stop\n----------------------\n");
                    fclose(fp);
                    exit(1);
                } else {
                    fwrite(tmp_pch, 1, strlen(tmp_pch), fp);
                }
                fclose(fp);
                free(x);
                strcpy(child[i], "");
            } else {
                // Create directory
                pthread_t tid;
                pthread_create(&tid, NULL, createDir, (void *)child[i]);
                // Send directory
                char ttmp[TMPBUFFER] = {'\0'};
                sprintf(ttmp, "GET %s HTTP/1.x\\r\\nHOST: %s:%s\\r\\n\\r\\n", child[i], LOCALHOST, PORT);
                find_all(ttmp, child[i]);
            }
        }
    }
    return;
}

void *createDir(void *arg)
{
    char *str = (char *)arg;
    char tmp[TMPBUFFER] = {'\0'};
    strcpy(tmp, "Output");
    strcat(tmp, str);

    char seq[TMPBUFFER] = {'\0'};
    char *saveptr = NULL;
    char *substr = NULL;
    substr = __strtok_r(tmp, "/", &saveptr);
    do {
        char tmp_gainMesg[TMPBUFFER] = {'\0'};
        strcpy(tmp_gainMesg, substr);
        char *type = strstr(tmp_gainMesg, ".");
        if(!type) {
            strcat(seq, substr);
            // create directory
            mkdir(seq, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        }
        substr = __strtok_r(NULL, "/", &saveptr);
        strcat(seq, "/");
    } while(substr);

    pthread_exit(NULL);
}
#endif
