#include <iostream>
#include <string>
#include <limits>
#include "VDevManager.hpp"

static void showMenu()
{
    std::cout << "\n===== VDevManager =====\n"
                 " 1. Open device\n"
                 " 2. Write data\n"
                 " 3. Read data\n"
                 " 4. Clear buffer\n"
                 " 5. Buffer status\n"
                 " 6. Set mode\n"
                 " 7. Get mode\n"
                 " 8. Statistics\n"
                 " 9. Close device\n"
                 "10. Exit\n"
                 "Choice: ";
}

static const char *modeName(unsigned m)
{
    return m == VDEV_MODE_UPPERCASE ? "UPPERCASE" : "NORMAL";
}

static void flushLine()
{
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

int main(int argc, char *argv[])
{
    VDevManager dev(argc > 1 ? argv[1] : "/dev/mydev");
    int choice = 0;

    while (true) {
        showMenu();
        if (!(std::cin >> choice)) {
            if (std::cin.eof()) break;
            std::cin.clear();
            flushLine();
            std::cout << "Invalid input.\n";
            continue;
        }
        flushLine();

        switch (choice) {
        case 1:
            if (dev.open()) std::cout << "Device opened.\n";
            else std::cout << "Error: " << dev.lastError() << "\n";
            break;
        case 2: {
            std::string text;
            std::cout << "Enter text: ";
            std::getline(std::cin, text);
            ssize_t n = dev.write(text);
            if (n >= 0) {
                std::cout << n << " bytes written.\n";
                if (static_cast<size_t>(n) < text.size())
                    std::cout << "(buffer full - partial write)\n";
            } else std::cout << "Error: " << dev.lastError() << "\n";
            break;
        }
        case 3: {
            std::string out;
            if (dev.read(out))
                std::cout << "Buffer contents (" << out.size() << " bytes): \"" << out << "\"\n";
            else std::cout << "Error: " << dev.lastError() << "\n";
            break;
        }
        case 4:
            if (dev.clearBuffer()) std::cout << "Buffer cleared.\n";
            else std::cout << "Error: " << dev.lastError() << "\n";
            break;
        case 5: {
            vdev_stats st;
            if (dev.getStats(st))
                std::cout << "Used " << st.buffer_used << " / " << st.buffer_size
                          << " bytes, free " << st.buffer_size - st.buffer_used << "\n";
            else std::cout << "Error: " << dev.lastError() << "\n";
            break;
        }
        case 6: {
            int m;
            std::cout << "Mode (0 = NORMAL, 1 = UPPERCASE): ";
            if (!(std::cin >> m)) { std::cin.clear(); m = -1; }
            flushLine();
            if (m < 0) { std::cout << "Invalid mode.\n"; break; }
            if (dev.setMode(static_cast<unsigned>(m))) std::cout << "Mode set to " << modeName(m) << ".\n";
            else std::cout << "Error: " << dev.lastError() << "\n";
            break;
        }
        case 7: {
            unsigned m;
            if (dev.getMode(m)) std::cout << "Current mode: " << modeName(m) << "\n";
            else std::cout << "Error: " << dev.lastError() << "\n";
            break;
        }
        case 8: {
            vdev_stats st;
            if (dev.getStats(st))
                std::cout << "--- Statistics ---\n"
                          << "Opens  : " << st.open_count << "\n"
                          << "Reads  : " << st.read_count << " (" << st.bytes_read << " bytes)\n"
                          << "Writes : " << st.write_count << " (" << st.bytes_written << " bytes)\n"
                          << "Buffer : " << st.buffer_used << "/" << st.buffer_size << "\n"
                          << "Mode   : " << modeName(st.mode) << "\n";
            else std::cout << "Error: " << dev.lastError() << "\n";
            break;
        }
        case 9:
            if (dev.isOpen()) { dev.close(); std::cout << "Device closed.\n"; }
            else std::cout << "Device is not open.\n";
            break;
        case 10:
            std::cout << "Goodbye.\n";
            return 0;
        default:
            std::cout << "Invalid choice (1-10).\n";
        }
    }
    return 0;
}
