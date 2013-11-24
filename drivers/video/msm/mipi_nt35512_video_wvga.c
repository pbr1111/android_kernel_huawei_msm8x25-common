/* Copyright (c) 2012, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "msm_fb.h"
#include "mipi_dsi.h"
#include "hw_lcd_common.h"

#define LCD_DEVICE_NAME "mipi_video_nt35512_wvga"

static lcd_panel_type lcd_panel_wvga = LCD_NONE;
static struct mipi_dsi_phy_ctrl dsi_video_mode_phy_db = {
 	/* DSI Bit Clock at 490 MHz, 2 lane, RGB888 */ 
	/* regulator */ 
	{0x03, 0x01, 0x01, 0x00}, 
	/* timing */ 
	{0x88, 0x32, 0x14, 0x00, 0x44, 0x4f, 0x18, 0x35, 
	0x17, 0x3, 0x04}, 
	/* phy ctrl */ 
	{0x7f, 0x00, 0x00, 0x00}, 
	/* strength */ 
	{0xbb, 0x02, 0x06, 0x00}, 
	/* pll control */ 
	{0x01, 0xE3, 0x31, 0xd2, 0x00, 0x40, 0x37, 0x62, 
	0x01, 0x0f, 0x07, 
	0x05, 0x14, 0x03, 0x0, 0x0, 0x0, 0x20, 0x0, 0x02, 0x0}, 
};

static struct dsi_buf nt35512_tx_buf;
static struct sequence * nt35512_lcd_init_table_debug = NULL;
#if 0
static const struct sequence nt35512_wvga_standby_enter_table[]= 
{
	{0x00028,MIPI_DCS_COMMAND,0}, //28h
	{0x00010,MIPI_DCS_COMMAND,20},
	{0x00029,MIPI_TYPE_END,120}, // add new command for 
};
static const struct sequence nt35512_wvga_standby_exit_table[]= 
{
	{0x00011,MIPI_DCS_COMMAND,0}, //29h
	{0x00029,MIPI_DCS_COMMAND,120},
	{0x00029,MIPI_TYPE_END,20}, // add new command for 
};
/*lcd resume function*/
#endif

static struct sequence nt35512_cabc_enable_table[] =
{	
	{0x00051,MIPI_DCS_COMMAND,0},
	{0x000FF,TYPE_PARAMETER,0},
	{0x00029,MIPI_TYPE_END,0}
};

static struct sequence nt35512_cabc_set_dimming_off_table[] =
{
	{0x00053,MIPI_DCS_COMMAND,0},
	{0x00024,TYPE_PARAMETER,0},//no dimming.
	{0x00029,MIPI_TYPE_END,0}
};
/*Add nt35512 video mode for byd*/
/*Add new gamma2.5 parameter and Optimize the display effect */
static struct sequence nt35512_byd_lcd_init_table[] =
{
	{0x000F0,MIPI_GEN_COMMAND,0},
	{0x00055,TYPE_PARAMETER,0},
	{0x000AA,TYPE_PARAMETER,0},
	{0x00052,TYPE_PARAMETER,0},
	{0x00008,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0}, //page 1

	{0x000B0,MIPI_GEN_COMMAND,0}, //AVDD
	{0x00009,TYPE_PARAMETER,0},
	{0x00009,TYPE_PARAMETER,0},
	{0x00009,TYPE_PARAMETER,0},

	{0x000B6,MIPI_GEN_COMMAND,0},
	{0x00034,TYPE_PARAMETER,0},
	{0x00034,TYPE_PARAMETER,0},
	{0x00034,TYPE_PARAMETER,0},
	
	{0x000B1,MIPI_GEN_COMMAND,0}, //AVEE
	{0x00009,TYPE_PARAMETER,0},
	{0x00009,TYPE_PARAMETER,0},
	{0x00009,TYPE_PARAMETER,0},

	{0x000B7,MIPI_GEN_COMMAND,0},
	{0x00034,TYPE_PARAMETER,0},
	{0x00034,TYPE_PARAMETER,0},
	{0x00034,TYPE_PARAMETER,0},

	{0x000B2,MIPI_GEN_COMMAND,0}, //VCL
	{0x00001,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},

