cmake_minimum_required(VERSION 3.27)

project(FelineOS)

enable_language(ASM)

set(CMAKE_CXX_STANDARD 23)

SET(LOG_PATH_PREFIX_TRUNCATE_LEN 0 CACHE STRING "How many characters from the full path should be truncated in log messages")
add_definitions(-DLOG_PATH_PREFIX_TRUNCATE_LEN=${LOG_PATH_PREFIX_TRUNCATE_LEN})

# Each subdirectory's CMakeLists.txt must set ${MODULE_OBJS} to be every object
# file that needs to be linked (use PARENT_SCOPE with the set() function)
# The ${CMAKE_SYSTEM_PROCESSOR} variable can be used to switch between i686 and arm
# The ${ARCHDIR} variable can also be used when the paths are otherwise equivalent

# If we're only building libFeline, don't build it as a kernel, or enforce one of our supported architectures (it should be arch-independent)
if (${LIBFELINE_ONLY})
	add_definitions(-DLIBFELINE_ONLY)
	include(CTest)
else()
	if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "i686")
		set(ARCHDIR system/kernel/arch/i386)
	elseif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "arm")
		set(ARCHDIR system/kernel/arch/arm)
	else()
		message(FATAL_ERROR "Unknown architecture " ${CMAKE_SYSTEM_PROCESSOR} ", expected i686 or arm")
	endif()
	add_compile_options(-ffreestanding -fno-exceptions)
endif()

add_compile_options(-Werror -Wall -Wextra -fstack-protector -fno-omit-frame-pointer -fno-rtti)

add_subdirectory(system/libFeline)

# Stop before building the rest of the stuff
if (${LIBFELINE_ONLY})
return()
endif()

# Only needs to be explicitly linked to libc when in freestanding
target_link_libraries(feline PRIVATE c gcc supc++ -nostdlib)

add_subdirectory(system/libc)
add_library(c STATIC ${LIBC_OBJS})
target_include_directories(c PUBLIC system/libc/include system/libc/include/c++)
target_link_libraries(c PRIVATE k)
target_compile_definitions(c PRIVATE __is_libk)

add_subdirectory(system/kernel)
add_library(k STATIC ${KERNEL_OBJS})
target_include_directories(k PUBLIC  system/kernel/include system/kernel/drivers/include)
target_include_directories(k PRIVATE ${ARCHDIR}/include)
target_compile_definitions(k PRIVATE __is_kernel)
target_link_libraries(k c feline)

set_target_properties(feline c k PROPERTIES SYSTEM TRUE)

add_custom_command(OUTPUT ${ARCHDIR}/linker.ld
	COMMAND mkdir -p ${ARCHDIR}
	COMMAND ${CMAKE_CXX_COMPILER} ${CMAKE_CXX_FLAGS} -CC -E -x c++
	${CMAKE_CURRENT_SOURCE_DIR}/${ARCHDIR}/linker.ldpp -o ${ARCHDIR}/linker.ld
	-isystem ${CMAKE_CURRENT_SOURCE_DIR}/system/kernel/include
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${ARCHDIR}/linker.ldpp
	${CMAKE_CURRENT_SOURCE_DIR}/system/kernel/include/kernel/vtopmem.h
	COMMENT "Preprocess the linker file"
	)
add_custom_target(linkerscript DEPENDS ${ARCHDIR}/linker.ld)

if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "arm")
	add_library(crts OBJECT ${ARCHDIR}/boot/crti.cpp ${ARCHDIR}/boot/crtn.cpp)
else()
	add_library(crts OBJECT ${ARCHDIR}/boot/crti.S ${ARCHDIR}/boot/crtn.S)
endif()

add_executable(FelineOS.kernel ${ARCHDIR}/boot/boot.S)
target_compile_definitions(FelineOS.kernel PRIVATE __is_kernel)
target_link_libraries(FelineOS.kernel felineTest c k gcc supc++ -nostdlib)
target_include_directories(FelineOS.kernel PRIVATE system/kernel/include ${ARCHDIR}/include)
add_dependencies(FelineOS.kernel linkerscript crts)
target_link_options(FelineOS.kernel PRIVATE -T ${ARCHDIR}/linker.ld)

