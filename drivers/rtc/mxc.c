/*
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

/*
 * Date & Time support for iMX53 RTC
 */

#include <common.h>
#include <asm/io.h>
#include <command.h>
#include <rtc.h>

#if defined(CONFIG_CMD_DATE)
#define SRTC_BASE_ADDR	(AIPS1_BASE_ADDR + 0x000A4000)
#define SRTC_LPSCMR		0x00	/* LP Secure Counter MSB Reg */

/* ------------------------------------------------------------------------- */

int rtc_get (struct rtc_time *tmp)
{
	to_tm(readl(SRTC_BASE_ADDR + SRTC_LPSCMR), tmp);

#ifdef DEBUG
	printf( "Get DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
		tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
		tmp->tm_hour, tmp->tm_min, tmp->tm_sec);
#endif

	return 0;
}

int rtc_set (struct rtc_time *tmp)
{
	unsigned long tim;

	if (tmp == NULL) {
		return -1;
	}

#ifdef DEBUG
	printf( "Set DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
		tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
		tmp->tm_hour, tmp->tm_min, tmp->tm_sec);
#endif

	/* Calculate number of seconds this incoming time represents */
	tim = mktime(tmp->tm_year, tmp->tm_mon + 1, tmp->tm_mday,
	                tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

	writel(tim, SRTC_BASE_ADDR + SRTC_LPSCMR);

	return 0;
}

void rtc_reset (void)
{
}

#endif
