#include <cart.h>
#include "cartint.h"
#include "edpro.h"

int edpro_init(void)
{
	__cart_acs_get();
	if (__cart_rd(EDPRO_EDID_REG) != EDPRO_EDID) CART_ABORT();
	if (
		(__cart_rd(EDPRO_SYSSTAT_REG) & EDPRO_SYSSTAT_CMSK) !=
			EDPRO_SYSSTAT_CVAL
	) CART_ABORT();
	/* Discard stale FIFO data */
	__edpro_fifo_flush();
	/* Enable ROM write, disable byteswap */
	if (__edpro_set_flags(
		EDPRO_DEV_FLAG_WREN|EDPRO_DEV_FLAG_WRSW, EDPRO_DEV_FLAG_WREN
	)) CART_ABORT();
	cart_size = 0x4000000; /* 64 MiB */
	__cart_acs_rel();
	return 0;
}
