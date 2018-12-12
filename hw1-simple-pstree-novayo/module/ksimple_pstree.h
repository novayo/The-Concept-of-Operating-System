#ifndef KSIMPLE_PSTREE
#define KSIMPLE_PSTREE

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/pid.h>
#include <linux/moduleparam.h>
#include <linux/netlink.h>
#include <net/netlink.h>
#include <net/net_namespace.h>

#define NETLINK_TEST 31
#define MAX_LEN_MESG 10000
#define NLMSG_SETECHO 0x11
#define NLMSG_GETECHO 0x12
#define next_thread(p)\
	list_entry((p)->thread_group.next, struct task_struct, thread_group)

struct nlmsghdr *receivedNL, *sendNL;
struct sk_buff *server_socket_buff;
void *receivedMesg;
void *sendOutMesg;
int len_receivedMesg; // with padding, but ok for echo
char formMesg[MAX_LEN_MESG];
int level;
char tmpSendMesg[100];

void get_thread(int target_pid);

void get_child(int target_pid)
{
    level++;
    /***** Find pid *****/
    pid_t pid = target_pid;
    struct task_struct *receivedPid_task1;
    receivedPid_task1 = pid_task(find_get_pid(pid), PIDTYPE_PID);

    // If pid doesn't existed
    if(receivedPid_task1 == NULL) {
        return;
    }

    struct task_struct *psibling;
    struct list_head *p = NULL;

    list_for_each(p, &receivedPid_task1->children) { // for, and point pp to p->children
        psibling = list_entry(p, struct task_struct, sibling);
        sprintf(tmpSendMesg, "|%d%s(%d)", level, psibling->comm, psibling->pid);
        strcat(formMesg, tmpSendMesg);
        get_child(psibling->pid);
        get_thread(psibling->pid);
        level--;
    }
}

void get_thread(int target_pid)
{
    struct task_struct *task = pid_task(find_get_pid(target_pid), PIDTYPE_PID);
    struct task_struct *t = task;
    while(1) {
        t = next_thread(t);
        if(t == task) break;
        sprintf(tmpSendMesg, "|%d- {%s(%d)}", level-1, t->comm, t->pid);
        strcat(formMesg, tmpSendMesg);
        get_child(t->pid);
        level--;
    }
}

int atoi(char* str, int max)
{
    int sum=0;
    int i=0;
    for(; i<max; ++i) {
        if((str[i] - '0') > 10) {
            return sum;
        }
        sum = sum*10+str[i]-'0';
    }
    return sum;
}
#endif
