/* linux/arch/arm/mach-s5pv210/mach-smdkv210.c
 *
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/i2c-gpio.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/fixed.h>
#include <linux/regulator/machine.h>
#include <linux/mfd/max8698.h>
#include <mach/power-domain.h>
#include <linux/init.h>
#include <linux/serial_core.h>
#include <linux/sysdev.h>
#include <linux/dm9000.h>
#include <linux/fb.h>
#include <linux/gpio.h>
#include <linux/videodev2.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/pwm_backlight.h>
#include <linux/usb/ch9.h>
#include <linux/spi/spi.h>
#include <linux/gpio_keys.h>
#include <linux/gpio_event.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/setup.h>
#include <asm/mach-types.h>

#include <video/platform_lcd.h>

#include <mach/map.h>
#include <mach/regs-clock.h>
#include <mach/spi-clocks.h>
#include <mach/regs-fb.h>

#ifdef CONFIG_VIDEO_S5K4BA
#include <media/s5k4ba_platform.h>
#undef	CAM_ITU_CH_A
#define	CAM_ITU_CH_B
#endif
#include <plat/regs-serial.h>
#include <plat/regs-srom.h>
#include <plat/gpio-cfg.h>
#include <plat/s3c64xx-spi.h>
#include <plat/s5pv210.h>
#include <plat/devs.h>
#include <plat/cpu.h>
#include <plat/adc.h>
#include <plat/ts.h>
#include <plat/ata.h>
#include <plat/iic.h>
#include <plat/keypad.h>
#include <plat/pm.h>
#include <plat/fb.h>
#include <plat/mfc.h>
#include <plat/s5p-time.h>
#include <plat/sdhci.h>
#include <plat/fimc.h>
#include <plat/csis.h>
#include <plat/jpeg.h>
#include <plat/clock.h> 
#include <plat/regs-otg.h>
#include <plat/otg.h>
#include <plat/ehci.h>
#include <plat/ohci.h>
#include <../../../drivers/video/samsung/s3cfb.h>
#include <mach/regs-gpio.h>
#include <mach/gpio.h>
#ifdef CONFIG_ANDROID_PMEM
#include <linux/android_pmem.h>
#endif
#include <plat/media.h>
#include <mach/media.h>
#include <mach/gpio-smdkc110.h>

#include <media/ov3640_platform.h>
#include <media/tvp5150.h>


#ifdef CONFIG_TOUCHSCREEN_EGALAX
#include <linux/i2c/egalax.h>
#define EETI_TS_DEV_NAME        "egalax_i2c"

static struct egalax_i2c_platform_data  egalax_platdata  = {
        .gpio_int = EGALAX_IRQ,
        .gpio_en = NULL,
        .gpio_rst = NULL,
};
#endif

/* Following are default values for UCON, ULCON and UFCON UART registers */
#define SMDKV210_UCON_DEFAULT	(S3C2410_UCON_TXILEVEL |	\
				 S3C2410_UCON_RXILEVEL |	\
				 S3C2410_UCON_TXIRQMODE |	\
				 S3C2410_UCON_RXIRQMODE |	\
				 S3C2410_UCON_RXFIFO_TOI |	\
				 S3C2443_UCON_RXERR_IRQEN)

#define SMDKV210_ULCON_DEFAULT	S3C2410_LCON_CS8

#define SMDKV210_UFCON_DEFAULT	(S3C2410_UFCON_FIFOMODE |	\
				 S5PV210_UFCON_TXTRIG4 |	\
				 S5PV210_UFCON_RXTRIG4)

static struct s3c2410_uartcfg smdkv210_uartcfgs[] __initdata = {
	[0] = {
		.hwport		= 0,
		.flags		= 0,
		.ucon		= SMDKV210_UCON_DEFAULT,
		.ulcon		= SMDKV210_ULCON_DEFAULT,
		.ufcon		= SMDKV210_UFCON_DEFAULT,
	},
	[1] = {
		.hwport		= 1,
		.flags		= 0,
		.ucon		= SMDKV210_UCON_DEFAULT,
		.ulcon		= SMDKV210_ULCON_DEFAULT,
		.ufcon		= SMDKV210_UFCON_DEFAULT,
	},
	[2] = {
		.hwport		= 2,
		.flags		= 0,
		.ucon		= SMDKV210_UCON_DEFAULT,
		.ulcon		= SMDKV210_ULCON_DEFAULT,
		.ufcon		= SMDKV210_UFCON_DEFAULT,
	},
	[3] = {
		.hwport		= 3,
		.flags		= 0,
		.ucon		= SMDKV210_UCON_DEFAULT,
		.ulcon		= SMDKV210_ULCON_DEFAULT,
		.ufcon		= SMDKV210_UFCON_DEFAULT,
	},
};

#if defined(CONFIG_REGULATOR_MAX8698)
/* LDO */
static struct regulator_consumer_supply smdkv210_ldo3_consumer[] = {
	REGULATOR_SUPPLY("pd_io", "s3c-usbgadget"),
	REGULATOR_SUPPLY("pd_io", "s5p-ohci"),
	REGULATOR_SUPPLY("pd_io", "s5p-ehci"),
};

static struct regulator_consumer_supply smdkv210_ldo5_consumer[] = {
	REGULATOR_SUPPLY("AVDD", "0-001b"),
	REGULATOR_SUPPLY("DVDD", "0-001b"),
};

static struct regulator_consumer_supply smdkv210_ldo8_consumer[] = {
	REGULATOR_SUPPLY("pd_core", "s3c-usbgadget"),
	REGULATOR_SUPPLY("pd_core", "s5p-ohci"),
	REGULATOR_SUPPLY("pd_core", "s5p-ehci"),
};

static struct regulator_init_data smdkv210_ldo2_data = {
	.constraints	= {
		.name		= "VALIVE_1.1V",
		.min_uV		= 1100000,
		.max_uV		= 1100000,
		.apply_uV	= 1,
		.always_on	= 1,
		.state_mem	= {
			.enabled = 1,
		},
	},
};

static struct regulator_init_data smdkv210_ldo3_data = {
	.constraints	= {
		.name		= "VUOTG_D+VUHOST_D_1.1V",
		.min_uV		= 1100000,
		.max_uV		= 1100000,
		.apply_uV	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.disabled = 1,
		},
	},
	.num_consumer_supplies	= ARRAY_SIZE(smdkv210_ldo3_consumer),
	.consumer_supplies	= smdkv210_ldo3_consumer,
};

static struct regulator_init_data smdkv210_ldo4_data = {
	.constraints	= {
		.name		= "V_MIPI_1.8V",
		.min_uV		= 1800000,
		.max_uV		= 1800000,
		.apply_uV	= 1,
		.always_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.disabled = 1,
		},
	},
};

static struct regulator_init_data smdkv210_ldo5_data = {
	.constraints	= {
		.name		= "VMMC+VEXT_2.8V",
		.min_uV		= 2800000,
		.max_uV		= 2800000,
		.apply_uV	= 1,
		.always_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.enabled = 1,
		},
	},
	.num_consumer_supplies	= ARRAY_SIZE(smdkv210_ldo5_consumer),
	.consumer_supplies	= smdkv210_ldo5_consumer,
};

static struct regulator_init_data smdkv210_ldo6_data = {
	.constraints	= {
		.name		= "VCC_2.6V",
		.min_uV		= 2600000,
		.max_uV		= 2600000,
		.apply_uV	= 1,
		.always_on	= 1,
		.valid_ops_mask	= REGULATOR_CHANGE_STATUS,
		.state_mem	 = {
			.disabled = 1,
		},
	},
};

static struct regulator_init_data smdkv210_ldo7_data = {
	.constraints	= {
		.name		= "VDAC_2.8V",
		.min_uV		= 2800000,
		.max_uV		= 2800000,
		.apply_uV	= 1,
		.valid_ops_mask	= REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.enabled = 1,
		},
	},
};

static struct regulator_init_data smdkv210_ldo8_data = {
	.constraints	= {
		.name		= "VUOTG_A+VUHOST_A_3.3V",
		.min_uV		= 3300000,
		.max_uV		= 3300000,
		.apply_uV	= 1,
		.valid_ops_mask	= REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.disabled = 1,
		},
	},
	.num_consumer_supplies	= ARRAY_SIZE(smdkv210_ldo8_consumer),
	.consumer_supplies	= smdkv210_ldo8_consumer,
};

