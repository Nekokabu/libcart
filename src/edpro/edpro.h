#ifndef __EDPRO_H__
#define __EDPRO_H__

/* EverDrive-64 PRO */
/* Reference: krikzz/ed64-pro-pub (MANUAL.md, edio/everdrive.c) */

#define EDPRO_BASE_REG          0x1F800000

#define EDPRO_FIFODATA_REG      (EDPRO_BASE_REG+0x00)
#define EDPRO_FIFOSTAT_REG      (EDPRO_BASE_REG+0x04)
#define EDPRO_SYSSTAT_REG       (EDPRO_BASE_REG+0x08)
#define EDPRO_TIMER_REG         (EDPRO_BASE_REG+0x0C)
#define EDPRO_MBX_REG           (EDPRO_BASE_REG+0x10)
#define EDPRO_EDID_REG          (EDPRO_BASE_REG+0x14)
#define EDPRO_MCTR_REG          (EDPRO_BASE_REG+0x18)

/* Save-state buffer, visible via PI and FCI */
#define EDPRO_SST_PI_ADDR       0x1F000000
#define EDPRO_SST_FCI_ADDR      0x0F000000

#define EDPRO_EDID              0xED640027

#define EDPRO_SYSSTAT_BUSY      0x01
#define EDPRO_SYSSTAT_STROBE    0x08
#define EDPRO_SYSSTAT_CMSK      0xF0
#define EDPRO_SYSSTAT_CVAL      0xA0

#define EDPRO_STATUS_KEY        0x5A

#define EDPRO_CMD_STATUS        0x10
#define EDPRO_CMD_FS            0x80
#define EDPRO_CMD_EPO           0x81
#define EDPRO_CMD_DEV           0x85

#define EDPRO_FS_SCMD_INIT      0x10
#define EDPRO_EPO_SCMD_XFER     0x10
#define EDPRO_EPO_SCMD_WRE      0x11
#define EDPRO_DEV_SCMD_SET_FLAGS 0x28

#define EDPRO_EPO_TYPE_LINK     0x10
#define EDPRO_EPO_TYPE_LINK_ACK 0x11
#define EDPRO_EPO_TYPE_FCI      0x13
#define EDPRO_EPO_TYPE_SDC      0x19

#define EDPRO_EPO_WRKEY_SDC     0xDEADBEEF

#define EDPRO_DEV_FLAG_WRSW     0x01    /* byteswap on SD -> ROM writes */
#define EDPRO_DEV_FLAG_WREN     0x02    /* ROM write enable */

#define EDPRO_ACK_BLOCK         1024
/* Sectors per transfer through the save-state buffer */
#define EDPRO_SST_SECTORS       128

/* Timeout for MCU responses (ms, EDIO timer) */
#define EDPRO_TIMEOUT           30000

extern int __edpro_sst;

extern int __edpro_fifo_rd(void *dst, u32 len);
extern void __edpro_fifo_wr(const void *src, u32 len);
extern void __edpro_fifo_flush(void);
extern int __edpro_wait_mcu(void);
extern int __edpro_status(void);
extern void __edpro_scmd(int cmd, int scmd);
extern int __edpro_set_flags(u32 mask, u32 val);
extern void __edpro_wr_enable(void);
extern void __edpro_xfer(int src_type, u32 src, int dst_type, u32 dst, u32 len);

#endif /* __EDPRO_H__ */
