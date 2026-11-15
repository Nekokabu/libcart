#include <cart.h>
#include "cartint.h"
#include "edpro.h"

/* SD card -> RDRAM through the MCU FIFO (1 KiB acknowledged blocks) */
int __edpro_card_rd_fifo(void *dram, u32 lba, u32 count)
{
	unsigned char *addr = dram;
	unsigned char ack = 0;
	u32 len = 512*count;
	u32 n;
	__edpro_xfer(EDPRO_EPO_TYPE_SDC, lba, EDPRO_EPO_TYPE_LINK_ACK, 0, len);
	while (len > 0)
	{
		n = len < EDPRO_ACK_BLOCK ? len : EDPRO_ACK_BLOCK;
		__edpro_fifo_wr(&ack, 1);
		if (__edpro_fifo_rd(addr, n)) return -1;
		addr += n;
		len -= n;
	}
	return __edpro_status();
}

/* RDRAM -> SD card through the MCU FIFO (1 KiB acknowledged blocks) */
int __edpro_card_wr_fifo(const void *dram, u32 lba, u32 count)
{
	const unsigned char *addr = dram;
	unsigned char ack;
	u32 len = 512*count;
	u32 n;
	__edpro_wr_enable();
	__edpro_xfer(EDPRO_EPO_TYPE_LINK_ACK, 0, EDPRO_EPO_TYPE_SDC, lba, len);
	while (len > 0)
	{
		n = len < EDPRO_ACK_BLOCK ? len : EDPRO_ACK_BLOCK;
		if (__edpro_fifo_rd(&ack, 1)) return -1;
		__edpro_fifo_wr(addr, n);
		addr += n;
		len -= n;
	}
	return __edpro_status();
}
