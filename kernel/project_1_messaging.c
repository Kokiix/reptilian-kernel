#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/list.h>
#include <linux/compiler.h>
#include <linux/slab.h>

struct msg_item {
	char* msg_contents;
	uid_t recipient;
	struct list_head list_node;
};

static LIST_HEAD(msg_q_head);

SYSCALL_DEFINE2(send_message_call, char __user *, msg, uid_t, recipient_user)
{
	char* test_message = "test message";

	struct msg_item *new_message;
	new_message = kmalloc(sizeof(*new_message), GFP_KERNEL);
	new_message->msg_contents = test_message;
	// new_message->recipient = 
	INIT_LIST_HEAD(&new_message->list_node);
	list_add_tail(&new_message->list_node, &msg_q_head);

	struct msg_item *pos;
	printk(KERN_INFO "List contents:\n");
    list_for_each_entry(pos, &msg_q_head, list_node) {
        printk(KERN_INFO "  Value: %s\n", pos->msg_contents);
    }
	return 0;
}

SYSCALL_DEFINE2(get_message_call, char __user *, msg, uid_t __user *,
		sending_user)
{
	printk(KERN_EMERG "hello world from get");
	return 0;
}
