/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#include <drivers/serial.h>
#include <cstddef>
#include <feline/fixed_width.h>

typedef uint32_t volatile pl011_reg;

/* Contains all the registers in a PL011 UART device, in the right order */
/* Simply create a pointer to this at the address of the device, and access the registers */
struct pl011 {
	pl011_reg dr;     /* Data Register */
	pl011_reg rscrer; /* Receive Status Register */
	uint32_t _padding_1[4];
	pl011_reg fr;     /* Flag Register */
	uint32_t _padding_2[1];
	pl011_reg ilpr;   /* unused */
	pl011_reg ibrd;   /* Integer Baud rate divisor */
	pl011_reg fbrd;   /* Fractional Baud rate divisor */
	pl011_reg lcrh;   /* Line Control register */
	pl011_reg cr;     /* Control register */
	pl011_reg ifls;   /* Interupt FIFO Level Select Register register */
	pl011_reg imsc;   /* Interupt Mask Set Clear Register */
	pl011_reg ris;    /* Raw Interupt Status Register */
	pl011_reg mis;    /* Masked Interupt Status Register */
	pl011_reg icr;    /* Interupt Clear Register */
	pl011_reg dmacr;  /* DMA Control Register */
	uint32_t _padding_3[13];
	pl011_reg itcr;   /* Test Control register */
	pl011_reg itip;   /* Integration test input reg */
	pl011_reg itop;   /* Integration test output reg */
	pl011_reg tdr;    /* Test Data reg */
};

/**
 * \brief The PL011 UART
 *
 * It is the kernel virtual address space version;
 * for use in the I/O space, turn the 0x20 prefix into 0x7E
 * */