	{0x000B8,MIPI_GEN_COMMAND,0},
	{0x00034,TYPE_PARAMETER,0},
	{0x00034,TYPE_PARAMETER,0},
	{0x00034,TYPE_PARAMETER,0},
	
	{0x000B3,MIPI_GEN_COMMAND,0}, //page1 B3 VGH
	{0x00005,TYPE_PARAMETER,0},
	{0x00005,TYPE_PARAMETER,0},
	{0x00005,TYPE_PARAMETER,0},

	{0x000B9,MIPI_GEN_COMMAND,0},
	{0x00026,TYPE_PARAMETER,0},
	{0x00026,TYPE_PARAMETER,0},
	{0x00026,TYPE_PARAMETER,0},

	{0x000BF,MIPI_DCS_COMMAND,0},
	{0x00001,TYPE_PARAMETER,0},

	{0x000B5,MIPI_GEN_COMMAND,0}, //VGL
	{0x0000B,TYPE_PARAMETER,0},
	{0x0000B,TYPE_PARAMETER,0},
	{0x0000B,TYPE_PARAMETER,0},
	
	{0x000BA,MIPI_GEN_COMMAND,0},
	{0x00034,TYPE_PARAMETER,0},
	{0x00034,TYPE_PARAMETER,0},
	{0x00034,TYPE_PARAMETER,0},
	
