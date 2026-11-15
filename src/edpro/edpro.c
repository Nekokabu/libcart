#include <cart.h>
#include "cartint.h"
#include "edpro.h"

/* Use the save-state buffer for transfers between RDRAM and the card */
int __edpro_sst;

/*
 * Buffer for FIFO DMA.  The FPGA holds the register address during a PI
 * burst, so a DMA must not make the PI issue a new cartridge address: keep
 * the buffer 512-byte aligned and each transfer at most 512 bytes.  Every
 * 32-bit word carries one FIFO byte in its low 8 bits.
 */
#ifdef __GNUC__
__attribute__((aligned(512)))
#endif
static u32 __edpro_buf[128];

static u32 __edpro_time(void)
{
	return __cart_rd(EDPRO_TIMER_REG);
}

int __edpro_fifo_rd(void *dst, u32 len)
{
	unsigned char *ptr = dst;
	u32 time = __edpro_time();
	u32 i;
	u32 n;
	while (len > 0)
	{
		n = __cart_rd(EDPRO_FIFOSTAT_REG) & 0xFFFF;
		if (n == 0)
		{
			if (__edpro_time() - time > EDPRO_TIMEOUT) return -1;
			continue;
		}
		if (n > len) n = len;
		if (n > 128) n = 128;
		if (n == 1)
		{
			*ptr++ = __cart_rd(EDPRO_FIFODATA_REG);
		}
		else
		{
			__cart_dma_rd(__edpro_buf, EDPRO_FIFODATA_REG, 4*n);
			for (i = 0; i < n; i++) *ptr++ = __edpro_buf[i];
		}
		len -= n;
		time = __edpro_time();
	}
	return 0;
}

void __edpro_fifo_wr(const void *src, u32 len)
{
	const unsigned char *ptr = src;
	u32 i;
	u32 n;
	while (len > 0)
	{
		n = len < 128 ? len : 128;
		if (n <= 8)
		{
			for (i = 0; i < n; i++)
			{
				__cart_wr(EDPRO_FIFODATA_REG, *ptr++);
			}
		}
		else
		{
			for (i = 0; i < n; i++) __edpro_buf[i] = *ptr++;
			__cart_dma_wr(__edpro_buf, EDPRO_FIFODATA_REG, 4*n);
		}
		len -= n;
	}
}

void __edpro_fifo_flush(void)
{
	unsigned char buf[128];
	u32 n;
	int i;
	for (i = 0; i < 64; i++)
	{
		n = __cart_rd(EDPRO_FIFOSTAT_REG) & 0xFFFF;
		if (n == 0) break;
		if (n > sizeof(buf)) n = sizeof(buf);
		if (__edpro_fifo_rd(buf, n)) break;
	}
}

static u32 __edpro_sysstat(void)
{
	u32 a, b;
	do
	{
		a = __cart_rd(EDPRO_SYSSTAT_REG) & ~EDPRO_SYSSTAT_STROBE;
		b = __cart_rd(EDPRO_SYSSTAT_REG) & ~EDPRO_SYSSTAT_STROBE;
	}
	while (a != b);
	return a;
}

int __edpro_wait_mcu(void)
{
	u32 time = __edpro_time();
	while (__edpro_sysstat() & EDPRO_SYSSTAT_BUSY)
	{
		if (__edpro_time() - time > EDPRO_TIMEOUT) return -1;
	}
	return 0;
}

int __edpro_status(void)
{
	unsigned char buf[4];
	buf[0] = '+';
	buf[1] = '+' ^ 0xFF;
	buf[2] = EDPRO_CMD_STATUS;
	buf[3] = EDPRO_CMD_STATUS ^ 0xFF;
	__edpro_fifo_wr(buf, 4);
	if (__edpro_fifo_rd(buf, 4)) return -1;
	if (buf[0] != EDPRO_STATUS_KEY) return -1;
	return buf[3] ? -1 : 0;
}

static void __edpro_put32(unsigned char *buf, u32 val)
{
	buf[0] = val >> 24;
	buf[1] = val >> 16;
	buf[2] = val >>  8;
	buf[3] = val >>  0;
}

static void __edpro_scmd_buf(unsigned char *buf, int cmd, int scmd)
{
	buf[0] = '+';
	buf[1] = '+' ^ 0xFF;
	buf[2] = cmd;
	buf[3] = cmd ^ 0xFF;
	buf[4] = scmd;
}

void __edpro_scmd(int cmd, int scmd)
{
	unsigned char buf[5];
	__edpro_scmd_buf(buf, cmd, scmd);
	__edpro_fifo_wr(buf, 5);
}

int __edpro_set_flags(u32 mask, u32 val)
{
	unsigned char buf[9];
	__edpro_scmd_buf(buf, EDPRO_CMD_DEV, EDPRO_DEV_SCMD_SET_FLAGS);
	__edpro_put32(&buf[5], (mask & 0xFFFF) << 16 | (val & 0xFFFF));
	__edpro_fifo_wr(buf, 9);
	return __edpro_wait_mcu();
}

/* One-time write enable, required before every SD card write */
void __edpro_wr_enable(void)
{
	unsigned char buf[9];
	__edpro_scmd_buf(buf, EDPRO_CMD_EPO, EDPRO_EPO_SCMD_WRE);
	__edpro_put32(&buf[5], EDPRO_EPO_WRKEY_SDC);
	__edpro_fifo_wr(buf, 9);
}

void __edpro_xfer(int src_type, u32 src, int dst_type, u32 dst, u32 len)
{
	/* SCMD + EpoXfer (big-endian, 16 bytes) + run byte */
	unsigned char buf[5+16+1];
	__edpro_scmd_buf(buf, EDPRO_CMD_EPO, EDPRO_EPO_SCMD_XFER);
	__edpro_put32(&buf[ 5], src);
	__edpro_put32(&buf[ 9], dst);
	__edpro_put32(&buf[13], len);
	buf[17] = src_type;
	buf[18] = dst_type;
	buf[19] = 0;
	buf[20] = 0;
	buf[21] = 0;
	__edpro_fifo_wr(buf, sizeof(buf));
}