static struct regulator_init_data smdkv210_ldo9_data = {
	.constraints	= {
		.name		= "VADC+VSYS+VKEY_2.8V",
		.min_uV		= 2800000,
		.max_uV		= 2800000,
		.apply_uV	= 1,
		.always_on	= 1,
		.state_mem	= {
			.enabled = 1,
		},
	},
};

/* BUCK */
static struct regulator_consumer_supply smdkv210_buck1_consumer =
	REGULATOR_SUPPLY("vddarm", NULL);

static struct regulator_consumer_supply smdkv210_buck2_consumer =
	REGULATOR_SUPPLY("vddint", NULL);

static struct regulator_init_data smdkv210_buck1_data = {
	.constraints	= {
		.name		= "VCC_ARM",
		.min_uV		= 750000,
		.max_uV		= 1500000,
		.apply_uV	= 1,
		.valid_ops_mask	= REGULATOR_CHANGE_VOLTAGE |
				  REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.uV	= 1250000,
			.mode	= REGULATOR_MODE_NORMAL,
			.disabled = 1,
		},
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &smdkv210_buck1_consumer,
};

static struct regulator_init_data smdkv210_buck2_data = {
	.constraints	= {
		.name		= "VCC_INTERNAL",
		.min_uV		= 950000,
		.max_uV		= 1200000,
		.valid_ops_mask	= REGULATOR_CHANGE_VOLTAGE |
				  REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.uV	= 1100000,
			.mode	= REGULATOR_MODE_NORMAL,
			.disabled = 1,
		},
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &smdkv210_buck2_consumer,
};

static struct regulator_init_data smdkv210_buck3_data = {
	.constraints	= {
		.name		= "VCC_MEM",
		.min_uV		= 1800000,
		.max_uV		= 1800000,
		.always_on	= 1,
		.apply_uV	= 1,
		.state_mem	= {
			.uV	= 1800000,
			.mode	= REGULATOR_MODE_NORMAL,
			.enabled = 1,
		},
	},
};

static struct max8698_regulator_data smdkv210_regulators[] = {
	{ MAX8698_LDO2,  &smdkv210_ldo2_data },
	{ MAX8698_LDO3,  &smdkv210_ldo3_data },
	{ MAX8698_LDO4,  &smdkv210_ldo4_data },
	{ MAX8698_LDO5,  &smdkv210_ldo5_data },
	{ MAX8698_LDO6,  &smdkv210_ldo6_data },
	{ MAX8698_LDO7,  &smdkv210_ldo7_data },
	{ MAX8698_LDO8,  &smdkv210_ldo8_data },
	{ MAX8698_LDO9,  &smdkv210_ldo9_data },
	{ MAX8698_BUCK1, &smdkv210_buck1_data },
	{ MAX8698_BUCK2, &smdkv210_buck2_data },
	{ MAX8698_BUCK3, &smdkv210_buck3_data },
};

static struct max8698_platform_data smdkv210_max8698_pdata = {
	.num_regulators = ARRAY_SIZE(smdkv210_regulators),
	.regulators     = smdkv210_regulators,

	/* 1GHz default voltage */
	.dvsarm1        = 0xa,  /* 1.25v */
	.dvsarm2        = 0x9,  /* 1.20V */
	.dvsarm3        = 0x6,  /* 1.05V */
	.dvsarm4        = 0x4,  /* 0.95V */
	.dvsint1        = 0x7,  /* 1.10v */
	.dvsint2        = 0x5,  /* 1.00V */

	.set1       = S5PV210_GPH1(6),
	.set2       = S5PV210_GPH1(7),
	.set3       = S5PV210_GPH0(4),
};
#endif


static struct s3c_ide_platdata smdkv210_ide_pdata __initdata = {
	.setup_gpio	= s5pv210_ide_setup_gpio,
};

#ifdef CONFIG_KEYBOARD_SAMSUNG
static uint32_t smdkv210_keymap[] __initdata = {
	/* KEY(row, col, keycode) */
	KEY(0, 3, KEY_1), KEY(0, 4, KEY_2), KEY(0, 5, KEY_3),
	KEY(0, 6, KEY_4), KEY(0, 7, KEY_5),
	KEY(1, 3, KEY_A), KEY(1, 4, KEY_B), KEY(1, 5, KEY_C),
	KEY(1, 6, KEY_D), KEY(1, 7, KEY_E), KEY(7, 1, KEY_LEFTBRACE)
};

static struct matrix_keymap_data smdkv210_keymap_data __initdata = {
	.keymap		= smdkv210_keymap,
	.keymap_size	= ARRAY_SIZE(smdkv210_keymap),
};

static struct samsung_keypad_platdata smdkv210_keypad_data __initdata = {
	.keymap_data	= &smdkv210_keymap_data,
	.rows		= 8,
	.cols		= 8,
};
#endif

#define S5PV210_PA_DM9000	(0x88000300)

static struct resource smdkv210_dm9000_resources[] = {
	[0] = {
		.start	= S5PV210_PA_DM9000,
		.end	= S5PV210_PA_DM9000,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= S5PV210_PA_DM9000 + 4,
		.end	= S5PV210_PA_DM9000 + 4,
		.flags	= IORESOURCE_MEM,
	},
	[2] = {
		.start	= IRQ_EINT(9),
		.end	= IRQ_EINT(9),
		.flags	= IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHLEVEL,
	},
};

static struct dm9000_plat_data smdkv210_dm9000_platdata = {
	.flags		= DM9000_PLATF_16BITONLY | DM9000_PLATF_NO_EEPROM,
	.dev_addr	= { 0x00, 0x09, 0xc0, 0xff, 0xec, 0x48 },
};

struct platform_device smdkv210_dm9000 = {
	.name		= "dm9000",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(smdkv210_dm9000_resources),
	.resource	= smdkv210_dm9000_resources,
	.dev		= {
		.platform_data	= &smdkv210_dm9000_platdata,
	},
};

/* GPIO KEYS */
#ifdef CONFIG_INPUT_GPIO
static struct gpio_event_direct_entry smdkv210_keypad_key_map[] = {
	{
		.code			= KEY_SEND,
		.gpio			= S5PV210_GPH0(0),	/* XEINT16 */
	}, {
		.code			= KEY_MENU,
		.gpio			= S5PV210_GPH0(1),	/* XEINT17 */
	}, {
		.code			= KEY_BACK,
		.gpio			= S5PV210_GPH0(3),	/* XEINT18 */
	}, {
		.code			= KEY_END,
		.gpio			= S5PV210_GPH0(4),	/* XEINT19 */
	}, {
		.code			= KEY_HOME,
		.gpio			= S5PV210_GPH0(5),	/* XEINT29 */
	},	
};

static struct gpio_event_input_info smdkv210_keypad_key_info = {
	.info.func = gpio_event_input_func,
	.info.no_suspend = true,
	.debounce_time.tv64 = 5 * NSEC_PER_MSEC,
	.type = EV_KEY,
	.keymap = smdkv210_keypad_key_map,
	.keymap_size = ARRAY_SIZE(smdkv210_keypad_key_map)
};

static struct gpio_event_info *smdkv210_input_info[] = {
	&smdkv210_keypad_key_info.info,
};


static struct gpio_event_platform_data smdkv210_input_data = {
	.names = {
		"gpio-keys",
		NULL,
	},
	.info = smdkv210_input_info,
	.info_count = ARRAY_SIZE(smdkv210_input_info),
};

static struct platform_device smdkv210_input_device = {
	.name = GPIO_EVENT_DEV_NAME,
	.id = 0,
	.dev = {
		.platform_data = &smdkv210_input_data,
	},
};
#endif

#ifdef CONFIG_REGULATOR
static struct regulator_consumer_supply smdkv210_b_pwr_5v_consumers[] = {
        {
                /* WM8580 */
                .supply         = "PVDD",
                .dev_name       = "0-001b",
        },
};

static struct regulator_init_data smdkv210_b_pwr_5v_data = {
        .constraints = {
                .always_on = 1,
        },
        .num_consumer_supplies  = ARRAY_SIZE(smdkv210_b_pwr_5v_consumers),
        .consumer_supplies      = smdkv210_b_pwr_5v_consumers,
};

static struct fixed_voltage_config smdkv210_b_pwr_5v_pdata = {
        .supply_name    = "B_PWR_5V",
        .microvolts     = 5000000,
        .init_data      = &smdkv210_b_pwr_5v_data,
};