static struct pl011 *serial_port=reinterpret_cast<pl011*>(0x2020'1000);

static void set(pl011_reg &reg, uint32_t const value) {
	reg=value;
}

static uint32_t get(pl011_reg const &reg) {
	return reg;
}

static bool is_set(pl011_reg const &reg, unsigned which_bit) {
	return reg & (0b1 << which_bit);
}

int init_serial() {
	/* Disable the mini UART */
	set(serial_port->cr, 0b0);
	/* Disable all interrupts */
	set(serial_port->imsc, 0b000011110010);
	//                       ┃┃┃┃┃┃┃┃┃┃┃┗━ unsupported (write 0)
	//                       ┃┃┃┃┃┃┃┃┃┃┗━━ uartcts modem
	//                       ┃┃┃┃┃┃┃┃┗┻━━━ unsupported [drsmim and dcdmim] (write 0)
	//                       ┃┃┃┃┃┃┗┻━━━━━ receive and transmit
	//                       ┃┃┃┃┃┗━━━━━━━ receive timeout
	//                       ┃┃┃┃┗━━━━━━━━ framing error
	//                       ┃┃┃┗━━━━━━━━━ parity error
	//                       ┃┃┗━━━━━━━━━━ break error
	//                       ┃┗━━━━━━━━━━━ overrun error
	//                       ┗━━━━━━━━━━━━ reserved for this and all more significant bits (write 0)
	/* Clear all old interrupts */
	set(serial_port->icr, 0b0); /* Everything is either "write 0 to clear", "unsupported - write 0" or "reserved - write 0" */
	/* Set sending rate to highest possible */
	/* Set to 115200 baud (with a 48MHz uart clock [https://forums.raspberrypi.com/viewtopic.php?t=226881#p1391755]) */
	/* Baud Rate Divisor=(48*10^6)/(16*115200)=26.041666*/
	/* BRD[i]=26 */
	/* BRD[f]=integer((0.041666*64)+0.5)=3*/
	set(serial_port->ibrd, 26);
	set(serial_port->fbrd, 3);
	/* General setup */
	set(serial_port->lcrh, 0b001110000);
	//                       ┃┃┃┃┃┃┃┃┗━ don't send break
	//                       ┃┃┃┃┃┃┃┗━━ disable parity
	//                       ┃┃┃┃┃┃┗━━━  set parity type to odd(0) or even(1) (doesn't matter when disabled)
	//                       ┃┃┃┃┃┗━━━━ only send one stop bit
	//                       ┃┃┃┃┗━━━━━ enable FIFOs
	//                       ┃┃┗┻━━━━━━ 8 bits per word
	//                       ┃┗━━━━━━━━ disable stick parity (what is it?)
	//                       ┗━━━━━━━━━ reserved for this and all more significant bits (write 0)
	/* Enable the UART in loopback mode */
	//set(serial_port->cr, 0b01100001110000001);
	//                       ┃┃┃┃┃?┃┃┃┃┃┃┃┃┃┃┗━ enable the UART
	//                       ┃┃┃┃┃┃┃┃┃┃┃┃┃┃┗┻━━ unsupported (write 0)
	//                       ┃┃┃┃┃┃┃┃┃┃┗┻┻┻━━━━ reserved (write 0)
	//                       ┃┃┃┃┃┃┃┃┃┗━━━━━━━━ enable loop-back (for testing)
	//                       ┃┃┃┃┃┃┃┃┗━━━━━━━━━ enable transmitting
	//                       ┃┃┃┃┃┃┃┗━━━━━━━━━━ enable receiving
	//                       ┃┃┃┃┃┃┗━━━━━━━━━━━ unsupported (write 0)
	//                       ┃┃┃┃┃┗━━━━━━━━━━━━ RTS
	//                       ┃┃┃┃┗━━━━━━━━━━━━━ unsupported (write 0)
	//                       ┃┃┃┗━━━━━━━━━━━━━━ unsupported (write 0)
	//                       ┃┃┗━━━━━━━━━━━━━━━ receiving hardware flow control
	//                       ┃┗━━━━━━━━━━━━━━━━ transmitting hardware flow control
	//                       ┗━━━━━━━━━━━━━━━━━ reserved for this and all more significant bits (write 0)
	/* enable IRQs (not now) */
	/* Test with loop-back mode */
	//put_serial('*');
	/* if the byte returned is different */
	//if (read_serial() != '*') {
		/* turn off the serial port, return an error */
		//set(serial_port->cr, 0b0);
		//return 1;
	//}
	/* Enable the UART for normal operation */
	set(serial_port->cr, 0b01100001100000001);
	//                     ┃┃┃┃┃?┃┃┃┃┃┃┃┃┃┃┗━ enable the UART
	//                     ┃┃┃┃┃┃┃┃┃┃┃┃┃┃┗┻━━ unsupported (write 0)
	//                     ┃┃┃┃┃┃┃┃┃┃┗┻┻┻━━━━ reserved (write 0)
	//                     ┃┃┃┃┃┃┃┃┃┗━━━━━━━━ disable loop-back
	//                     ┃┃┃┃┃┃┃┃┗━━━━━━━━━ enable transmitting
	//                     ┃┃┃┃┃┃┃┗━━━━━━━━━━ enable receiving
	//                     ┃┃┃┃┃┃┗━━━━━━━━━━━ unsupported (write 0)
	//                     ┃┃┃┃┃┗━━━━━━━━━━━━ RTS
	//                     ┃┃┃┃┗━━━━━━━━━━━━━ unsupported (write 0)
	//                     ┃┃┃┗━━━━━━━━━━━━━━ unsupported (write 0)
	//                     ┃┃┗━━━━━━━━━━━━━━━ receiving hardware flow control
	//                     ┃┗━━━━━━━━━━━━━━━━ transmitting hardware flow control
	//                     ┗━━━━━━━━━━━━━━━━━ reserved for this and all more significant bits (write 0)
	return 0;
}

static bool is_receive_empty() {
	//return the RXFE (Received FIFO Empty)
	return is_set(serial_port->fr, 4);
}

char read_serial() {
	while (is_receive_empty());

	auto input = get(serial_port->dr);
	/* If there was an error */
	if ((input & 0b111100000000) != 0) {
		/* Return null (TODO: retry some amount of time before changing settings) */
		return '\0';
	}
	return input & 0xFF;
}

static bool is_transmit_full() {
	//return the TXFF (Sending FIFO Full)
	return is_set(serial_port->fr, 5);
}

void put_serial(char a) {
	while (is_transmit_full());

	set(serial_port->dr, a);
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
