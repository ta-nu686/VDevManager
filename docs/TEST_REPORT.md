# Test Report (fill the "Actual" column after your own run)

| ID | Test | Expected | Actual |
|---|---|---|---|
| TC1 | Open device | success | |
| TC2 | Write "hello" | returns 5 | |
| TC3 | Read | "hello" | |
| TC4 | Buffer size | 4096 | |
| TC5 | Set mode UPPERCASE | success | |
| TC6 | Write "abc" in UPPERCASE | read = "helloABC" | |
| TC7 | Get mode | 1 | |
| TC8 | Set invalid mode 7 | rejected (EINVAL) | |
| TC9 | Clear buffer | read returns empty | |
| TC10 | Write 5000 bytes | 4096 accepted (partial) | |
| TC11 | Write when full | error ENOSPC | |
| TC12 | Statistics | used = 4096, writes >= 3 | |
| TC13 | Write after close | error | |

Run: `make test`. Paste the terminal output / screenshot here.
