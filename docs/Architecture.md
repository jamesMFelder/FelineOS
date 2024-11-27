# FelineOS Architecture

## Layout

* system/libc
  * merged libc and libk
  * headers are self-contained
  * source files may reference anything
* system/libFeline
  * theoretically reusable utilities library
    * anything that is not in standard c, but also isn't inherently kernel-specific
  * may reference libc headers
* system/kernel
  * the actual kernel that's the core of this project
  * may reference anything

## Userland

### System Calls
3 registers used: which system call, pointer to args structure and pointer to
the result structure. The actual syscall function is
`bool raw_syscall(enum syscall_number which, void *args, void *result);`
A macro generates all the structures, error enums and basic wrapper functions.
Ex: open has the following types:

    struct __syscall_open_args {
        USER_PTR(char const) path;
        int flags;
    };
    enum __syscall_open_errors {
        open_none; // auto-added
        open_invalid_syscall; // auto-added
        open_any;
    };
    struct __syscall_open_result {
        __syscall_open_errors error; // auto-added
        int fd;
    };
    void __syscall_open(__syscall_open_args &args, __syscall_open_result &result) {
        raw_syscall(syscall_open, &args, &result);
    }

This allows typesafeish direct calling, while still having an escape hatch
(`raw_syscall`). One nice thing about structs for the escape hatch is that
they can be created more platform-independently (one function for all number
of arguments, on all compilers).

USER_PTR(T) expands to T* in userland, and UserPtr\<T\> in the kernel.

**Note: please only call the __syscall\_\* functions in direct wrappers. It gets very verbose otherwise.**

This isn't used currently, but the kernel returns false if the syscall doesn't exist,
and true if it does. It also doesn't actually do anything if args and result are null
so one could check the existence of a syscall by simply calling it with two nulls.

## Kernel

### System Calls
The syscall_handler function switches on the syscall number, and handles
getting the right function or returning an error.

The USER_PTR(T) macro in the kernel is turned into a UserPtr\<T\>, which
handles verification of the pointer before you can access it.

### Memory management
* There's a c++ allocator `KGeneralAllocator` in `<kernel/kstdallocator.h>`
(which incidentally disables `std::allocator` so you don't accidentally use it in the kernel).
* It calls `malloc()`/`free()` in `libc`
* If it's being built for the kernel, `malloc` and `free` call `get_mem()` and `free_mem()`
  * Otherwise, it fails to compile.
* `get_mem`, `free_mem` wrap calls to `get_mem_area`, `free_mem_area` (PMM) and `map_range`, `unmap_range` (VMM)
  * There's also `get_mem_from` (fixed physical addr - eg. for a device)
* PMM: `get_mem_area` reserves some physical memory if possible, and you can
have it only reserve a specific location instead of any location.
  * `free_mem_area` does what you expect
* VMM: `map_range` adds a mapping for all address spaces (we only support 1 so far)
  * it comes with 4 variations of specificity
  * `unmap_range` also does what you expect
  * For now it supports automatically calling `get_mem_area` and `free_mem_area`,
  but this is deprecated.

### IO

#### Formatting

##### The kout logging
TL;DR: k(Dbg|Log|Warn|Err|Crit)() << "Strings!" << dec(543210) << hex(0xDEADBEEF) << nullptr;

Slightly longer:
Create an kout object with one of the following macros: `kDbg()`, `kLog()`, `kWarn()`, `kError()` or `kCritical()`

- currently the only difference is what they prefix the lines with
- There's also `kDbgNoAlloc()` and `kCriticalNoAlloc()` which don't do any allocation
  - very useful for early boot debugging before the memory managers aren't fully functional
  - also for when the system is in a bad state and about to crash
  - however, the dec(), hex() and ptr() functions still make allocations
    - see the deprecated section below for alternatives

Then use operator<< with KString(View)?s, char* strings and void*
- note about void*: it sometimes picks the wrong overload, just use ptr(…) in that case

For the default versions, it will output them all as one string when its destructor is called
For the NoAlloc versions it outputs them at the end of each function

##### Deprecated: k(log|warn|error|critical)f?
Basic printf-style logging, or simply logging a string.

As mentioned above, it is the shortest way to format a number without any
heap allocations, so the main valid use-case I see for it is debugging the
memory-managers (PMM, VMM and malloc).

This used to be everywhere, so it's not uncommon to see it in the codebase,
but new cases should be avoided when possible. It's also the only way to
do formatting from assembly (which shouldn't happen too much).

It has not and probably will not be ported to use the Settings::Logging stuff.

#### Displaying

- Settings::Logging
  - `output_func`: typedef for a function taking in a `char*` and `size_t` that logs them
  - everything else: functions of type `output_func`
    - they don't actually do the writing, just forward it to the correct function

- stdio
  - `putchar()` and `puts()`: forward to `__internal_putchar` and `__internal_writeStr`
  - __internal_*: in the kernel, they write to the serial port. They don't compile for user space yet.
    - they also write to the screen on x86 VGA monitors if available

For functions beyond here, you probably meant to call something from Settings::Logging or stdio

- Serial Port (blocking)
  - identical abstraction over UART (i686) and PL011 (arm)
  - init_serial(): configure the serial port
  - read_serial(): read the next character (not used)
  - put_serial(): write a character
  - write_serial(): write len characters from str
  - writestr_serial(): equivalent to write_serial(str, strlen(str))

- VGA text terminal (i686 old computers only)
  - vga_text_term::putchar(): only enabled if the screen is available
