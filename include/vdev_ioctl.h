/* Shared between kernel driver and user-space C++ app (the "contract") */
#ifndef VDEV_IOCTL_H
#define VDEV_IOCTL_H

#ifdef __KERNEL__
#include <linux/ioctl.h>
#include <linux/types.h>
#else
#include <sys/ioctl.h>
#include <linux/types.h>
#endif

#define VDEV_MAGIC        'V'
#define VDEV_BUFFER_SIZE  4096

#define VDEV_MODE_NORMAL    0   /* store data as written                     */
#define VDEV_MODE_UPPERCASE 1   /* driver converts written data to UPPERCASE */

struct vdev_stats {
    __u32 open_count;
    __u32 read_count;
    __u32 write_count;
    __u32 buffer_used;
    __u32 buffer_size;
    __u32 mode;
    __u64 bytes_read;
    __u64 bytes_written;
};

#define VDEV_CLEAR_BUFFER     _IO(VDEV_MAGIC, 1)
#define VDEV_GET_BUFFER_SIZE  _IOR(VDEV_MAGIC, 2, __u32)
#define VDEV_SET_MODE         _IOW(VDEV_MAGIC, 3, __u32)
#define VDEV_GET_MODE         _IOR(VDEV_MAGIC, 4, __u32)
#define VDEV_GET_STATISTICS   _IOR(VDEV_MAGIC, 5, struct vdev_stats)

#endif