	{0x000BC,MIPI_GEN_COMMAND,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000A3,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	
	{0x000BD,MIPI_GEN_COMMAND,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000A3,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	
//	{0x000BE,MIPI_GEN_COMMAND,0}, //VCOM
//	{0x00000,TYPE_PARAMETER,0},
//	{0x00050,TYPE_PARAMETER,0},
	
	/* gamma2.5 parameter configure start */
	{0x000D1,MIPI_GEN_COMMAND,0}, //GAMMA2.5 Red+
	{0x00000,TYPE_PARAMETER,0},
	{0x00037,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00063,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00091,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000B1,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000C2,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000E9,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x0000E,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x0003F,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x00065,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x000A3,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x000D4,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00021,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x0005E,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x0005F,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x0009A,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x000D4,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x000FB,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00029,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x0004B,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00071,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x0008B,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000A2,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000AE,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000BB,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000C1,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000C1,TYPE_PARAMETER,0},

	{0x000D2,MIPI_GEN_COMMAND,0}, //GAMMA2.5 Green+
	{0x00000,TYPE_PARAMETER,0},
	{0x00037,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00063,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00091,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000B1,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000C2,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000E9,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x0000E,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x0003F,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x00065,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x000A3,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x000D4,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00021,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x0005E,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x0005F,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x0009A,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x000D4,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x000FB,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00029,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x0004B,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00071,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x0008B,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000A2,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000AE,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000BB,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000C1,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000C1,TYPE_PARAMETER,0},

	{0x000D3,MIPI_GEN_COMMAND,0}, //GAMMA2.5 Blue+
	{0x00000,TYPE_PARAMETER,0},
	{0x00037,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00063,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00091,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000B1,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000C2,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000E9,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x0000E,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x0003F,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x00065,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x000A3,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x000D4,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00021,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x0005E,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x0005F,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x0009A,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x000D4,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x000FB,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00029,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x0004B,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00071,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x0008B,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000A2,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000AE,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000BB,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000C1,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000C1,TYPE_PARAMETER,0},

	{0x000D4,MIPI_GEN_COMMAND,0}, //GAMMA2.5 Red-
	{0x00000,TYPE_PARAMETER,0},
	{0x00037,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00063,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00091,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000B1,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000C2,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000E9,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x0000E,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x0003F,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x00065,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x000A3,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x000D4,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00021,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x0005E,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x0005F,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x0009A,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x000D4,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x000FB,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00029,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x0004B,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00071,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x0008B,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000A2,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000AE,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000BB,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000C1,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000C1,TYPE_PARAMETER,0},

	{0x000D5,MIPI_GEN_COMMAND,0}, //GAMMA2.5 Green-
	{0x00000,TYPE_PARAMETER,0},
	{0x00037,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00063,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00091,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000B1,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000C2,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000E9,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x0000E,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x0003F,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x00065,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x000A3,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x000D4,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00021,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x0005E,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x0005F,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x0009A,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x000D4,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x000FB,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00029,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x0004B,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00071,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x0008B,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000A2,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000AE,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000BB,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000C1,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000C1,TYPE_PARAMETER,0},

	{0x000D6,MIPI_GEN_COMMAND,0}, //GAMMA2.5 Blue-
	{0x00000,TYPE_PARAMETER,0},
	{0x00037,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00063,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00091,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000B1,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000C2,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000E9,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x0000E,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x0003F,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x00065,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x000A3,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x000D4,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00021,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x0005E,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x0005F,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x0009A,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x000D4,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x000FB,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00029,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x0004B,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00071,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x0008B,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000A2,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000AE,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000BB,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000C1,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000C1,TYPE_PARAMETER,0},
	/* gamma2.5 parameter configure end */

	{0x000F0,MIPI_GEN_COMMAND,0},
	{0x00055,TYPE_PARAMETER,0},
	{0x000AA,TYPE_PARAMETER,0},
	{0x00052,TYPE_PARAMETER,0},
	{0x00008,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0}, //page 0

	{0x000BC,MIPI_GEN_COMMAND,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},

	{0x000B6,MIPI_GEN_COMMAND,0},
	{0x00005,TYPE_PARAMETER,0},

	{0x000B7,MIPI_GEN_COMMAND,0},
	{0x00070,TYPE_PARAMETER,0},
	{0x00070,TYPE_PARAMETER,0},

	{0x000B8,MIPI_GEN_COMMAND,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},

	{0x000BA,MIPI_GEN_COMMAND,0}, //Source Driver Control
	{0x00001,TYPE_PARAMETER,0},

	{0x000B1,MIPI_GEN_COMMAND,0},
	{0x000FC,TYPE_PARAMETER,0},
	{0x00006,TYPE_PARAMETER,0},

	{0x000CC,MIPI_GEN_COMMAND,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},

	{0x000BD,MIPI_GEN_COMMAND,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x00084,TYPE_PARAMETER,0},
	{0x0001C,TYPE_PARAMETER,0},
	{0x0001C,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},

	{0x000B0,MIPI_GEN_COMMAND,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00005,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00005,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},

	{0x000B4,MIPI_GEN_COMMAND,0},
	{0x00010,TYPE_PARAMETER,0},
	
	{0x000C9,MIPI_GEN_COMMAND,0},
	{0x000D0,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00050,TYPE_PARAMETER,0},
	{0x00050,TYPE_PARAMETER,0},
	{0x00050,TYPE_PARAMETER,0},

	{0x0003A,MIPI_GEN_COMMAND,0},//Interface Pixel Format
	{0x00077,TYPE_PARAMETER,0},//24-bit

	{0x00036,MIPI_GEN_COMMAND,0},//36 scan dir
	{0x00092,TYPE_PARAMETER,0},

	{0x00051,MIPI_DCS_COMMAND,0}, //WRDISBV(51H) Write display brightness
	{0x00000,TYPE_PARAMETER,0}, 

	{0x00053,MIPI_DCS_COMMAND,0}, //WRCTRLD(53H) Write CTRL display
	{0x00024,TYPE_PARAMETER,0}, 

	{0x00055,MIPI_DCS_COMMAND,0}, //WRCABC(55H) Write content adaptive brightness control 
	{0x00001,TYPE_PARAMETER,0},

	{0x00011,MIPI_DCS_COMMAND,0},   //0x11	Sleep out
	{0x00029,MIPI_DCS_COMMAND,120},//0x29	Display On
	{0x00029,MIPI_TYPE_END,20},
};
static struct sequence nt35512_boe_lcd_init_table[] =
{
	{0x000F0,MIPI_GEN_COMMAND,0},//page0
	{0x00055,TYPE_PARAMETER,0},
	{0x000AA,TYPE_PARAMETER,0},
	{0x00052,TYPE_PARAMETER,0},
	{0x00008,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	
	{0x000BC,MIPI_GEN_COMMAND,0},//page0 inversion
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	
	{0x000B6,MIPI_GEN_COMMAND,0},//Gate EQ 
	{0x0000A,TYPE_PARAMETER,0},
	
	{0x000B7,MIPI_GEN_COMMAND,0},//Gate EQ 
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	
	{0x000B8,MIPI_GEN_COMMAND,0},//source EQ
	{0x00001,TYPE_PARAMETER,0},
	{0x00005,TYPE_PARAMETER,0},
	{0x00005,TYPE_PARAMETER,0},
	{0x00005,TYPE_PARAMETER,0},
	
	{0x000BA,MIPI_GEN_COMMAND,0},//Source Driver Control
	{0x00001,TYPE_PARAMETER,0},
	
	{0x000B1,MIPI_GEN_COMMAND,0},
	{0x000FC,TYPE_PARAMETER,0},
	{0x00006,TYPE_PARAMETER,0},
	
	{0x000CC,MIPI_GEN_COMMAND,0},//BOE's Setting (default)
	{0x00003,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	
	{0x000BD,MIPI_DCS_COMMAND,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x00084,TYPE_PARAMETER,0},
	{0x0001C,TYPE_PARAMETER,0}, 
	{0x0001C,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	
	{0x000F0,MIPI_GEN_COMMAND,0},//page1
	{0x00055,TYPE_PARAMETER,0},
	{0x000AA,TYPE_PARAMETER,0},
	{0x00052,TYPE_PARAMETER,0},
	{0x00008,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	
	{0x000D1,MIPI_GEN_COMMAND,0},//gamma R+ D1
	{0x00000,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00040,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00070,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00099,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000B2,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000DA,TYPE_PARAMETER,0},//244
	{0x00001,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x00032,TYPE_PARAMETER,0},//224
	{0x00001,TYPE_PARAMETER,0},
	{0x00053,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x0008A,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x000C1,TYPE_PARAMETER,0},//11 192
	{0x00002,TYPE_PARAMETER,0},
	{0x00011,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00052,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00053,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x0008F,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x000CB,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x000F6,TYPE_PARAMETER,0},//47
	{0x00003,TYPE_PARAMETER,0},
	{0x00022,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00053,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00079,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x0009C,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000C5,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000D8,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000ED,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000F8,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},//255
	{0x000FB,TYPE_PARAMETER,0},//F3
	
	{0x000D2,MIPI_GEN_COMMAND,0},//gamma G+ D2
	{0x00000,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00040,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00070,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00099,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000B2,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000DA,TYPE_PARAMETER,0},//244
	{0x00001,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x00032,TYPE_PARAMETER,0},//224
	{0x00001,TYPE_PARAMETER,0},
	{0x00053,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x0008A,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x000C1,TYPE_PARAMETER,0},//11 192
	{0x00002,TYPE_PARAMETER,0},
	{0x00011,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00052,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00053,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x0008F,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x000CB,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x000F6,TYPE_PARAMETER,0},//47
	{0x00003,TYPE_PARAMETER,0},
	{0x00022,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00053,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00079,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x0009C,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000C5,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000D8,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000ED,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000F8,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},//255
	{0x000FB,TYPE_PARAMETER,0},//F3
	
	{0x000D3,MIPI_GEN_COMMAND,0},//gamma B+ D1
	{0x00000,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00040,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00070,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00099,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000B2,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000DA,TYPE_PARAMETER,0},//244
	{0x00001,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x00032,TYPE_PARAMETER,0},//224
	{0x00001,TYPE_PARAMETER,0},
	{0x00053,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x0008A,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x000C1,TYPE_PARAMETER,0},//11 192
	{0x00002,TYPE_PARAMETER,0},
	{0x00011,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00052,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00053,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x0008F,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x000CB,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x000F6,TYPE_PARAMETER,0},//47
	{0x00003,TYPE_PARAMETER,0},
	{0x00022,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00053,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00079,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x0009C,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000C5,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000D8,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000ED,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000F8,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},//255
	{0x000FB,TYPE_PARAMETER,0},//F3
	
	{0x000D4,MIPI_GEN_COMMAND,0},//gamma R- D4
	{0x00000,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00040,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00070,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00099,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000B2,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000DA,TYPE_PARAMETER,0},//244
	{0x00001,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x00032,TYPE_PARAMETER,0},//224
	{0x00001,TYPE_PARAMETER,0},
	{0x00053,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x0008A,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x000C1,TYPE_PARAMETER,0},//11 192
	{0x00002,TYPE_PARAMETER,0},
	{0x00011,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00052,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00053,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x0008F,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x000CB,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x000F6,TYPE_PARAMETER,0},//47
	{0x00003,TYPE_PARAMETER,0},
	{0x00022,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00053,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00079,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x0009C,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000C5,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000D8,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000ED,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000F8,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},//255
	{0x000FB,TYPE_PARAMETER,0},//F3
	
	{0x000D5,MIPI_GEN_COMMAND,0},//gamma G- D5
	{0x00000,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00040,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00070,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00099,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000B2,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000DA,TYPE_PARAMETER,0},//244
	{0x00001,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x00032,TYPE_PARAMETER,0},//224
	{0x00001,TYPE_PARAMETER,0},
	{0x00053,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x0008A,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x000C1,TYPE_PARAMETER,0},//11 192
	{0x00002,TYPE_PARAMETER,0},
	{0x00011,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00052,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00053,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x0008F,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x000CB,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x000F6,TYPE_PARAMETER,0},//47
	{0x00003,TYPE_PARAMETER,0},
	{0x00022,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00053,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00079,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x0009C,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000C5,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000D8,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000ED,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000F8,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},//255
	{0x000FB,TYPE_PARAMETER,0},//F3
	
	{0x000D6,MIPI_GEN_COMMAND,0},//gamma B- D6
	{0x00000,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00040,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00070,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00099,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000B2,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000DA,TYPE_PARAMETER,0},//244
	{0x00001,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x00032,TYPE_PARAMETER,0},//224
	{0x00001,TYPE_PARAMETER,0},
	{0x00053,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x0008A,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x000C1,TYPE_PARAMETER,0},//11 192
	{0x00002,TYPE_PARAMETER,0},
	{0x00011,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00052,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00053,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x0008F,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x000CB,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x000F6,TYPE_PARAMETER,0},//47
	{0x00003,TYPE_PARAMETER,0},
	{0x00022,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00053,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00079,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x0009C,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000C5,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000D8,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000ED,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000F8,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},//255
	{0x000FB,TYPE_PARAMETER,0},//F3
	
	{0x000B0,MIPI_GEN_COMMAND,0},//page1 B0 AVDD
	{0x00009,TYPE_PARAMETER,0},
	{0x00009,TYPE_PARAMETER,0},
	{0x00009,TYPE_PARAMETER,0},
	
	{0x000B6,MIPI_GEN_COMMAND,0},//page1 B6
	{0x00044,TYPE_PARAMETER,0},//AVDD: manual, 6V (0x44: 2.5xVCI0)
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	
	{0x000B1,MIPI_GEN_COMMAND,0},//page1 B1 AVEE
	{0x00009,TYPE_PARAMETER,0},
	{0x00009,TYPE_PARAMETER,0},
	{0x00009,TYPE_PARAMETER,0},
	
	{0x000B7,MIPI_GEN_COMMAND,0},//page1 B7
	{0x00034,TYPE_PARAMETER,0},//AVEE: manual, -6V (0x34: -2.5xVCI)
	{0x00034,TYPE_PARAMETER,0},
	{0x00034,TYPE_PARAMETER,0},
	
	{0x000B3,MIPI_GEN_COMMAND,0},//page1 B3 VGH
	{0x00005,TYPE_PARAMETER,0},
	{0x00005,TYPE_PARAMETER,0},
	{0x00005,TYPE_PARAMETER,0},
	
	{0x000B9,MIPI_GEN_COMMAND,0},//page1 B9
	{0x00026,TYPE_PARAMETER,0},//VGH: Clamp Enable, 2*AVDD-AVEE, 11V(0x00,0x34,0x0B)
	{0x00026,TYPE_PARAMETER,0},
	{0x00026,TYPE_PARAMETER,0},
	
	{0x000B5,MIPI_GEN_COMMAND,0},//page1 B4 VGLX
	{0x0000B,TYPE_PARAMETER,0},//-12.0V
	{0x0000B,TYPE_PARAMETER,0},//-12.0V
	{0x0000B,TYPE_PARAMETER,0},//-12.0V

	{0x000BA,MIPI_GEN_COMMAND,0},//page1 BA
	{0x00024,TYPE_PARAMETER,0},//VGL(LVGL)
	{0x00024,TYPE_PARAMETER,0},
	{0x00024,TYPE_PARAMETER,0},
	
	{0x000C2,MIPI_GEN_COMMAND,0},//page1 B2 VCL
	{0x00001,TYPE_PARAMETER,0},//-4.0V
	
	{0x000BF,MIPI_GEN_COMMAND,0},//page1 BF
	{0x00001,TYPE_PARAMETER,0},
	
	{0x000BC,MIPI_GEN_COMMAND,0},//page1 BC 
	{0x00000,TYPE_PARAMETER,0},//VGMP:-5.0V,VGSN:0.3V
	{0x00090,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	
	{0x000BD,MIPI_GEN_COMMAND,0},//page1 BD
	{0x00000,TYPE_PARAMETER,0},
	{0x00090,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	
	{0x0003A,MIPI_GEN_COMMAND,0},//Interface Pixel Format
	{0x00077,TYPE_PARAMETER,0},//24-bit
	
	{0x00036,MIPI_GEN_COMMAND,0},//36 scan dir
	{0x00092,TYPE_PARAMETER,0},

	{0x00051,MIPI_DCS_COMMAND,0},
	{0x00000,TYPE_PARAMETER,0}, 
	
	{0x00053,MIPI_DCS_COMMAND,0}, // dimming
	{0x00024,TYPE_PARAMETER,0}, 
	
	{0x00055,MIPI_DCS_COMMAND,0}, // UI MODE
	{0x00001,TYPE_PARAMETER,0},

	{0x0005E,MIPI_DCS_COMMAND,0},//Write CABC minimum brightness 
	{0x0009D,TYPE_PARAMETER,0},
	
	{0x00011,MIPI_DCS_COMMAND,0},
	{0x00029,MIPI_DCS_COMMAND,120},
	{0x00029,MIPI_TYPE_END,20},
};


/*setting for support continuous splash */
static int mipi_nt35512_lcd_on(struct platform_device *pdev)
{
	boolean para_debug_flag = FALSE;
	uint32 para_num = 0;
	struct msm_fb_data_type *mfd;
	mfd = platform_get_drvdata(pdev);
	
	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL; 
	

	para_debug_flag = lcd_debug_malloc_get_para( "nt35512_lcd_init_table_debug", 
			(void**)&nt35512_lcd_init_table_debug,&para_num);

	lcd_reset();
	mipi_set_tx_power_mode(1);//Low power mode 
	
	if( (TRUE == para_debug_flag) && (NULL != nt35512_lcd_init_table_debug))
	{
		process_mipi_table(mfd,&nt35512_tx_buf,nt35512_lcd_init_table_debug,
			para_num, lcd_panel_wvga);
	}
	else
	{
		/*Add nt35512 video mode for byd*/
		if (MIPI_VIDEO_NT35512_BOE_WVGA == lcd_panel_wvga  ){
			process_mipi_table(mfd,&nt35512_tx_buf,(struct sequence*)&nt35512_boe_lcd_init_table,
				ARRAY_SIZE(nt35512_boe_lcd_init_table), lcd_panel_wvga);
		}
		else if ( MIPI_VIDEO_NT35512_BYD_WVGA == lcd_panel_wvga ){
			process_mipi_table(mfd,&nt35512_tx_buf,(struct sequence*)&nt35512_byd_lcd_init_table,
				ARRAY_SIZE(nt35512_byd_lcd_init_table), lcd_panel_wvga);
		}

	}
	mipi_set_tx_power_mode(0);	//High speed mode 
	
	if((TRUE == para_debug_flag)&&(NULL != nt35512_lcd_init_table_debug))
	{
		lcd_debug_free_para((void *)nt35512_lcd_init_table_debug);
	}
	
	if (!mfd->cont_splash_done) 
	{
		mfd->cont_splash_done = 1;
		nt35512_cabc_enable_table[1].reg = 0x00064;
		process_mipi_table(mfd,&nt35512_tx_buf,(struct sequence*)&nt35512_cabc_enable_table,
			ARRAY_SIZE(nt35512_cabc_enable_table), lcd_panel_wvga);
	}
	
	LCD_DEBUG("leave mipi_nt35512_lcd_on \n");
	return 0;
}
/*lcd suspend function*/
static int mipi_nt35512_lcd_off(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;
#if 0
	process_mipi_table(mfd,&nt35512_tx_buf,(struct sequence*)&nt35512_wvga_standby_enter_table,
		 ARRAY_SIZE(nt35512_wvga_standby_enter_table), lcd_panel_wvga);
#endif

	lcd_reset();
	LCD_DEBUG("leave mipi_nt35512_lcd_off \n");
	return 0;
}

#ifdef CONFIG_FB_AUTO_CABC
static struct sequence nt35512_auto_cabc_set_table[] =
{
	{0x00053,MIPI_DCS_COMMAND,0},
	{0x00024,TYPE_PARAMETER,0},//open cabc
	{0x00055,MIPI_DCS_COMMAND,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00029,MIPI_TYPE_END,0},
};

/***************************************************************
Function: nt35512_config_cabc
Description: Set CABC configuration
Parameters:
    struct msmfb_cabc_config cabc_cfg: CABC configuration struct
Return:
    0: success
***************************************************************/
static int nt35512_config_auto_cabc(struct msmfb_cabc_config cabc_cfg,struct msm_fb_data_type *mfd)
{
	int ret = 0;

	switch(cabc_cfg.mode)
	{
		case CABC_MODE_UI:
			/* the value of cabc register should be 24 in UI mode */
			nt35512_auto_cabc_set_table[1].reg=0x00024;
			nt35512_auto_cabc_set_table[3].reg=0x00001;
			break;
		case CABC_MODE_MOVING:
		case CABC_MODE_STILL:
			/* the value of cabc register should be 2C in moving mode and still mode */
			nt35512_auto_cabc_set_table[1].reg=0x0002C;
			nt35512_auto_cabc_set_table[3].reg=0x00003;
			break;
		default:
			LCD_DEBUG("%s: invalid cabc mode: %d\n", __func__, cabc_cfg.mode);
	        ret = -EINVAL;
			break;
	}
	if(likely(0 == ret))
	{
		process_mipi_table(mfd,&nt35512_tx_buf,(struct sequence*)&nt35512_auto_cabc_set_table,
			 ARRAY_SIZE(nt35512_auto_cabc_set_table), lcd_panel_wvga);
	}

	LCD_DEBUG("%s: change cabc mode to %d\n",__func__,cabc_cfg.mode);
	return ret;
}
#endif

static int __devinit mipi_nt35512_lcd_probe(struct platform_device *pdev)
{
	msm_fb_add_device(pdev);

	return 0;
}
	/*delete some lines */

void nt35512_set_cabc_backlight(struct msm_fb_data_type *mfd,uint32 bl_level)
{	
	nt35512_cabc_enable_table[1].reg = bl_level; // 1 will be changed if modify init code

    if(0 == bl_level)
    {
        /* 0 level will disable dimming func */
        process_mipi_table(mfd,&nt35512_tx_buf,(struct sequence*)&nt35512_cabc_set_dimming_off_table,
                    ARRAY_SIZE(nt35512_cabc_set_dimming_off_table), lcd_panel_wvga);
    }
    process_mipi_table(mfd,&nt35512_tx_buf,(struct sequence*)&nt35512_cabc_enable_table,
                                ARRAY_SIZE(nt35512_cabc_enable_table), lcd_panel_wvga);
}

static struct platform_driver this_driver = {
	.probe  = mipi_nt35512_lcd_probe,
	.driver = {
		.name   = LCD_DEVICE_NAME,
	},
};
static struct msm_fb_panel_data nt35512_panel_data = {
	.on					= mipi_nt35512_lcd_on,
	.off					= mipi_nt35512_lcd_off,
	.set_backlight 		= pwm_set_backlight,
	/*add cabc control backlight*/
	.set_cabc_brightness 	= nt35512_set_cabc_backlight,
#ifdef CONFIG_FB_AUTO_CABC
	.config_cabc = nt35512_config_auto_cabc,
#endif
};

static struct platform_device this_device = {
	.name   = LCD_DEVICE_NAME,
	.id	= 0,
	.dev	= {
		.platform_data = &nt35512_panel_data,
	}
};

static __init int mipi_video_nt35512_wvga_init(void)
{
	int ret = 0;
	struct msm_panel_info *pinfo = NULL;
	
	lcd_panel_wvga = get_lcd_panel_type();
	/*Add nt35512 video mode for byd*/
	if ((MIPI_VIDEO_NT35512_BOE_WVGA != lcd_panel_wvga)
 		&&  (MIPI_VIDEO_NT35512_BYD_WVGA != lcd_panel_wvga) )
	{
		return 0;
	}
	LCD_DEBUG("enter mipi_video_nt35512_wvga_init \n");
	mipi_dsi_buf_alloc(&nt35512_tx_buf, DSI_BUF_SIZE);

	ret = platform_driver_register(&this_driver);
	if (!ret)
	{
		pinfo = &nt35512_panel_data.panel_info;
		pinfo->xres = 480;
		pinfo->yres = 800;
		pinfo->type = MIPI_VIDEO_PANEL;
		pinfo->pdest = DISPLAY_1;
		pinfo->wait_cycle = 0;
		pinfo->bpp = 24;		
		/*the value below are all experience values*/
		pinfo->lcdc.h_back_porch = 180;
		pinfo->lcdc.h_front_porch = 180;
		pinfo->lcdc.h_pulse_width = 8;
		pinfo->lcdc.v_back_porch = 7;
		pinfo->lcdc.v_front_porch = 8;
		pinfo->lcdc.v_pulse_width = 1;
		pinfo->lcdc.border_clr = 0;	/* blk */
		pinfo->lcdc.underflow_clr = 0xff;	/* blue */
		/* number of dot_clk cycles HSYNC active edge is
		delayed from VSYNC active edge */
		pinfo->lcdc.hsync_skew = 0;
		pinfo->bl_max = 255;
		pinfo->bl_min = 30;		
		pinfo->fb_num = 3;
		
		pinfo->clk_rate = 490000000;
		
		pinfo->mipi.mode = DSI_VIDEO_MODE;
		pinfo->mipi.pulse_mode_hsa_he = TRUE;
		pinfo->mipi.hfp_power_stop = TRUE; /* LP-11 during the HFP period */
		pinfo->mipi.hbp_power_stop = TRUE; /* LP-11 during the HBP period */
		pinfo->mipi.hsa_power_stop = TRUE; /* LP-11 during the HSA period */
		/* LP-11 or let Command Mode Engine send packets in
		HS or LP mode for the BLLP of the last line of a frame */
		pinfo->mipi.eof_bllp_power_stop = TRUE;
		/* LP-11 or let Command Mode Engine send packets in
		HS or LP mode for packets sent during BLLP period */
		pinfo->mipi.bllp_power_stop = TRUE;

		pinfo->mipi.traffic_mode = DSI_BURST_MODE;
		pinfo->mipi.dst_format = DSI_VIDEO_DST_FORMAT_RGB888;
		pinfo->mipi.vc = 0;
		pinfo->mipi.rgb_swap = DSI_RGB_SWAP_RGB;
		pinfo->mipi.data_lane0 = TRUE;
		pinfo->mipi.data_lane1 = TRUE;
		pinfo->mipi.t_clk_post = 0x20;
		pinfo->mipi.t_clk_pre = 0x2f;
		pinfo->mipi.stream = 0; /* dma_p */
		pinfo->mipi.mdp_trigger = DSI_CMD_TRIGGER_SW;
		pinfo->mipi.dma_trigger = DSI_CMD_TRIGGER_SW;
		pinfo->mipi.frame_rate = 60; /* FIXME */

		pinfo->mipi.dsi_phy_db = &dsi_video_mode_phy_db;
		pinfo->mipi.tx_eot_append = 0x01;

		pinfo->mipi.dlane_swap = 0x1;

		ret = platform_device_register(&this_device);
		if (ret)
			LCD_DEBUG("%s: failed to register device!\n", __func__);
	}
	return ret;
}

module_init(mipi_video_nt35512_wvga_init);
