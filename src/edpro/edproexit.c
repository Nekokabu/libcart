#include <cart.h>
#include "cartint.h"
#include "edpro.h"

int edpro_exit(void)
{
	__cart_acs_get();
	if (__edpro_set_flags(
		EDPRO_DEV_FLAG_WREN|EDPRO_DEV_FLAG_WRSW, 0
	)) CART_ABORT();
	__cart_acs_rel();
	return 0;
}
