#include "VDevManager.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>

VDevManager::VDevManager(const std::string &path) : fd_(-1), path_(path) {}

VDevManager::~VDevManager() { close(); }

void VDevManager::setError(const std::string &what)
{
    err_ = what + ": " + std::strerror(errno);
}

bool VDevManager::open()
{
    if (isOpen()) { err_ = "device already open"; return false; }
    fd_ = ::open(path_.c_str(), O_RDWR);
    if (fd_ < 0) { setError("open " + path_); return false; }
    return true;
}

void VDevManager::close()
{
    if (fd_ >= 0) { ::close(fd_); fd_ = -1; }
}

ssize_t VDevManager::write(const std::string &data)
{
    if (!isOpen()) { err_ = "device not open"; return -1; }
    ssize_t n = ::write(fd_, data.data(), data.size());
    if (n < 0) setError("write");
    return n;
}

bool VDevManager::read(std::string &out)
{
    if (!isOpen()) { err_ = "device not open"; return false; }
    out.clear();
    ::lseek(fd_, 0, SEEK_SET);              // always read from the start
    char chunk[512];
    ssize_t n;
    while ((n = ::read(fd_, chunk, sizeof(chunk))) > 0)
        out.append(chunk, static_cast<size_t>(n));
    if (n < 0) { setError("read"); return false; }
    return true;
}

bool VDevManager::clearBuffer()
{
    if (!isOpen()) { err_ = "device not open"; return false; }
    if (ioctl(fd_, VDEV_CLEAR_BUFFER) < 0) { setError("ioctl CLEAR_BUFFER"); return false; }
    return true;
}

bool VDevManager::getBufferSize(unsigned int &size)
{
    if (!isOpen()) { err_ = "device not open"; return false; }
    __u32 v = 0;
    if (ioctl(fd_, VDEV_GET_BUFFER_SIZE, &v) < 0) { setError("ioctl GET_BUFFER_SIZE"); return false; }
    size = v;
    return true;
}

bool VDevManager::setMode(unsigned int mode)
{
    if (!isOpen()) { err_ = "device not open"; return false; }
    __u32 v = mode;
    if (ioctl(fd_, VDEV_SET_MODE, &v) < 0) { setError("ioctl SET_MODE"); return false; }
    return true;
}

bool VDevManager::getMode(unsigned int &mode)
{
    if (!isOpen()) { err_ = "device not open"; return false; }
    __u32 v = 0;
    if (ioctl(fd_, VDEV_GET_MODE, &v) < 0) { setError("ioctl GET_MODE"); return false; }
    mode = v;
    return true;
}

bool VDevManager::getStats(vdev_stats &st)
{
    if (!isOpen()) { err_ = "device not open"; return false; }
    if (ioctl(fd_, VDEV_GET_STATISTICS, &st) < 0) { setError("ioctl GET_STATISTICS"); return false; }
    return true;
}
