if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "i686")
	set(KERN_ARCH_OBJS
		system/kernel/arch/i386/boot/cpuid.cpp
		system/kernel/arch/i386/gdt/gdt.cpp
		system/kernel/arch/i386/gdt/set_gdt.S
		system/kernel/arch/i386/interrupts/idt.cpp
		system/kernel/arch/i386/interrupts/isr.S
		system/kernel/arch/i386/io/io.cpp
		system/kernel/drivers/uart/uart.cpp
		)
elseif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "arm")
	set(KERN_ARCH_OBJS
		system/kernel/arch/arm/devicetree/devicetree.cpp
		system/kernel/arch/arm/interrupts/idt.S
		system/kernel/arch/arm/interrupts/isr.S
		system/kernel/arch/arm/interrupts/handle_abort.cpp
		system/kernel/drivers/pl011/pl011.cpp
		)
else()
	message(FATAL_ERROR "Unknown architecture " ${CMAKE_SYSTEM_PROCESSOR} ", expected i686 or arm")
endif()

set(KERNEL_OBJS
	${KERN_ARCH_OBJS}

	${ARCHDIR}/arch.cpp
	${ARCHDIR}/boot/boot.S
	${ARCHDIR}/mem/phys_mem.cpp
	${ARCHDIR}/stack/walk_stack.S
	${ARCHDIR}/virt_mem/paging.cpp
	${ARCHDIR}/virt_mem/enable_paging.S

	system/kernel/drivers/framebuffer/framebuffer.cpp

	system/kernel/kernel/backtrace.cpp
	system/kernel/kernel/kernel.cpp
	system/kernel/kernel/log.cpp
	system/kernel/kernel/mem.cpp
	system/kernel/kernel/page.cpp
	system/kernel/kernel/phys_mem.cpp
	system/kernel/kernel/syscall.cpp
	PARENT_SCOPE)
