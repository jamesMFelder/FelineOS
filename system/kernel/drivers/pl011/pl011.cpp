/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2022 James McNaughton Felder */
#include <drivers/serial.h>
#include <cstddef>
#include <feline/fixed_width.h>

/**
 * \brief The PL011 UART base address
 *
 * It is the kernel virtual address space version;
 * for use in the I/O space, turn the 0x20 prefix into 0x7E
 * */
unsigned char volatile * const PL011_BASE_ADDR=reinterpret_cast<unsigned char volatile*>(0x2020'1000);

typedef uint32_t volatile pl011_reg;

void set(pl011_reg &reg, uint32_t const value) {
	reg=value;
}

uint32_t get(pl011_reg const &reg) {
	return reg;
}

bool is_set(pl011_reg const &reg, unsigned which_bit) {
	return reg & (0b1 << which_bit);
}


pl011_reg &pl011_dr     = *reinterpret_cast<volatile uint32_t *>(PL011_BASE_ADDR + 0x00); /* Data Register */
pl011_reg &pl011_rscrer = *reinterpret_cast<volatile uint32_t *>(PL011_BASE_ADDR + 0x04); /* Receive Status Register */
pl011_reg &pl011_fr     = *reinterpret_cast<volatile uint32_t *>(PL011_BASE_ADDR + 0x18); /* Flag Register */
pl011_reg &pl011_ilpr   = *reinterpret_cast<volatile uint32_t *>(PL011_BASE_ADDR + 0x20); /* unused */
pl011_reg &pl011_ibrd   = *reinterpret_cast<volatile uint32_t *>(PL011_BASE_ADDR + 0x24); /* Integer Baud rate divisor */
pl011_reg &pl011_fbrd   = *reinterpret_cast<volatile uint32_t *>(PL011_BASE_ADDR + 0x28); /* Fractional Baud rate divisor */
pl011_reg &pl011_lcrh   = *reinterpret_cast<volatile uint32_t *>(PL011_BASE_ADDR + 0x2C); /* Line Control register */
pl011_reg &pl011_cr     = *reinterpret_cast<volatile uint32_t *>(PL011_BASE_ADDR + 0x30); /* Control register */
pl011_reg &pl011_ifls   = *reinterpret_cast<volatile uint32_t *>(PL011_BASE_ADDR + 0x34); /* Interupt FIFO Level Select Register register */
pl011_reg &pl011_imsc   = *reinterpret_cast<volatile uint32_t *>(PL011_BASE_ADDR + 0x38); /* Interupt Mask Set Clear Register */
pl011_reg &pl011_ris    = *reinterpret_cast<volatile uint32_t *>(PL011_BASE_ADDR + 0x3C); /* Raw Interupt Status Register */
pl011_reg &pl011_mis    = *reinterpret_cast<volatile uint32_t *>(PL011_BASE_ADDR + 0x40); /* Masked Interupt Status Register */
pl011_reg &pl011_icr    = *reinterpret_cast<volatile uint32_t *>(PL011_BASE_ADDR + 0x44); /* Interupt Clear Register */
pl011_reg &pl011_dmacr  = *reinterpret_cast<volatile uint32_t *>(PL011_BASE_ADDR + 0x48); /* DMA Control Register */
pl011_reg &pl011_itcr   = *reinterpret_cast<volatile uint32_t *>(PL011_BASE_ADDR + 0x80); /* Test Control register */
pl011_reg &pl011_itip   = *reinterpret_cast<volatile uint32_t *>(PL011_BASE_ADDR + 0x84); /* Integration test input reg */
pl011_reg &pl011_itop   = *reinterpret_cast<volatile uint32_t *>(PL011_BASE_ADDR + 0x88); /* Integration test output reg */
pl011_reg &pl011_tdr    = *reinterpret_cast<volatile uint32_t *>(PL011_BASE_ADDR + 0x8C); /* Test Data reg */

int init_serial() {
	/* Disable the mini UART */
	set(pl011_cr, 0b0);
	/* Disable all interrupts */
	set(pl011_imsc, 0b000011110010);
	//                ┃┃┃┃┃┃┃┃┃┃┃┗━ unsupported (write 0)
	//                ┃┃┃┃┃┃┃┃┃┃┗━━ uartcts modem
	//                ┃┃┃┃┃┃┃┃┗┻━━━ unsupported [drsmim and dcdmim] (write 0)
	//                ┃┃┃┃┃┃┗┻━━━━━ receive and transmit
	//                ┃┃┃┃┃┗━━━━━━━ receive timeout
	//                ┃┃┃┃┗━━━━━━━━ framing error
	//                ┃┃┃┗━━━━━━━━━ parity error
	//                ┃┃┗━━━━━━━━━━ break error
	//                ┃┗━━━━━━━━━━━ overrun error
	//                ┗━━━━━━━━━━━━ reserved for this and all more significant bits (write 0)
	/* Clear all old interrupts */
	set(pl011_icr, 0b0); /* Everything is either "write 0 to clear", "unsupported - write 0" or "reserved - write 0" */
	/* Set sending rate to highest possible */
	/* Set to 115200 baud (with a 48MHz uart clock [https://forums.raspberrypi.com/viewtopic.php?t=226881#p1391755]) */
	/* Baud Rate Divisor=(48*10^6)/(16*115200)=26.041666*/
	/* BRD[i]=26 */
	/* BRD[f]=integer((0.041666*64)+0.5)=3*/
	set(pl011_ibrd, 26);
	set(pl011_fbrd, 3);
	/* General setup */
	set(pl011_lcrh, 0b001110000);
	//                ┃┃┃┃┃┃┃┃┗━ don't send break
	//                ┃┃┃┃┃┃┃┗━━ disable parity
	//                ┃┃┃┃┃┃┗━━━  set parity type to odd(0) or even(1) (doesn't matter when disabled)
	//                ┃┃┃┃┃┗━━━━ only send one stop bit
	//                ┃┃┃┃┗━━━━━ enable FIFOs
	//                ┃┃┗┻━━━━━━ 8 bits per word
	//                ┃┗━━━━━━━━ disable stick parity (what is it?)
	//                ┗━━━━━━━━━ reserved for this and all more significant bits (write 0)
	/* Enable the UART in loopback mode */
	//set(pl011_cr, 0b01100001110000001);
	//              ┃┃┃┃┃?┃┃┃┃┃┃┃┃┃┃┗━ enable the UART
	//              ┃┃┃┃┃┃┃┃┃┃┃┃┃┃┗┻━━ unsupported (write 0)
	//              ┃┃┃┃┃┃┃┃┃┃┗┻┻┻━━━━ reserved (write 0)
	//              ┃┃┃┃┃┃┃┃┃┗━━━━━━━━ enable loop-back (for testing)
	//              ┃┃┃┃┃┃┃┃┗━━━━━━━━━ enable transmitting
	//              ┃┃┃┃┃┃┃┗━━━━━━━━━━ enable receiving
	//              ┃┃┃┃┃┃┗━━━━━━━━━━━ unsupported (write 0)
	//              ┃┃┃┃┃┗━━━━━━━━━━━━ RTS
	//              ┃┃┃┃┗━━━━━━━━━━━━━ unsupported (write 0)
	//              ┃┃┃┗━━━━━━━━━━━━━━ unsupported (write 0)
	//              ┃┃┗━━━━━━━━━━━━━━━ receiving hardware flow control
	//              ┃┗━━━━━━━━━━━━━━━━ transmitting hardware flow control
	//              ┗━━━━━━━━━━━━━━━━━ reserved for this and all more significant bits (write 0)
	/* enable IRQs (not now) */
	/* Test with loop-back mode */
	//put_serial('*');
	/* if the byte returned is different */
	//if (read_serial() != '*') {
		/* turn off the serial port, return an error */
		//set(pl011_cr, 0b0);
		//return 1;
	//}
	/* Enable the UART for normal operation */
	set(pl011_cr, 0b01100001100000001);
	//              ┃┃┃┃┃?┃┃┃┃┃┃┃┃┃┃┗━ enable the UART
	//              ┃┃┃┃┃┃┃┃┃┃┃┃┃┃┗┻━━ unsupported (write 0)
	//              ┃┃┃┃┃┃┃┃┃┃┗┻┻┻━━━━ reserved (write 0)
	//              ┃┃┃┃┃┃┃┃┃┗━━━━━━━━ disable loop-back
	//              ┃┃┃┃┃┃┃┃┗━━━━━━━━━ enable transmitting
	//              ┃┃┃┃┃┃┃┗━━━━━━━━━━ enable receiving
	//              ┃┃┃┃┃┃┗━━━━━━━━━━━ unsupported (write 0)
	//              ┃┃┃┃┃┗━━━━━━━━━━━━ RTS
	//              ┃┃┃┃┗━━━━━━━━━━━━━ unsupported (write 0)
	//              ┃┃┃┗━━━━━━━━━━━━━━ unsupported (write 0)
	//              ┃┃┗━━━━━━━━━━━━━━━ receiving hardware flow control
	//              ┃┗━━━━━━━━━━━━━━━━ transmitting hardware flow control
	//              ┗━━━━━━━━━━━━━━━━━ reserved for this and all more significant bits (write 0)
	return 0;
}

static bool is_receive_empty() {
	//return the RXFE (Received FIFO Empty)
	return is_set(pl011_fr, 4);
}

char read_serial() {
	while (is_receive_empty());

	auto input = get(pl011_dr);
	/* If there was an error */
	if ((input & 0b111100000000) != 0) {
		/* Return null (TODO: retry some amount of time before changing settings) */
		return '\0';
	}
	return input & 0xFF;
}

static bool is_transmit_full() {
	//return the TXFF (Sending FIFO Full)
	return is_set(pl011_fr, 5);
}

void put_serial(char a) {
	while (is_transmit_full());

	set(pl011_dr, a);
}

void write_serial(const char *str, const size_t len){
	for(size_t i=0; i<len; i++){
		put_serial(str[i]);
	}
	return;
}

void writestr_serial(const char *str){
	for(const char *c=str; *c!='\0'; c++){
		put_serial(*c);
	}
	return;
}