static struct platform_device smdkv210_b_pwr_5v = {
        .name          = "reg-fixed-voltage",
        .id            = -1,
        .dev = {
                .platform_data = &smdkv210_b_pwr_5v_pdata,
        },
};
#endif

#ifdef CONFIG_TOUCHSCREEN_EGALAX
static struct i2c_gpio_platform_data i2c5_platdata = {
        .sda_pin                = S5PV210_GPB(6),
        .scl_pin                = S5PV210_GPB(7),
        .udelay                 = 2,
        .sda_is_open_drain      = 0,
        .scl_is_open_drain      = 0,
        .scl_is_output_only     = 0.
};

//static struct platform_device   s3c_device_i2c5 = {
struct platform_device   s3c_device_i2c5 = {
        .name                   = "i2c-gpio",
        .id                     = 5,
        .dev.platform_data      = &i2c5_platdata,
};

static struct i2c_board_info i2c_devs5[] __initdata = {
        {
                I2C_BOARD_INFO(EETI_TS_DEV_NAME, 0x04),
                .platform_data = &egalax_platdata,
                .irq = IRQ_EINT6,
        },
};
#endif


#ifdef CONFIG_FB_S3C_LTE480WV
#define S5PV210_LCD_WIDTH 1024
#define S5PV210_LCD_HEIGHT 600
#define NUM_BUFFER 4
#endif

#ifdef CONFIG_FB_S3C_101WA01S
//#define S5PV210_LCD_WIDTH 1366
#define S5PV210_LCD_WIDTH 1024
#define S5PV210_LCD_HEIGHT 768
#endif

#ifdef CONFIG_FB_S3C_TL2796
#define S5PV210_LCD_WIDTH 1024 //1280
#define S5PV210_LCD_HEIGHT 768 //800
#define NUM_BUFFER 8
#endif

#ifdef CONFIG_CH7026_800_600
#define S5PV210_LCD_WIDTH 800
#define S5PV210_LCD_HEIGHT 600
#define NUM_BUFFER 4
#endif

#ifdef CONFIG_CH7034_1024_768
#define S5PV210_LCD_WIDTH 1024
#define S5PV210_LCD_HEIGHT 768
#define NUM_BUFFER 4
#endif

#define  S5PV210_VIDEO_SAMSUNG_MEMSIZE_FIMC0 (6144 * SZ_1K)
#define  S5PV210_VIDEO_SAMSUNG_MEMSIZE_FIMC1 (9900 * SZ_1K)
#define  S5PV210_VIDEO_SAMSUNG_MEMSIZE_FIMC2 (6144 * SZ_1K)
#define  S5PV210_VIDEO_SAMSUNG_MEMSIZE_MFC0 (36864 * SZ_1K)
#define  S5PV210_VIDEO_SAMSUNG_MEMSIZE_MFC1 (36864 * SZ_1K)
#define  S5PV210_VIDEO_SAMSUNG_MEMSIZE_FIMD (S5PV210_LCD_WIDTH * \
                                             S5PV210_LCD_HEIGHT * NUM_BUFFER * \
                                             (CONFIG_FB_S3C_NR_BUFFERS + \
                                                 (CONFIG_FB_S3C_NUM_OVLY_WIN * \
                                                  CONFIG_FB_S3C_NUM_BUF_OVLY_WIN))) //sayanta macro values need to be set
#define  S5PV210_VIDEO_SAMSUNG_MEMSIZE_JPEG (8192 * SZ_1K)
#define  S5PV210_ANDROID_PMEM_MEMSIZE_PMEM_GPU1 (1800 * SZ_1K)
#define  S5PV210_VIDEO_SAMSUNG_MEMSIZE_G2D (8192 * SZ_1K)

static struct s5p_media_device s5pv210_media_devs[] = {
        [0] = {
                .id = S5P_MDEV_MFC,
                .name = "mfc",
                .bank = 0,
                .memsize = S5PV210_VIDEO_SAMSUNG_MEMSIZE_MFC0,
                .paddr = 0,
        },
        [1] = {
                .id = S5P_MDEV_MFC,
                .name = "mfc",
                .bank = 1,
                .memsize = S5PV210_VIDEO_SAMSUNG_MEMSIZE_MFC1,
                .paddr = 0,
        },
        [2] = {
                .id = S5P_MDEV_FIMC0,
                .name = "fimc0",
                .bank = 1,
                .memsize = S5PV210_VIDEO_SAMSUNG_MEMSIZE_FIMC0,
                .paddr = 0,
        },
        [3] = {
                .id = S5P_MDEV_FIMC1,
                .name = "fimc1",
                .bank = 1,
                .memsize = S5PV210_VIDEO_SAMSUNG_MEMSIZE_FIMC1,
                .paddr = 0,
        },
        [4] = {
                .id = S5P_MDEV_FIMC2,
                .name = "fimc2",
                .bank = 1,
                .memsize = S5PV210_VIDEO_SAMSUNG_MEMSIZE_FIMC2,
                .paddr = 0,
        },
        [5] = {
                .id = S5P_MDEV_JPEG,
                .name = "jpeg",
                .bank = 0,
                .memsize = S5PV210_VIDEO_SAMSUNG_MEMSIZE_JPEG,
                .paddr = 0,
        },
	[6] = {
                .id = S5P_MDEV_FIMD,
                .name = "fimd",
                .bank = 1,
                .memsize = S5PV210_VIDEO_SAMSUNG_MEMSIZE_FIMD,
                .paddr = 0,
        },
	[7] = {
                .id = S5P_MDEV_PMEM_GPU1,
                .name = "pmem_gpu1",
                .bank = 0, /* OneDRAM */
                .memsize = S5PV210_ANDROID_PMEM_MEMSIZE_PMEM_GPU1,
                .paddr = 0,
        },
	[8] = {
		.id = S5P_MDEV_G2D,
		.name = "g2d",
		.bank = 0,
		.memsize = S5PV210_VIDEO_SAMSUNG_MEMSIZE_G2D,
		.paddr = 0,
	},
};

#ifdef CONFIG_ANDROID_PMEM
static struct android_pmem_platform_data pmem_pdata = {
        .name = "pmem",
        .no_allocator = 1,
        .cached = 1,
        .start = 0,
        .size = 0,
};

static struct android_pmem_platform_data pmem_gpu1_pdata = {
        .name = "pmem_gpu1",
        .no_allocator = 1,
        .cached = 1,
        .buffered = 1,
        .start = 0,
        .size = 0,
};
 
static struct android_pmem_platform_data pmem_adsp_pdata = {
        .name = "pmem_adsp",
        .no_allocator = 1,
        .cached = 1,
        .buffered = 1,
        .start = 0,
        .size = 0,
};      

static struct platform_device pmem_device = {
        .name = "android_pmem",
        .id = 0,
        .dev = { .platform_data = &pmem_pdata },
};

static struct platform_device pmem_gpu1_device = {
        .name = "android_pmem",
        .id = 1,
        .dev = { .platform_data = &pmem_gpu1_pdata },
};

static struct platform_device pmem_adsp_device = {
        .name = "android_pmem",
        .id = 2,
        .dev = { .platform_data = &pmem_adsp_pdata },
};

static void __init android_pmem_set_platdata(void)
{
        pmem_pdata.start = (u32)s5p_get_media_memory_bank(S5P_MDEV_PMEM, 0);
        pmem_pdata.size = (u32)s5p_get_media_memsize_bank(S5P_MDEV_PMEM, 0);

        pmem_gpu1_pdata.start =
                (u32)s5p_get_media_memory_bank(S5P_MDEV_PMEM_GPU1, 0);
        pmem_gpu1_pdata.size =
                (u32)s5p_get_media_memsize_bank(S5P_MDEV_PMEM_GPU1, 0);

        pmem_adsp_pdata.start =
                (u32)s5p_get_media_memory_bank(S5P_MDEV_PMEM_ADSP, 0);
        pmem_adsp_pdata.size =
                (u32)s5p_get_media_memsize_bank(S5P_MDEV_PMEM_ADSP, 0);
}
#endif

