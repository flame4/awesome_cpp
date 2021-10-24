#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/sched.h>

#define DEVICE_NAME "datenlord5"
#define DEVICE_FILE_NAME "datenlord5"

static DEFINE_MUTEX(read_mutex);
static DEFINE_MUTEX(write_mutex);

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

static DECLARE_WAIT_QUEUE_HEAD(writer_q);
static DECLARE_WAIT_QUEUE_HEAD(reader_q);

static int dev_open(struct inode* inodep, struct file* filep) {
  printk(KERN_INFO "datenlord device opened\n");
  return 0;
}

static int dev_release(struct inode* inodep, struct file* filep) {
  printk(KERN_INFO "datenlord device closed\n");
  return 0;
}

static atomic_t already_write = ATOMIC_INIT(0);
static ssize_t dev_write(struct file* filep, const char* buffer, size_t len, loff_t* offset) {
  while (true) {
    int status = atomic_cmpxchg(&already_write, 0, 1);
    printk("write inside\n");
    if (status == 0) {
      // this process get lock.
      // some data need to write, go ahead
      if (MAX_LEN - msg_end + msg_begin >= len) { break; }
      atomic_set(&already_write, 0);
    }
    int ret = wait_event_interruptible(writer_q, !atomic_read(&already_write) && (MAX_LEN - msg_end + msg_begin >= len));
    // wake up by signal.
    if (ret == -ERESTARTSYS) { return -EINTR; }
  }
  printk("datenlord write(%s, %ld)\n", buffer, len);
  // max len can write in.
  len = min(len, MAX_LEN - msg_end + msg_begin);
  // first put from msg_end to buffer end.
  int p1_len = min(len, MAX_LEN - (msg_end & (MAX_LEN - 1)));
  printk("write, len = %ld, p1 = %d, end = %ld, begin = %ld, bf_len = %ld\n", len, p1_len, msg_end & (MAX_LEN-1), msg_begin & (MAX_LEN-1), msg_end - msg_begin);
  memmove(msg + (msg_end & (MAX_LEN - 1)), buffer, p1_len);
  // then put rest data to buffer head.
  memmove(msg, buffer + p1_len, len - p1_len);
  msg_end += len;
  atomic_set(&already_write, 0);
  // different with read.
  // maybe only many writer, no reader.
  // In this case, one writer finished its job should also wake up other writer
  wake_up(&writer_q);
  wake_up(&reader_q);
  return len;
}

static atomic_t already_read = ATOMIC_INIT(0);

static ssize_t dev_read(struct file* filep, char* buffer, size_t len, loff_t* offset) {
  while (true) {
    int status = atomic_cmpxchg(&already_read, 0, 1);
    printk("read inside\n");
    if (status == 0) {
      // this process get lock.
      // some data need to read, go ahead.
      if (msg_end > msg_begin) { break; }
      // no data, just hang up.
      atomic_set(&already_read, 0);
    }
    int ret = wait_event_interruptible(reader_q, !atomic_read(&already_read) && (msg_end > msg_begin));
    if (ret == -ERESTARTSYS) { return -EINTR; }
  }
  printk("read inside done\n");
  len = min(len, msg_end - msg_begin);
  // first copy msg_end to buffer end.
  int p1_len = min(len, MAX_LEN - (msg_begin & (MAX_LEN-1)));
  printk("read, len = %ld, p1 = %d, end = %ld, begin = %ld, bf_len = %ld\n", len, p1_len, msg_end & (MAX_LEN-1), msg_begin & (MAX_LEN-1), msg_end - msg_begin);
  memmove(buffer, msg + (msg_begin & (MAX_LEN-1)), p1_len);
  memmove(buffer + p1_len, msg, len - p1_len);
  msg_begin += len;
  atomic_set(&already_read, 0);
  wake_up(&writer_q);
  return len;
}

MODULE_LICENSE("GPL");
