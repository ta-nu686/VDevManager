/* Automated tests - run AFTER loading the driver:  ./tests/test_vdev */
#include <iostream>
#include <string>
#include "../userspace/VDevManager.hpp"

static int passed = 0, failed = 0;

#define CHECK(name, cond) do { \
    if (cond) { std::cout << "[PASS] " << name << "\n"; ++passed; } \
    else      { std::cout << "[FAIL] " << name << "\n"; ++failed; } } while (0)

int main()
{
    VDevManager dev;
    CHECK("TC1  open device", dev.open());
    if (!dev.isOpen()) { std::cout << dev.lastError() << "\nIs the driver loaded?\n"; return 1; }

    dev.clearBuffer();
    CHECK("TC2  write 'hello' returns 5", dev.write("hello") == 5);

    std::string out;
    CHECK("TC3  read back 'hello'", dev.read(out) && out == "hello");

    unsigned size = 0;
    CHECK("TC4  buffer size is 4096", dev.getBufferSize(size) && size == 4096);

    CHECK("TC5  set UPPERCASE mode", dev.setMode(VDEV_MODE_UPPERCASE));
    dev.write("abc");
    CHECK("TC6  uppercase conversion", dev.read(out) && out == "helloABC");

    unsigned mode = 99;
    CHECK("TC7  get mode == 1", dev.getMode(mode) && mode == 1);
    CHECK("TC8  invalid mode rejected", !dev.setMode(7));
    dev.setMode(VDEV_MODE_NORMAL);

    CHECK("TC9  clear buffer", dev.clearBuffer() && dev.read(out) && out.empty());

    std::string big(5000, 'x');
    CHECK("TC10 overflow -> partial write of 4096", dev.write(big) == 4096);
    CHECK("TC11 full buffer -> ENOSPC", dev.write("y") == -1);

    vdev_stats st;
    CHECK("TC12 statistics", dev.getStats(st) && st.buffer_used == 4096 && st.write_count >= 3);

    dev.clearBuffer();
    dev.close();
    CHECK("TC13 operations after close fail", dev.write("z") == -1);

    std::cout << "\nPassed: " << passed << "  Failed: " << failed << "\n";
    return failed ? 1 : 0;
}
