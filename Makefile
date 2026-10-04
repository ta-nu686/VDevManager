CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

all: driver app tests_bin

driver:
	$(MAKE) -C driver

app: userspace/main.cpp userspace/VDevManager.cpp userspace/VDevManager.hpp include/vdev_ioctl.h
	$(CXX) $(CXXFLAGS) -o vdevmanager userspace/main.cpp userspace/VDevManager.cpp

tests_bin: tests/test_vdev.cpp userspace/VDevManager.cpp
	$(CXX) $(CXXFLAGS) -o tests/test_vdev tests/test_vdev.cpp userspace/VDevManager.cpp

load: driver
	sudo insmod driver/vdev.ko
	sudo chmod 666 /dev/mydev
	@echo "Loaded. Check: ls -l /dev/mydev ; dmesg | tail"

unload:
	sudo rmmod vdev

test: tests_bin
	./tests/test_vdev

clean:
	$(MAKE) -C driver clean
	rm -f vdevmanager tests/test_vdev

.PHONY: all driver app tests_bin load unload test clean
