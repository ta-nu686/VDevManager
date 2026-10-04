# VDevManager - Virtual Character Device Driver with C++ System Programming Interface

Author: Tanisha Mohapatra | Project: 20-day Linux / C++ / Computer Architecture training

## What it is
A Linux kernel module that creates a virtual character device `/dev/mydev` backed by a
4 KB kernel buffer, plus a menu-driven C++ application that talks to it through system calls.

```
 C++ app (user space)  --open/read/write/ioctl-->  VFS  -->  vdev.ko (kernel space)  -->  4 KB kernel buffer
```

## Folder structure
| Path | Purpose |
|---|---|
| `driver/vdev.c` | Kernel module (cdev, file_operations, ioctl, mutex) |
| `include/vdev_ioctl.h` | Shared ioctl numbers + structs (kernel and user) |
| `userspace/` | C++ `VDevManager` class + menu app |
| `tests/test_vdev.cpp` | 13 automated test cases |
| `docs/` | Design, test report, viva Q&A |

## Features
- open / read / write / release / llseek / ioctl
- Modes: NORMAL and UPPERCASE (driver converts written text)
- ioctl: CLEAR_BUFFER, GET_BUFFER_SIZE, SET_MODE, GET_MODE, GET_STATISTICS
- Mutex-protected buffer, partial write when full, `-ENOSPC` when full
- Statistics: opens, reads, writes, bytes, buffer usage

## Requirements (Ubuntu / Debian)
```
sudo apt update
sudo apt install build-essential linux-headers-$(uname -r)
```
Needs a real Linux machine or VM (not WSL1; Secure Boot must be off, or the module must be signed).

## Build and run
```
make driver          # builds driver/vdev.ko
make app             # builds ./vdevmanager
make tests_bin       # builds ./tests/test_vdev
make load            # insmod + chmod 666 /dev/mydev
./vdevmanager        # run the menu app
make test            # run the 13 automated tests
dmesg | tail -20     # see kernel log messages
make unload          # rmmod vdev
```

## Demo script (for presentation)
1. `make load` then `ls -l /dev/mydev` and `dmesg | tail`
2. `./vdevmanager` -> 1 (open) -> 2 (write "hello kernel") -> 3 (read) -> 5 (status)
3. 6 (mode 1) -> 2 (write "abc") -> 3 (read shows `hello kernelABC`)
4. 8 (statistics) -> 4 (clear) -> 9 (close) -> 10 (exit)
5. `make test` -> all PASS, then `make unload` and `dmesg | tail`

## Training topics covered
Linux (modules, /dev, insmod/rmmod, dmesg, Makefile, permissions), C++ (classes, RAII,
STL string, encapsulation, error handling), system programming (open/read/write/lseek/ioctl/close,
errno), computer architecture (user vs kernel mode, system-call boundary, memory/buffers),
hardware-software interface (device file abstraction, driver as the bridge), Git/documentation.
