/*
 * linux/arch/arm/mach-s5pv210/setup-keypad.c
 *
 * Copyright (C) 2010 Samsung Electronics Co.Ltd
 * Author: Joonyoung Shim <jy0922.shim@samsung.com>
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 */

#include <linux/gpio.h>
#include <plat/gpio-cfg.h>

void samsung_keypad_cfg_gpio(unsigned int rows, unsigned int cols)
{
#if 0
	/* Set all the necessary GPH3 pins to special-function 3: KP_ROW[x] */
	s3c_gpio_cfgrange_nopull(S5PV210_GPH3(0), rows, S3C_GPIO_SFN(3));

	/* Set all the necessary GPH2 pins to special-function 3: KP_COL[x] */
	s3c_gpio_cfgrange_nopull(S5PV210_GPH2(0), cols, S3C_GPIO_SFN(3));
#else
	unsigned int gpio;  
	unsigned int end; 

	s3c_gpio_cfgpin(S5PV210_GPJ2(7), S3C_GPIO_SFN(3));  
        s3c_gpio_setpull(S5PV210_GPJ2(7), S3C_GPIO_PULL_UP); 

	end = S5PV210_GPJ3(rows -1);
	for (gpio = S5PV210_GPJ3(0); gpio < end; gpio++)
	{  
		s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(3));  
		s3c_gpio_setpull(gpio, S3C_GPIO_PULL_UP);
	}


	
	s3c_gpio_cfgpin(S5PV210_GPJ1(5), S3C_GPIO_SFN(3));  
        s3c_gpio_setpull(S5PV210_GPJ1(5), S3C_GPIO_PULL_NONE); 

	end = S5PV210_GPJ2(cols -1);
	for (gpio = S5PV210_GPJ2(0); gpio < end; gpio++)
	{  
		s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(3));  
		s3c_gpio_setpull(gpio, S3C_GPIO_PULL_NONE);
	}
#endif
}