#ifdef CONFIG_FB_S3C_LTE480WV
static void smdkv210_lte480wv_set_power(struct plat_lcd_data *pd,
					unsigned int power)
{
	if (power) {
#if !defined(CONFIG_BACKLIGHT_PWM)
		gpio_request(S5PV210_GPD0(0), "GPD0");
		gpio_direction_output(S5PV210_GPD0(0), 1);
		gpio_free(S5PV210_GPD0(0));
#endif

	} else {
#if !defined(CONFIG_BACKLIGHT_PWM)
		gpio_request(S5PV210_GPD0(0), "GPD0");
		gpio_direction_output(S5PV210_GPD0(0), 0);
		gpio_free(S5PV210_GPD0(0));
#endif
	}
}

static struct plat_lcd_data smdkv210_lcd_lte480wv_data = {
	.set_power	= smdkv210_lte480wv_set_power,
};

static struct platform_device smdkv210_lcd_lte480wv = {
	.name			= "platform-lcd",
	.dev.parent		= &s3c_device_fb.dev,
	.dev.platform_data	= &smdkv210_lcd_lte480wv_data,
};

static struct s3cfb_lcd lte480wv = {
	.width	= S5PV210_LCD_WIDTH,
	.height	= S5PV210_LCD_HEIGHT,
	.bpp	= 24,
	.freq	= 40,

	.timing = {
		.h_fp	= 210,
		.h_bp	= 16,
		.h_sw	= 30,
		.v_fp	= 22,
		.v_fpe	= 1,
		.v_bp	= 10,
		.v_bpe	= 1,
		.v_sw	= 13,
	},

	.polarity = {
		.rise_vclk	= 0,
		.inv_hsync	= 1,
		.inv_vsync	= 1,
		.inv_vden	= 0,
	},
};

static void lte480wv_cfg_gpio(struct platform_device *pdev)
{
        int i;

        for (i = 0; i < 8; i++) {
                s3c_gpio_cfgpin(S5PV210_GPF0(i), S3C_GPIO_SFN(2));
                s3c_gpio_setpull(S5PV210_GPF0(i), S3C_GPIO_PULL_NONE);
        }

        for (i = 0; i < 8; i++) {
                s3c_gpio_cfgpin(S5PV210_GPF1(i), S3C_GPIO_SFN(2));
                s3c_gpio_setpull(S5PV210_GPF1(i), S3C_GPIO_PULL_NONE);
        }

        for (i = 0; i < 8; i++) {
                s3c_gpio_cfgpin(S5PV210_GPF2(i), S3C_GPIO_SFN(2));
                s3c_gpio_setpull(S5PV210_GPF2(i), S3C_GPIO_PULL_NONE);
        }
        for (i = 0; i < 4; i++) {
                s3c_gpio_cfgpin(S5PV210_GPF3(i), S3C_GPIO_SFN(2));
                s3c_gpio_setpull(S5PV210_GPF3(i), S3C_GPIO_PULL_NONE);
        }

        /* mDNIe SEL: why we shall write 0x2 ? */
        writel(0x2, S5P_MDNIE_SEL);

        writel(0xC0, S5PV210_GPF0_BASE + 0xc);
}



#define S5PV210_GPD_0_0_TOUT_0  (0x2)
static int lte480wv_backlight_on(struct platform_device *pdev)
{
 	int err;

	err = gpio_request(S5PV210_GPD0(0), "GPD0");

	if (err) {
		printk(KERN_ERR "failed to request GPD0 for "
			"lcd backlight control\n");
		return err;
	}

	gpio_direction_output(S5PV210_GPD0(0), 1);

	s3c_gpio_cfgpin(S5PV210_GPD0(0), S5PV210_GPD_0_0_TOUT_0);

	gpio_free(S5PV210_GPD0(0));

	return 0;       
}


static int lte480wv_backlight_off(struct platform_device *pdev, int onoff)
{
    int err;
	err = gpio_request(S5PV210_GPD0(0), "GPD0");
	if (err) {
		printk(KERN_ERR "failed to request GPD0 for "
				"lcd backlight control\n");
		return err;
	}

	gpio_direction_output(S5PV210_GPD0(0), 0);

	gpio_free(S5PV210_GPD0(0));
	
	return 0;
}


static int lte480wv_reset_lcd(struct platform_device *pdev)
{
        return 0;
}

static struct s3c_platform_fb lte480wv_fb_data __initdata = {
        .hw_ver = 0x62,
        .clk_name       = "sclk_fimd",
        .nr_wins = 5,
        .default_win = CONFIG_FB_S3C_DEFAULT_WINDOW,
        .swap = FB_SWAP_WORD | FB_SWAP_HWORD,

        .lcd = &lte480wv,
        .cfg_gpio       = lte480wv_cfg_gpio,
        .backlight_on   = lte480wv_backlight_on,
        .backlight_onoff    = lte480wv_backlight_off,
        .reset_lcd      = lte480wv_reset_lcd,
};
#endif

#if defined(CONFIG_CH7026_800_600) || defined(CONFIG_CH7034_1024_768)
static struct s3cfb_lcd vga = {
	.width	= S5PV210_LCD_WIDTH,
	.height	= S5PV210_LCD_HEIGHT,
	.bpp	= 24,
	.freq	= 60,
	
	.timing = {
		.h_fp	= 10,
		.h_bp	= 20,
		.h_sw	= 10,
		.v_fp	= 10,
		.v_fpe	= 1,
		.v_bp	= 20,
		.v_bpe	= 1,
		.v_sw	= 10,
	},

	.polarity = {
		.rise_vclk	= 0,
		.inv_hsync	= 1,
		.inv_vsync	= 1,
		.inv_vden	= 0,
	},
};

static void vga_cfg_gpio(struct platform_device *pdev)
{
	int i;

	for (i = 0; i < 8; i++) {
		s3c_gpio_cfgpin(S5PV210_GPF0(i), S3C_GPIO_SFN(2));
		s3c_gpio_setpull(S5PV210_GPF0(i), S3C_GPIO_PULL_NONE);
	}

	for (i = 0; i < 8; i++) {
		s3c_gpio_cfgpin(S5PV210_GPF1(i), S3C_GPIO_SFN(2));
		s3c_gpio_setpull(S5PV210_GPF1(i), S3C_GPIO_PULL_NONE);
	}

	for (i = 0; i < 8; i++) {
		s3c_gpio_cfgpin(S5PV210_GPF2(i), S3C_GPIO_SFN(2));
		s3c_gpio_setpull(S5PV210_GPF2(i), S3C_GPIO_PULL_NONE);
	}

	for (i = 0; i < 4; i++) {
		s3c_gpio_cfgpin(S5PV210_GPF3(i), S3C_GPIO_SFN(2));
		s3c_gpio_setpull(S5PV210_GPF3(i), S3C_GPIO_PULL_NONE);
	}

	/* mDNIe SEL: why we shall write 0x2 ? */
	writel(0x2, S5P_MDNIE_SEL);

	/* drive strength to max */
	writel(0xffffffff, S5PV210_GPF0_BASE + 0xc);
	writel(0xffffffff, S5PV210_GPF1_BASE + 0xc);
	writel(0xffffffff, S5PV210_GPF2_BASE + 0xc);
	writel(0x000000ff, S5PV210_GPF3_BASE + 0xc);
}


static int vga_backlight_on(struct platform_device *pdev)
{
	return 0;
}

static int vga_backlight_off(struct platform_device *pdev, int onoff)
{
	return 0;
}

static int vga_reset_lcd(struct platform_device *pdev)
{
	return 0;
}

static struct s3c_platform_fb vga_fb_data __initdata = {
	.hw_ver	= 0x62,
    .clk_name       = "sclk_fimd",
	.nr_wins = 5,
	.default_win = CONFIG_FB_S3C_DEFAULT_WINDOW,
	.swap = FB_SWAP_WORD | FB_SWAP_HWORD,

	.lcd = &vga,
	.cfg_gpio	= vga_cfg_gpio,
	.backlight_on	= vga_backlight_on,
	.backlight_onoff    = vga_backlight_off,
	.reset_lcd	= vga_reset_lcd,
};

static void smdkv210_vga_set_power(struct plat_lcd_data *pd,
					unsigned int power)
{

}

static struct plat_lcd_data smdkv210_lcd_vga_data = {
	.set_power	= smdkv210_vga_set_power,
};

static struct platform_device smdkv210_lcd_vga = {
	.name			= "platform-lcd",
	.dev.parent		= &s3c_device_fb.dev,
	.dev.platform_data	= &smdkv210_lcd_vga_data,
};
#endif

