# VDev Manager

**Virtual Character Device Driver with a C++ User-Space Manager**

Author: Tanisha Mohapatra | 20-day Linux / C++ / Computer Architecture Training Project

---

## About the Project

VDev Manager is a Linux system programming project that shows how a user-space C++ application talks to a kernel-space device driver.

The driver is a kernel module (`vdev.ko`). It creates a virtual character device, `/dev/mydev`, which is backed by a 4 KB buffer in kernel memory. The C++ application, `vdevmanager`, is a menu-driven program that opens this device and works with it using system calls. No real hardware is used. The driver only simulates a device, which makes it a safe way to learn how drivers work.

## Project Objective

The aim of this project is to understand:

- How Linux character device drivers and kernel modules work
- How user space and kernel space communicate
- Device file operations: `open()`, `read()`, `write()`, `lseek()`, `ioctl()` and `close()`
- C++ system programming on Linux
- How a driver acts as the bridge between a program and a device

## How It Works

```
 C++ app (VDevManager)                       -- user space
        |
        |  open / read / write / lseek / ioctl / close
        v
 ----------- system-call boundary (user mode -> kernel mode) -----------
        |
   VFS  -->  /dev/mydev  -->  vdev.ko         -- kernel space
                                  |
                                  v
                          4 KB kernel buffer (mutex-protected)
```

The driver is loaded into the kernel and creates the device file `/dev/mydev`. When the app writes to this file, the kernel passes the call to the driver, which stores the data in its buffer. Reading works the same way in reverse.

## Features

- Supports `open`, `read`, `write`, `release`, `llseek` and `ioctl`
- Two modes: **NORMAL** (stores text as written) and **UPPERCASE** (the driver converts written text to capital letters)
- Mutex protection so the buffer is safe when used by more than one process
- Partial write when the buffer is almost full, and `-ENOSPC` when it is completely full
- Statistics: number of opens, reads, writes, bytes transferred and buffer usage
- 13 automated test cases

### ioctl commands

| Command | What it does |
|---|---|
| `CLEAR_BUFFER` | Empties the buffer |
| `GET_BUFFER_SIZE` | Returns the buffer size |
| `SET_MODE` / `GET_MODE` | Sets or reads the mode (NORMAL / UPPERCASE) |
| `GET_STATISTICS` | Returns the usage statistics |

## Project Structure

```
VDevManager/
├── driver/
│   └── vdev.c               # kernel module (cdev, file_operations, ioctl, mutex)
├── include/
│   └── vdev_ioctl.h         # ioctl numbers and structs shared by driver and app
├── userspace/               # C++ VDevManager class and menu application
├── tests/
│   └── test_vdev.cpp        # 13 automated test cases
├── docs/                    # design document, test report, viva Q&A
├── Makefile
└── README.md
```

## Technologies Used

C, C++, Linux, Linux kernel module, character device driver, system calls, Makefile, Git

## Requirements

- A real Linux machine or a Linux VM (Ubuntu / Debian recommended). Standard WSL is not suitable for loading kernel modules.
- Secure Boot turned off, or the module must be signed
- GCC/G++, Make, kernel headers and sudo access

```bash
sudo apt update
sudo apt install build-essential linux-headers-$(uname -r)
```

## Build and Run

```bash
make driver       # builds driver/vdev.ko
make app          # builds ./vdevmanager
make tests_bin    # builds ./tests/test_vdev
make load         # loads the module (insmod) and sets permissions on /dev/mydev
./vdevmanager     # runs the menu application
make test         # runs the 13 automated tests
dmesg | tail -20  # shows kernel log messages
make unload       # removes the module (rmmod)
```

To check that the driver loaded: `lsmod | grep vdev` and `ls -l /dev/mydev`.

## Demo Steps

1. `make load`, then `ls -l /dev/mydev` and `dmesg | tail`
2. Run `./vdevmanager` and choose `1` (open), `2` (write "hello kernel"), `3` (read), `5` (status)
3. Choose `6` (set mode 1), then `2` (write "abc"), then `3` (read). The output is `hello kernelABC`, because only the text written after the mode change is converted to uppercase.
4. Choose `8` (statistics), `4` (clear), `9` (close), `10` (exit)
5. `make test` (all tests should PASS), then `make unload` and `dmesg | tail`

## Output

> TODO: paste your own output here (this is the part that shows the project really ran).

```
(paste: ls -l /dev/mydev)
(paste: dmesg | tail)
(paste: make test result)
```

You can also add a screenshot of the menu application:

`![Menu app](docs/menu_screenshot.png)`

## Challenges I Faced

During the project, I initially faced difficulties while building and loading the Linux kernel module and understanding how the driver communicates with the user-space application. I also had to debug issues related to device-file operations such as reading and writing data. Understanding the difference between kernel-space and user-space was confusing at first, but testing each operation step by step helped me understand the communication flow. I also faced some Git/GitHub issues while uploading the project, which I resolved by configuring Git authentication correctly.


## What I Learned

This project gave me practical knowledge of Linux character device drivers, kernel modules, and user-space to kernel-space communication. I learned how a C++ application can interact with a device through a device file and how Linux system programming concepts work in practice. It also improved my debugging, testing, and Git/GitHub skills.

## Concepts Covered

- **Linux:** kernel modules, `/dev`, `insmod` / `rmmod`, `dmesg`, Makefiles, file permissions
- **C++:** classes, RAII, `std::string`, encapsulation, error handling
- **System programming:** `open` / `read` / `write` / `lseek` / `ioctl` / `close`, `errno`
- **Computer architecture:** user mode vs kernel mode, system-call boundary, memory and buffers
- **Hardware-software interface:** the device file as an abstraction, the driver as the bridge
- **Git and documentation:** version control, README, design and test documents

