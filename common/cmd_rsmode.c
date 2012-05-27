/*
 * Copyright (C) 2012 Boundary Devices Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <common.h>
#include <asm/errno.h>
#include <asm/io.h>

#define mkval(cfg1, cfg2, cfg3, cfg4) \
	((cfg4) << 24) | ((cfg3) << 16) | ((cfg2) << 8) | (cfg1)

struct reset_mode {
	const char *name;
	unsigned cfg_val;
};

#ifdef CONFIG_MX53
/*
 * cfg_val will be used for
 * Boot_cfg3[7:0]:Boot_cfg2[7:0]:Boot_cfg1[7:0]
 *
 * If bit 28 of LPGR is set upon watchdog reset,
 * bits[25:0] of LPGR will move to SBMR.
 */
#define SBMR_COPY_ADDR &((struct srtc_regs *)SRTC_BASE_ADDR)->lpgr
const struct reset_mode modes[] = {
	{"normal",	mkval(0x00, 0x00, 0x00, 0x00)},
	{"usb",		mkval(0x00, 0x00, 0x00, 0x13)},	/* or serial download */
	{"sata",	mkval(0x28, 0x00, 0x00, 0x12)},
	{"escpi1:0",	mkval(0x38, 0x20, 0x00, 0x12)},
	{"escpi1:1",	mkval(0x38, 0x20, 0x04, 0x12)},
	{"escpi1:2",	mkval(0x38, 0x20, 0x08, 0x12)},
	{"escpi1:3",	mkval(0x38, 0x20, 0x0c, 0x12)},
	{"esdhc1",	mkval(0x40, 0x20, 0x00, 0x12)},	/* 4 bit bus width */
	{"esdhc2",	mkval(0x40, 0x20, 0x08, 0x12)},
	{"esdhc3",	mkval(0x40, 0x20, 0x10, 0x12)},
	{"esdhc4",	mkval(0x40, 0x20, 0x18, 0x12)},
};
#endif

#ifdef CONFIG_MX6Q
/*
 * cfg_val will be used for
 * Boot_cfg4[7:0]:Boot_cfg3[7:0]:Boot_cfg2[7:0]:Boot_cfg1[7:0]
 */
#define SBMR_COPY_ADDR &((struct src_regs *)SRC_BASE_ADDR)->gpr9
/*
 * After reset, if GPR10[28] is 1, ROM will copy GPR9[25:0]
 * to SBMR1, which will determine the boot device.
 */
#define SMBR_COPY_ENABLE_ADDR &((struct src_regs *)SRC_BASE_ADDR)->gpr10
const struct reset_mode modes[] = {
	{"normal",	mkval(0x00, 0x00, 0x00, 0x00)},
	/* reserved value should start rom usb */
	{"usb",		mkval(0x01, 0x00, 0x00, 0x00)},
	{"sata",	mkval(0x20, 0x00, 0x00, 0x00)},
	{"escpi1:0",	mkval(0x30, 0x00, 0x00, 0x08)},
	{"escpi1:1",	mkval(0x30, 0x00, 0x00, 0x18)},
	{"escpi1:2",	mkval(0x30, 0x00, 0x00, 0x28)},
	{"escpi1:3",	mkval(0x30, 0x00, 0x00, 0x38)},
	{"esdhc1",	mkval(0x40, 0x20, 0x00, 0x00)},	/* 4 bit bus width */
	{"esdhc2",	mkval(0x40, 0x28, 0x00, 0x00)},
	{"esdhc3",	mkval(0x40, 0x30, 0x00, 0x00)},
	{"esdhc4",	mkval(0x40, 0x38, 0x00, 0x00)},
};
#endif

int do_rsmode(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int i;
	if (argc < 2) {
options:
		printf("Options:\n");
		for (i = 0; i < ARRAY_SIZE(modes); i++)
			printf("%s\n", modes[i].name);
		return 0;
	}
	for (i = 0; i < ARRAY_SIZE(modes); i++) {
		if (!strcmp(modes[i].name, argv[1]))
			break;
	}
	if (i >= ARRAY_SIZE(modes))
		goto options;
	writel(modes[i].cfg_val, SBMR_COPY_ADDR);
#ifdef SMBR_COPY_ENABLE_ADDR
	{
		unsigned reg = readl(SMBR_COPY_ENABLE_ADDR);
		if (i)
			reg |= 1 << 28;
		else
			reg &= ~(1 << 28);
		writel(reg, SMBR_COPY_ENABLE_ADDR);
	}
#endif
	return 0;
}

U_BOOT_CMD(
	rsmode, 2, 0, do_rsmode,
	"change reset mode to normal/usb/sata/ecspi1:1/esdhc1",
	"");
