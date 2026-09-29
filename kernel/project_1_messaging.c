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
	kuid_t sending_user;
	kuid_t recipient;
	struct list_head list_node;
};

static LIST_HEAD(msg_q_head);

SYSCALL_DEFINE2(send_message_call, char __user *, msg, uid_t, recipient_id)
{
	// copy/allocate string
	char* kspace_msg = kmalloc(1025, GFP_KERNEL);
	if (!kspace_msg) {
		return -ENOMEM;
	}
	int cpy_result = strncpy_from_user(kspace_msg, msg, 1024);
	if (cpy_result < 0) {
		kfree(kspace_msg);
		return cpy_result;
	}
	kspace_msg[1024] = '\0';

	// allocate msg struct
	struct msg_item *new_message;
	new_message = kmalloc(sizeof(*new_message), GFP_KERNEL);
	if (!new_message) {
		kfree(kspace_msg);
		return -ENOMEM;
	}

	// fill out msg struct
	kuid_t kuid = make_kuid(current_user_ns(), recipient_id);
	if (!uid_valid(kuid)) {
		kfree(kspace_msg);
		kfree(new_message);
		return -EINVAL;
	}

	new_message->msg_contents = kspace_msg;
	new_message -> sending_user = current_uid();
	new_message->recipient = kuid;
	INIT_LIST_HEAD(&new_message->list_node);
	list_add_tail(&new_message->list_node, &msg_q_head);

	// DEBUG: print list
	// struct msg_item *pos;
	// printk(KERN_INFO "List contents:\n");
    // list_for_each_entry(pos, &msg_q_head, list_node) {
    //     printk(KERN_INFO "  Msg: %s\t For: %d\t From: %d", 
	// 		pos->msg_contents,
	// 		from_kuid(current_user_ns(), pos->recipient),
	// 		from_kuid(current_user_ns(), pos->sending_user));
    // }

	return 0;
}

SYSCALL_DEFINE2(get_message_call, char __user *, msg, uid_t __user *,
		sending_user)
{
	kuid_t curr_kuid = current_uid();

	struct msg_item *cursor, *tmp;
	list_for_each_entry_safe(cursor, tmp, &msg_q_head, list_node) {
		if (uid_eq(cursor->recipient, curr_kuid)) {
			char* kmsg = cursor->msg_contents;

			int bytes_not_copied = 0;

			bytes_not_copied += copy_to_user(msg, kmsg, strlen(kmsg) + 1);

			uid_t uid = from_kuid(current_user_ns(), cursor->sending_user);
			bytes_not_copied += copy_to_user(sending_user, &uid, sizeof(uid));

			if (bytes_not_copied > 0) {return EFAULT;}
			else {
				list_del(&cursor->list_node);
				kfree(kmsg);
				kfree(cursor);
			}

			return 0;
		}
	}

	return 1;
}
