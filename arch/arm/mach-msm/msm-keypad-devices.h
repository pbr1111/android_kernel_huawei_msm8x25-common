/*
 * Copyright (c) 2009, Code Aurora Forum. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef _MSM_KEYPAD_DEVICES_H
#define _MSM_KEYPAD_DEVICES_H

extern struct platform_device keypad_device_7k_ffa;
extern struct platform_device keypad_device_8k_ffa;
extern struct platform_device keypad_device_surf;

#if defined(CONFIG_HUAWEI_GPIO_KEYPAD)
extern struct platform_device keypad_device_default;
/* because all production KEY_VOLUMEUP and KEY_VOLUMEDOWN sameness,
 * so use keypad_device_default ,
 * del keypad_device_u8655, del 1 row,
 * for tending to promote code unity.
 */
extern struct platform_device keypad_device_u8185;
extern struct platform_device keypad_device_y300j1;
#endif
#endif
