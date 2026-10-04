#ifndef VDEV_MANAGER_HPP
#define VDEV_MANAGER_HPP

#include <string>
#include <sys/types.h>
#include "../include/vdev_ioctl.h"

/* C++ wrapper around /dev/mydev system calls (RAII: fd closed in destructor) */
class VDevManager {
public:
    explicit VDevManager(const std::string &path = "/dev/mydev");
    ~VDevManager();
    VDevManager(const VDevManager &) = delete;
    VDevManager &operator=(const VDevManager &) = delete;

    bool open();
    void close();
    bool isOpen() const { return fd_ >= 0; }

    ssize_t write(const std::string &data);   // bytes written, -1 on error
    bool    read(std::string &out);           // reads whole buffer from start
    bool    clearBuffer();
    bool    getBufferSize(unsigned int &size);
    bool    setMode(unsigned int mode);
    bool    getMode(unsigned int &mode);
    bool    getStats(vdev_stats &st);

    const std::string &lastError() const { return err_; }

private:
    void setError(const std::string &what);
    int         fd_;
    std::string path_;
    std::string err_;
};

#endif
