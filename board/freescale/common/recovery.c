/*
 * Freescale Android Recovery mode checking routing
 *
 * Copyright (C) 2010 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */
#include <common.h>
#include <malloc.h>
#include "recovery.h"
#ifdef CONFIG_MXC_KPD
#include <mxc_keyb.h>
#elif defined(CONFIG_MX53_BEJ) || defined(CONFIG_MX53_BEJ2)
#include <asm/arch/iomux.h>
#include <asm/arch/mx53_pins.h>
char key_status_at_startup[6]="_____";
int start_mode=0;
#endif

extern int check_recovery_cmd_file(void);
extern enum boot_device get_boot_device(void);

#ifdef CONFIG_MXC_KPD

#define PRESSED_HOME	0x01
#define PRESSED_POWER	0x02
#define RECOVERY_KEY_MASK (PRESSED_HOME | PRESSED_POWER)

inline int test_key(int value, struct kpp_key_info *ki)
{
	return (ki->val == value) && (ki->evt == KDepress);
}

int check_key_pressing(void)
{
	struct kpp_key_info *key_info;
	int state = 0, keys, i;

	mxc_kpp_init();

	puts("Detecting HOME+POWER key for recovery ...\n");

	/* Check for home + power */
	keys = mxc_kpp_getc(&key_info);
	if (keys < 2)
		return 0;

	for (i = 0; i < keys; i++) {
		if (test_key(CONFIG_POWER_KEY, &key_info[i]))
			state |= PRESSED_HOME;
		else if (test_key(CONFIG_HOME_KEY, &key_info[i]))
			state |= PRESSED_POWER;
	}

	free(key_info);

	if ((state & RECOVERY_KEY_MASK) == RECOVERY_KEY_MASK)
		return 1;

	return 0;
}
#elif defined(CONFIG_MX53_BEJ)
int check_key_pressing(void)
{
	volatile unsigned long reg,key;

	mxc_request_iomux(MX53_PIN_ATA_DIOR,IOMUX_CONFIG_ALT1);   // [MENU]
	reg=readl(GPIO7_BASE_ADDR+0x4);
	reg&=~0x0008;
	writel(reg,GPIO7_BASE_ADDR+0x4);
	
	mxc_request_iomux(MX53_PIN_ATA_DATA12,IOMUX_CONFIG_ALT1); // [HOME]
	mxc_request_iomux(MX53_PIN_ATA_DATA13,IOMUX_CONFIG_ALT1); // [RETURN]
	mxc_request_iomux(MX53_PIN_ATA_DATA14,IOMUX_CONFIG_ALT1); // [VOL-]
	mxc_request_iomux(MX53_PIN_ATA_DATA15,IOMUX_CONFIG_ALT1); // [VOL+]
	reg=readl(GPIO2_BASE_ADDR+0x4);
	reg&=~0x0000f000;
	writel(reg,GPIO2_BASE_ADDR+0x4);
	
	key=(~readl(GPIO2_BASE_ADDR+0x0))&0x0000f000;
	key|=(~readl(GPIO7_BASE_ADDR+0x0))&0x00000008;
	udelay(1000000);
	reg=(~readl(GPIO2_BASE_ADDR+0x0))&0x0000f000;
	reg|=(~readl(GPIO7_BASE_ADDR+0x0))&0x00000008;
	reg&=key;
	strncpy(key_status_at_startup,"_____",sizeof(key_status_at_startup));
	start_mode=0;
	if(reg&0x0008){
		key_status_at_startup[0]='M';
		start_mode|=0x01;
		printf("[Menu] key is pressed.\n");
	}
	if(reg&0x1000){
		key_status_at_startup[1]='H';
		start_mode|=0x02;
		printf("[Home] key is pressed.\n");
	}
	if(reg&0x2000){
		key_status_at_startup[2]='R';
		start_mode|=0x04;
		printf("[Return] key is pressed.\n");
	}
	if(reg&0x4000){
		key_status_at_startup[3]='-';
		start_mode|=0x08;
		printf("[VOL-] key is pressed.\n");
	}
	if(reg&0x8000){
		key_status_at_startup[4]='+';
		start_mode|=0x10;
		printf("[VOL+] key is pressed.\n");
	}
	
	return reg;
}
#elif defined(CONFIG_MX53_BEJ2)
#define RECOVERY (1 << 11)
#define DEVELOP (1 << 9)

int check_key_pressing(void)
{
	volatile unsigned long reg,key;

	mxc_request_iomux(MX53_PIN_EIM_DA11,IOMUX_CONFIG_ALT1);   // RECOVERY
	reg = readl(GPIO3_BASE_ADDR + 0x4);
	reg &= ~RECOVERY;
	writel(reg,GPIO3_BASE_ADDR + 0x4);

	mxc_request_iomux(MX53_PIN_ATA_CS_0,IOMUX_CONFIG_ALT1); // DEVELOP_MODE
	reg = readl(GPIO7_BASE_ADDR + 0x4);
	reg &= ~DEVELOP;
	writel(reg,GPIO7_BASE_ADDR + 0x4);

	key = readl(GPIO3_BASE_ADDR+0x0) & RECOVERY;
	key |= readl(GPIO7_BASE_ADDR+0x0) & DEVELOP;
	udelay(1000000);
	reg = readl(GPIO3_BASE_ADDR+0x0) & RECOVERY;
	reg |= readl(GPIO7_BASE_ADDR+0x0) & DEVELOP;
	reg &= key;
	strncpy(key_status_at_startup,"_____",sizeof(key_status_at_startup));
	start_mode = 0;
	if(reg & RECOVERY){
		key_status_at_startup[0]='R';
		start_mode |= 0x01;
		printf("Recovery sw is pressed.\n");
	}
	if(reg & DEVELOP){
		key_status_at_startup[1]='D';
		start_mode |= 0x02;
		printf("Develop sw is pressed.\n");
	}

	return reg;
}
#else
/* If not using mxc keypad, currently we will detect power key on board */
int check_key_pressing(void)
{
	return 0;
}
#endif

extern struct reco_envs supported_reco_envs[];

void setup_recovery_env(void)
{
	char *env, *boot_args, *boot_cmd;
	int bootdev = get_boot_device();

	boot_cmd = supported_reco_envs[bootdev].cmd;
	boot_args = supported_reco_envs[bootdev].args;

	if (boot_cmd == NULL) {
		printf("Unsupported bootup device for recovery\n");
		return;
	}

	printf("setup env for recovery..\n");

	env = getenv("bootargs_android_recovery");
	/* Set env to recovery mode */
	/* Only set recovery env when these env not exist, give user a
	 * chance to change their recovery env */
	if (!env)
		setenv("bootargs_android_recovery", boot_args);

	env = getenv("bootcmd_android_recovery");
	if (!env)
		setenv("bootcmd_android_recovery", boot_cmd);
	setenv("bootcmd", "run bootcmd_android_recovery");
}

/* export to lib_arm/board.c */
void check_recovery_mode(void)
{
#if defined(CONFIG_MX53_BEJ)
	if (check_key_pressing()==0x00003008)
#elif defined(CONFIG_MX53_BEJ2)
	if (check_key_pressing()==0x00000a00)
#else
	if (check_key_pressing())
#endif
		setup_recovery_env();
	else if (check_recovery_cmd_file()) {
		puts("Recovery command file founded!\n");
		setup_recovery_env();
	}
}
