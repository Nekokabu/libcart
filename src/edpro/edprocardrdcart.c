#include <cart.h>
#include "cartint.h"
#include "edpro.h"

int edpro_card_rd_cart(u32 cart, u32 lba, u32 count)
{
	int result;
	__cart_acs_get();
	if (cart_card_byteswap)
	{
		if (__edpro_set_flags(
			EDPRO_DEV_FLAG_WRSW, EDPRO_DEV_FLAG_WRSW
		)) CART_ABORT();
	}
	__edpro_xfer(
		EDPRO_EPO_TYPE_SDC, lba,
		EDPRO_EPO_TYPE_FCI, cart & 0x3FFFFFF, 512*count
	);
	result = __edpro_status();
	if (cart_card_byteswap)
	{
		if (__edpro_set_flags(EDPRO_DEV_FLAG_WRSW, 0)) CART_ABORT();
	}
	if (result) CART_ABORT();
	__cart_acs_rel();
	return 0;
}
