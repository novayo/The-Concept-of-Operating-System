#include "ksimple_pstree.h"

static struct sock *socket_kernel; // kernel socket
static void nl_custom_data_ready(struct sk_buff *client_socket_buff); // Receive Data

static int Initial(void)
{
    struct netlink_kernel_cfg nlcfg = {
        .input = nl_custom_data_ready,
    };
    socket_kernel = netlink_kernel_create(&init_net, NETLINK_TEST, &nlcfg);
    printk(KERN_INFO "Initial!\n");
    return 0;
}
static void Exit(void)
{
    printk(KERN_INFO "Exit!\n");
    netlink_kernel_release(socket_kernel);
}

static void nl_custom_data_ready(struct sk_buff *client_socket_buff)
{
    level = 0;
    memset(formMesg, 0, MAX_LEN_MESG);
    memset(tmpSendMesg, 0, 100);

    /***** Initial Vars  *****/
    int target_pid = 0;
    int if_fullnum = 0;

    /***** Frag parameters from mesg  *****/
    receivedNL = nlmsg_hdr(client_socket_buff); // get mesg from user
    switch(receivedNL->nlmsg_type) {
    case NLMSG_SETECHO:
        break;
    case NLMSG_GETECHO:
        receivedMesg = nlmsg_data(receivedNL); //Messages we received
        len_receivedMesg = nlmsg_len(receivedNL);

        char label[1] = "E";
        char fragReceivedMesg[20];
        strcpy(fragReceivedMesg, receivedMesg);
        if(strlen(fragReceivedMesg) == 1) {
            label[0] = '0';
            target_pid = 1;
        } else if(strlen(fragReceivedMesg) == 2) {
            label[0] = fragReceivedMesg[1];
            if(label[0] == 'c') {
                target_pid = 1;
            } else {
                target_pid = receivedNL->nlmsg_pid;
            }
        } else if(strlen(fragReceivedMesg) > 2) {
            label[0] = fragReceivedMesg[1];
            char tmp_pid[10] = "";
            int i=0;
            if(fragReceivedMesg[0] == '-') {
                for(i=0; i<strlen(fragReceivedMesg)-2; ++i) {
                    tmp_pid[i] = fragReceivedMesg[i+2];
                }
                target_pid = atoi(tmp_pid, strlen(fragReceivedMesg)-2);
            } else {
                target_pid = atoi(fragReceivedMesg, strlen(fragReceivedMesg));
                if_fullnum = 1;
            }
        }

        /***** create a message send to user *****/
        server_socket_buff = nlmsg_new(MAX_LEN_MESG, GFP_ATOMIC); //create sk_buff
        sendNL = nlmsg_put(server_socket_buff, 0, 0, NLMSG_SETECHO, len_receivedMesg, 0);
        sendOutMesg = nlmsg_data(sendNL); // create sending message : sendOutMesg (is empty now)

        pid_t pid = target_pid;
        struct task_struct *receivedPid_task;
        receivedPid_task = pid_task(find_get_pid(pid), PIDTYPE_PID);
        // If pid doesn't existed
        if(receivedPid_task == NULL) {
            strcpy(sendOutMesg, ""); // add "<str>" in sendOutMesg
            nlmsg_unicast(socket_kernel, server_socket_buff, receivedNL->nlmsg_pid); //Send message out  *sk -> useless *server_socket_buff
            return;
        }

        // Children
        if((label[0] == '0') || label[0] == 'c' || if_fullnum == 1) {
            sprintf(formMesg, "%s(%d)", receivedPid_task->comm, receivedPid_task->pid);
            get_child(target_pid);
        } else if(label[0] == 's') {
            struct task_struct *psibling;
            struct list_head *p = NULL;
            list_for_each(p, &receivedPid_task->parent->children) {
                psibling = list_entry(p, struct task_struct, sibling);
                sprintf(tmpSendMesg, "%s(%d)\n", psibling->comm, psibling->pid);
                if(strcmp(psibling->comm, receivedPid_task->comm)) {
                    strcat(formMesg, tmpSendMesg);
                }
            }
            formMesg[strlen(formMesg)-1] = '\0'; // delete the last |
        } else if(label[0] == 'p') {
            int i = 0;
            for(i=0; i<50; ++i) {
                if(receivedPid_task->parent == NULL) {
                    break;
                } else {
                    if(i == 0) {
                        sprintf(tmpSendMesg, "%s(%d),", receivedPid_task->comm, receivedPid_task->pid);
                        strcpy(formMesg, tmpSendMesg);
                        continue;
                    }
                    sprintf(tmpSendMesg, "%s(%d),", receivedPid_task->parent->comm, receivedPid_task->parent->pid);
                    strcat(tmpSendMesg, formMesg);
                    strcpy(formMesg, tmpSendMesg);
                }
                if(receivedPid_task->parent->pid == 1) {
                    break;
                } else {
                    receivedPid_task = receivedPid_task->parent;
                }
            }
            formMesg[strlen(formMesg)-1] = '\0'; // delete the last |
        }

        /***** Send Mesg *****/
        strcpy(sendOutMesg, formMesg); // add "<str>" in sendOutMesg
        nlmsg_unicast(socket_kernel, server_socket_buff, receivedNL->nlmsg_pid); //Send mesg out
        break;
    default:
        printk(KERN_INFO "Miss message type!\n");
    }
    return;
}
module_init(Initial);
module_exit(Exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("OS Assignment 1");
MODULE_AUTHOR("F14051172");