# Ask GCC to get the full path name of crtbegin. and crtend.o
execute_process(COMMAND ${CMAKE_CXX_COMPILER} -print-file-name=crtbegin.o OUTPUT_VARIABLE CRTBEGIN_O OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND ${CMAKE_CXX_COMPILER} -print-file-name=crtend.o OUTPUT_VARIABLE CRTEND_O OUTPUT_STRIP_TRAILING_WHITESPACE)

if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "i686")
	# Setup files before creating the iso
	add_custom_target(isodir_FelineOS.kernel
		COMMAND ${CMAKE_COMMAND} -E copy
		${CMAKE_CURRENT_BINARY_DIR}/FelineOS.kernel
		${CMAKE_CURRENT_BINARY_DIR}/isodir/boot/FelineOS.kernel
		DEPENDS FelineOS.kernel)
	add_custom_target(grub.cfg
		COMMAND ${CMAKE_COMMAND} -E copy
		${CMAKE_CURRENT_SOURCE_DIR}/grub.cfg
		${CMAKE_CURRENT_BINARY_DIR}/isodir/boot/grub/grub.cfg
		DEPENDS grub.cfg)
	# Add a target for an iso file (not used on all targets)
	add_custom_target(FelineOS.iso
		COMMAND grub-mkrescue -o FelineOS.iso isodir
		DEPENDS isodir_FelineOS.kernel grub.cfg)
	# Add targets for running and debugging the os directly from qemu or using grub
	add_custom_target(run_qemu
		COMMAND qemu-system-i386 -serial stdio -kernel FelineOS.kernel
		DEPENDS FelineOS.kernel)
	add_custom_target(debug_qemu
		COMMAND qemu-system-i386 -serial stdio -kernel FelineOS.kernel -S -s
		DEPENDS FelineOS.kernel)
	add_custom_target(run_grub
		COMMAND qemu-system-i386 -serial stdio -cdrom FelineOS.iso -boot d
		DEPENDS FelineOS.iso)
	add_custom_target(debug_grub
		COMMAND qemu-system-i386 -serial stdio -cdrom FelineOS.iso -boot d -S -s
		DEPENDS FelineOS.iso)

	# Objects 'crti.o' and 'crtbegin.o' will be linked before all other objects and libraries.
	# Objects 'crtend.o' and 'crtn.o' will be linked after all other objects and libraries.
	SET(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_CXX_COMPILER> <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> CMakeFiles/crts.dir/${ARCHDIR}/boot/crti.S.obj ${CRTBEGIN_O} <OBJECTS> -o <TARGET> <LINK_LIBRARIES> ${CRTEND_O} CMakeFiles/crts.dir/${ARCHDIR}/boot/crtn.S.obj")

elseif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "arm")
	# Add a target for creating a raw binary file (not used on all targets)
	add_custom_target(kernel.bin
		COMMAND arm-none-eabi-objcopy -O binary FelineOS.kernel kernel.bin
		DEPENDS FelineOS.kernel)
	# Add targets for running and debugging the os directly from qemu
	add_custom_target(run_qemu
		COMMAND qemu-system-arm -M raspi1ap -serial stdio -dtb ../../bcm2708-rpi-b.dtb -device loader,file=kernel.bin,addr=0x8000,cpu-num=0
		DEPENDS kernel.bin)
	add_custom_target(debug_qemu
		COMMAND qemu-system-arm -M raspi1ap -serial stdio -dtb ../../bcm2708-rpi-b.dtb -device loader,file=kernel.bin,addr=0x8000,cpu-num=0 -S -s
		DEPENDS kernel.bin)

	# Objects 'crti.o' and 'crtbegin.o' will be linked before all other objects and libraries.
	# Objects 'crtend.o' and 'crtn.o' will be linked after all other objects and libraries.
	SET(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_CXX_COMPILER> <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> CMakeFiles/crts.dir/${ARCHDIR}/boot/crti.cpp.obj ${CRTBEGIN_O} <OBJECTS> -o <TARGET> <LINK_LIBRARIES> ${CRTEND_O} CMakeFiles/crts.dir/${ARCHDIR}/boot/crtn.cpp.obj")
else()
	message(FATAL_ERROR "Unknown architecture " ${CMAKE_SYSTEM_PROCESSOR} ", expected i686 or arm")
endif()
