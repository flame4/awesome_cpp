#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>


#define DEVICE_NAME "datenlord4"
#define DEVICE_FILE_NAME "datenlord4"

static DEFINE_MUTEX(mymutex);

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
static unsigned long msg_begin;
static unsigned long msg_end;

int init_module(void) {
  // major number will be auto distributed by kernel.
  major = register_chrdev(0, DEVICE_NAME, &fops);
  if (major < 0) {
    printk(KERN_ALERT "datenlord get major number fail\n");
    return major;
  }
  cls = class_create(THIS_MODULE, DEVICE_FILE_NAME);
  device_create(cls, NULL, MKDEV(major, 0), NULL, DEVICE_FILE_NAME);
  msg_begin = 0;
  msg_end = 0;
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
  int ret = mutex_lock_interruptible(&mymutex);
  // receive kill signal
  printk("write inside\n");
  if (ret == -EINTR) {
    mutex_unlock(&mymutex);
    return -EINTR;
  }
  printk(KERN_INFO "datenlord write(%s, %ld)\n", buffer, len);
  // max len can write in.
  len = min(len, MAX_LEN - msg_end + msg_begin);
  // first put from msg_end to buffer end.
  int p1_len = min(len, MAX_LEN - (msg_end & (MAX_LEN - 1)));
  printk("write, len = %ld, p1 = %d, end = %ld, begin = %ld, bf_len = %ld\n", len, p1_len, msg_end & (MAX_LEN-1), msg_begin & (MAX_LEN-1), msg_end - msg_begin);
  memmove(msg + (msg_end & (MAX_LEN - 1)), buffer, p1_len);
  // then put rest data to buffer head.
  memmove(msg, buffer + p1_len, len - p1_len);
  msg_end += len;
  mutex_unlock(&mymutex);
  return len;
}

static ssize_t dev_read(struct file* filep, char* buffer, size_t len, loff_t* offset) {
  int ret = mutex_lock_interruptible(&mymutex);
  printk("read inside\n");
  // mutex not get, but kill signal received.
  if (ret == -EINTR) {
    mutex_unlock(&mymutex);
    // just return 0 to stop this read.
    return -EINTR;
  }
  len = min(len, msg_end - msg_begin);
  // first copy msg_end to buffer end.
  int p1_len = min(len, MAX_LEN - (msg_begin & (MAX_LEN-1)));
  printk("read, len = %ld, p1 = %d, end = %ld, begin = %ld, bf_len = %ld\n", len, p1_len, msg_end & (MAX_LEN-1), msg_begin & (MAX_LEN-1), msg_end - msg_begin);
  memmove(buffer, msg + (msg_begin & (MAX_LEN-1)), p1_len);
  memmove(buffer + p1_len, msg, len - p1_len);
  msg_begin += len;
  mutex_unlock(&mymutex);
  return len;
}

MODULE_LICENSE("GPL");
