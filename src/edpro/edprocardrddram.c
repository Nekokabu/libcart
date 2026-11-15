#include <cart.h>
#include "cartint.h"
#include "edpro.h"

extern int __edpro_card_rd_fifo(void *dram, u32 lba, u32 count);

int edpro_card_rd_dram(void *dram, u32 lba, u32 count)
{
	char *addr = dram;
	int i;
	int n;
	__cart_acs_get();
	if (!__edpro_sst)
	{
		if (__edpro_card_rd_fifo(dram, lba, count)) CART_ABORT();
		__cart_acs_rel();
		return 0;
	}
	while (count > 0)
	{
		n = count < EDPRO_SST_SECTORS ? count : EDPRO_SST_SECTORS;
		__edpro_xfer(
			EDPRO_EPO_TYPE_SDC, lba,
			EDPRO_EPO_TYPE_FCI, EDPRO_SST_FCI_ADDR, 512*n
		);
		if (__edpro_status()) CART_ABORT();
		if ((long)addr & 7)
		{
			for (i = 0; i < n; i++)
			{
				__cart_dma_rd(
					__cart_buf, EDPRO_SST_PI_ADDR+512*i, 512
				);
				__cart_buf_wr(addr);
				addr += 512;
			}
		}
		else
		{
			__cart_dma_rd(addr, EDPRO_SST_PI_ADDR, 512*n);
			addr += 512*n;
		}
		lba += n;
		count -= n;
	}
	__cart_acs_rel();
	return 0;
}
