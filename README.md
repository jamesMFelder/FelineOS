# FelineOS
A place for me to experiment with how operating system kernels work, based on the [OSDev Meaty Skeleton template](https://wiki.osdev.org/Meaty_Skeleton). Not intended to be actually used.

It currently supports:
- booting on x86 (using [GRUB](https://www.gnu.org/software/grub)) and a Raspberry Pi (only tested on the 1B)
- creating and switching between threads
- system calls (but not starting a user program, so they aren't very helpful yet)
- regular timer interrupts
- drawing rectangles on the screen (x86 only)
- writing to the serial port

See the [Architecture file](docs/Architecture.md) for more details.
