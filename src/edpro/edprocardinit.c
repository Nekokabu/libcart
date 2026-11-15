#include <cart.h>
#include "cartint.h"
#include "edpro.h"

extern int __edpro_card_rd_fifo(void *dram, u32 lba, u32 count);

#define EDPRO_TEST0               0x5AA5C33C
#define EDPRO_TEST1               0x0FF0E11E

/* Check that the save-state buffer is coherent between PI and FCI */
static int __edpro_sst_test(void)
{
	unsigned char buf[8];
	int i;
	/* PI -> FCI */
	__cart_wr(EDPRO_SST_PI_ADDR+0, EDPRO_TEST0);
	__cart_wr(EDPRO_SST_PI_ADDR+4, EDPRO_TEST1);
	__edpro_xfer(
		EDPRO_EPO_TYPE_FCI, EDPRO_SST_FCI_ADDR,
		EDPRO_EPO_TYPE_LINK, 0, 8
	);
	if (__edpro_fifo_rd(buf, 8)) return -1;
	if (
		(u32)(buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3]) !=
			EDPRO_TEST0 ||
		(u32)(buf[4] << 24 | buf[5] << 16 | buf[6] << 8 | buf[7]) !=
			EDPRO_TEST1
	) return -1;
	/* FCI -> PI: LBA 0 read through the FIFO and through the buffer */
	__edpro_xfer(
		EDPRO_EPO_TYPE_SDC, 0,
		EDPRO_EPO_TYPE_FCI, EDPRO_SST_FCI_ADDR, 512
	);
	if (__edpro_status()) return -1;
	if (__edpro_card_rd_fifo(__cart_buf, 0, 1)) return -1;
	for (i = 0; i < 512/8; i++)
	{
		u64 x =
			(u64)__cart_rd(EDPRO_SST_PI_ADDR+8*i+0) << 32 |
			(u64)__cart_rd(EDPRO_SST_PI_ADDR+8*i+4);
		if (x != __cart_buf[i]) return -1;
	}
	return 0;
}

int edpro_card_init(void)
{
	__cart_acs_get();
	__edpro_sst = 0;
	/* Check card access; reinitialize SD/filesystem once on failure */
	if (__edpro_card_rd_fifo(__cart_buf, 0, 1))
	{
		__edpro_fifo_flush();
		__edpro_scmd(EDPRO_CMD_FS, EDPRO_FS_SCMD_INIT);
		if (__edpro_status()) CART_ABORT();
		if (__edpro_card_rd_fifo(__cart_buf, 0, 1)) CART_ABORT();
	}
	/* Use the faster save-state buffer path if it works */
	if (!__edpro_sst_test()) __edpro_sst = 1;
	else __edpro_fifo_flush();
	__cart_acs_rel();
	return 0;
}