static int smdkv210_backlight_init(struct device *dev)
{
	return 0;
}

static void smdkv210_backlight_exit(struct device *dev)
{
}

static struct platform_pwm_backlight_data smdkv210_backlight_data = {
	.pwm_id		= 0,
	.max_brightness	= 255,
	.dft_brightness	= 155,
	.pwm_period_ns	= 78770,
	.init		= smdkv210_backlight_init,
	.exit		= smdkv210_backlight_exit,
};

static struct platform_device smdkv210_backlight_device = {
	.name		= "pwm-backlight",
	.dev		= {
		.parent		= &s3c_device_timer[0].dev,
		.platform_data	= &smdkv210_backlight_data,
	},
};

struct s3c_adc_mach_info {
        /* if you need to use some platform data, add in here*/
        int delay;
        int presc;
        int resolution;
};

static struct platform_device *smdkv210_devices[] __initdata = {
	&s3c_device_adc,
	&s3c_device_cfcon,
	&s3c_device_fb,
	&s3c_device_hsmmc0,
//	&s3c_device_hsmmc1,
	&s3c_device_hsmmc2,
	&s3c_device_hsmmc3,


	&s3c_device_i2c0,
	&s3c_device_i2c1,
	&s3c_device_i2c2,
#ifdef CONFIG_TOUCHSCREEN_EGALAX
	&s3c_device_i2c5,
#endif
	&s3c_device_rtc,
	&s3c_device_ts,
	&s3c_device_wdt,
#ifdef CONFIG_SND_SAMSUNG_AC97
	&s5pv210_device_ac97,
#endif
#ifdef CONFIG_SND_SAMSUNG_I2S
	&s5pv210_device_iis0,
#endif
	&s5pv210_device_spdif,
#ifdef CONFIG_SND_SAMSUNG_PCM
#ifdef CONFIG_SND_SAMSUNG_PCM_USE_I2S1_MCLK
	&s5pv210_device_pcm0,
#endif
#endif 	/*end of CONFIG_SND_SAMSUNG_PCM*/
	&samsung_asoc_dma,
	&samsung_device_keypad,
	&smdkv210_dm9000,
	
#ifdef CONFIG_FB_S3C_LTE480WV
	&smdkv210_lcd_lte480wv,
#endif

#if defined(CONFIG_CH7026_800_600) || defined(CONFIG_CH7034_1024_768)
	&smdkv210_lcd_vga,
#endif

	&s3c_device_timer[0],
	&smdkv210_backlight_device,
	&s5p_device_ehci,
	&s5p_device_ohci,
#ifdef CONFIG_USB_GADGET
	&s3c_device_usbgadget,
#endif
#ifdef CONFIG_VIDEO_FIMC
        &s3c_device_fimc0,
        &s3c_device_fimc1,
        &s3c_device_fimc2,
#endif
#ifdef CONFIG_VIDEO_FIMC_MIPI
	&s3c_device_csis,
#endif
#ifdef CONFIG_VIDEO_JPEG_V2
	&s3c_device_jpeg,
#endif
#ifdef CONFIG_VIDEO_MFC50
        &s3c_device_mfc,
#endif
#ifdef CONFIG_ANDROID_PMEM
	&pmem_gpu1_device,
#endif
#ifdef CONFIG_SPI_S3C64XX
	&s5pv210_device_spi0,
	&s5pv210_device_spi1,
#endif
#ifdef CONFIG_REGULATOR
        &smdkv210_b_pwr_5v,
#endif
#ifdef CONFIG_S5PV210_POWER_DOMAIN
        &s5pv210_pd_tv,
        &s5pv210_pd_lcd,
        &s5pv210_pd_g3d,
        &s5pv210_pd_mfc,
        &s5pv210_pd_audio,
#endif
	&s3c_device_g3d,
#ifdef CONFIG_VIDEO_G2D
        &s3c_device_g2d,
#endif
#ifdef CONFIG_VIDEO_TV20
        &s5p_device_tvout,
        &s5p_device_cec,
        &s5p_device_hpd,
#endif

#ifdef CONFIG_MTD_NAND
	&s3c_device_nand,
#endif

#ifdef CONFIG_INPUT_GPIO
	&smdkv210_input_device,
#endif
};

#if defined(CONFIG_CAMERA_TVP5150_ONBOARD) || defined(CONFIG_CAMERA_TVP5150_MODULE)

/*
 * External camera reset
 * Because the most of cameras take i2c bus signal, so that
 * you have to reset at the boot time for other i2c slave devices.
 * This function also called at fimc_init_camera()
 * Do optimization for cameras on your platform.
*/
static int tvp5150_power(int onoff)//low_reset GPE1_4
{
	int err;
	if(onoff)
	{
		err = gpio_request(S5PV210_GPH2(1), "GPH2_1");
		if (err)
			printk(KERN_ERR "#### failed to request GPH2_1 \n");

		s3c_gpio_cfgpin(S5PV210_GPH2(1), S3C_GPIO_OUTPUT);
		s3c_gpio_setpull(S5PV210_GPH2(1), S3C_GPIO_PULL_UP);
		gpio_direction_output(S5PV210_GPH2(1), 1);	
		gpio_free(S5PV210_GPH2(1));		
	
		err = gpio_request(S5PV210_GPE1(4), "GPE1_4");
		if (err)
			printk(KERN_ERR "#### failed to request GPE1_4 for RESET\n");	

		s3c_gpio_setpull(S5PV210_GPE1(4), S3C_GPIO_PULL_NONE);
		gpio_direction_output(S5PV210_GPE1(4), 0);
		mdelay(1);
		gpio_direction_output(S5PV210_GPE1(4), 1);
		gpio_free(S5PV210_GPE1(4));
	}
	else
	{
		err = gpio_request(S5PV210_GPH2(1), "GPH2_1");
		if (err)
			printk(KERN_ERR "#### failed to request GPH2_1 \n");

		s3c_gpio_cfgpin(S5PV210_GPH2(1), S3C_GPIO_OUTPUT);
		s3c_gpio_setpull(S5PV210_GPH2(1), S3C_GPIO_PULL_UP);
		gpio_direction_output(S5PV210_GPH2(1), 0);	
		gpio_free(S5PV210_GPH2(1));		
	}

	return 0;
}

static struct tvp5150_platform_data tvp5150_plat =
{
	.default_width = 720,
	.default_height = 288,
	.pixelformat = V4L2_PIX_FMT_VYUY,
	.freq = 27000000,
	.is_mipi = 0,
};

static struct i2c_board_info  tvp5150_i2c_info = 
{
#ifdef CONFIG_CAMERA_TVP5150_ONBOARD
	I2C_BOARD_INFO("TVP5150", 0xBA>>1),
#endif
#ifdef CONFIG_CAMERA_TVP5150_MODULE
	I2C_BOARD_INFO("TVP5150", 0xB8>>1),
#endif
	.platform_data = &tvp5150_plat,
};

static struct s3c_platform_camera tvp5150 = {
	.id		= CAMERA_PAR_A,	
	.type		= CAM_TYPE_ITU,
	.fmt		= ITU_601_YCBCR422_8BIT,
	.order422	= CAM_ORDER422_8BIT_CBYCRY,
	.i2c_busnum	= 0,
	.info		= &tvp5150_i2c_info,
	.pixelformat	= V4L2_PIX_FMT_VYUY,
	.srclk_name	= "mout_mpll",
	.clk_name	= "sclk_cam0",
	.clk_rate	= 27000000,
	.line_length	= 720,
	.width		= 720,
	.height		= 288,
	.window		= {
		.left	= 0,
		.top	= 17,
		.width	= 720,
		.height	= 288,
	},

	/* Polarity */
	.inv_pclk	= 1,
	.inv_vsync	= 0,
	.inv_href	= 0,
	.inv_hsync	= 0,

	.initialized	= 0,

	.cam_power	= tvp5150_power,

};
#endif

#ifdef CONFIG_CAMERA_OV3640

