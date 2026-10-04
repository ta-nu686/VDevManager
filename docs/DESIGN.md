# Design

## Requirements (PRD summary)
- FR1 Create /dev/mydev on module load; remove on unload
- FR2 Write appends to a 4096-byte kernel buffer; read returns stored data
- FR3 ioctl: clear, get size, set/get mode, get statistics
- FR4 UPPERCASE mode converts incoming data
- FR5 C++ menu application with 10 options
- NFR Thread-safe (mutex), clean error codes, clean unload

## Architecture
User space: `main.cpp` (menu) -> `VDevManager` (wrapper class) -> libc syscalls.
Kernel space: VFS -> `vdev_fops` -> buffer protected by `struct mutex`.

## Class diagram (UML)
```
+--------------------------------+
| VDevManager                    |
+--------------------------------+
| - fd_ : int                    |
| - path_ : string               |
| - err_ : string                |
+--------------------------------+
| + open() : bool                |
| + close() : void               |
| + write(data) : ssize_t        |
| + read(out) : bool             |
| + clearBuffer() : bool         |
| + getBufferSize(size) : bool   |
| + setMode(mode) : bool         |
| + getMode(mode) : bool         |
| + getStats(st) : bool          |
| + lastError() : string         |
+--------------------------------+
```

## Sequence: write("abc") in UPPERCASE mode
App -> VDevManager.write -> write() syscall -> vdev_write -> lock -> copy_from_user ->
toupper loop -> used += len -> unlock -> return bytes -> App prints count.

## Error handling
-ENOSPC buffer full | -EFAULT bad user pointer | -EINVAL bad mode | -ENOTTY unknown ioctl |
-ERESTARTSYS interrupted while waiting for the mutex.
