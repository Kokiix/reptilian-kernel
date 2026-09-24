#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/compiler.h>

SYSCALL_DEFINE2(send_message_call, char __user *, msg, uid_t, recipient_user)
{
	printk(KERN_EMERG "hello world from send");
	return 0;
}

SYSCALL_DEFINE2(get_message_call, char __user *, msg, uid_t __user *,
		sending_user)
{
	printk(KERN_EMERG "hello world from get");
	return 0;
}
