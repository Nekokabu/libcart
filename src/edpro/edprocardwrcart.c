#include <cart.h>
#include "cartint.h"
#include "edpro.h"

int edpro_card_wr_cart(u32 cart, u32 lba, u32 count)
{
	__cart_acs_get();
	__edpro_wr_enable();
	__edpro_xfer(
		EDPRO_EPO_TYPE_FCI, cart & 0x3FFFFFF,
		EDPRO_EPO_TYPE_SDC, lba, 512*count
	);
	if (__edpro_status()) CART_ABORT();
	__cart_acs_rel();
	return 0;
}
