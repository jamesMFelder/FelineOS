# the name of the target operating system
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# which compilers to use for C and C++
set(CMAKE_C_COMPILER   arm-none-eabi-gcc)
add_compile_options(-mcpu=arm1176jzf-s -mfloat-abi=soft -mapcs-frame -mno-unaligned-access)

# where is the target environment located
set(CMAKE_FIND_ROOT_PATH  /home/james/src/FelineOS/cross/arm-none-eabi /home/james/src/FelineOS/cross/lib/gcc/arm-none-eabi)

# don't try to compile a test program because the kernel isn't built yet
# instead, just assume it works
set(CMAKE_C_COMPILER_WORKS TRUE)
set(CMAKE_CXX_COMPILER_WORKS TRUE)

# adjust the default behavior of the FIND_XXX() commands:
# search programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# search headers and libraries in the target environment
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
