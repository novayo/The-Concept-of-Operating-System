#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <linux/netlink.h>
#include <sys/socket.h>

/** Message type **/
#define NETLINK_TEST 31 // my own protocol 
#define MAX_LEN_MESG 10000
#define NLMSG_SETECHO 0x11
#define NLMSG_GETECHO 0x12

char receivedMesg1[MAX_LEN_MESG];
struct sockaddr_nl src_addr, dst_addr;
struct iovec iov;
struct nlmsghdr *sendingNL;
struct msghdr message;
int client_socket;

int main(int argc, char *argv[])
{
    if(argc > 1) {
        int i;
        for(i=0; i<strlen(argv[1]); ++i) {
            if(!isdigit(argv[1][i]))
                break;
        }
        if(i == strlen(argv[1])) {
            //
        } else if(argc > 2 || !(argv[1][0] == '-' || argv[1][1] == 'c' || argv[1][1] == 's' || argv[1][1] == 'p')) {
            printf("Run Error!\nUsage: ./simple_pstree -[c|s|p][pid]\n");
            return 1;
        }
    }

    client_socket = socket(AF_NETLINK, SOCK_DGRAM, NETLINK_TEST); // Create the socket of NETLINK_TEST

    /*** Bind local point to listen ***/
    bzero(&src_addr, sizeof(src_addr)); // In string.h -> clean src_addr
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid();
    src_addr.nl_groups = 0;
    bind(client_socket, (struct sockaddr*)&src_addr, sizeof(src_addr));

    /*** Create a point to send messages ***/
    bzero(&dst_addr, sizeof(dst_addr)); // In string.h -> clean dst_addr
    dst_addr.nl_family = AF_NETLINK;
    dst_addr.nl_pid = 0; // Send to kernel
    dst_addr.nl_groups = 0;

    /***Create a format for messages  ***/
    sendingNL = malloc(NLMSG_SPACE(MAX_LEN_MESG));
    sendingNL->nlmsg_len = NLMSG_SPACE(MAX_LEN_MESG); //align messages
    sendingNL->nlmsg_pid = getpid();  //. self pid
    sendingNL->nlmsg_flags = 0;
    sendingNL->nlmsg_type = NLMSG_GETECHO;

    /*** Set content of messages ***/
    if(argc > 1) {
        strcpy(NLMSG_DATA(sendingNL), argv[1]);
    } else {
        strcpy(NLMSG_DATA(sendingNL), "0");
    }
    iov.iov_base = (void *)sendingNL;
    iov.iov_len = sendingNL->nlmsg_len;
    message.msg_name = (void *)&dst_addr;
    message.msg_namelen = sizeof(dst_addr);
    message.msg_iov = &iov;
    message.msg_iovlen = 1;

    /*** Send messages to kernel ***/
    sendmsg(client_socket, &message, 0);

    /*** Print out received messages ***/
    memset(sendingNL, 0, NLMSG_SPACE(MAX_LEN_MESG));
    recvmsg(client_socket, &message, 0);

    strcpy(receivedMesg1, (char *)NLMSG_DATA(sendingNL));
    memset(sendingNL, 0, NLMSG_SPACE(MAX_LEN_MESG)); //saved memory

    int count=-1, pcount=1, i, j;
    for(i=0; i<strlen(receivedMesg1); ++i) {
        // print out parent and sibling
        if(receivedMesg1[i] == ',') {
            printf("\n");
            for(j=0; j<pcount*4; ++j) {
                printf(" ");
            }
            ++pcount;
            continue; // skip this non-word
        }
        // print out children
        else {
            if(receivedMesg1[i] == '|') {
                // if get 100~999
                if(receivedMesg1[i+4] == '-' && isdigit(receivedMesg1[i+1]) && isdigit(receivedMesg1[i+2]) && isdigit(receivedMesg1[i+3])) {
                    count = ((int)(receivedMesg1[i+1] - '0'))*100 + ((int)(receivedMesg1[i+2] - '0'))*10 + ((int)(receivedMesg1[i+3] - '0'));
                    i+=3;
                }
                // if get 10~99
                else if(receivedMesg1[i+3] == '-' && isdigit(receivedMesg1[i+1]) && isdigit(receivedMesg1[i+2])) {
                    count = ((int)(receivedMesg1[i+1] - '0'))*10 + ((int)(receivedMesg1[i+2] - '0'));
                    i+=2;
                }
                // if get 0~9
                else if(receivedMesg1[i+2] == '-' && isdigit(receivedMesg1[i+1])) {
                    count = (int)(receivedMesg1[i+1] - '0');
                    i++;
                }
                // non thread
                // if get 100~999
                if(receivedMesg1[i+4] != '-' && isdigit(receivedMesg1[i+1]) && isdigit(receivedMesg1[i+2]) && isdigit(receivedMesg1[i+3])) {
                    count = ((int)(receivedMesg1[i+1] - '0'))*100 + ((int)(receivedMesg1[i+2] - '0'))*10 + ((int)(receivedMesg1[i+3] - '0'));
                    i+=3;
                }
                // if get 10~99
                else if(receivedMesg1[i+3] != '-' && isdigit(receivedMesg1[i+1]) && isdigit(receivedMesg1[i+2])) {
                    count = ((int)(receivedMesg1[i+1] - '0'))*10 + ((int)(receivedMesg1[i+2] - '0'));
                    i+=2;
                }
                // if get 0~9
                else if(receivedMesg1[i+2] != '-' && isdigit(receivedMesg1[i+1])) {
                    count = (int)(receivedMesg1[i+1] - '0');
                    i++;
                }

            } else count = 0;

            if(count) {
                printf("\n");
                int j;
                for(j=0; j<count*4; ++j) {
                    printf(" ");
                }
                continue; // skip this non-word
            }
        }
        printf("%c", receivedMesg1[i]);
    }
    if(!(pcount == 1 && count == -1)) {
        printf("\n");
    }

    return 0;
}
