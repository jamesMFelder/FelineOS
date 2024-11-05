/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#include "kernel/mem.h"
#include "kernel/paging.h"
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <drivers/serial.h>
#include <feline/fixed_width.h>
#include <limits>

typedef uint32_t volatile pl011_reg;

/* Contains all the registers in a PL011 UART device, in the right order */
/* Simply create a pointer to this at the address of the device, and access the
 * registers */
struct pl011 {
		pl011_reg dr;     /* Data Register */
		pl011_reg rscrer; /* Receive Status Register */
		uint32_t _padding_1[4];
		pl011_reg fr; /* Flag Register */
		uint32_t _padding_2[1];
		pl011_reg ilpr;  /* unused */
		pl011_reg ibrd;  /* Integer Baud rate divisor */
		pl011_reg fbrd;  /* Fractional Baud rate divisor */
		pl011_reg lcrh;  /* Line Control register */
		pl011_reg cr;    /* Control register */
		pl011_reg ifls;  /* Interupt FIFO Level Select Register register */
		pl011_reg imsc;  /* Interupt Mask Set Clear Register */
		pl011_reg ris;   /* Raw Interupt Status Register */
		pl011_reg mis;   /* Masked Interupt Status Register */
		pl011_reg icr;   /* Interupt Clear Register */
		pl011_reg dmacr; /* DMA Control Register */
		uint32_t _padding_3[13];
		pl011_reg itcr; /* Test Control register */
		pl011_reg itip; /* Integration test input reg */
		pl011_reg itop; /* Integration test output reg */
		pl011_reg tdr;  /* Test Data reg */
};

/**
 * \brief The PL011 UART
 *
 * It is the kernel virtual address space version;
 * for use in the I/O space, turn the 0x20 prefix into 0x7E
 * */
static struct pl011 *serial_port = reinterpret_cast<pl011 *>(0x2020'1000);

map_results map_serial() {
	return map_range(reinterpret_cast<void *>(0x2020'1000), sizeof(pl011),
	                 reinterpret_cast<void **>(&serial_port), MAP_DEVICE);
}

static void set(pl011_reg &reg, uint32_t const value) { reg = value; }

static uint32_t get(pl011_reg const &reg) { return reg; }

static bool is_set(pl011_reg const &reg, unsigned which_bit) {
	return reg & (0b1 << which_bit);
}

namespace magic_numbers {
constexpr uint32_t disable_all = 0;
namespace brd {
/* Set to 115200 baud (with a 48MHz uart clock
 * [https://forums.raspberrypi.com/viewtopic.php?t=226881#p1391755]) */
constexpr uint32_t clock_speed = 48 * 1'000'000;
constexpr float baud = 115200;
constexpr float baud_rate_divisor = clock_speed / (16 * baud);
constexpr uint16_t ibrd = static_cast<uint16_t>(baud_rate_divisor);
constexpr uint8_t fbrd = (baud_rate_divisor - ibrd) * 64 + 0.5;
} // namespace brd
namespace cr {
constexpr uint32_t enable = 1 << 0;
constexpr uint32_t loopback = 1 << 7;
constexpr uint32_t transmit = 1 << 8;
constexpr uint32_t receive = 1 << 9;
constexpr uint32_t hw_flow_control_rx = 1 << 14;
constexpr uint32_t hw_flow_control_tx = 1 << 15;
} // namespace cr
namespace imsc {
constexpr uint32_t CTSMIM = 1 << 1; // TODO: why did I enable this?
constexpr uint32_t RXIM = 1 << 4;   // receive interrupt
constexpr uint32_t TXIM = 1 << 5;   // transmit interrupt
constexpr uint32_t RTIM = 1 << 4;   // receive timeout
constexpr uint32_t FEIM = 1 << 4;   // framing error
} // namespace imsc
namespace lcr {
constexpr uint32_t FIFO = 1 << 4;
constexpr uint32_t WORD_LEN = (std::numeric_limits<char>::digits - 5) << 5;
} // namespace lcr
} // namespace magic_numbers

int init_serial() {
	/* Disable the mini UART */
	set(serial_port->cr, magic_numbers::disable_all);
	/* Disable all interrupts */
	set(serial_port->imsc,
	    magic_numbers::imsc::CTSMIM | magic_numbers::imsc::RXIM |
	        magic_numbers::imsc::TXIM | magic_numbers::imsc::RTIM |
	        magic_numbers::imsc::FEIM);
	/* Clear all old interrupts */
	set(serial_port->icr,
	    magic_numbers::disable_all); /* Everything is either "write 0 to clear",
	                                    "unsupported - write 0" or "reserved -
	                                    write 0" */
	/* Set sending rate to highest possible */
	set(serial_port->ibrd, magic_numbers::brd::ibrd);
	set(serial_port->fbrd, magic_numbers::brd::fbrd);
	/* General setup */
	set(serial_port->lcrh,
	    magic_numbers::lcr::FIFO | magic_numbers::lcr::WORD_LEN);
	set(serial_port->lcrh, 0b001110000);
	/* Enable the UART in loopback mode */
	set(serial_port->cr,
	    magic_numbers::cr::enable | magic_numbers::cr::loopback |
	        magic_numbers::cr::transmit | magic_numbers::cr::receive |
	        magic_numbers::cr::hw_flow_control_rx |
	        magic_numbers::cr::hw_flow_control_tx);
	/* enable IRQs (not now) */
	/* Test with loop-back mode */
	put_serial('*');
	/* if the byte returned is different */
	if (read_serial() != '*') {
		/* turn off the serial port, return an error */
		set(serial_port->cr, 0b0);
		return 1;
	}
	/* Enable the UART for normal operation */
	set(serial_port->cr,
	    magic_numbers::cr::enable | magic_numbers::cr::transmit |
	        magic_numbers::cr::receive | magic_numbers::cr::hw_flow_control_rx |
	        magic_numbers::cr::hw_flow_control_tx);
	// FIXME: loopback mode still prints the test character in QEMU, so this
	// lets the next character overwrite it.
	put_serial('\b');
	return 0;
}

static bool is_receive_empty() {
	// return the RXFE (Received FIFO Empty)
	return is_set(serial_port->fr, 4);
}

char read_serial() {
	while (is_receive_empty())
		;

	auto input = get(serial_port->dr);
	/* If there was an error */
	if ((input & 0b111100000000) != 0) {
		/* Return null (TODO: retry some amount of time before changing
		 * settings) */
		return '\0';
	}
	return input & 0xFF;
}

static bool is_transmit_full() {
	// return the TXFF (Sending FIFO Full)
	return is_set(serial_port->fr, 5);
}

void put_serial(char a) {
	while (is_transmit_full())
		;

	set(serial_port->dr, a);
}

void write_serial(const char *str, const size_t len) {
	for (size_t i = 0; i < len; i++) {
		put_serial(str[i]);
	}
	return;
}

void writestr_serial(const char *str) {
	for (const char *c = str; *c != '\0'; c++) {
		put_serial(*c);
	}
	return;
}
