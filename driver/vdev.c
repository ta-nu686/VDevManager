// SPDX-License-Identifier: GPL-2.0
/*
 * vdev.c - Virtual character device driver (/dev/mydev)
 * Features: open/read/write/release/llseek, ioctl, 4 KB kernel buffer,
 *           mutex protection, two modes (normal / uppercase), statistics.
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/ctype.h>
#include <linux/version.h>
#include "../include/vdev_ioctl.h"

#define DEVICE_NAME "mydev"
#define CLASS_NAME  "vdev_class"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tanisha Mohapatra");
MODULE_DESCRIPTION("VDevManager - virtual character device driver");
MODULE_VERSION("1.0");

struct vdev_device {
    dev_t             devnum;
    struct cdev       cdev;
    struct class     *cls;
    struct device    *dev;
    char             *buf;      /* kernel buffer             */
    size_t            used;     /* bytes currently stored    */
    u32               mode;
    struct mutex      lock;     /* protects buf/used/mode/stats */
    struct vdev_stats stats;
};

static struct vdev_device vd;

static int vdev_open(struct inode *inode, struct file *filp)
{
    mutex_lock(&vd.lock);
    vd.stats.open_count++;
    mutex_unlock(&vd.lock);
    pr_info("vdev: device opened\n");
    return 0;
}

static int vdev_release(struct inode *inode, struct file *filp)
{
    pr_info("vdev: device closed\n");
    return 0;
}

static ssize_t vdev_read(struct file *filp, char __user *ubuf,
                         size_t len, loff_t *off)
{
    ssize_t ret;

    if (mutex_lock_interruptible(&vd.lock))
        return -ERESTARTSYS;

    if (*off >= (loff_t)vd.used) {          /* end of data */
        ret = 0;
        goto out;
    }
    if (len > vd.used - *off)
        len = vd.used - *off;

    if (copy_to_user(ubuf, vd.buf + *off, len)) {
        ret = -EFAULT;
        goto out;
    }
    *off += len;
    vd.stats.read_count++;
    vd.stats.bytes_read += len;
    ret = len;
out:
    mutex_unlock(&vd.lock);
    return ret;
}

static ssize_t vdev_write(struct file *filp, const char __user *ubuf,
                          size_t len, loff_t *off)
{
    ssize_t ret;
    size_t avail, i;

    if (mutex_lock_interruptible(&vd.lock))
        return -ERESTARTSYS;

    avail = VDEV_BUFFER_SIZE - vd.used;
    if (avail == 0) {
        ret = -ENOSPC;                      /* buffer full */
        goto out;
    }
    if (len > avail)
        len = avail;                        /* partial write */

    if (copy_from_user(vd.buf + vd.used, ubuf, len)) {
        ret = -EFAULT;
        goto out;
    }
    if (vd.mode == VDEV_MODE_UPPERCASE)
        for (i = 0; i < len; i++)
            vd.buf[vd.used + i] = toupper(vd.buf[vd.used + i]);

    vd.used += len;
    vd.stats.write_count++;
    vd.stats.bytes_written += len;
    ret = len;
out:
    mutex_unlock(&vd.lock);
    return ret;
}

static long vdev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    long ret = 0;
    u32 val;
    struct vdev_stats snap;

    if (_IOC_TYPE(cmd) != VDEV_MAGIC)
        return -ENOTTY;

    if (mutex_lock_interruptible(&vd.lock))
        return -ERESTARTSYS;

    switch (cmd) {
    case VDEV_CLEAR_BUFFER:
        memset(vd.buf, 0, VDEV_BUFFER_SIZE);
        vd.used = 0;
        pr_info("vdev: buffer cleared\n");
        break;

    case VDEV_GET_BUFFER_SIZE:
        val = VDEV_BUFFER_SIZE;
        if (copy_to_user((u32 __user *)arg, &val, sizeof(val)))
            ret = -EFAULT;
        break;

    case VDEV_SET_MODE:
        if (copy_from_user(&val, (u32 __user *)arg, sizeof(val))) {
            ret = -EFAULT;
            break;
        }
        if (val > VDEV_MODE_UPPERCASE) {
            ret = -EINVAL;
            break;
        }
        vd.mode = val;
        pr_info("vdev: mode set to %u\n", val);
        break;

    case VDEV_GET_MODE:
        val = vd.mode;
        if (copy_to_user((u32 __user *)arg, &val, sizeof(val)))
            ret = -EFAULT;
        break;

    case VDEV_GET_STATISTICS:
        snap = vd.stats;
        snap.buffer_used = vd.used;
        snap.buffer_size = VDEV_BUFFER_SIZE;
        snap.mode = vd.mode;
        if (copy_to_user((struct vdev_stats __user *)arg, &snap, sizeof(snap)))
            ret = -EFAULT;
        break;

    default:
        ret = -ENOTTY;
    }

    mutex_unlock(&vd.lock);
    return ret;
}

static const struct file_operations vdev_fops = {
    .owner          = THIS_MODULE,
    .open           = vdev_open,
    .release        = vdev_release,
    .read           = vdev_read,
    .write          = vdev_write,
    .llseek         = default_llseek,
    .unlocked_ioctl = vdev_ioctl,
};

static int __init vdev_init(void)
{
    int ret;

    mutex_init(&vd.lock);
    vd.buf = kzalloc(VDEV_BUFFER_SIZE, GFP_KERNEL);
    if (!vd.buf)
        return -ENOMEM;

    ret = alloc_chrdev_region(&vd.devnum, 0, 1, DEVICE_NAME);
    if (ret < 0)
        goto err_buf;

    cdev_init(&vd.cdev, &vdev_fops);
    vd.cdev.owner = THIS_MODULE;
    ret = cdev_add(&vd.cdev, vd.devnum, 1);
    if (ret < 0)
        goto err_region;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0)
    vd.cls = class_create(CLASS_NAME);
#else
    vd.cls = class_create(THIS_MODULE, CLASS_NAME);
#endif
    if (IS_ERR(vd.cls)) {
        ret = PTR_ERR(vd.cls);
        goto err_cdev;
    }

    vd.dev = device_create(vd.cls, NULL, vd.devnum, NULL, DEVICE_NAME);
    if (IS_ERR(vd.dev)) {
        ret = PTR_ERR(vd.dev);
        goto err_class;
    }

    pr_info("vdev: loaded, /dev/%s (major %d, minor %d)\n",
            DEVICE_NAME, MAJOR(vd.devnum), MINOR(vd.devnum));
    return 0;

err_class:
    class_destroy(vd.cls);
err_cdev:
    cdev_del(&vd.cdev);
err_region:
    unregister_chrdev_region(vd.devnum, 1);
err_buf:
    kfree(vd.buf);
    return ret;
}

static void __exit vdev_exit(void)
{
    device_destroy(vd.cls, vd.devnum);
    class_destroy(vd.cls);
    cdev_del(&vd.cdev);
    unregister_chrdev_region(vd.devnum, 1);
    kfree(vd.buf);
    pr_info("vdev: unloaded\n");
}

module_init(vdev_init);
module_exit(vdev_exit);
