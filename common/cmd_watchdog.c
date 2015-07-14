/*
 * Command for watchdog
 *
 * Copyright (C) 2011 SANYO CE
 */
#include <common.h>
#include <asm/io.h> 

static int started=0;

void hw_watchdog_reset(void)
{
	if(started){
		writew(0x5555,WDOG1_BASE_ADDR+0x0002);
		writew(0xaaaa,WDOG1_BASE_ADDR+0x0002);
	}
}

static void hw_watchdog_stop(void)
{
	if(started){
		writew(0x0000,WDOG1_BASE_ADDR+0x0008);
		started=0;
	}
}

static void hw_watchdog_init(void)
{
	volatile unsigned short reg;

	if(started)
		return;
	hw_watchdog_stop();
	reg=readw(WDOG1_BASE_ADDR);
	reg|=(0xf6<<8);	/* 123 sec */
	reg|=0x30;	/* set WDA and SRS */
	reg&=~0x08;	/* reset in expiring */
	writew(reg,WDOG1_BASE_ADDR);
	udelay(50);
	reg=readw(WDOG1_BASE_ADDR);
	reg|=0x04;	/* enable watchdog */
	writew(reg,WDOG1_BASE_ADDR);
	started=1;
	hw_watchdog_reset();
}

static int do_watchdog(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	const char *cmd;

	/* need at least two arguments */
	if (argc < 2)
		goto usage;

	cmd = argv[1];

	if (strcmp(cmd, "start") == 0) {
		hw_watchdog_init();
		return 0;
	}

	if (strcmp(cmd, "kick") == 0) {
		hw_watchdog_reset();		
		return 0;
	}

usage:
	cmd_usage(cmdtp);
	return 1;
}

U_BOOT_CMD(
	watchdog,	2,	1,	do_watchdog,
	"control watchdog",
	"start - start watchdog timer\n"
	"watchdog kick - kick watchdog");