static int smdkv210_OV3640_power(int onoff)
{
	int err;

	err = gpio_request(S5PV210_GPH2(1), "GPH2_1");
	if (err)
		printk(KERN_ERR "#### failed to request GPH2_1 \n");

	s3c_gpio_cfgpin(S5PV210_GPH2(1), S3C_GPIO_OUTPUT);
	s3c_gpio_setpull(S5PV210_GPH2(1), S3C_GPIO_PULL_UP);
	gpio_direction_output(S5PV210_GPH2(1), 0);	
	gpio_free(S5PV210_GPH2(1));		

	err = gpio_request(S5PV210_GPE1(4), "GPE1_4");
	if (err)
		printk(KERN_ERR "#### failed to request GPE1_4 for RESET\n");	

	s3c_gpio_setpull(S5PV210_GPE1(4), S3C_GPIO_PULL_NONE);
	gpio_direction_output(S5PV210_GPE1(4), 0);
	if(onoff)
	{
		mdelay(1);
		gpio_direction_output(S5PV210_GPE1(4), 1);
	}

	gpio_free(S5PV210_GPE1(4));

	return 0;
}

static struct ov3640_platform_data ov3640_plat =
{
	.default_width = 640,
	.default_height = 480,
	.pixelformat = V4L2_PIX_FMT_VYUY,
	.freq = 24000000,
	.is_mipi = 0,
};

static struct i2c_board_info  ov3640_i2c_info = 
{
	I2C_BOARD_INFO("OV3640", 0x78>>1),
	.platform_data = &ov3640_plat,
};

static struct s3c_platform_camera ov3640 = {
	.id		= CAMERA_PAR_A,
	.type		= CAM_TYPE_ITU,
	.fmt		= ITU_601_YCBCR422_8BIT,
	.order422	= CAM_ORDER422_8BIT_YCBYCR,
	.i2c_busnum	= 0,
	.info		= &ov3640_i2c_info,
	.pixelformat	= V4L2_PIX_FMT_VYUY,
	.srclk_name	= "mout_mpll",
	.clk_name	= "sclk_cam0",
	.clk_rate	= 24000000,
	.line_length	= 480,//640,
	.width		= 640,
	.height		= 480,
	.window		= {
		.left	= 0,
		.top	= 0,
		.width	= 640,
		.height	= 480,
	},

	/* Polarity */
	.inv_pclk	= 0,
	.inv_vsync	= 0,
	.inv_href	= 0,
	.inv_hsync	= 0,

	.initialized	= 0,

	.cam_power	= smdkv210_OV3640_power,

};
#endif



/* Interface setting */
static struct s3c_platform_fimc fimc_plat_lsi = {
	.srclk_name	= "mout_mpll",
	.clk_name	= "sclk_fimc",
	.lclk_name	= "fimc",//"sclk_fimc_lclk",
	.clk_rate	= 166750000,
	.default_cam	= CAMERA_PAR_A,
	.camera		= {
#ifdef CONFIG_CAMERA_OV3640
		&ov3640,
#endif
#if defined(CONFIG_CAMERA_TVP5150_ONBOARD) || defined(CONFIG_CAMERA_TVP5150_MODULE)
		&tvp5150,
#endif
	},
	.hw_ver		= 0x43,
};


#ifdef CONFIG_VIDEO_JPEG_V2
static struct s3c_platform_jpeg jpeg_plat __initdata = {
	.max_main_width	= 800,
	.max_main_height	= 480,
	.max_thumb_width	= 320,
	.max_thumb_height	= 240,
};
#endif

static void __init smdkv210_dm9000_init(void)
{
/*
	unsigned int tmp;

	gpio_request(S5PV210_MP01(1), "nCS1");
	s3c_gpio_cfgpin(S5PV210_MP01(1), S3C_GPIO_SFN(2));
	gpio_free(S5PV210_MP01(1));

	tmp = (5 << S5P_SROM_BCX__TACC__SHIFT);
	__raw_writel(tmp, S5P_SROM_BC1);

	tmp = __raw_readl(S5P_SROM_BW);
	tmp &= (S5P_SROM_BW__CS_MASK << S5P_SROM_BW__NCS1__SHIFT);
	tmp |= (1 << S5P_SROM_BW__NCS1__SHIFT);
	__raw_writel(tmp, S5P_SROM_BW);*/
	
	unsigned int tmp;

	tmp = ((0<<28)|(3<<24)|(7<<16)|(1<<12)|(3<<8)|(6<<4)|(0<<0));
	__raw_writel(tmp, (S5P_SROM_BW+0x08));//Bc1
	
	tmp = __raw_readl(S5P_SROM_BW);
	tmp &= ~(0xf << 4);
	

	tmp |= (0x1 << 4);
	tmp |= (0x2 << 4);

	__raw_writel(tmp, S5P_SROM_BW);

	tmp = __raw_readl(S5PV210_MP01CON);
	tmp &= ~(0xf << 4);
	tmp |= (2 << 4);

	__raw_writel(tmp, S5PV210_MP01CON);
}

#if defined(CONFIG_MPU_SENSORS_MPU3050) || defined(CONFIG_MPU_SENSORS_MPU3050_MODULE)
#include <linux/mpu.h>
#define SENSOR_MPU_NAME "mpu3050"
//#define MPUGPIO (EXYNOS4_GPX3(3))

static struct mpu3050_platform_data mpu_data = {
	.int_config  = 0x10,
#ifdef CONFIG_TC4_PORTRAIT_MODE
	.orientation = {  0,  1,  0, 
		1,  0,  0, 
		0,  0, -1 },
#else
	.orientation = {  -1,  0,  0, 
		0,  1,  0, 
		0,  0, -1 },
#endif
	/* accel */
	.accel = {
#ifdef CONFIG_MPU_SENSORS_MPU3050_MODULE
		.get_slave_descr = NULL,
#else
		.get_slave_descr = get_accel_slave_descr,
#endif
		.adapt_num   = 5,
		.bus         = EXT_SLAVE_BUS_SECONDARY,
		.address     = (0x30>>1),//0x0F,
#ifdef CONFIG_TC4_PORTRAIT_MODE
		.orientation = {  1,  0,  0, 
			0,  -1,  0, 
			0,  0, -1 },
	},
#else
		.orientation = {  0,  1,  0, 
			1,  0,  0, 
			0,  0, -1 },
	},
#endif
	/* compass */
	.compass = {
#ifdef CONFIG_MPU_SENSORS_MPU3050_MODULE
		.get_slave_descr = NULL,
#else
		.get_slave_descr = get_compass_slave_descr,
#endif
		.adapt_num   = 5,
		.bus         = EXT_SLAVE_BUS_PRIMARY,
		.address     = (0x3c>>1),//0x0E,
#ifdef CONFIG_TC4_PORTRAIT_MODE
		.orientation = { -1, 0, 0, 
			0, 1, 0, 
			0, 0, 1 },
	},
#else
		.orientation = { 0, -1, 0, 
			-1, 0, 0, 
			0, 0, -1 },
	},
#endif
	/* pressure */
	.pressure = {
#ifdef CONFIG_MPU_SENSORS_MPU3050_MODULE
		.get_slave_descr = NULL,
#else
		.get_slave_descr = get_pressure_slave_descr,
#endif
		.adapt_num   = 2,
		.bus         = EXT_SLAVE_BUS_PRIMARY,
		.address     = 0x77,
		.orientation = { 1, 0, 0, 
			0, 1, 0, 
			0, 0, 1 },
	},
};
#endif


#ifdef CONFIG_TOUCHSCREEN_TSC2007
#include <linux/i2c/tsc2007.h>
#define TSC2007_INT_GPIO S5PV210_GPH2(0)
// can't use gpio_to_irq,the same pin
#define TSC2007_INT_GPIO_IRQ IRQ_EINT(16)
static int tsc2007_hw_init(void)
{
	int err;

//	gpio_free(TSC2007_INT_GPIO);
	err=gpio_request(TSC2007_INT_GPIO,"tsc2007 irq");
	if(err){
		pr_err("tsc2007 irqgpio request err\n");
		return err;
	}
	err=gpio_direction_input(TSC2007_INT_GPIO);
	if(err){
		pr_err("tsc2007 irqgpio init input err %d\n",err);
		gpio_free(TSC2007_INT_GPIO);
		return err;
	}
	s3c_gpio_cfgpin(TSC2007_INT_GPIO,S3C_GPIO_SFN(0x0f));
	s3c_gpio_setpull(TSC2007_INT_GPIO,S3C_GPIO_PULL_NONE);
	//writel(readl(S5PV210_GPH2CON)|(0x0f<<16),S5PV210_GPH2CON);
//	printk("tsc2007 gpio initok\n");
	return 0;
}

