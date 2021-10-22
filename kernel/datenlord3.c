#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>


#define DEVICE_NAME "datenlord3"
#define DEVICE_FILE_NAME "datenlord3"


static int dev_open(struct inode*, struct file*);
static int dev_release(struct inode*, struct file*);
static ssize_t dev_read(struct file*, char*, size_t, loff_t*);
static ssize_t dev_write(struct file*, const char*, size_t, loff_t*);

static struct class *cls;

static struct file_operations fops = {
  .open = dev_open,
  .read = dev_read,
  .write = dev_write,
  .release = dev_release,
};

static int major;
#define MAX_LEN 1024
static char msg[MAX_LEN];
static int msg_len;

int init_module(void) {
  // major number will be auto distributed by kernel.
  major = register_chrdev(0, DEVICE_NAME, &fops);
  if (major < 0) {
    printk(KERN_ALERT "datenlord get major number fail\n");
    return major;
  }
  cls = class_create(THIS_MODULE, DEVICE_FILE_NAME);
  device_create(cls, NULL, MKDEV(major, 0), NULL, DEVICE_FILE_NAME);
  msg_len = 0;
  printk(KERN_INFO "datenlord module init, major = %d, device=/dev/%s\n", major, DEVICE_FILE_NAME);
  return 0;
}

void cleanup_module(void) {
  device_destroy(cls, MKDEV(major, 0));
  class_destroy(cls);
  unregister_chrdev(major, DEVICE_NAME);
  printk(KERN_INFO "datenlord module exit\n");
}

static int dev_open(struct inode* inodep, struct file* filep) {
  printk(KERN_INFO "datenlord device opened\n");
  return 0;
}

static int dev_release(struct inode* inodep, struct file* filep) {
  printk(KERN_INFO "datenlord device closed\n");
  return 0;
}

static ssize_t dev_write(struct file* filep, const char* buffer, size_t len, loff_t* offset) {
  printk(KERN_INFO "datenlord write(%p, %p, %ld)\n", filep, buffer, len);
  msg_len = MAX_LEN;
  if (msg_len > len) { msg_len = len; }
  memmove(msg, buffer, msg_len);
  printk(KERN_INFO "write buffer: %s", msg);
  return msg_len;
}

// when read request received, it means there's a syscall from user space
// to request some file, a driver need to read actual context from file
// in disk or memory, then copy them to user space.
// buffer is given by user_space to copy text into.
// len means buffer's size. if file is too big, you should just copy len
// and set offset to specific value, for your next reading request.
// a total file reading could be finish by call dev_read multiple times.
// return value is msg_len read this time.
static ssize_t dev_read(struct file* filep, char* buffer, size_t len, loff_t* offset) {
  int errors = 0;
  if (*offset == msg_len) {
    return 0;
  }
  // send to where, send what, send context len.
  errors = copy_to_user(buffer, msg, msg_len);
  *offset = msg_len;
  printk("read, buffer = %s, result = %s, msg_len = %d", msg, buffer, msg_len);
  return errors == 0 ? msg_len : -EFAULT;
}

MODULE_LICENSE("GPL");
