#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/list.h>
#include <linux/compiler.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/uidgid.h>
#include <linux/cred.h>

struct msg_item {
	char* msg_contents;
	kuid_t recipient;
	struct list_head list_node;
};

static LIST_HEAD(msg_q_head);

SYSCALL_DEFINE2(send_message_call, char __user *, msg, uid_t, recipient_id)
{
	// copy/allocate string
	char* kspace_msg = strndup_user(msg, 1024);
	if (IS_ERR(kspace_msg)) {return PTR_ERR(kspace_msg);}

	// allocate msg struct
	struct msg_item *new_message;
	new_message = kmalloc(sizeof(*new_message), GFP_KERNEL);
	if (!new_message) {
		kfree(kspace_msg);
		return -ENOMEM;
	}

	// fill out msg struct
	new_message->msg_contents = kspace_msg;
	new_message->recipient = make_kuid(current_user_ns(), recipient_id);
	INIT_LIST_HEAD(&new_message->list_node);
	list_add_tail(&new_message->list_node, &msg_q_head);

	// DEBUG: print list
	struct msg_item *pos;
	printk(KERN_INFO "List contents:\n");
    list_for_each_entry(pos, &msg_q_head, list_node) {
        printk(KERN_INFO "  Msg: %s\t For: %d", 
			pos->msg_contents,
			from_kuid(current_user_ns(), pos->recipient));
    }

	return 0;
}

SYSCALL_DEFINE2(get_message_call, char __user *, msg, uid_t __user *,
		sending_user)
{
	kuid_t curr_kuid = current_uid();

	struct msg_item *cursor, *tmp;
	list_for_each_entry_safe(cursor, tmp, &msg_q_head, list_node) {
		if (uid_eq(cursor->recipient, curr_kuid)) {
			// write to msg pointer
			// TODO

			// write to sending user 
			// TODO

			list_del(&cursor->list_node);
			kfree(cursor->msg_contents);
			kfree(cursor);

			return 0;
		}
	}

	return -ENOMSG;
}