static void tsc2007_hw_remove(void)
{
	s3c_gpio_cfgpin(TSC2007_INT_GPIO,S3C_GPIO_INPUT);
	gpio_free(TSC2007_INT_GPIO);
} 

static int tsc2007_get_pandown_state(void)
{
	int dat;
	//s3c_gpio_cfgpin(TSC2007_INT)
	dat=gpio_get_value(TSC2007_INT_GPIO);
//	printk("tsc2007 intpin:%X\n",dat);
	return (dat==0)?1:0;
}

static struct tsc2007_platform_data tsc2007_data={
	.model=2007,
	.x_plate_ohms=180,//这个值看字面意思是x轴电阻，但找不到相关参数，就在网上随便搜了一个，按压力度计算的时候会用到。
	.init_platform_hw=tsc2007_hw_init,
	.exit_platform_hw=tsc2007_hw_remove,
	.get_pendown_state=tsc2007_get_pandown_state,
	//.clear_penirq
}; 
#endif

static struct i2c_board_info smdkv210_i2c_devs0[] __initdata = {
	{ I2C_BOARD_INFO("24c08", 0x50), },     /* Samsung S524AD0XD1 */
	{ I2C_BOARD_INFO("wm8580", 0x1b), },
	{ I2C_BOARD_INFO("Goodix-TS",0x5d),}, // gt813 or gt9xx
	#ifdef CONFIG_TOUCHSCREEN_TSC2007
	{ I2C_BOARD_INFO("tsc2007",(0x90>>1)),
		.irq=TSC2007_INT_GPIO_IRQ,//gpio_to_irq(TSC2007_INT_GPIO),
		.platform_data=&tsc2007_data,
	}, 
	#endif
	#if defined(CONFIG_MPU_SENSORS_MPU3050) || defined(CONFIG_MPU_SENSORS_MPU3050_MODULE)
	// liang
	{
		I2C_BOARD_INFO(SENSOR_MPU_NAME, 0x68),
		//.irq = gpio_to_irq(MPUGPIO),
		.irq = IRQ_EINT(21),
		.platform_data = &mpu_data,
	},
#endif  //yulu for test sleep
};

static struct i2c_board_info smdkv210_i2c_devs1[] __initdata = {
#ifdef CONFIG_VIDEO_TV20
        {
                I2C_BOARD_INFO("s5p_ddc", (0x74>>1)),
        },
#endif	
};

static struct i2c_board_info smdkv210_i2c_devs2[] __initdata = {
#if defined(CONFIG_REGULATOR_MAX8698)
        {
                I2C_BOARD_INFO("max8698", 0xCC >> 1),
                .platform_data  = &smdkv210_max8698_pdata,
        },
#endif
};

#ifdef CONFIG_SPI_S3C64XX

#define SMDK_MMCSPI_CS 0

static struct s3c64xx_spi_csinfo smdk_spi0_csi[] = {
	[SMDK_MMCSPI_CS] = {
		.line = S5PV210_GPB(1),
		.set_level = gpio_set_value,
		.fb_delay = 0x2,
	},
};

static struct s3c64xx_spi_csinfo smdk_spi1_csi[] = {
	[SMDK_MMCSPI_CS] = {
		.line = S5PV210_GPB(5),
		.set_level = gpio_set_value,
		.fb_delay = 0x2,
	},
};

#ifdef CONFIG_SERIAL_MAX3100
#include <linux/serial_max3100.h>
static struct plat_max3100 max3100_plat_data = {
	.loopback = 0,
	.crystal = 1, // 1:3.6864MHz 	0:1.8432MHz
	.poll_time = 100,
};
#endif

static struct spi_board_info s3c_spi_devs[] __initdata = {
	{
		.modalias        = "spidev", /* MMC SPI */
		.mode            = SPI_MODE_0,  /* CPOL=0, CPHA=0 */
		.max_speed_hz    = 10000000,
		/* Connected to SPI-0 as 1st Slave */
		.bus_num         = 0,
		.chip_select     = 0,
		.controller_data = &smdk_spi0_csi[SMDK_MMCSPI_CS],
	},
	#ifdef CONFIG_SERIAL_MAX3100
	{
		.modalias      = "max3100",
		.platform_data = &max3100_plat_data,
		.irq           = S5PV210_GPH2(5),
		.mode            = SPI_MODE_0,  /* CPOL=0, CPHA=0 */
		.max_speed_hz  = 5*1000*1000,
		.chip_select   =  0,
		.bus_num         = 1,
		.controller_data = &smdk_spi1_csi[SMDK_MMCSPI_CS],
	},
	#else
	{
		.modalias        = "spidev", /* MMC SPI */
		.mode            = SPI_MODE_0,  /* CPOL=0, CPHA=0 */
		.max_speed_hz    = 10000000,
		/* Connected to SPI-0 as 1st Slave */
		.bus_num         = 1,
		.chip_select     = 0,
		.controller_data = &smdk_spi1_csi[SMDK_MMCSPI_CS],
	},
	#endif
};

#endif

static struct s3c2410_ts_mach_info s3c_ts_platform __initdata = {
	.delay			= 10000,
	.presc			= 49,
	.oversampling_shift	= 4,
	.cal_x_max              = 0,
    .cal_y_max              = 0,
};


static void __init smdkv210_map_io(void)
{
	s5p_init_io(NULL, 0, S5P_VA_CHIPID);
	s3c24xx_init_clocks(24000000);
	s5pv210_gpiolib_init();
	s3c24xx_init_uarts(smdkv210_uartcfgs, ARRAY_SIZE(smdkv210_uartcfgs));
	s5p_set_timer_source(S5P_PWM2, S5P_PWM4);
	s5p_reserve_bootmem(s5pv210_media_devs,
                        ARRAY_SIZE(s5pv210_media_devs), S5P_RANGE_MFC);
              
#ifdef CONFIG_MTD_NAND
	s3c_device_nand.name = "s5pv210-nand";
#endif
}

#ifdef CONFIG_S3C_DEV_HSMMC
static struct s3c_sdhci_platdata smdkc110_hsmmc0_pdata __initdata = {
        .cd_type                = S3C_SDHCI_CD_INTERNAL,
#if defined(CONFIG_S5PV210_SD_CH0_8BIT)
        .max_width              = 8,
        .host_caps              = MMC_CAP_8_BIT_DATA,
#endif
};

static struct s3c_sdhci_platdata smdkc110_hsmmc2_pdata __initdata = {
	.cd_type		= S3C_SDHCI_CD_NONE,//S3C_SDHCI_CD_INTERNAL,
};
#endif
 
static void __init smdkc110_setup_clocks(void)
{
        struct clk *pclk;
        struct clk *clk;

        /* set MMC0 clock */
        clk = clk_get(&s3c_device_hsmmc0.dev, "sclk_mmc");
        pclk = clk_get(NULL, "mout_mpll");
        clk_set_parent(clk, pclk);
        clk_set_rate(clk, 50*MHZ);

        pr_info("%s: %s: source is %s, rate is %ld\n",
                                __func__, clk->name, clk->parent->name,
                                clk_get_rate(clk));
}

void usb_host_phy_init(void)
{
	struct clk *otg_clk;

	otg_clk = clk_get(NULL, "otg");
	clk_enable(otg_clk);

	if (readl(S5P_USB_PHY_CONTROL) & (0x1<<1))
		return;

    __raw_writel((__raw_readl(S5PV210_ETC2_BASE + 8) & ~(3<<14)) | (1<<14), S5PV210_ETC2_BASE + 8);	

	__raw_writel(__raw_readl(S5P_USB_PHY_CONTROL) | (0x1<<1),
			S5P_USB_PHY_CONTROL);
	__raw_writel((__raw_readl(S3C_USBOTG_PHYPWR)
			& ~(0x1<<7) & ~(0x1<<6)) | (0x1<<8) | (0x1<<5),
			S3C_USBOTG_PHYPWR);
	__raw_writel((__raw_readl(S3C_USBOTG_PHYCLK) & ~(0x1<<7)) | (0x3<<0),
			S3C_USBOTG_PHYCLK);
	__raw_writel((__raw_readl(S3C_USBOTG_RSTCON)) | (0x1<<4) | (0x1<<3),
			S3C_USBOTG_RSTCON);
	__raw_writel(__raw_readl(S3C_USBOTG_RSTCON) & ~(0x1<<4) & ~(0x1<<3),
			S3C_USBOTG_RSTCON);
}
EXPORT_SYMBOL(usb_host_phy_init);

