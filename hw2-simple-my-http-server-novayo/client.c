#include "client.h"
#include <dirent.h>

int main(int argc, char *argv[])
{
    /***** Get param *****/
    // ./client -t DIR -h 127.0.0.1 -p port
    if (argc != 7) {
        printf("Invalid num of instr.\n");
        return -1;
    }
    int y=0;
    for(y=0; y<strlen(argv[4]); y++) LOCALHOST[y] = argv[4][y];
    if (strlen(argv[2]) > 128 || (strcmp(argv[1], "-t") && strcmp(argv[3], "-h") && strcmp(argv[5], "-p") && strcmp(LOCALHOST, "127.0.0.1"))) {
        printf("Invalid key words\n");
        return -2;
    }
    strcpy(PORT, argv[6]);
    port = atoi(argv[6]);
    if (port < 0) {
        printf("Invalid port or thread\n");
        return -3;
    }
    struct dirent *checkdir;
    DIR *check = opendir(".");
    while((checkdir = readdir(check)) != NULL) {
        if(!strcmp(checkdir->d_name, "Output")) {
            printf("Please delete \"Output\" folder first...\n");
            return -1;
        }
    }

    char *sendMesg = argv[2];
    char gMesg[TMPBUFFER] = {'\0'};
    strcpy(gMesg, sendMesg);

    // GET on sendMesg
    char tmp[TMPBUFFER] = {'\0'};
    sprintf(tmp, "GET %s HTTP/1.x\\r\\nHOST: %s:%s\\r\\n\\r\\n", sendMesg, LOCALHOST, PORT);
    strcpy(sendMesg, tmp);

    // Send
    find_all(sendMesg, gMesg);
    return 0;
}

