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
 * Date & Time support for DA9052 RTC
 */

#include <common.h>
#include <command.h>
#include <rtc.h>
#if CONFIG_I2C_MXC
#include <i2c.h>
#endif

//#define DEBUG

#if defined(CONFIG_CMD_DATE)

#define DA9052_COUNT_S_REG	111
#define DA9052_COUNT_MI_REG	112
#define DA9052_COUNT_H_REG	113
#define DA9052_COUNT_D_REG	114
#define DA9052_COUNT_MO_REG	115
#define DA9052_COUNT_Y_REG	116

/* ------------------------------------------------------------------------- */

int rtc_get_da9052 (struct rtc_time *tmp)
{
	uchar sec, min, hour, mday, mon, year;
	int retry = 1;
	uchar sec2;

	do {
		i2c_read(0x48, DA9052_COUNT_S_REG, 1, &sec, 1);
		i2c_read(0x48, DA9052_COUNT_MI_REG,1, &min, 1);
		i2c_read(0x48, DA9052_COUNT_H_REG, 1, &hour,1);
		i2c_read(0x48, DA9052_COUNT_D_REG, 1, &mday,1);
		i2c_read(0x48, DA9052_COUNT_MO_REG,1, &mon, 1);
		i2c_read(0x48, DA9052_COUNT_Y_REG, 1, &year,1);

		/*
		 * Check for seconds rollover
		 */
		i2c_read(0x48, DA9052_COUNT_S_REG, 1, &sec2, 1);
		sec &= 0x3f;
		sec2 &= 0x3f;
		if ((sec != 59) || (sec2 == sec)){
			retry = 0;
		}
	} while (retry);

	tmp->tm_sec  = sec;
	tmp->tm_min  = min & 0x3f;
	tmp->tm_hour = hour & 0x1f;
	tmp->tm_mday = mday & 0x1f;
	tmp->tm_mon  = (mon & 0x0f) - 1;
	tmp->tm_year = (year & 0x3f) + 2000;
	tmp->tm_wday = 0;
	tmp->tm_yday = 0;
	tmp->tm_isdst= 0;

#ifdef DEBUG
	printf( "Get DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
		tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
		tmp->tm_hour, tmp->tm_min, tmp->tm_sec);
#endif

	return 0;
}

int rtc_set_da9052 (struct rtc_time *tmp)
{
	uchar sec, min, hour, mday, mon, year;

#ifdef DEBUG
	printf( "Set DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
		tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
		tmp->tm_hour, tmp->tm_min, tmp->tm_sec);
#endif

	sec = tmp->tm_sec;
	min = tmp->tm_min;
	hour = tmp->tm_hour;
	mday = tmp->tm_mday;
	mon = tmp->tm_mon + 1;
	year = tmp->tm_year - 100;

	i2c_write(0x48, DA9052_COUNT_Y_REG, 1, &year,1);
	i2c_write(0x48, DA9052_COUNT_MO_REG,1, &mon, 1);
	i2c_write(0x48, DA9052_COUNT_D_REG, 1, &mday,1);
	i2c_write(0x48, DA9052_COUNT_H_REG, 1, &hour,1);
	i2c_write(0x48, DA9052_COUNT_MI_REG,1, &min, 1);
	i2c_write(0x48, DA9052_COUNT_S_REG, 1, &sec, 1);

	return 0;
}

void rtc_reset_da9052 (void)
{
}

#endif