void usb_host_phy_off(void)
{
	__raw_writel(__raw_readl(S3C_USBOTG_PHYPWR) | (0x1<<7)|(0x1<<6),
			S3C_USBOTG_PHYPWR);
	__raw_writel(__raw_readl(S5P_USB_PHY_CONTROL) & ~(1<<1),
			S5P_USB_PHY_CONTROL);
}
EXPORT_SYMBOL(usb_host_phy_off);

/* USB EHCI */
static struct s5p_ehci_platdata smdkv210_ehci_pdata;
static void __init smdkv210_ehci_init(void)
{
        struct s5p_ehci_platdata *pdata = &smdkv210_ehci_pdata;

        s5p_ehci_set_platdata(pdata);
}

/*USB OHCI*/
static struct s5p_ohci_platdata smdkv210_ohci_pdata;
static void __init smdkv210_ohci_init(void)
{
        struct s5p_ohci_platdata *pdata = &smdkv210_ohci_pdata;

        s5p_ohci_set_platdata(pdata);
}

/*USB DEVICE*/
static struct s5p_otg_platdata smdkv210_otg_pdata;
static void __init smdkv210_otg_init(void)
{
        struct s5p_otg_platdata *pdata = &smdkv210_otg_pdata;

        s5p_otg_set_platdata(pdata);
}

static void __init sound_init(void)
{
	u32 reg;

	reg = __raw_readl(S5P_CLK_OUT);
	reg &= ~(0x1f << 12);
	reg &= ~(0xf << 20);
	reg |= 0x12 << 12;
	reg |= 0x1  << 20;
	__raw_writel(reg, S5P_CLK_OUT);

	reg = __raw_readl(S5P_OTHERS);
	reg &= ~(0x3 << 8);
	reg |= 0x0 << 8;
	__raw_writel(reg, S5P_OTHERS);
}

static void gps_power_on(void)
{
	int err;
	err = gpio_request(S5PV210_GPJ2(5), "GPJ2_5");
	if (err)
		printk(KERN_ERR "#### failed to request GPJ2_5 for GPS power\n");	

	s3c_gpio_setpull(S5PV210_GPJ2(5), S3C_GPIO_PULL_NONE);

	gpio_direction_output(S5PV210_GPJ2(5), 0);
	mdelay(10);
	gpio_direction_output(S5PV210_GPJ2(5), 1);
	mdelay(100);
	gpio_direction_output(S5PV210_GPJ2(5), 0);

}

static int unifi_wifi_init(void)
{
	int err;
	err = gpio_request(S5PV210_GPJ2(4), "GPJ2_4");
	if (err)
		printk(KERN_ERR "#### failed to request GPJ2_4 for RESET\n");	

	s3c_gpio_setpull(S5PV210_GPJ2(4), S3C_GPIO_PULL_NONE);
	gpio_direction_output(S5PV210_GPJ2(4), 1);


	err = gpio_request(S5PV210_GPJ3(4), "GPJ3_4");
	if (err)
		printk(KERN_ERR "#### failed to request GPJ3_4 for RESET\n");	

	s3c_gpio_setpull(S5PV210_GPJ3(4), S3C_GPIO_PULL_NONE);
	gpio_direction_output(S5PV210_GPJ3(4), 1);


	mdelay(100);
	gpio_direction_output(S5PV210_GPJ2(4), 0);
	gpio_direction_output(S5PV210_GPJ3(4), 0);

	mdelay(100);
	gpio_direction_output(S5PV210_GPJ2(4), 1);
	gpio_direction_output(S5PV210_GPJ3(4), 1);

	gpio_free(S5PV210_GPJ2(4));
	gpio_free(S5PV210_GPJ3(4));
}


static void __init smdkv210_machine_init(void)
{
	sound_init();
	s3c_pm_init();

	smdkv210_dm9000_init();
	platform_add_devices(smdkv210_devices, ARRAY_SIZE(smdkv210_devices));

#ifdef CONFIG_ANDROID_PMEM
        android_pmem_set_platdata();
#endif

#ifdef CONFIG_KEYBOARD_SAMSUNG
	samsung_keypad_set_platdata(&smdkv210_keypad_data);
#endif

#ifdef CONFIG_TOUCHSCREEN_S3C2410
	s3c24xx_ts_set_platdata(&s3c_ts_platform);
#endif

	s3c_i2c0_set_platdata(NULL);
	s3c_i2c1_set_platdata(NULL);
	s3c_i2c2_set_platdata(NULL);
	i2c_register_board_info(0, smdkv210_i2c_devs0,
			ARRAY_SIZE(smdkv210_i2c_devs0));
	i2c_register_board_info(1, smdkv210_i2c_devs1,
			ARRAY_SIZE(smdkv210_i2c_devs1));
	i2c_register_board_info(2, smdkv210_i2c_devs2,
		 	ARRAY_SIZE(smdkv210_i2c_devs2));
#ifdef CONFIG_TOUCHSCREEN_EGALAX
	i2c_register_board_info(5, i2c_devs5, ARRAY_SIZE(i2c_devs5));
#endif

	s3c_ide_set_platdata(&smdkv210_ide_pdata);

#ifdef CONFIG_FB_S3C_LTE480WV
	s3c_fb_set_platdata(&lte480wv_fb_data);
#endif

#if defined(CONFIG_CH7026_800_600) || defined(CONFIG_CH7034_1024_768)
	s3c_fb_set_platdata(&vga_fb_data);
#endif

#ifdef CONFIG_S3C_DEV_HSMMC
    s3c_sdhci0_set_platdata(&smdkc110_hsmmc0_pdata);
	s3c_sdhci2_set_platdata(&smdkc110_hsmmc2_pdata);
#endif

#ifdef CONFIG_VIDEO_FIMC
    /* fimc */
    s3c_fimc0_set_platdata(&fimc_plat_lsi);
    s3c_fimc1_set_platdata(&fimc_plat_lsi);
    s3c_fimc2_set_platdata(&fimc_plat_lsi);
        
#endif

#ifdef CONFIG_VIDEO_FIMC_MIPI
	s3c_csis_set_platdata(NULL);
#endif

#ifdef CONFIG_VIDEO_JPEG_V2
	s3c_jpeg_set_platdata(&jpeg_plat);
#endif
#ifdef CONFIG_VIDEO_MFC50
        /* mfc */
        s3c_mfc_set_platdata(NULL);
#endif
        /* spi */
#ifdef CONFIG_SPI_S3C64XX
	if (!gpio_request(S5PV210_GPB(1), "SPI_CS0")) {
	    gpio_direction_output(S5PV210_GPB(1), 1);
	    s3c_gpio_cfgpin(S5PV210_GPB(1), S3C_GPIO_SFN(1));
	    s3c_gpio_setpull(S5PV210_GPB(1), S3C_GPIO_PULL_UP);
	    s5pv210_spi_set_info(0, S5PV210_SPI_SRCCLK_PCLK,
		    ARRAY_SIZE(smdk_spi0_csi));
	}
	if (!gpio_request(S5PV210_GPB(5), "SPI_CS1")) {
	    gpio_direction_output(S5PV210_GPB(5), 1);
	    s3c_gpio_cfgpin(S5PV210_GPB(5), S3C_GPIO_SFN(1));
	    s3c_gpio_setpull(S5PV210_GPB(5), S3C_GPIO_PULL_UP);
	    s5pv210_spi_set_info(1, S5PV210_SPI_SRCCLK_PCLK,
		    ARRAY_SIZE(smdk_spi1_csi));
	}
	spi_register_board_info(s3c_spi_devs, ARRAY_SIZE(s3c_spi_devs));
#endif

	smdkv210_otg_init();
    smdkv210_ehci_init();
    smdkv210_ohci_init();
    clk_xusbxti.rate = 24000000;

	smdkc110_setup_clocks(); 
	
	unifi_wifi_init();

	gps_power_on();
}

MACHINE_START(SMDKV210, "SMDKV210")
	/* Maintainer: Kukjin Kim <kgene.kim@samsung.com> */
	.boot_params	= S5P_PA_SDRAM + 0x100,
	.init_irq	= s5pv210_init_irq,
	.map_io		= smdkv210_map_io,
	.init_machine	= smdkv210_machine_init,
	.timer		= &s5p_timer,
MACHINE_END
