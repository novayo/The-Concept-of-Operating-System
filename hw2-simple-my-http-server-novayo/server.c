#include "server.h"
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

int server_fd, new_socket;
long valread;
struct sockaddr_in address;
int addrlen = sizeof(address);
char root[100] = {'\0'};
void *skt(void *arg);

int main(int argc, char const *argv[])
{
    /***** Get param *****/
    //./server –r root -p port -n thread_number
    if (argc != 7) {
        printf("Invalid num of instr.\n");
        return -1;
    }
    if (strcmp(argv[1], "-r") && strcmp(argv[3], "-p") && strcmp(argv[5], "-n")) {
        printf("Invalid key words\n");
        return -2;
    }
    strcpy(root, argv[2]);
    int port = atoi(argv[4]);
    int thread = atoi(argv[6]);
    if (port < 0 || thread < 0) {
        printf("Invalid port or thread\n");
        return -3;
    }

    /***** Initial Vars  *****/
    tpool_t pool;
    tpool_init(&pool, thread, MAX_QUEUE);

    // create socket file descriptor
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    memset(address.sin_zero, '\0', sizeof address.sin_zero);
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) {
        printf("Server in bind\n");
        return -1;
    }
    listen(server_fd, thread);

    printf("Server On !!\n");
    while(1) {
        /***** wait for new connection *****/
        char *gainMesg = malloc(TMPBUFFER);
        memset(gainMesg, '\0', TMPBUFFER);
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        recv(new_socket, gainMesg, TMPBUFFER, 0);
        printf("Gain : %s\n", gainMesg);
        tpool_add_work(pool, skt, (void *)gainMesg);
    }
    return 0;
}

void *skt(void *arg)
{
sleep(10);
    char *gainMesg = (char *)arg;

    /***** Extract Gain Mesg *****/
    char orig_gainMesg[TMPBUFFER] = {'\0'};
    if (valread == -1) return NULL;
    strcpy(orig_gainMesg, gainMesg);

    // gain | GET "DIR" HTTP/1.x\r\nHOST: "LOCALHOST":"PORT" \r\n\r\n
    char *subsave = NULL;
    char *substr = NULL;
    substr = __strtok_r(gainMesg, " ", &subsave);
    substr = __strtok_r(NULL, " ", &subsave);
    strcpy(gainMesg, substr);

    char sendMesg[BUFFER] = {'\0'};
    // Bad Request
    if (strncmp(gainMesg, "/", 1)) {
        char status[TMPBUFFER] = {'\0'};
        sprintf(status, "%d", status_code[BAD_REQUEST]);
        strcat(sendMesg, status);
        strcat(sendMesg, " ");
        strcat(sendMesg, "BAD_REQUEST|");
    }
    // Method Not Allowed
    else if (strncmp(orig_gainMesg, "GET", 3)) {
        char status[TMPBUFFER] = {'\0'};
        sprintf(status, "%d", status_code[METHOD_NOT_ALLOWED]);
        strcat(sendMesg, status);
        strcat(sendMesg, " ");
        strcat(sendMesg, "METHOD_NOT_ALLOWED|");
    }
    // Unsupported Media Type
    char tmp_gainMesg[TMPBUFFER] = {'\0'};
    strcpy(tmp_gainMesg, gainMesg);
    char *type = strstr(tmp_gainMesg, ".");
    int int_type=0;
    if (strlen(sendMesg) == 0) {
        if (type) {
            for (int_type=0; int_type<strlen(type-1); int_type++) type[int_type] = type[int_type+1];
            type[int_type] = '\0'; // get type

            for (int_type=0; extensions[int_type].ext; int_type++) {
                if (!strcmp(type, extensions[int_type].ext)) break;
            }
            if (!extensions[int_type].ext) {
                char status[TMPBUFFER] = {'\0'};
                sprintf(status, "%d", status_code[UNSUPPORT_MEDIA_TYPE]);
                strcat(sendMesg, status);
                strcat(sendMesg, " ");
                strcat(sendMesg, "UNSUPPORT_MEDIA_TYPE|");
            }
        }
    }
    int do_not_find_file = 1;
    // if status is ok
    if (strlen(sendMesg) == 0) {
        /***** Operation *****/
        char dir[TMPBUFFER] = {'\0'};
        strcpy(dir, root);
        strcat(dir, gainMesg);

        // find file
        if (type) {
            char *x = get_SubDir(dir, --do_not_find_file);
            strcpy(sendMesg, x);
            free(x);
        }
        // find dir
        else {
            char *x = get_SubDir(dir, do_not_find_file);
            strcpy(sendMesg, x);
            free(x);
        }
    }
    // send | HTTP/1.x "int_status" "char_status"\r\nContent-Type: "text"\r\nServer: httpserver/1.x\r\n\r\n"CONTENT"
    // status isn't ok -> "int_status" "char_status" "CONTENT" are all NULL
    char tmp_sendMesg[BUFFER] = {'\0'};
    strcpy(tmp_sendMesg, sendMesg);
    char *NotOk = strstr(tmp_sendMesg, "|");
    if(NotOk) {
        sendMesg[strlen(sendMesg)-1] = '\0'; // remove "|"
        char tmp[BUFFER] = {'\0'};
        sprintf(tmp, "HTTP/1.x %s\r\nContent-Type: \r\nServer: httpserver/1.x\r\n\r\n", sendMesg);
        strcpy(sendMesg, tmp);
    }
    // status is ok
    else {
        // find dir
        if (do_not_find_file) {
            char tmp[BUFFER] = {'\0'};
            sprintf(tmp, "HTTP/1.x %d OK\r\nContent-Type: directory\r\nServer: httpserver/1.x\r\n\r\n%s", status_code[OK], sendMesg);
            strcpy(sendMesg, tmp);
        }
        // find file
        else {
            char tmp[BUFFER] = {'\0'};
            sprintf(tmp, "HTTP/1.x %d OK\r\nContent-Type: %s\r\nServer: httpserver/1.x\r\n\r\n%s", status_code[OK], extensions[int_type].mime_type, sendMesg);
            strcpy(sendMesg, tmp);
        }
    }
    send(new_socket, sendMesg, strlen(sendMesg), 0);  //hello -> send to client
    close(new_socket);
    free(gainMesg);
    return NULL;
}
