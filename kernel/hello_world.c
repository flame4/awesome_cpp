#include <linux/kernel.h>
#include <linux/module.h>

int init_module(void) {
pr_info("hello world\n");
return 0;
}

void cleanup_module(void) {
pr_info("bye world\n");
}

MODULE_LICENSE("GPL");
