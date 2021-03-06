/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */

/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2005
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE. 
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/
/*****************************************************************************
 *
 * Filename:
 * ---------
 *   sensor.c
 *
 * Project:
 * --------
 *   RAW
 *
 * Description:
 * ------------
 *   Source code of Sensor driver
 *
 *
 * Author:
 * -------
 *   HengJun (MTK70677)
 *
 *============================================================================
 *             HISTORY
 * Below this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * $Revision:$
 * $Modtime:$
 * $Log:$
 *
 * 02 19 2012 koli.lin
 * [ALPS00237113] [Performance][Video recording]Recording preview the screen have flash
 * [Camera] 1. Modify the AE converge speed in the video mode.
 *                2. Modify the isp gain delay frame with sensor exposure time and gain synchronization.
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/

#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>
#include <asm/system.h>

#include "kd_camera_hw.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"

#include "ov5647truly_Sensor.h"
#include "ov5647truly_Camera_Sensor_para.h"
#include "ov5647truly_CameraCustomized.h"

MSDK_SCENARIO_ID_ENUM tCurrentScenarioId = MSDK_SCENARIO_ID_CAMERA_PREVIEW;
static kal_bool OV5647tMIPIAutoFlicKerMode = KAL_FALSE;
//static kal_bool OV5647tMIPIZsdCameraPreview = KAL_FALSE;

/*
 * Gionee. Qux. 20120709, for double ov5647 camear. 
 * Hal layer don't permits that sensor_ids are same
 */
#define OV5647_CLONING

#define MODULE_ID  0x02
#define MODULE_ID_TRULY  0x02

#if defined(OV5647_CLONING)
#define OV5647tMIPI_SENSOR_ID  0x5647 //(OV5647TRULY_SENSOR_ID-0x02)
#endif
/* Gionee. QUx. END */

//#define OV5647tMIPI_DRIVER_TRACE
//#define OV5647tMIPI_DEBUG

#ifdef OV5647tMIPI_DEBUG
#define SENSORDB printk
#else
#define SENSORDB(x,...)
#endif
//#define ACDK
extern int iReadRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u8 * a_pRecvData, u16 a_sizeRecvData, u16 i2cId);
extern int iWriteRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u16 i2cId);

static DEFINE_SPINLOCK(ov5647tmipi_drv_lock);

UINT32 OV5647tMIPISetMaxFrameRate(UINT16 u2FrameRate);

kal_uint16 OV5647tMIPI_write_cmos_sensor(kal_uint32 addr, kal_uint32 para)
{
    char puSendCmd[3] = {(char)(addr >> 8) , (char)(addr & 0xFF) ,(char)(para & 0xFF)};
	
	iWriteRegI2C(puSendCmd , 3,OV5647tMIPI_WRITE_ID);

}
kal_uint16 OV5647tMIPI_read_cmos_sensor(kal_uint32 addr)
{
	kal_uint16 get_byte=0;
    char puSendCmd[2] = {(char)(addr >> 8) , (char)(addr & 0xFF) };
	iReadRegI2C(puSendCmd , 2, (u8*)&get_byte,1,OV5647tMIPI_WRITE_ID);
    return get_byte;
}

#if defined(OV5647tMIPI_USE_OTP)

#if defined(OV5647tMIPI_USE_AWB_OTP)
//index:index of otp group.(0,1,2)
//return:	0:group index is empty.
//		1.group index has invalid data
	//		2.group index has valid data
kal_uint16 OV5647tMIPI_check_otp_wb(kal_uint16 index)
{
	kal_uint16 temp,flag;
	kal_uint32 address;

       OV5647tMIPI_write_cmos_sensor(0x0100, 0x01);	
	 mdelay(1);  
	//read otp into buffer
	OV5647tMIPI_write_cmos_sensor(0x3d21,0x01);

	//read flag
	address = 0x3d05+index*9;
	flag = OV5647tMIPI_read_cmos_sensor(address);

	//SENSORDB("OV5647tMIPI_check_otp_wb=%d\r\n",flag);

	//do not clear OTP buffer
	//clear otp buffer
	//for(temp=0;temp<32;temp++){
	//	OV5647tMIPI_write_cmos_sensor(0x3d00+temp,0x00);
	//}
	
	
	//disable otp read
	OV5647tMIPI_write_cmos_sensor(0x3d21,0x0);
	OV5647tMIPI_write_cmos_sensor(0x0100, 0x00);	

	if(NULL == flag)
		{
			
			SENSORDB("[OV5647tMIPI_check_otp_awb]index[%x]read flag[%x][0]\n",index,flag);
			return 0;
			
		}
	else if(!(flag&0x80) && (flag&0x7f))
		{
			SENSORDB("[OV5647tMIPI_check_otp_awb]index[%x]read flag[%x][2]\n",index,flag);
			return 2;
		}
	else
		{
			SENSORDB("[OV5647tMIPI_check_otp_awb]index[%x]read flag[%x][1]\n",index,flag);
		    return 1;
		}
	
}

//index:index of otp group.(0,1,2)
//return: 0
kal_uint16 OV5647tMIPI_read_otp_wb(kal_uint16 index, struct OV5647tMIPI_otp_struct *otp)
{
	kal_uint16 temp;
	kal_uint32 address;
       OV5647tMIPI_write_cmos_sensor(0x0100, 0x01);	
	 mdelay(1);  
	//read otp into buffer
	OV5647tMIPI_write_cmos_sensor(0x3d21,0x01);

	address = 0x3d05 +index*9;

	////4 modified the start address
	//address = 0x05 + index*9 +1;

	otp->customer_id = (OV5647tMIPI_read_cmos_sensor(address)&0x7f);
	SENSORDB("[OV5647tMIPI_read_otp_wb]address[%x]module_integrator_id[%x]\n",address,otp->customer_id);
	
	otp->module_integrator_id = OV5647tMIPI_read_cmos_sensor(address);
	SENSORDB("[OV5647tMIPI_read_otp_wb]address[%x]module_integrator_id[%x]\n",address,otp->module_integrator_id);
	
	otp->lens_id = OV5647tMIPI_read_cmos_sensor(address+1);
	SENSORDB("[OV5647tMIPI_read_otp_wb]address[%x]lens_id[%x]\n",address,otp->lens_id);
	
	otp->rg_ratio = OV5647tMIPI_read_cmos_sensor(address+2);
	SENSORDB("[OV5647tMIPI_read_otp_wb]address[%x]rg_ratio[%x]\n",address,otp->rg_ratio);

	otp->bg_ratio = OV5647tMIPI_read_cmos_sensor(address+3);
	SENSORDB("[OV5647tMIPI_read_otp_wb]address[%x]bg_ratio[%x]\n",address,otp->bg_ratio);

	otp->user_data[0] = OV5647tMIPI_read_cmos_sensor(address+4);
	SENSORDB("[OV5647tMIPI_read_otp_wb]address[%x]user_data[0][%x]\n",address,otp->user_data[0]);

	otp->user_data[1] = OV5647tMIPI_read_cmos_sensor(address+5);
	SENSORDB("[OV5647tMIPI_read_otp_wb]address[%x]user_data[1][%x]\n",address,otp->user_data[1]);

	otp->user_data[2] = OV5647tMIPI_read_cmos_sensor(address+6);	
	SENSORDB("[OV5647tMIPI_read_otp_wb]address[%x]user_data[2][%x]\n",address,otp->user_data[2]);

	otp->user_data[3] = OV5647tMIPI_read_cmos_sensor(address+7);	
	SENSORDB("[OV5647tMIPI_read_otp_wb]address[%x]user_data[3][%x]\n",address,otp->user_data[3]);

	otp->user_data[4] = OV5647tMIPI_read_cmos_sensor(address+8);	
	SENSORDB("[OV5647tMIPI_read_otp_wb]address[%x]user_data[3][%x]\n",address,otp->user_data[3]);

	//disable otp read
	OV5647tMIPI_write_cmos_sensor(0x3d21,00);
	
       OV5647tMIPI_write_cmos_sensor(0x0100, 0x00);	
	
	//do not clear OTP buffer
		//clear otp buffer
	//for(temp=0;temp<32;temp++){
	//	OV5647tMIPI_write_cmos_sensor(0x3d00+temp,0x00);
	//}

	return 0;
	
	
}

//R_gain: red gain of sensor AWB, 0x400 = 1
//G_gain: green gain of sensor AWB, 0x400 = 1
//B_gain: blue gain of sensor AWB, 0x400 = 1
//reutrn 0
kal_uint16 OV5647tMIPI_update_wb_gain(kal_uint32 R_gain, kal_uint32 G_gain, kal_uint32 B_gain)
{

    SENSORDB("[OV5647tMIPI_update_wb_gain]R_gain[%x]G_gain[%x]B_gain[%x]\n",R_gain,G_gain,B_gain);

	if(R_gain > 0x400)
		{
			OV5647tMIPI_write_cmos_sensor(0x5186,R_gain >> 8);
			OV5647tMIPI_write_cmos_sensor(0x5187,(R_gain&0x00ff));
		}
	if(G_gain > 0x400)
		{
			OV5647tMIPI_write_cmos_sensor(0x5188,G_gain >> 8);
			OV5647tMIPI_write_cmos_sensor(0x5189,(G_gain&0x00ff));
		}
	if(B_gain >0x400)
		{
			OV5647tMIPI_write_cmos_sensor(0x518a,B_gain >> 8);
			OV5647tMIPI_write_cmos_sensor(0x518b,(B_gain&0x00ff));
		}
	return 0;
}

//R/G and B/G ratio of typical camera module is defined here

kal_uint32 tRG_Ratio_typical = RG_TYPICAL;
kal_uint32 tBG_Ratio_typical = BG_TYPICAL;

//call this function after OV5647tMIPI initialization
//return value:	0 update success
//				1 no 	OTP

kal_uint16 OV5647tMIPI_update_wb_register_from_otp(void)
{
	kal_uint16 temp, i, otp_index;
	struct OV5647tMIPI_otp_struct current_otp;
	kal_uint32 R_gain, B_gain, G_gain, G_gain_R,G_gain_B;

	SENSORDB("OV5647tMIPI_update_wb_register_from_otp\n");

	//update white balance setting from OTP
	//check first wb OTP with valid OTP
	for(i = 0; i < 3; i++)
		{
			temp = OV5647tMIPI_check_otp_wb(i);
			if(temp == 2)
				{
					otp_index = i;
					break;
				}
		}
	if( 3 == i)
		{
		 	SENSORDB("[OV5647tMIPI_update_wb_register_from_otp]no valid wb OTP data!\r\n");
			return 1;
		}
	OV5647tMIPI_read_otp_wb(otp_index,&current_otp);

	//calculate gain
	//0x400 = 1x gain
	if(current_otp.bg_ratio < tBG_Ratio_typical)
		{
			if(current_otp.rg_ratio < tRG_Ratio_typical)
				{
					//current_opt.bg_ratio < tBG_Ratio_typical &&
					//cuttent_otp.rg < tRG_Ratio_typical

					G_gain = 0x400;
					B_gain = 0x400 * tBG_Ratio_typical / current_otp.bg_ratio;
					R_gain = 0x400 * tRG_Ratio_typical / current_otp.rg_ratio;
				}
			else
				{
					//current_otp.bg_ratio < tBG_Ratio_typical &&
			        //current_otp.rg_ratio >= tRG_Ratio_typical
			        R_gain = 0x400;
					G_gain = 0x400 * current_otp.rg_ratio / tRG_Ratio_typical;
					B_gain = G_gain * tBG_Ratio_typical / current_otp.bg_ratio;
					
			        
				}
		}
	else
		{
			if(current_otp.rg_ratio < tRG_Ratio_typical)
				{
					//current_otp.bg_ratio >= tBG_Ratio_typical &&
			        //current_otp.rg_ratio < tRG_Ratio_typical
			        B_gain = 0x400;
					G_gain = 0x400 * current_otp.bg_ratio / tBG_Ratio_typical;
					R_gain = G_gain * tRG_Ratio_typical / current_otp.rg_ratio;
					
				}
			else
				{
					//current_otp.bg_ratio >= tBG_Ratio_typical &&
			        //current_otp.rg_ratio >= tRG_Ratio_typical
			        G_gain_B = 0x400*current_otp.bg_ratio / tBG_Ratio_typical;
				    G_gain_R = 0x400*current_otp.rg_ratio / tRG_Ratio_typical;
					
					if(G_gain_B > G_gain_R)
						{
							B_gain = 0x400;
							G_gain = G_gain_B;
							R_gain = G_gain * tRG_Ratio_typical / current_otp.rg_ratio;
						}
					else

						{
							R_gain = 0x400;
							G_gain = G_gain_R;
							B_gain = G_gain * tBG_Ratio_typical / current_otp.bg_ratio;
						}
			        
				}
			
		}
	//write sensor wb gain to register
	OV5647tMIPI_update_wb_gain(R_gain,G_gain,B_gain);

	//success
	return 0;
}

#endif

#if defined(OV5647tMIPI_USE_LENC_OTP)    //copy form OV5650 LENC OTP

//index:index of otp group.(0,1,2)
//return:	0.group index is empty.
//		1.group index has invalid data
//		2.group index has valid data

kal_uint16 OV5647tMIPI_check_otp_lenc(kal_uint16 index)
{
   kal_uint16 temp,flag;
   kal_uint32 address;

   address = 0x20 + index*71;
   OV5647tMIPI_write_cmos_sensor(0x3d00,address);
   
   flag = OV5647tMIPI_read_cmos_sensor(0x3d04);
   flag = flag & 0xc0;

   OV5647tMIPI_write_cmos_sensor(0x3d00,0);

   if(NULL == flag)
   	{
   		SENSORDB("[OV5647tMIPI_check_otp_lenc]index[%x]read flag[%x][0]\n",index,flag);
   	    return 0;
   	}
   else if(0x40 == flag)
   	{
   		SENSORDB("[OV5647tMIPI_check_otp_lenc]index[%x]read flag[%x][2]\n",index,flag);
   	    return 2;
   	}
   else
   	{
   		SENSORDB("[OV5647tMIPI_check_otp_lenc]index[%x]read flag[%x][1]\n",index,flag);
		return 1;
   	}
}


kal_uint16 OV5647tMIPI_read_otp_lenc(kal_uint16 index,struct OV5647tMIPI_otp_struct *otp)
{
	kal_uint16 bank,temp1,temp2,i;
	kal_uint32 address;

	address = 0x20 + index*71 +1;
	
	

	//read lenc_g
	for(i = 0; i < 36; i++)
		{
			OV5647tMIPI_write_cmos_sensor(0x3d00,address);
			otp->lenc_g[i] = OV5647tMIPI_read_cmos_sensor(0x3d04);
			
			SENSORDB("[OV5647tMIPI_read_otp_lenc]address[%x]otp->lenc_g[%d][%x]\n",address,i,otp->lenc_g[i]);
			address++;
		}
	//read lenc_b
	for(i = 0; i <8; i++)
		{
			OV5647tMIPI_write_cmos_sensor(0x3d00,address);
			temp1 = OV5647tMIPI_read_cmos_sensor(0x3d04);

			SENSORDB("[OV5647tMIPI_read_otp_lenc]address[%x]temp1[%x]\n",address,temp1);

			address++;
			OV5647tMIPI_write_cmos_sensor(0x3d00,address);
			temp2 = OV5647tMIPI_read_cmos_sensor(0x3d04);

			SENSORDB("[OV5647tMIPI_read_otp_lenc]address[%x]temp2[%x]\n",address,temp2);

			address++;

			otp->lenc_b[i*3] = temp1&0x1f;
			otp->lenc_b[i*3+1] = temp2&0x1f;
			otp->lenc_b[i*3+2] = (((temp1 >> 2)&0x18) | (temp2 >> 5));
		}
	OV5647tMIPI_write_cmos_sensor(0x3d00,address);
	temp1 = OV5647tMIPI_read_cmos_sensor(0x3d04);
	SENSORDB("[OV5647tMIPI_read_otp_lenc]address[%x]temp1[%x]\n",address,temp1);
	otp->lenc_b[24] = temp1&0x1f;
	address++;

	//read lenc_r
	for(i = 0; i <8; i++)
		{
		   OV5647tMIPI_write_cmos_sensor(0x3d00,address);
		   temp1 = OV5647tMIPI_read_cmos_sensor(0x3d04);

		   SENSORDB("[OV5647tMIPI_read_otp_lenc]address[%x]temp1[%x]\n",address,temp1);
		   
		   address++;

		   OV5647tMIPI_write_cmos_sensor(0x3d00,address);
		   temp2 = OV5647tMIPI_read_cmos_sensor(0x3d04);
		   
			
		   SENSORDB("[OV5647tMIPI_read_otp_lenc]address[%x]temp2[%x]\n",address,temp2);
		   address++;

		   otp->lenc_r[i*3] = temp1&0x1f;
		   otp->lenc_r[i*3+1] = temp2&0x1f;
		   otp->lenc_r[i*3+2] = (((temp1 >> 2)&0x18) | (temp2 >>5));
		}
	OV5647tMIPI_write_cmos_sensor(0x3d00,address);
	temp1 = OV5647tMIPI_read_cmos_sensor(0x3d04);
	SENSORDB("[OV5647tMIPI_read_otp_lenc]address[%x]temp1[%x]\n",address,temp1);
	otp->lenc_r[24] = temp1 & 0x1f;

	OV5647tMIPI_write_cmos_sensor(0x3d00,0);

	return 0;
}


//return 0
kal_uint16 OV5647tMIPI_update_lenc(struct OV5647tMIPI_otp_struct *otp)
{
	kal_uint16 i, temp;
	//lenc g
	for(i = 0; i < 36; i++)
		{
		 	OV5647tMIPI_write_cmos_sensor(0x5800+i,otp->lenc_g[i]);
			
			SENSORDB("[OV5647tMIPI_update_lenc]otp->lenc_g[%d][%x]\n",i,otp->lenc_g[i]);
		}
	//lenc b
	for(i = 0; i < 25; i++)
		{
			OV5647tMIPI_write_cmos_sensor(0x5824+i,otp->lenc_b[i]);
			SENSORDB("[OV5647tMIPI_update_lenc]otp->lenc_b[%d][%x]\n",i,otp->lenc_b[i]);
		}
	//lenc r
	for(i = 0; i < 25; i++)
		{
			OV5647tMIPI_write_cmos_sensor(0x583d+i,otp->lenc_r[i]);
			SENSORDB("[OV5647tMIPI_update_lenc]otp->lenc_r[%d][%x]\n",i,otp->lenc_r[i]);
		}
	return 0;
}

//call this function after OV5647tMIPI initialization
//return value:	0 update success
//				1 no otp

kal_uint16 OV5647tMIPI_update_lenc_register_from_otp(void)
{
	kal_uint16 temp,i,otp_index;
    struct OV5647tMIPI_otp_struct current_otp;

	for(i = 0; i < 3; i++)
		{
			temp = OV5647tMIPI_check_otp_lenc(i);
			if(2 == temp)
				{
					otp_index = i;
					break;
				}
		}
	if(3 == i)
		{
		 	SENSORDB("[OV5647tMIPI_update_lenc_register_from_otp]no valid wb OTP data!\r\n");
			return 1;
		}
	OV5647tMIPI_read_otp_lenc(otp_index,&current_otp);

	OV5647tMIPI_update_lenc(&current_otp);

	//at last should enable the shading enable register
	OV5647tMIPI_read_cmos_sensor(0x5000);
	temp |= 0x80;
	OV5647tMIPI_write_cmos_sensor(0x5000,temp);
	

	//success
	return 0;
}

#endif


#endif



static OV5647tMIPI_sensor_struct OV5647tMIPI_sensor =
{
  .eng =
  {
    .reg = OV5647tMIPI_CAMERA_SENSOR_REG_DEFAULT_VALUE,
    .cct = OV5647tMIPI_CAMERA_SENSOR_CCT_DEFAULT_VALUE,
  },
  .eng_info =
  {
    .SensorId = 128,
    .SensorType = CMOS_SENSOR,
    .SensorOutputDataFormat = OV5647tMIPI_COLOR_FORMAT,
  },
  .shutter = 0x20,  
  .gain = 0x20,
  .pclk = OV5647tMIPI_PREVIEW_CLK,
  .frame_height = OV5647tMIPI_PV_PERIOD_LINE_NUMS,
  .line_length = OV5647tMIPI_PV_PERIOD_PIXEL_NUMS,
};

static void OV5647tMIPI_Write_Shutter(kal_uint16 iShutter)
{
	kal_uint16 extra_line = 0;
	
    /* 0x3500,0x3501, 0x3502 will increase VBLANK to get exposure larger than frame exposure */
    /* AE doesn't update sensor gain at capture mode, thus extra exposure lines must be updated here. */
	if (!iShutter) iShutter = 1; /* avoid 0 */

	printk("OV5647tMIPI_Write_Shutter :0x%x\n",iShutter);

	if(OV5647tMIPIAutoFlicKerMode){
		if(OV5647tMIPI_sensor.video_mode == KAL_FALSE){
		   if(tCurrentScenarioId== MSDK_SCENARIO_ID_CAMERA_ZSD)
		   {
		      //Change frame 14.7fps ~ 14.9fps to do auto flick
		      OV5647tMIPISetMaxFrameRate(148);
		   }
		   else
		   {
		      //Change frame 29.5fps ~ 29.8fps to do auto flick
					OV5647tMIPISetMaxFrameRate(296);
			 }
		}
	}
	
	if(iShutter > OV5647tMIPI_sensor.frame_height)
	{
		extra_line = iShutter - OV5647tMIPI_sensor.frame_height;
		if((0x49a<iShutter)&&(0x49f>iShutter))
			extra_line+=20;
	}
if(iShutter< (OV5647tMIPI_sensor.pclk/(OV5647tMIPI_sensor.line_length+50)/100))
{
     OV5647tMIPISetMaxFrameRate(250);
     extra_line= (OV5647tMIPI_sensor.pclk *10 / (OV5647tMIPI_sensor.line_length * 250));
   //SENSORDB("[luoxiaofei realtime_fp=250][height:%d]",OV8830_sensor.frame_height);
}
//add end
	// Update Extra shutter
	OV5647tMIPI_write_cmos_sensor(0x350c, (extra_line >> 8) & 0xFF);	
	OV5647tMIPI_write_cmos_sensor(0x350d, (extra_line) & 0xFF);

	//Update Shutter
	OV5647tMIPI_write_cmos_sensor(0x3500, (iShutter >> 12) & 0xF);
	OV5647tMIPI_write_cmos_sensor(0x3501, (iShutter >> 4) & 0xFF);	
	OV5647tMIPI_write_cmos_sensor(0x3502, (iShutter << 4) & 0xFF);

	#if 0

    printk("line_length[0x380c][%x]\n",OV5647tMIPI_read_cmos_sensor(0x380c));
	printk("line_length[0x380d][%x]\n",OV5647tMIPI_read_cmos_sensor(0x380d));
	printk("frame_height[0x380e][%x]\n",OV5647tMIPI_read_cmos_sensor(0x380e));
	printk("frame_height[0x380f][%x]\n",OV5647tMIPI_read_cmos_sensor(0x380f));


	printk("[OV5647t_Write_Shutter]OV5647t_sensor.pclk=%d\n",OV5647tMIPI_sensor.pclk);
	printk("[OV5647t_Write_Shutter]OV5647t_sensor.line_length=%d\n",OV5647tMIPI_sensor.line_length);
	printk("[OV5647t_Write_Shutter]OV5647t_sensor.frame_height=%d\n",OV5647tMIPI_sensor.frame_height);
	printk("[OV5647t_Write_Shutter]iShutter=%d,extra_line=%d\n",iShutter,extra_line);

		{

	kal_uint16 fps;

	if(extra_line >0)
		{
		 	fps= (10 * OV5647tMIPI_sensor.pclk) / iShutter / OV5647tMIPI_sensor.line_length;		 
		}
	else
		{
			fps= (10 * OV5647tMIPI_sensor.pclk) / OV5647tMIPI_sensor.frame_height / OV5647tMIPI_sensor.line_length;	
		}
	printk("[OV5647t_Write_Shutter]fps=%d",fps);
			}



	#endif
}   /*  OV5647tMIPI_Write_Shutter    */

static void OV5647tMIPI_Set_Dummy(const kal_uint16 iPixels, const kal_uint16 iLines)
{
	kal_uint16 hactive, vactive, line_length, frame_height;
	printk("[soso][OV5647tMIPI_Set_Dummy]iPixels=%d,iLines=%d\n",iPixels,iLines);

	if (OV5647tMIPI_sensor.pv_mode){
	  line_length = OV5647tMIPI_PV_PERIOD_PIXEL_NUMS + iPixels;
	  frame_height = OV5647tMIPI_PV_PERIOD_LINE_NUMS + iLines;
	}else{
	  line_length = OV5647tMIPI_FULL_PERIOD_PIXEL_NUMS + iPixels;
	  frame_height = OV5647tMIPI_FULL_PERIOD_LINE_NUMS + iLines;
	}
	
	if ((line_length >= 0x1FFF)||(frame_height >= 0xFFF))
		return ;
	
    /*  Add dummy pixels: */
    /* 0x380c [0:4], 0x380d defines the PCLKs in one line of OV5647tMIPI  */  
    /* Add dummy lines:*/
    /* 0x380e [0:1], 0x380f defines total lines in one frame of OV5647tMIPI */
    OV5647tMIPI_write_cmos_sensor(0x380c, line_length >> 8);
    OV5647tMIPI_write_cmos_sensor(0x380d, line_length & 0xFF);
    OV5647tMIPI_write_cmos_sensor(0x380e, frame_height >> 8);
    OV5647tMIPI_write_cmos_sensor(0x380f, frame_height & 0xFF);
	
}   /*  OV5647tMIPI_Set_Dummy    */

/*Avoid Folat, frame rate =10 * u2FrameRate */
UINT32 OV5647tMIPISetMaxFrameRate(UINT16 u2FrameRate)
{
	kal_int16 dummy_line;
	kal_uint16 FrameHeight = OV5647tMIPI_sensor.frame_height;
	  unsigned long flags;
		
	printk("[soso][OV5647tMIPISetMaxFrameRate]u2FrameRate=%d\n",u2FrameRate);

	//dummy_line = OV5647tMIPI_sensor.pclk / u2FrameRate / OV5647tMIPI_PV_PERIOD_PIXEL_NUMS - OV5647tMIPI_PV_PERIOD_LINE_NUMS;
	FrameHeight= (10 * OV5647tMIPI_sensor.pclk) / u2FrameRate / OV5647tMIPI_sensor.line_length;
	//if(FrameHeight!=OV5647tMIPI_sensor.frame_height){
	    spin_lock_irqsave(&ov5647tmipi_drv_lock,flags);
		OV5647tMIPI_sensor.frame_height = FrameHeight;
		spin_unlock_irqrestore(&ov5647tmipi_drv_lock,flags);
		if(tCurrentScenarioId==MSDK_SCENARIO_ID_CAMERA_ZSD)
		    dummy_line = FrameHeight - OV5647tMIPI_FULL_PERIOD_LINE_NUMS;
		else
			dummy_line = FrameHeight - OV5647tMIPI_PV_PERIOD_LINE_NUMS;
	    /* to fix VSYNC, to fix frame rate */
		OV5647tMIPI_Set_Dummy(0, dummy_line); /* modify dummy_pixel must gen AE table again */
	//}
}

/*************************************************************************
* FUNCTION
*	OV5647tMIPI_SetShutter
*
* DESCRIPTION
*	This function set e-shutter of OV5647tMIPI to change exposure time.
*
* PARAMETERS
*   iShutter : exposured lines
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
void set_OV5647tMIPI_shutter(kal_uint16 iShutter)
{
    unsigned long flags;
    spin_lock_irqsave(&ov5647tmipi_drv_lock,flags);
	OV5647tMIPI_sensor.shutter = iShutter;
	spin_unlock_irqrestore(&ov5647tmipi_drv_lock,flags);
    OV5647tMIPI_Write_Shutter(iShutter);
}   /*  Set_OV5647tMIPI_Shutter */

static kal_uint16 OV5647tMIPIReg2Gain(const kal_uint8 iReg)
{
    kal_uint16 iGain ;   
    /* Range: 1x to 32x */
	iGain = (iReg >> 4) * BASEGAIN + (iReg & 0xF) * BASEGAIN / 16; 
    return iGain ;
}

 kal_uint8 OV5647tMIPIGain2Reg(const kal_uint16 iGain)
{
    kal_uint16 iReg = 0x00;
	iReg = ((iGain / BASEGAIN) << 4) + ((iGain % BASEGAIN) * 16 / BASEGAIN);
	iReg = iReg & 0xFF;
    return (kal_uint8)iReg;
}

/*************************************************************************
* FUNCTION
*	OV5647tMIPI_SetGain
*
* DESCRIPTION
*	This function is to set global gain to sensor.
*
* PARAMETERS
*   iGain : sensor global gain(base: 0x40)
*
* RETURNS
*	the actually gain set to sensor.
*
* GLOBALS AFFECTED
*
*************************************************************************/
kal_uint16 OV5647tMIPI_SetGain(kal_uint16 iGain)
{
	kal_uint8 iReg;
	//V5647t_sensor.gain = iGain;
	/* 0x350a[0:1], 0x350b AGC real gain */
	/* [0:3] = N meams N /16 X  */
	/* [4:9] = M meams M X  */
	/* Total gain = M + N /16 X */
	printk("OV5747MIPI_SetGain::%x \n",iGain);
	iReg = OV5647tMIPIGain2Reg(iGain);
	printk("OV5647tMIPI_SetGain,iReg:%x",iReg);
	if (iReg < 0x10) iReg = 0x10;
	//OV5647tMIPI_write_cmos_sensor(0x350a, iReg);
	OV5647tMIPI_write_cmos_sensor(0x350b, iReg);
	return iGain;
}
/*************************************************************************
* FUNCTION
*	OV5647tMIPI_NightMode
*
* DESCRIPTION
*	This function night mode of OV5647tMIPI.
*
* PARAMETERS
*	bEnable: KAL_TRUE -> enable night mode, otherwise, disable night mode
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
void OV5647tMIPI_night_mode(kal_bool enable)
{
/*No Need to implement this function*/
#if 0 
	const kal_uint16 dummy_pixel = OV5647tMIPI_sensor.line_length - OV5647tMIPI_PV_PERIOD_PIXEL_NUMS;
	const kal_uint16 pv_min_fps =  enable ? OV5647tMIPI_sensor.night_fps : OV5647tMIPI_sensor.normal_fps;
	kal_uint16 dummy_line = OV5647tMIPI_sensor.frame_height - OV5647tMIPI_PV_PERIOD_LINE_NUMS;
	kal_uint16 max_exposure_lines;
	
	printk("[soso][OV5647tMIPI_night_mode]enable=%d",enable);
	if (!OV5647tMIPI_sensor.video_mode) return;
	max_exposure_lines = OV5647tMIPI_sensor.pclk * OV5647tMIPI_FPS(1) / (pv_min_fps * OV5647tMIPI_sensor.line_length);
	if (max_exposure_lines > OV5647tMIPI_sensor.frame_height) /* fix max frame rate, AE table will fix min frame rate */
//	{
//	  dummy_line = max_exposure_lines - OV5647tMIPI_PV_PERIOD_LINE_NUMS;
//	  OV5647tMIPI_Set_Dummy(dummy_pixel, dummy_line);
//	}
#endif
}   /*  OV5647tMIPI_NightMode    */


/* write camera_para to sensor register */
static void OV5647tMIPI_camera_para_to_sensor(void)
{
  kal_uint32 i;
#ifdef OV5647tMIPI_DRIVER_TRACE
	 SENSORDB("OV5647tMIPI_camera_para_to_sensor\n");
#endif
  for (i = 0; 0xFFFFFFFF != OV5647tMIPI_sensor.eng.reg[i].Addr; i++)
  {
    OV5647tMIPI_write_cmos_sensor(OV5647tMIPI_sensor.eng.reg[i].Addr, OV5647tMIPI_sensor.eng.reg[i].Para);
  }
  for (i = OV5647tMIPI_FACTORY_START_ADDR; 0xFFFFFFFF != OV5647tMIPI_sensor.eng.reg[i].Addr; i++)
  {
    OV5647tMIPI_write_cmos_sensor(OV5647tMIPI_sensor.eng.reg[i].Addr, OV5647tMIPI_sensor.eng.reg[i].Para);
  }
  OV5647tMIPI_SetGain(OV5647tMIPI_sensor.gain); /* update gain */
}

/* update camera_para from sensor register */
static void OV5647tMIPI_sensor_to_camera_para(void)
{
  kal_uint32 i,temp_data;
#ifdef OV5647tMIPI_DRIVER_TRACE
   SENSORDB("OV5647tMIPI_sensor_to_camera_para\n");
#endif
  for (i = 0; 0xFFFFFFFF != OV5647tMIPI_sensor.eng.reg[i].Addr; i++)
  {
     temp_data =OV5647tMIPI_read_cmos_sensor(OV5647tMIPI_sensor.eng.reg[i].Addr);
	 
     spin_lock(&ov5647tmipi_drv_lock);
	 OV5647tMIPI_sensor.eng.reg[i].Para = temp_data;
     spin_unlock(&ov5647tmipi_drv_lock);
  }
  for (i = OV5647tMIPI_FACTORY_START_ADDR; 0xFFFFFFFF != OV5647tMIPI_sensor.eng.reg[i].Addr; i++)
  {
    temp_data =OV5647tMIPI_read_cmos_sensor(OV5647tMIPI_sensor.eng.reg[i].Addr);
	
	spin_lock(&ov5647tmipi_drv_lock);
	OV5647tMIPI_sensor.eng.reg[i].Para = temp_data;
	 spin_unlock(&ov5647tmipi_drv_lock);
  }
}

/* ------------------------ Engineer mode ------------------------ */
inline static void OV5647tMIPI_get_sensor_group_count(kal_int32 *sensor_count_ptr)
{
#ifdef OV5647tMIPI_DRIVER_TRACE
   SENSORDB("OV5647tMIPI_get_sensor_group_count\n");
#endif
  *sensor_count_ptr = OV5647tMIPI_GROUP_TOTAL_NUMS;
}

inline static void OV5647tMIPI_get_sensor_group_info(MSDK_SENSOR_GROUP_INFO_STRUCT *para)
{
#ifdef OV5647tMIPI_DRIVER_TRACE
   SENSORDB("OV5647tMIPI_get_sensor_group_info\n");
#endif
  switch (para->GroupIdx)
  {
  case OV5647tMIPI_PRE_GAIN:
    sprintf(para->GroupNamePtr, "CCT");
    para->ItemCount = 5;
    break;
  case OV5647tMIPI_CMMCLK_CURRENT:
    sprintf(para->GroupNamePtr, "CMMCLK Current");
    para->ItemCount = 1;
    break;
  case OV5647tMIPI_FRAME_RATE_LIMITATION:
    sprintf(para->GroupNamePtr, "Frame Rate Limitation");
    para->ItemCount = 2;
    break;
  case OV5647tMIPI_REGISTER_EDITOR:
    sprintf(para->GroupNamePtr, "Register Editor");
    para->ItemCount = 2;
    break;
  default:
    ASSERT(0);
  }
}

inline static void OV5647tMIPI_get_sensor_item_info(MSDK_SENSOR_ITEM_INFO_STRUCT *para)
{

  const static kal_char *cct_item_name[] = {"SENSOR_BASEGAIN", "Pregain-R", "Pregain-Gr", "Pregain-Gb", "Pregain-B"};
  const static kal_char *editer_item_name[] = {"REG addr", "REG value"};
  
#ifdef OV5647tMIPI_DRIVER_TRACE
	 SENSORDB("OV5647tMIPI_get_sensor_item_info\n");
#endif
  switch (para->GroupIdx)
  {
  case OV5647tMIPI_PRE_GAIN:
    switch (para->ItemIdx)
    {
    case OV5647tMIPI_SENSOR_BASEGAIN:
    case OV5647tMIPI_PRE_GAIN_R_INDEX:
    case OV5647tMIPI_PRE_GAIN_Gr_INDEX:
    case OV5647tMIPI_PRE_GAIN_Gb_INDEX:
    case OV5647tMIPI_PRE_GAIN_B_INDEX:
      break;
    default:
      ASSERT(0);
    }
    sprintf(para->ItemNamePtr, cct_item_name[para->ItemIdx - OV5647tMIPI_SENSOR_BASEGAIN]);
    para->ItemValue = OV5647tMIPI_sensor.eng.cct[para->ItemIdx].Para * 1000 / BASEGAIN;
    para->IsTrueFalse = para->IsReadOnly = para->IsNeedRestart = KAL_FALSE;
    para->Min = OV5647tMIPI_MIN_ANALOG_GAIN * 1000;
    para->Max = OV5647tMIPI_MAX_ANALOG_GAIN * 1000;
    break;
  case OV5647tMIPI_CMMCLK_CURRENT:
    switch (para->ItemIdx)
    {
    case 0:
      sprintf(para->ItemNamePtr, "Drv Cur[2,4,6,8]mA");
      switch (OV5647tMIPI_sensor.eng.reg[OV5647tMIPI_CMMCLK_CURRENT_INDEX].Para)
      {
      case ISP_DRIVING_2MA:
        para->ItemValue = 2;
        break;
      case ISP_DRIVING_4MA:
        para->ItemValue = 4;
        break;
      case ISP_DRIVING_6MA:
        para->ItemValue = 6;
        break;
      case ISP_DRIVING_8MA:
        para->ItemValue = 8;
        break;
      default:
        ASSERT(0);
      }
      para->IsTrueFalse = para->IsReadOnly = KAL_FALSE;
      para->IsNeedRestart = KAL_TRUE;
      para->Min = 2;
      para->Max = 8;
      break;
    default:
      ASSERT(0);
    }
    break;
  case OV5647tMIPI_FRAME_RATE_LIMITATION:
    switch (para->ItemIdx)
    {
    case 0:
      sprintf(para->ItemNamePtr, "Max Exposure Lines");
      para->ItemValue = 5998;
      break;
    case 1:
      sprintf(para->ItemNamePtr, "Min Frame Rate");
      para->ItemValue = 5;
      break;
    default:
      ASSERT(0);
    }
    para->IsTrueFalse = para->IsNeedRestart = KAL_FALSE;
    para->IsReadOnly = KAL_TRUE;
    para->Min = para->Max = 0;
    break;
  case OV5647tMIPI_REGISTER_EDITOR:
    switch (para->ItemIdx)
    {
    case 0:
    case 1:
      sprintf(para->ItemNamePtr, editer_item_name[para->ItemIdx]);
      para->ItemValue = 0;
      para->IsTrueFalse = para->IsReadOnly = para->IsNeedRestart = KAL_FALSE;
      para->Min = 0;
      para->Max = (para->ItemIdx == 0 ? 0xFFFF : 0xFF);
      break;
    default:
      ASSERT(0);
    }
    break;
  default:
    ASSERT(0);
  }
}

inline static kal_bool OV5647tMIPI_set_sensor_item_info(MSDK_SENSOR_ITEM_INFO_STRUCT *para)
{
  kal_uint16 temp_para;
#ifdef OV5647tMIPI_DRIVER_TRACE
   SENSORDB("OV5647tMIPI_set_sensor_item_info\n");
#endif
  switch (para->GroupIdx)
  {
  case OV5647tMIPI_PRE_GAIN:
    switch (para->ItemIdx)
    {
    case OV5647tMIPI_SENSOR_BASEGAIN:
    case OV5647tMIPI_PRE_GAIN_R_INDEX:
    case OV5647tMIPI_PRE_GAIN_Gr_INDEX:
    case OV5647tMIPI_PRE_GAIN_Gb_INDEX:
    case OV5647tMIPI_PRE_GAIN_B_INDEX:
	  spin_lock(&ov5647tmipi_drv_lock);
      OV5647tMIPI_sensor.eng.cct[para->ItemIdx].Para = para->ItemValue * BASEGAIN / 1000;
      spin_unlock(&ov5647tmipi_drv_lock);
	  OV5647tMIPI_SetGain(OV5647tMIPI_sensor.gain); /* update gain */
      break;
    default:
      ASSERT(0);
    }
    break;
  case OV5647tMIPI_CMMCLK_CURRENT:
    switch (para->ItemIdx)
    {
    case 0:
      switch (para->ItemValue)
      	{
      case 2:
        temp_para = ISP_DRIVING_2MA;
        break;
      case 3:
      case 4:
        temp_para = ISP_DRIVING_4MA;
        break;
      case 5:
      case 6:
        temp_para = ISP_DRIVING_6MA;
        break;
      default:
        temp_para = ISP_DRIVING_8MA;
        break;
      }
	  spin_lock(&ov5647tmipi_drv_lock);
      //OV5647tMIPI_set_isp_driving_current(temp_para);
      OV5647tMIPI_sensor.eng.reg[OV5647tMIPI_CMMCLK_CURRENT_INDEX].Para = temp_para;
	  spin_unlock(&ov5647tmipi_drv_lock);
      break;
    default:
      ASSERT(0);
    }
    break;
  case OV5647tMIPI_FRAME_RATE_LIMITATION:
    ASSERT(0);
    break;
  case OV5647tMIPI_REGISTER_EDITOR:
    switch (para->ItemIdx)
    {
      static kal_uint32 fac_sensor_reg;
    case 0:
      if (para->ItemValue < 0 || para->ItemValue > 0xFFFF) return KAL_FALSE;
      fac_sensor_reg = para->ItemValue;
      break;
    case 1:
      if (para->ItemValue < 0 || para->ItemValue > 0xFF) return KAL_FALSE;
      OV5647tMIPI_write_cmos_sensor(fac_sensor_reg, para->ItemValue);
      break;
    default:
      ASSERT(0);
    }
    break;
  default:
    ASSERT(0);
  }
  return KAL_TRUE;
}

static void OV5647tMIPI_Sensor_Init(void)
{
  #if 1
  //@@ global setting																  
	OV5647tMIPI_write_cmos_sensor(0x0100, 0x00);									  
	OV5647tMIPI_write_cmos_sensor(0x0103, 0x01);									  
	//(5ms)		
//	kal_sleep_task(2);
    mdelay(6);


	OV5647tMIPI_write_cmos_sensor(0x3035, 0x11); //  system clk div
	OV5647tMIPI_write_cmos_sensor(0x303c, 0x11); //PLL ctrol
	OV5647tMIPI_write_cmos_sensor(0x370c, 0x03); // no
	
	OV5647tMIPI_write_cmos_sensor(0x5000, 0x06); //isp control
    OV5647tMIPI_write_cmos_sensor(0x5003, 0x08);
	OV5647tMIPI_write_cmos_sensor(0x5a00, 0x08);
	
	OV5647tMIPI_write_cmos_sensor(0x3000, 0xff); //system control
	OV5647tMIPI_write_cmos_sensor(0x3001, 0xff);
    OV5647tMIPI_write_cmos_sensor(0x3002, 0xff);
	OV5647tMIPI_write_cmos_sensor(0x301d, 0xf0);
//	OV5647tMIPI_write_cmos_sensor(0x3a18, 0x00);
	//OV5647tMIPI_write_cmos_sensor(0x3a19, 0xf8);
	OV5647tMIPI_write_cmos_sensor(0x3a18, 0x01);
	OV5647tMIPI_write_cmos_sensor(0x3a19, 0xe0);
	
	
	OV5647tMIPI_write_cmos_sensor(0x3c01, 0x80);
	OV5647tMIPI_write_cmos_sensor(0x3b07, 0x0c);
	OV5647tMIPI_write_cmos_sensor(0x3708, 0x64);
	OV5647tMIPI_write_cmos_sensor(0x3630, 0x2e);
	OV5647tMIPI_write_cmos_sensor(0x3632, 0xe2);
	OV5647tMIPI_write_cmos_sensor(0x3633, 0x23);
	OV5647tMIPI_write_cmos_sensor(0x3634, 0x44);
	OV5647tMIPI_write_cmos_sensor(0x3620, 0x64);
	OV5647tMIPI_write_cmos_sensor(0x3621, 0xe0);
	OV5647tMIPI_write_cmos_sensor(0x3600, 0x37);
	OV5647tMIPI_write_cmos_sensor(0x3704, 0xa0);
	OV5647tMIPI_write_cmos_sensor(0x3703, 0x5a);
	OV5647tMIPI_write_cmos_sensor(0x3715, 0x78);
	OV5647tMIPI_write_cmos_sensor(0x3717, 0x01);
	OV5647tMIPI_write_cmos_sensor(0x3731, 0x02);
	OV5647tMIPI_write_cmos_sensor(0x370b, 0x60);
	OV5647tMIPI_write_cmos_sensor(0x3705, 0x1a);
	OV5647tMIPI_write_cmos_sensor(0x3f05, 0x02);
	OV5647tMIPI_write_cmos_sensor(0x3f06, 0x10);
	OV5647tMIPI_write_cmos_sensor(0x3f01, 0x0a);
	//OV5647tMIPI_write_cmos_sensor(0x3a08, 0x01);
	OV5647tMIPI_write_cmos_sensor(0x3a08, 0x00);
	OV5647tMIPI_write_cmos_sensor(0x3a0f, 0x58);
	OV5647tMIPI_write_cmos_sensor(0x3a10, 0x50);
	OV5647tMIPI_write_cmos_sensor(0x3a1b, 0x58);
	OV5647tMIPI_write_cmos_sensor(0x3a1e, 0x50);
	OV5647tMIPI_write_cmos_sensor(0x3a11, 0x60);
	OV5647tMIPI_write_cmos_sensor(0x3a1f, 0x28);
	
	OV5647tMIPI_write_cmos_sensor(0x4001, 0x02);  //blc
	OV5647tMIPI_write_cmos_sensor(0x4000, 0x09);

	OV5647tMIPI_write_cmos_sensor(0x3000, 0x00);
	OV5647tMIPI_write_cmos_sensor(0x3001, 0x00);
	OV5647tMIPI_write_cmos_sensor(0x3002, 0x00); //modify
	OV5647tMIPI_write_cmos_sensor(0x3017, 0xe0);
	OV5647tMIPI_write_cmos_sensor(0x301c, 0xfc);
    OV5647tMIPI_write_cmos_sensor(0x3636, 0x06);
	OV5647tMIPI_write_cmos_sensor(0x3016, 0x08);
	OV5647tMIPI_write_cmos_sensor(0x3827, 0xec);
	OV5647tMIPI_write_cmos_sensor(0x3018, 0x44);
    OV5647tMIPI_write_cmos_sensor(0x3035, 0x21);
	OV5647tMIPI_write_cmos_sensor(0x3106, 0xf5);
    OV5647tMIPI_write_cmos_sensor(0x3034, 0x1a);
    OV5647tMIPI_write_cmos_sensor(0x301c, 0xf8);
	
 
	
	OV5647tMIPI_write_cmos_sensor(0x3503, 0x07);
	OV5647tMIPI_write_cmos_sensor(0x3501, 0x3c);
	OV5647tMIPI_write_cmos_sensor(0x3502, 0x00);

	//OV5647tMIPI_write_cmos_sensor(0x3501, 0x10);
	//OV5647tMIPI_write_cmos_sensor(0x3502, 0x80);
	
	OV5647tMIPI_write_cmos_sensor(0x350a, 0x00);
	OV5647tMIPI_write_cmos_sensor(0x350b, 0x7f);
	OV5647tMIPI_write_cmos_sensor(0x5001, 0x01);
	OV5647tMIPI_write_cmos_sensor(0x5180, 0x08);
	OV5647tMIPI_write_cmos_sensor(0x5186, 0x04);
	OV5647tMIPI_write_cmos_sensor(0x5187, 0x00);
	OV5647tMIPI_write_cmos_sensor(0x5188, 0x04);
	OV5647tMIPI_write_cmos_sensor(0x5189, 0x00);
	OV5647tMIPI_write_cmos_sensor(0x518a, 0x04);
	OV5647tMIPI_write_cmos_sensor(0x518b, 0x00);
	OV5647tMIPI_write_cmos_sensor(0x5000, 0x06);

	OV5647tMIPI_write_cmos_sensor(0x4001, 0x02);		 //add							  
	OV5647tMIPI_write_cmos_sensor(0x4000, 0x09);		//add							  
	//OV5647tMIPI_write_cmos_sensor(0x0010, 0x01);	    //add
	OV5647tMIPI_write_cmos_sensor(0x0100,0x01);   //modify
	
	#else
	
	OV5647tMIPI_write_cmos_sensor(0x0100, 0x00);
	OV5647tMIPI_write_cmos_sensor(0x0103, 0x01);
	mdelay(5);	
	OV5647tMIPI_write_cmos_sensor(0x0100, 0x00);
	OV5647tMIPI_write_cmos_sensor(0x0100, 0x00);
	OV5647tMIPI_write_cmos_sensor(0x0100, 0x00);
	OV5647tMIPI_write_cmos_sensor(0x0100, 0x00);
	
	OV5647tMIPI_write_cmos_sensor(0x3011, 0x42);
	OV5647tMIPI_write_cmos_sensor(0x3013, 0x08);//0x04-->0x00-->0x08  Turn off internal LDO
	OV5647tMIPI_write_cmos_sensor(0x4708, 0x00);
	OV5647tMIPI_write_cmos_sensor(0x5000, 0x06);
	OV5647tMIPI_write_cmos_sensor(0x5003, 0x08);
	OV5647tMIPI_write_cmos_sensor(0x5a00, 0x08);
	OV5647tMIPI_write_cmos_sensor(0x3000, 0xff);
	OV5647tMIPI_write_cmos_sensor(0x3001, 0xff);
	OV5647tMIPI_write_cmos_sensor(0x3002, 0xff);
	OV5647tMIPI_write_cmos_sensor(0x3a18, 0x01);
	OV5647tMIPI_write_cmos_sensor(0x3a19, 0xe0);
	OV5647tMIPI_write_cmos_sensor(0x3c01, 0x80);
	OV5647tMIPI_write_cmos_sensor(0x3b07, 0x0c);
	OV5647tMIPI_write_cmos_sensor(0x3630, 0x2e);
	OV5647tMIPI_write_cmos_sensor(0x3632, 0xe2);
	OV5647tMIPI_write_cmos_sensor(0x3633, 0x23);
	OV5647tMIPI_write_cmos_sensor(0x3634, 0x44);
	OV5647tMIPI_write_cmos_sensor(0x3620, 0x64);
	OV5647tMIPI_write_cmos_sensor(0x3621, 0xe0);
	OV5647tMIPI_write_cmos_sensor(0x3600, 0x37);
	OV5647tMIPI_write_cmos_sensor(0x3704, 0xa0);
	OV5647tMIPI_write_cmos_sensor(0x3703, 0x5a);
	OV5647tMIPI_write_cmos_sensor(0x3715, 0x78);
	OV5647tMIPI_write_cmos_sensor(0x3717, 0x01);
	OV5647tMIPI_write_cmos_sensor(0x3731, 0x02);
	OV5647tMIPI_write_cmos_sensor(0x370b, 0x60);
	OV5647tMIPI_write_cmos_sensor(0x3705, 0x1a);
	OV5647tMIPI_write_cmos_sensor(0x3f05, 0x02);
	OV5647tMIPI_write_cmos_sensor(0x3f06, 0x10);
	OV5647tMIPI_write_cmos_sensor(0x3f01, 0x0a);
	OV5647tMIPI_write_cmos_sensor(0x3a08, 0x00);
	OV5647tMIPI_write_cmos_sensor(0x3a0a, 0x00);
	OV5647tMIPI_write_cmos_sensor(0x3a0f, 0x58);
	OV5647tMIPI_write_cmos_sensor(0x3a10, 0x50);
	OV5647tMIPI_write_cmos_sensor(0x3a1b, 0x58);
	OV5647tMIPI_write_cmos_sensor(0x3a1e, 0x50);
	OV5647tMIPI_write_cmos_sensor(0x3a11, 0x60);
	OV5647tMIPI_write_cmos_sensor(0x3a1f, 0x28);
	OV5647tMIPI_write_cmos_sensor(0x4001, 0x02);
	OV5647tMIPI_write_cmos_sensor(0x4000, 0x09);
	OV5647tMIPI_write_cmos_sensor(0x4003, 0x08);
	  // manual AWB,manual AE,close Lenc,open WBC				
	OV5647tMIPI_write_cmos_sensor(0x3503, 0x07);  //;manual AE 					 
	OV5647tMIPI_write_cmos_sensor(0x3501, 0x3c); 								 
	OV5647tMIPI_write_cmos_sensor(0x3502, 0x00); 								 
	OV5647tMIPI_write_cmos_sensor(0x350a, 0x00); 								 
	OV5647tMIPI_write_cmos_sensor(0x350b, 0x7f); 								 
	OV5647tMIPI_write_cmos_sensor(0x5001, 0x01);  //;manual AWB					 
	OV5647tMIPI_write_cmos_sensor(0x5180, 0x08); 								 
	OV5647tMIPI_write_cmos_sensor(0x5186, 0x04); 								 
	OV5647tMIPI_write_cmos_sensor(0x5187, 0x00); 								 
	OV5647tMIPI_write_cmos_sensor(0x5188, 0x04); 								 
	OV5647tMIPI_write_cmos_sensor(0x5189, 0x00); 								 
	OV5647tMIPI_write_cmos_sensor(0x518a, 0x04); 								 
	OV5647tMIPI_write_cmos_sensor(0x518b, 0x00); 								 
	OV5647tMIPI_write_cmos_sensor(0x5000, 0x06);  //;No lenc,WBC on	
	
	OV5647tMIPI_write_cmos_sensor(0x3034, 0x1a); /* PLL ctrl0 */
	OV5647tMIPI_write_cmos_sensor(0x3035, 0x21); /* Debug mode  */
	OV5647tMIPI_write_cmos_sensor(0x3036, 0x4a); /*	48.1M */
	OV5647tMIPI_write_cmos_sensor(0x3037, 0x02); /* PLL ctrl3 */
	
	OV5647tMIPI_write_cmos_sensor(0x3106, 0xf9);	
	OV5647tMIPI_write_cmos_sensor(0x303c, 0x11); /*	PLLS control2  0x12*/

#ifdef OV5647tMIPI_TEST_PATTEM
	OV5647tMIPI_write_cmos_sensor(0x503D, 0xa3);
#endif
#endif
	OV5647tMIPI_write_cmos_sensor(0x0100, 0x01);
	

}   /*  OV5647tMIPI_Sensor_Init  */   /*  OV5647tMIPI_Sensor_Init  */

BOOL OV5647t_MIPI_Init(void)
{
	int retry = 3;
	kal_uint16 sensor_id = 0; 
	do 
	{
		// check if sensor ID correct
		sensor_id=((OV5647tMIPI_read_cmos_sensor(0x300A) << 8) | OV5647tMIPI_read_cmos_sensor(0x300B));   
		SENSORDB("Open Sensor ID : 0x%04x\n", sensor_id);
		if (sensor_id == OV5647tMIPI_SENSOR_ID)
			break;
		retry--; 
	} while (retry > 0);

	if (sensor_id != OV5647tMIPI_SENSOR_ID)
		return FALSE;

	OV5647tMIPI_Sensor_Init();

/* Gionee Qux. Correct the IO settings. 09.3rd, 2012 */
#if 1
	OV5647tMIPI_write_cmos_sensor(0x3000, 0x00);
	OV5647tMIPI_write_cmos_sensor(0x3001, 0x00);
	OV5647tMIPI_write_cmos_sensor(0x3002, 0x00);
#else
	OV5647tMIPI_write_cmos_sensor(0x3000, 0xfc);
	OV5647tMIPI_write_cmos_sensor(0x3001, 0x00);
	OV5647tMIPI_write_cmos_sensor(0x3002, 0x1f);
#endif

	OV5647tMIPI_write_cmos_sensor(0x3011, 0x02);
	OV5647tMIPI_write_cmos_sensor(0x0100, 0x00);

	mdelay(10);
}


static void OV5647tMIPI_Sensor_1M(void)
{
	//-------------------------------------------------------------------------------
	// PLL MY_OUTPUT clock(fclk)
	// fclk = (0x40 - 0x300E[5:0]) x N x Bit8Div x MCLK / M, where
	//		N = 1, 1.5, 2, 3 for 0x300F[7:6] = 0~3, respectively
	//		M = 1, 1.5, 2, 3 for 0x300F[1:0] = 0~3, respectively
	//		Bit8Div = 1, 1, 4, 5 for 0x300F[5:4] = 0~3, respectively
	// Sys Clk = fclk / Bit8Div / SenDiv
	// Sensor MY_OUTPUT clock(DVP PCLK)
	// DVP PCLK = ISP CLK / DVPDiv, where
	//		ISP CLK =  fclk / Bit8Div / SenDiv / CLKDiv / 2, where
	//			Bit8Div = 1, 1, 4, 5 for 0x300F[5:4] = 0~3, respectively
	//			SenDiv = 1, 2 for 0x3010[4] = 0 or 1 repectively
	//			CLKDiv = (0x3011[5:0] + 1)
	//		DVPDiv = 0x304C[3:0] * (2 ^ 0x304C[4]), if 0x304C[3:0] = 0, use 16 instead
	//
	// Base shutter calculation
	//		60Hz: (1/120) * ISP Clk / QXGA_MODE_WITHOUT_DUMMY_PIXELS
	//		50Hz: (1/100) * ISP Clk / QXGA_MODE_WITHOUT_DUMMY_PIXELS
	//-------------------------------------------------------------------------------
	
	
		SENSORDB("OV5647tMIPIPreview Setting \n");

	OV5647tMIPI_write_cmos_sensor(0x4005, 0x1A); // update BLC
	
	OV5647tMIPI_write_cmos_sensor(0x0100, 0x00);	
	OV5647tMIPI_write_cmos_sensor(0x3035, 0x21); 
	OV5647tMIPI_write_cmos_sensor(0x3036, 0x46);	
//GIONEE suxt 20120621 add for 13A camera mirror begin	 
#ifdef GN_MTK_BSP_MAIN_CAMMERA_HVMIRROR
	OV5647tMIPI_write_cmos_sensor(0x3821, 0x00); //suxt modify
	OV5647tMIPI_write_cmos_sensor(0x3820, 0x47); //suxt modify
#else
	OV5647tMIPI_write_cmos_sensor(0x3821, 0x06); 
	OV5647tMIPI_write_cmos_sensor(0x3820, 0x41);
#endif
//GIONEE suxt 20120621 add for 13A camera mirror end
		
	OV5647tMIPI_write_cmos_sensor(0x3612, 0x09);	
	OV5647tMIPI_write_cmos_sensor(0x3618, 0x00);	
	OV5647tMIPI_write_cmos_sensor(0x380C, 0x07);	
	OV5647tMIPI_write_cmos_sensor(0x380D, 0x68);	
	OV5647tMIPI_write_cmos_sensor(0x380E, 0x03);	
	OV5647tMIPI_write_cmos_sensor(0x380F, 0xD8);	
	OV5647tMIPI_write_cmos_sensor(0x3814, 0x31);	
	OV5647tMIPI_write_cmos_sensor(0x3815, 0x31);	
    OV5647tMIPI_write_cmos_sensor(0x3709, 0x52);	
    
    #if 0
	OV5647tMIPI_write_cmos_sensor(0x3808, 0x05);	
	OV5647tMIPI_write_cmos_sensor(0x3809, 0x00);	
	OV5647tMIPI_write_cmos_sensor(0x380A, 0x03);	
	OV5647tMIPI_write_cmos_sensor(0x380B, 0xC0);	
	OV5647tMIPI_write_cmos_sensor(0x3800, 0x00);	
	OV5647tMIPI_write_cmos_sensor(0x3801, 0x18);	
	OV5647tMIPI_write_cmos_sensor(0x3802, 0x00);	
	OV5647tMIPI_write_cmos_sensor(0x3803, 0x0E);	
	OV5647tMIPI_write_cmos_sensor(0x3804, 0x0A);	
	OV5647tMIPI_write_cmos_sensor(0x3805, 0x27);	
	OV5647tMIPI_write_cmos_sensor(0x3806, 0x07);	
	OV5647tMIPI_write_cmos_sensor(0x3807, 0x95);	
	#else
	
    OV5647tMIPI_write_cmos_sensor(0x3808, 0x05); /* TIMING X OUTPUT SIZE 0x0500 */
	OV5647tMIPI_write_cmos_sensor(0x3809, 0x10); /* TIMING X OUTPUT SIZE  1280 */
	OV5647tMIPI_write_cmos_sensor(0x380a, 0x03); /* TIMING Y OUTPUT SIZE  0x03c0 */
	OV5647tMIPI_write_cmos_sensor(0x380b, 0xcc); /* TIMING Y OUTPUT SIZE  960 */
	OV5647tMIPI_write_cmos_sensor(0x3800, 0x00); /* TIMING X ADDR START	*/
	OV5647tMIPI_write_cmos_sensor(0x3801, 0x08); /* TIMING X ADDR START	*/
	OV5647tMIPI_write_cmos_sensor(0x3802, 0x00); /* TIMING Y ADDR START	*/
	OV5647tMIPI_write_cmos_sensor(0x3803, 0x02); /* TIMING Y ADDR START	*/
	OV5647tMIPI_write_cmos_sensor(0x3804, 0x0a); /* TIMING X ADDR END  */
	OV5647tMIPI_write_cmos_sensor(0x3805, 0x37); /* TIMING X ADDR END  */
	OV5647tMIPI_write_cmos_sensor(0x3806, 0x07); /* TIMING Y ADDR END  */
	OV5647tMIPI_write_cmos_sensor(0x3807, 0xa1); /* TIMING Y ADDR END  */
	#endif
	
	

	OV5647tMIPI_write_cmos_sensor(0x3a09, 0x27);	
	OV5647tMIPI_write_cmos_sensor(0x3a0a, 0x00);	
	OV5647tMIPI_write_cmos_sensor(0x3a0b, 0xf6);	
	OV5647tMIPI_write_cmos_sensor(0x3a0d, 0x04);
		
	OV5647tMIPI_write_cmos_sensor(0x3a0e, 0x03);	
	OV5647tMIPI_write_cmos_sensor(0x4004, 0x02);	
	OV5647tMIPI_write_cmos_sensor(0x4837, 0x23);
	OV5647tMIPI_write_cmos_sensor(0x0100, 0x01);	

}


static void OV5647tMIPI_Sensor_5M(void)
{
    SENSORDB("OV5647tMIPICapture Setting\n");

   // MIPI 2592x1944 15fps
	// 15fps at 26MHz input clock
	// PCLK = 83.2MHz,mipi bit rate=832M bps,dummy>180


  
	OV5647tMIPI_write_cmos_sensor(0x0100,0x00); 	
    OV5647tMIPI_write_cmos_sensor(0x3035,0x21); 	
	OV5647tMIPI_write_cmos_sensor(0x3036,0x64);
//GIONEE suxt 20120621 add for 13A camera mirror begin	 	
#ifdef GN_MTK_BSP_MAIN_CAMMERA_HVMIRROR
	OV5647tMIPI_write_cmos_sensor(0x3821, 0x00); //suxt modify
	OV5647tMIPI_write_cmos_sensor(0x3820, 0x06); //suxt modify47
#else
	OV5647tMIPI_write_cmos_sensor(0x3821,0x06); 
	OV5647tMIPI_write_cmos_sensor(0x3820,0x00); 
#endif
//GIONEE suxt 20120621 add for 13A camera mirror end
	//OV5647tMIPI_write_cmos_sensor(0x3821, 0x00);	
	//OV5647tMIPI_write_cmos_sensor(0x3820, 0x06);	
	OV5647tMIPI_write_cmos_sensor(0x3612,0x0b); 
	OV5647tMIPI_write_cmos_sensor(0x3618,0x04); 
	OV5647tMIPI_write_cmos_sensor(0x380c,0x0a); 	
	OV5647tMIPI_write_cmos_sensor(0x380d,0x8c); 	
	OV5647tMIPI_write_cmos_sensor(0x380e,0x07); 	
	OV5647tMIPI_write_cmos_sensor(0x380f,0xb6); 	
	OV5647tMIPI_write_cmos_sensor(0x3814,0x11); 	
	OV5647tMIPI_write_cmos_sensor(0x3815,0x11); 	
	OV5647tMIPI_write_cmos_sensor(0x3709,0x12);
		
	
	#if 0
	OV5647tMIPI_write_cmos_sensor(0x3808,0x0a);//dvp output h[11-8] 											  
	OV5647tMIPI_write_cmos_sensor(0x3809,0x20);//dvp output h[7-0]	2592											  
	OV5647tMIPI_write_cmos_sensor(0x380a,0x07);//dvp output v[11-8] 											  
	OV5647tMIPI_write_cmos_sensor(0x380b,0x98);//dvp output v[7-0]	1944												
	OV5647tMIPI_write_cmos_sensor(0x3800,0x00);// x_addr_start[11-8]											
	OV5647tMIPI_write_cmos_sensor(0x3801,0x0c);// x_addr_start[7-0] 											
	OV5647tMIPI_write_cmos_sensor(0x3802,0x00);// y_addr_start[11-8]											  
	OV5647tMIPI_write_cmos_sensor(0x3803,0x04);// y_addr_start[7-0] 									
	OV5647tMIPI_write_cmos_sensor(0x3804,0x0a);// x_addr_end[11-8]												
	OV5647tMIPI_write_cmos_sensor(0x3805,0x33);// x_addr_end[7-0]													
	OV5647tMIPI_write_cmos_sensor(0x3806,0x07);//y_addr_end[11-8]													  
	OV5647tMIPI_write_cmos_sensor(0x3807,0xa3);// y_addr_start[7-0] 
	#else

	OV5647tMIPI_write_cmos_sensor(0x3808, 0x0a); /* TIMING X OUTPUT SIZE 0x0a20 */
	OV5647tMIPI_write_cmos_sensor(0x3809, 0x20); /* TIMING X OUTPUT SIZE 2592 */
	OV5647tMIPI_write_cmos_sensor(0x380a, 0x07); /* TIMING Y OUTPUT SIZE 0x0798*/
	OV5647tMIPI_write_cmos_sensor(0x380b, 0x98); /* TIMING Y OUTPUT SIZE 1944 */
	OV5647tMIPI_write_cmos_sensor(0x3800, 0x00); /* TIMING X ADDR START	*/
	OV5647tMIPI_write_cmos_sensor(0x3801, 0x0c); /* TIMING X ADDR START	*/
	OV5647tMIPI_write_cmos_sensor(0x3802, 0x00); /* TIMING Y ADDR START	*/
	OV5647tMIPI_write_cmos_sensor(0x3803, 0x04); /* TIMING Y ADDR START	*/
	OV5647tMIPI_write_cmos_sensor(0x3804, 0x0a); /* TIMING X ADDR END  */
	OV5647tMIPI_write_cmos_sensor(0x3805, 0x33); /* TIMING X ADDR END  */
	OV5647tMIPI_write_cmos_sensor(0x3806, 0x07); /* TIMING Y ADDR END  */
	OV5647tMIPI_write_cmos_sensor(0x3807, 0xa3); /* TIMING Y ADDR END  */
	#endif
		
	OV5647tMIPI_write_cmos_sensor(0x3a09,0x28); 	
	OV5647tMIPI_write_cmos_sensor(0x3a0a,0x00); 	
	OV5647tMIPI_write_cmos_sensor(0x3a0b,0xf6); 	
	OV5647tMIPI_write_cmos_sensor(0x3a0d,0x08); 	
	OV5647tMIPI_write_cmos_sensor(0x3a0e,0x06); 	
	OV5647tMIPI_write_cmos_sensor(0x4004,0x04); 
	OV5647tMIPI_write_cmos_sensor(0x4837,0x19);
	OV5647tMIPI_write_cmos_sensor(0x0100,0x01); 	
	
}

//Qux
extern UINT32 OV5647GetModuleID(void);

/* Gionee Qux. Add for OV5647 AC module compatibility. 09.3rd, 2012 */
#if defined (GN_MTK_BSP_CAM_OV5647_AC)
extern void kd_set_pwdn_pin(int hl);
#endif
/* Gionee Qux. END */

/*****************************************************************************/
/* Windows Mobile Sensor Interface */
/*****************************************************************************/
/*************************************************************************
* FUNCTION
*	OV5647tMIPIOpen
*
* DESCRIPTION
*	This function initialize the registers of CMOS sensor
*
* PARAMETERS
*	None
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/

UINT32 OV5647tMIPIOpen(void)
{
	kal_uint16 sensor_id=0, otp_ret; 
	kal_uint32 module_id; 

/* Gionee Qux. Add for OV5647 AC module compatibility. 09.3rd, 2012 */
#if defined (GN_MTK_BSP_CAM_OV5647_AC)
	/* Gionee Qux: Set High for Truly AC Module */
	kd_set_pwdn_pin(1);
#endif

	// check if sensor ID correct
	sensor_id=((OV5647tMIPI_read_cmos_sensor(0x300A) << 8) | OV5647tMIPI_read_cmos_sensor(0x300B));   
#ifdef OV5647tMIPI_DRIVER_TRACE
	SENSORDB("OV5647tMIPIOpen, sensor_id:%x \n",sensor_id);
#endif		

/* Gionee Qux. Add for OV5647 AC module compatibility. 09.3rd, 2012 */
//Default level is for Ac module
#if defined (GN_MTK_BSP_CAM_OV5647_AC)
	if (sensor_id != OV5647tMIPI_SENSOR_ID)
	{
		kd_set_pwdn_pin(0);
		sensor_id=((OV5647tMIPI_read_cmos_sensor(0x300A) << 8) | OV5647tMIPI_read_cmos_sensor(0x300B));
	}
#endif

	if (sensor_id != OV5647tMIPI_SENSOR_ID)
		return ERROR_SENSOR_CONNECT_FAIL;
	

#if 1

	module_id = OV5647GetModuleID();
	SENSORDB("OV5647TRULYOpen:%x\n", module_id);

	if(module_id != MODULE_ID_TRULY)
		return ERROR_SENSOR_CONNECT_FAIL;

	SENSORDB("OV5647TRULY Initial...\n");

#endif

	/* initail sequence write in  */
	OV5647tMIPI_Sensor_Init();

  #ifdef OV5647tMIPI_USE_OTP

	#ifdef OV5647tMIPI_USE_AWB_OTP
	
		 otp_ret = OV5647tMIPI_update_wb_register_from_otp();
		if(1 == otp_ret)
			{
				SENSORDB("OV5647tMIPI_update_wb_register_from_otp invalid\n");
			}
		else if(0 == otp_ret)
			{
				SENSORDB("OV5647tMIPI_update_wb_register_from_otp success\n");
			}
	#endif

	#ifdef OV5647tMIPI_USE_LENC_OTP   //copy form OV5650 LENC OTP
	
	
		ret = OV5647tMIPI_update_lenc_register_from_otp();
		if(1 == ret)
			{
				SENSORDB("OV5647tMIPI_update_lenc_register_from_otp invalid\n");
			}
		else if(0 == ret)
			{
				SENSORDB("OV5647tMIPI_update_lenc_register_from_otp success\n");
			}
	#endif
	
#endif
 

    spin_lock(&ov5647tmipi_drv_lock);
	OV5647tMIPIAutoFlicKerMode = KAL_FALSE;
	spin_unlock(&ov5647tmipi_drv_lock);
	//OV5647tMIPIZsdCameraPreview= KAL_FALSE;
	return ERROR_NONE;
}   /* OV5647tMIPIOpen  */

/*************************************************************************
* FUNCTION
*   OV5647tGetSensorID
*
* DESCRIPTION
*   This function get the sensor ID 
*
* PARAMETERS
*   *sensorID : return the sensor ID 
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 OV5647tGetSensorID(UINT32 *sensorID) 
{
	kal_uint32 module_id;

/* Gionee Qux. Add for OV5647 AC module compatibility. 09.3rd, 2012 */
#if defined (GN_MTK_BSP_CAM_OV5647_AC)
	kd_set_pwdn_pin(1);
#endif

	// check if sensor ID correct
	*sensorID=((OV5647tMIPI_read_cmos_sensor(0x300A) << 8) | OV5647tMIPI_read_cmos_sensor(0x300B));	
#ifdef OV5647tMIPI_DRIVER_TRACE
	SENSORDB("OV5647tGetSensorID sensor_id:%x \n",*sensorID);
#endif

/* Gionee Qux. Add for OV5647 AC module compatibility. 09.3rd, 2012 */
//Default level is for Ac module
#if defined (GN_MTK_BSP_CAM_OV5647_AC)
	if (*sensorID != OV5647tMIPI_SENSOR_ID)
	{
		kd_set_pwdn_pin(0);
		*sensorID=((OV5647tMIPI_read_cmos_sensor(0x300A) << 8) | OV5647tMIPI_read_cmos_sensor(0x300B));
	}
#endif

	if (*sensorID != OV5647tMIPI_SENSOR_ID) {
		return ERROR_SENSOR_CONNECT_FAIL;
	}

//Gionee:Qux
	module_id = OV5647GetModuleID();
	SENSORDB("OV5647TrulyMIPIOpen module:%x\n", module_id);

	if(module_id != MODULE_ID_TRULY)
	{
		//Qux. New
		*sensorID = 0;
		return ERROR_SENSOR_CONNECT_FAIL;
	}

#if defined(OV5647_CLONING)
	*sensorID = OV5647TRULY_SENSOR_ID;
#endif
//

   return ERROR_NONE;
}

/*************************************************************************
* FUNCTION
*	OV5647tMIPIClose
*
* DESCRIPTION
*	This function is to turn off sensor module power.
*
* PARAMETERS
*	None
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 OV5647tMIPIClose(void)
{
#ifdef OV5647tMIPI_DRIVER_TRACE
   SENSORDB("OV5647tMIPIClose\n");
#endif
  //CISModulePowerOn(FALSE);
//	DRV_I2CClose(OV5647tMIPIhDrvI2C);
	return ERROR_NONE;
}   /* OV5647tMIPIClose */

/*************************************************************************
* FUNCTION
* OV5647tMIPIPreview
*
* DESCRIPTION
*	This function start the sensor preview.
*
* PARAMETERS
*	*image_window : address pointer of pixel numbers in one period of HSYNC
*  *sensor_config_data : address pointer of line numbers in one period of VSYNC
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 OV5647tMIPIPreview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	kal_uint16 dummy_line;
	
	OV5647tMIPI_Sensor_1M();

	spin_lock(&ov5647tmipi_drv_lock);
	OV5647tMIPI_sensor.pv_mode = KAL_TRUE;
	spin_unlock(&ov5647tmipi_drv_lock);
	
	//OV5647tMIPIZsdCameraPreview=KAL_FALSE;
	
	//OV5647tMIPI_set_mirror(sensor_config_data->SensorImageMirror);
	switch (sensor_config_data->SensorOperationMode)
	{
	  case MSDK_SENSOR_OPERATION_MODE_VIDEO: 
	  	spin_lock(&ov5647tmipi_drv_lock);
		OV5647tMIPI_sensor.video_mode = KAL_TRUE;
		spin_unlock(&ov5647tmipi_drv_lock);
		dummy_line = 0;
	  default: /* ISP_PREVIEW_MODE */
	  	spin_lock(&ov5647tmipi_drv_lock);
		OV5647tMIPI_sensor.video_mode = KAL_FALSE;
		spin_unlock(&ov5647tmipi_drv_lock);
		dummy_line = 0;
	}
	spin_lock(&ov5647tmipi_drv_lock);
	OV5647tMIPI_sensor.line_length = OV5647tMIPI_PV_PERIOD_PIXEL_NUMS;
	OV5647tMIPI_sensor.frame_height = OV5647tMIPI_PV_PERIOD_LINE_NUMS+dummy_line;

	OV5647tMIPI_sensor.pclk = OV5647tMIPI_PREVIEW_CLK;
	spin_unlock(&ov5647tmipi_drv_lock);

	OV5647tMIPI_Set_Dummy(0, dummy_line); /* modify dummy_pixel must gen AE table again */
	//OV5647tMIPI_Write_Shutter(OV5647tMIPI_sensor.shutter);
	
	//printk("[soso][OV5647tMIPIPreview]shutter=%x,shutter=%d\n",OV5647tMIPI_sensor.shutter,OV5647tMIPI_sensor.shutter);
	mdelay(20);
	return ERROR_NONE;
}   /*  OV5647tMIPIPreview   */

UINT32 OV5647tMIPIZsdPreview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{

   kal_uint16 dummy_line;
	
#ifdef OV5647tMIPI_DRIVER_TRACE
	SENSORDB("OV5647tMIPIZsdPreview \n");
#endif
   //OV5647tMIPIZsdCameraPreview = KAL_TRUE;
	OV5647tMIPI_Sensor_5M();

	spin_lock(&ov5647tmipi_drv_lock);
	OV5647tMIPI_sensor.pv_mode = KAL_FALSE;
	spin_unlock(&ov5647tmipi_drv_lock);
	//OV5647tMIPI_set_mirror(sensor_config_data->SensorImageMirror);
	switch (sensor_config_data->SensorOperationMode)
	{
	  case MSDK_SENSOR_OPERATION_MODE_VIDEO: 
	  	spin_lock(&ov5647tmipi_drv_lock);
		OV5647tMIPI_sensor.video_mode = KAL_TRUE;
		spin_unlock(&ov5647tmipi_drv_lock);
		dummy_line = 0;
#ifdef OV5647tMIPI_DRIVER_TRACE
		SENSORDB("Video mode \n");
#endif
	  default: /* ISP_PREVIEW_MODE */
	    spin_lock(&ov5647tmipi_drv_lock);
		OV5647tMIPI_sensor.video_mode = KAL_FALSE;
		spin_unlock(&ov5647tmipi_drv_lock);
		dummy_line = 0;
#ifdef OV5647tMIPI_DRIVER_TRACE
		//SENSORDB("Camera preview mode \n");
#endif
	}
	spin_lock(&ov5647tmipi_drv_lock);
	OV5647tMIPI_sensor.pclk = OV5647tMIPI_CAPTURE_CLK;
	OV5647tMIPI_sensor.line_length = OV5647tMIPI_FULL_PERIOD_PIXEL_NUMS;
	OV5647tMIPI_sensor.frame_height = OV5647tMIPI_FULL_PERIOD_LINE_NUMS+dummy_line;
	spin_unlock(&ov5647tmipi_drv_lock);
	
	//OV5647tMIPI_Set_Dummy(0, dummy_line); /* modify dummy_pixel must gen AE table again */
	//OV5647tMIPI_Write_Shutter(OV5647tMIPI_sensor.shutter);	  
	return ERROR_NONE;
   
}
/*************************************************************************
* FUNCTION
*	OV5647tMIPICapture
*
* DESCRIPTION
*	This function setup the CMOS sensor in capture MY_OUTPUT mode
*
* PARAMETERS
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 OV5647tMIPICapture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
						  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	const kal_uint32 pv_line_length = (kal_uint32)OV5647tMIPI_sensor.line_length;
	kal_uint32 shutter = (kal_uint32)OV5647tMIPI_sensor.shutter;
	kal_uint16 dummy_pixel, cap_fps;


	spin_lock(&ov5647tmipi_drv_lock);
	OV5647tMIPI_sensor.video_mode = KAL_FALSE;
	OV5647tMIPIAutoFlicKerMode = KAL_FALSE;
	spin_unlock(&ov5647tmipi_drv_lock);

	if(OV5647tMIPI_sensor.pv_mode == KAL_TRUE)
	{

		OV5647tMIPI_Sensor_5M();
		
		spin_lock(&ov5647tmipi_drv_lock);
		OV5647tMIPI_sensor.pv_mode = KAL_FALSE;
	
		OV5647tMIPI_sensor.pclk = 80000000;
		spin_unlock(&ov5647tmipi_drv_lock);
		
		cap_fps = OV5647tMIPI_FPS(15);

		OV5647tMIPI_Set_Dummy(0, 0);

		spin_lock(&ov5647tmipi_drv_lock);
		OV5647tMIPI_sensor.line_length = OV5647tMIPI_FULL_PERIOD_PIXEL_NUMS;
		OV5647tMIPI_sensor.frame_height = OV5647tMIPI_FULL_PERIOD_LINE_NUMS;
		spin_unlock(&ov5647tmipi_drv_lock);
		
		//806 is cpature PCLK  481 is preivew PCLK
		//shutter = shutter * pv_line_length * 806/OV5647tMIPI_sensor.line_length/481;
		//shutter = shutter * (((kal_uint32)(pv_line_length * 806))/((kal_uint32)(OV5647tMIPI_sensor.line_length * 481)));
		//shutter = shutter * (((kal_uint32)(pv_line_length * 806 ))/((kal_uint32)(OV5647tMIPI_sensor.line_length * 481)));
		//shutter = (shutter * (((kal_uint32)(pv_line_length * 806 * 1000))/((kal_uint32)(OV5647tMIPI_sensor.line_length * 481))))/1000 - 3;	
		printk("OV5647tMIPI preview shutter:%x \n",shutter);
		//shutter = ((shutter * (kal_uint32)(pv_line_length * 80))/((kal_uint32)(OV5647tMIPI_sensor.line_length * 56)));	

		shutter=(shutter *(((kal_uint32)(pv_line_length*80)*1000)/((kal_uint32)(OV5647tMIPI_sensor.line_length*56)))/1000);
		printk("OV5647tMIPI capture shutter:%x \n",shutter);
		OV5647tMIPI_Write_Shutter(shutter);
	}
	mdelay(20);
	return ERROR_NONE;
}   /* OV5647tMIPI_Capture() */

UINT32 OV5647tMIPIGetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution)
{
	pSensorResolution->SensorFullWidth=OV5647tMIPI_IMAGE_SENSOR_FULL_WIDTH;
	pSensorResolution->SensorFullHeight=OV5647tMIPI_IMAGE_SENSOR_FULL_HEIGHT;
	pSensorResolution->SensorPreviewWidth=OV5647tMIPI_IMAGE_SENSOR_PV_WIDTH;
	pSensorResolution->SensorPreviewHeight=OV5647tMIPI_IMAGE_SENSOR_PV_HEIGHT;
	
	return ERROR_NONE;
}	/* OV5647tMIPIGetResolution() */

UINT32 OV5647tMIPIGetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId,
					  MSDK_SENSOR_INFO_STRUCT *pSensorInfo,
					  MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
#ifdef OV5647tMIPI_DRIVER_TRACE
	//SENSORDB("OV5647tMIPIGetInfo£¬FeatureId:%d\n",ScenarioId);
#endif
#if 1

  switch(ScenarioId)
    {
        #if defined(MT6575)
    	case MSDK_SCENARIO_ID_CAMERA_ZSD:
			 pSensorInfo->SensorPreviewResolutionX=OV5647tMIPI_IMAGE_SENSOR_FULL_WIDTH;
	         pSensorInfo->SensorPreviewResolutionY=OV5647tMIPI_IMAGE_SENSOR_FULL_HEIGHT;
			 pSensorInfo->SensorCameraPreviewFrameRate=15;
			 break;
		
		default:
		#endif
			 pSensorInfo->SensorPreviewResolutionX=OV5647tMIPI_IMAGE_SENSOR_PV_WIDTH;
	         pSensorInfo->SensorPreviewResolutionY=OV5647tMIPI_IMAGE_SENSOR_PV_HEIGHT;
			 pSensorInfo->SensorCameraPreviewFrameRate=30;
    	}
    	
	//pSensorInfo->SensorPreviewResolutionX=OV5647tMIPI_IMAGE_SENSOR_PV_WIDTH;
	//pSensorInfo->SensorPreviewResolutionY=OV5647tMIPI_IMAGE_SENSOR_PV_HEIGHT;
	pSensorInfo->SensorFullResolutionX=OV5647tMIPI_IMAGE_SENSOR_FULL_WIDTH;
	pSensorInfo->SensorFullResolutionY=OV5647tMIPI_IMAGE_SENSOR_FULL_HEIGHT;

	//pSensorInfo->SensorCameraPreviewFrameRate=30;
	pSensorInfo->SensorVideoFrameRate=30;
	pSensorInfo->SensorStillCaptureFrameRate=10;
	pSensorInfo->SensorWebCamCaptureFrameRate=15;
	pSensorInfo->SensorResetActiveHigh=FALSE; //low active
	pSensorInfo->SensorResetDelayCount=5; 
#endif
	pSensorInfo->SensorOutputDataFormat=OV5647tMIPI_COLOR_FORMAT;
	pSensorInfo->SensorClockPolarity=SENSOR_CLOCK_POLARITY_LOW;	
	pSensorInfo->SensorClockFallingPolarity=SENSOR_CLOCK_POLARITY_LOW;
	pSensorInfo->SensorHsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
	pSensorInfo->SensorVsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
#if 1
	pSensorInfo->SensorInterruptDelayLines = 4;
	
	#ifdef MIPI_INTERFACE
   		pSensorInfo->SensroInterfaceType        = SENSOR_INTERFACE_TYPE_MIPI;
   	#else
   		pSensorInfo->SensroInterfaceType		= SENSOR_INTERFACE_TYPE_PARALLEL;
   	#endif

    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_100_MODE].MaxWidth=CAM_SIZE_5M_WIDTH;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_100_MODE].MaxHeight=CAM_SIZE_5M_HEIGHT;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_100_MODE].ISOSupported=TRUE;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_100_MODE].BinningEnable=FALSE;

	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_200_MODE].MaxWidth=CAM_SIZE_5M_WIDTH;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_200_MODE].MaxHeight=CAM_SIZE_5M_HEIGHT;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_200_MODE].ISOSupported=TRUE;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_200_MODE].BinningEnable=FALSE;

	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_400_MODE].MaxWidth=CAM_SIZE_5M_WIDTH;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_400_MODE].MaxHeight=CAM_SIZE_5M_HEIGHT;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_400_MODE].ISOSupported=TRUE;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_400_MODE].BinningEnable=FALSE;

	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_800_MODE].MaxWidth=CAM_SIZE_1M_WIDTH;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_800_MODE].MaxHeight=CAM_SIZE_1M_HEIGHT;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_800_MODE].ISOSupported=TRUE;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_800_MODE].BinningEnable=TRUE;

	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_1600_MODE].MaxWidth=CAM_SIZE_1M_WIDTH;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_1600_MODE].MaxHeight=CAM_SIZE_1M_HEIGHT;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_1600_MODE].ISOSupported=TRUE;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_1600_MODE].BinningEnable=TRUE;
#endif
	pSensorInfo->CaptureDelayFrame = 1; 
	pSensorInfo->PreviewDelayFrame = 2; 
	pSensorInfo->VideoDelayFrame = 2; 	

	pSensorInfo->SensorMasterClockSwitch = 0; 
    pSensorInfo->SensorDrivingCurrent = ISP_DRIVING_6MA;
    pSensorInfo->AEShutDelayFrame = 0;		   /* The frame of setting shutter default 0 for TG int */
	pSensorInfo->AESensorGainDelayFrame = 0;	   /* The frame of setting sensor gain */
	pSensorInfo->AEISPGainDelayFrame = 2;    
	switch (ScenarioId)
	{
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
		case MSDK_SCENARIO_ID_VIDEO_CAPTURE_MPEG4:
			pSensorInfo->SensorClockFreq=24;
			pSensorInfo->SensorClockDividCount=	3;
			pSensorInfo->SensorClockRisingCount= 0;
			pSensorInfo->SensorClockFallingCount= 2;
			pSensorInfo->SensorPixelClockCount= 3;
			pSensorInfo->SensorDataLatchCount= 2;
			pSensorInfo->SensorGrabStartX=  OV5647tMIPI_PV_X_START;
			pSensorInfo->SensorGrabStartY = OV5647tMIPI_PV_Y_START; 
			
				#ifdef MIPI_INTERFACE
	            pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_2_LANE;			
	            pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0; 
		        pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 14; 
		        pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
	            pSensorInfo->SensorWidthSampling = 0;  // 0 is default 1x
	            pSensorInfo->SensorHightSampling = 0;   // 0 is default 1x 
	            pSensorInfo->SensorPacketECCOrder = 1;
	        	#endif


		break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_MEM:
		case MSDK_SCENARIO_ID_CAMERA_ZSD:
			pSensorInfo->SensorClockFreq=24;
			pSensorInfo->SensorClockDividCount= 3;
			pSensorInfo->SensorClockRisingCount=0;
			pSensorInfo->SensorClockFallingCount=2;
			pSensorInfo->SensorPixelClockCount=3;
			pSensorInfo->SensorDataLatchCount=2;
			pSensorInfo->SensorGrabStartX = OV5647tMIPI_FULL_X_START; 
			pSensorInfo->SensorGrabStartY = OV5647tMIPI_FULL_Y_START; 
			
			   			#ifdef MIPI_INTERFACE
	            pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_2_LANE;			
	            pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0; 
		        pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 14; 
		        pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
	            pSensorInfo->SensorWidthSampling = 0;  // 0 is default 1x
	            pSensorInfo->SensorHightSampling = 0;   // 0 is default 1x 
	            pSensorInfo->SensorPacketECCOrder = 1;
	        #endif

		break;
		default:
			pSensorInfo->SensorClockFreq=24;
			pSensorInfo->SensorClockDividCount=3;
			pSensorInfo->SensorClockRisingCount=0;
			pSensorInfo->SensorClockFallingCount=2;		
			pSensorInfo->SensorPixelClockCount=3;
			pSensorInfo->SensorDataLatchCount=2;
			pSensorInfo->SensorGrabStartX = OV5647tMIPI_PV_X_START; 
			pSensorInfo->SensorGrabStartY = OV5647tMIPI_PV_Y_START; 
		break;
	}
#if 0
	//OV5647tMIPIPixelClockDivider=pSensorInfo->SensorPixelClockCount;
	memcpy(pSensorConfigData, &OV5647tMIPISensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
#endif		
  return ERROR_NONE;
}	/* OV5647tMIPIGetInfo() */


UINT32 OV5647tMIPIControl(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow,
					  MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
   
    tCurrentScenarioId =ScenarioId;
	switch (ScenarioId)
	{
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
		case MSDK_SCENARIO_ID_VIDEO_CAPTURE_MPEG4:	
			  OV5647tMIPIPreview(pImageWindow, pSensorConfigData);
		break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_MEM:
	    	OV5647tMIPICapture(pImageWindow, pSensorConfigData);
	    break;
		case MSDK_SCENARIO_ID_CAMERA_ZSD:
     		OV5647tMIPIZsdPreview(pImageWindow, pSensorConfigData);
		break;		
        default:
            return ERROR_INVALID_SCENARIO_ID;
	}
	
	return TRUE;
}	/* OV5647tMIPIControl() */



UINT32 OV5647tMIPISetVideoMode(UINT16 u2FrameRate)
{
	printk("[soso][OV5647tMIPISetMaxFrameRate]u2FrameRate=%d",u2FrameRate);

	//chiva for recorder die.
	if(0==u2FrameRate)
		return 0;
	//\\

	spin_lock(&ov5647tmipi_drv_lock);
	OV5647tMIPI_sensor.video_mode = KAL_TRUE;
	spin_unlock(&ov5647tmipi_drv_lock);

	if(u2FrameRate == 30){
		spin_lock(&ov5647tmipi_drv_lock);
		OV5647tMIPI_sensor.NightMode = KAL_FALSE;
		spin_unlock(&ov5647tmipi_drv_lock);
	}else if(u2FrameRate == 15){
	    spin_lock(&ov5647tmipi_drv_lock);
		OV5647tMIPI_sensor.NightMode = KAL_TRUE;
		spin_unlock(&ov5647tmipi_drv_lock);
	}else{
		// TODO: Wrong configuratioin
	}

	spin_lock(&ov5647tmipi_drv_lock);
	OV5647tMIPI_sensor.FixedFps = u2FrameRate;
	spin_unlock(&ov5647tmipi_drv_lock);

	if((u2FrameRate == 30)&&(OV5647tMIPIAutoFlicKerMode==KAL_TRUE))
		u2FrameRate = 296;
	else
		u2FrameRate = 10 * u2FrameRate;
	
	OV5647tMIPISetMaxFrameRate(u2FrameRate);
	OV5647tMIPI_Write_Shutter(OV5647tMIPI_sensor.shutter);//From Meimei Video issue
    return TRUE;
}

UINT32 OV5647tMIPISetAutoFlickerMode(kal_bool bEnable, UINT16 u2FrameRate)
{
    SENSORDB("OV5647tMIPISetAutoFlickerMode:%d",bEnable);
    
	if(bEnable){

		spin_lock(&ov5647tmipi_drv_lock);
		OV5647tMIPIAutoFlicKerMode = KAL_TRUE;

		spin_unlock(&ov5647tmipi_drv_lock);

		/*Change frame rate 29.5fps to 29.8fps to do Auto flick*/
		if((OV5647tMIPI_sensor.FixedFps == 30)&&(OV5647tMIPI_sensor.video_mode==KAL_TRUE))
			OV5647tMIPISetMaxFrameRate(296);
	}else{//Cancel Auto flick
	    spin_lock(&ov5647tmipi_drv_lock);
		OV5647tMIPIAutoFlicKerMode = KAL_FALSE;
		spin_unlock(&ov5647tmipi_drv_lock);
		if((OV5647tMIPI_sensor.FixedFps == 30)&&(OV5647tMIPI_sensor.video_mode==KAL_TRUE))
			OV5647tMIPISetMaxFrameRate(300);
	}
}
UINT32 OV5647tMIPIFeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId,
							 UINT8 *pFeaturePara,UINT32 *pFeatureParaLen)
{
	UINT16 *pFeatureReturnPara16=(UINT16 *) pFeaturePara;
	UINT16 *pFeatureData16=(UINT16 *) pFeaturePara;
	UINT32 *pFeatureReturnPara32=(UINT32 *) pFeaturePara;
	UINT32 *pFeatureData32=(UINT32 *) pFeaturePara;
	UINT32 OV5647tMIPISensorRegNumber;
	UINT32 i;
	PNVRAM_SENSOR_DATA_STRUCT pSensorDefaultData=(PNVRAM_SENSOR_DATA_STRUCT) pFeaturePara;
	MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData=(MSDK_SENSOR_CONFIG_STRUCT *) pFeaturePara;
	MSDK_SENSOR_REG_INFO_STRUCT *pSensorRegData=(MSDK_SENSOR_REG_INFO_STRUCT *) pFeaturePara;
	MSDK_SENSOR_GROUP_INFO_STRUCT *pSensorGroupInfo=(MSDK_SENSOR_GROUP_INFO_STRUCT *) pFeaturePara;
	MSDK_SENSOR_ITEM_INFO_STRUCT *pSensorItemInfo=(MSDK_SENSOR_ITEM_INFO_STRUCT *) pFeaturePara;
	MSDK_SENSOR_ENG_INFO_STRUCT	*pSensorEngInfo=(MSDK_SENSOR_ENG_INFO_STRUCT *) pFeaturePara;

	switch (FeatureId)
	{
		case SENSOR_FEATURE_GET_RESOLUTION:
			*pFeatureReturnPara16++=OV5647tMIPI_IMAGE_SENSOR_FULL_WIDTH;
			*pFeatureReturnPara16=OV5647tMIPI_IMAGE_SENSOR_FULL_HEIGHT;
			*pFeatureParaLen=4;
		break;
		case SENSOR_FEATURE_GET_PERIOD:	/* 3 */
			switch(tCurrentScenarioId)
			{
				case MSDK_SCENARIO_ID_CAMERA_ZSD:
					*pFeatureReturnPara16++=OV5647tMIPI_FULL_PERIOD_PIXEL_NUMS;
					*pFeatureReturnPara16=OV5647tMIPI_sensor.frame_height;
					*pFeatureParaLen=4;
					break;

				default:
					
					*pFeatureReturnPara16++=OV5647tMIPI_PV_PERIOD_PIXEL_NUMS;
					*pFeatureReturnPara16=OV5647tMIPI_sensor.frame_height;
					*pFeatureParaLen=4;
					break;
				}
			
			break;
		case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:  /* 3 */
			switch(tCurrentScenarioId)
			{
				case MSDK_SCENARIO_ID_CAMERA_ZSD:
					
					*pFeatureReturnPara32 = OV5647tMIPI_CAPTURE_CLK;
					*pFeatureParaLen=4;
					break;

				default:
					
					*pFeatureReturnPara32 = OV5647tMIPI_PREVIEW_CLK;
					*pFeatureParaLen=4;
					break;
				}
			break;
		case SENSOR_FEATURE_SET_ESHUTTER:	/* 4 */
			set_OV5647tMIPI_shutter(*pFeatureData16);
		break;
		case SENSOR_FEATURE_SET_NIGHTMODE:
			OV5647tMIPI_night_mode((BOOL) *pFeatureData16);
		break;
		case SENSOR_FEATURE_SET_GAIN:	/* 6 */			
			OV5647tMIPI_SetGain((UINT16) *pFeatureData16);
		break;
		case SENSOR_FEATURE_SET_FLASHLIGHT:
		break;
		case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
		break;
		case SENSOR_FEATURE_SET_REGISTER:
		OV5647tMIPI_write_cmos_sensor(pSensorRegData->RegAddr, pSensorRegData->RegData);
		break;
		case SENSOR_FEATURE_GET_REGISTER:
			pSensorRegData->RegData = OV5647tMIPI_read_cmos_sensor(pSensorRegData->RegAddr);
		break;
		case SENSOR_FEATURE_SET_CCT_REGISTER:
			memcpy(&OV5647tMIPI_sensor.eng.cct, pFeaturePara, sizeof(OV5647tMIPI_sensor.eng.cct));
			break;
		break;
		case SENSOR_FEATURE_GET_CCT_REGISTER:	/* 12 */
			if (*pFeatureParaLen >= sizeof(OV5647tMIPI_sensor.eng.cct) + sizeof(kal_uint32))
			{
			  *((kal_uint32 *)pFeaturePara++) = sizeof(OV5647tMIPI_sensor.eng.cct);
			  memcpy(pFeaturePara, &OV5647tMIPI_sensor.eng.cct, sizeof(OV5647tMIPI_sensor.eng.cct));
			}
			break;
		case SENSOR_FEATURE_SET_ENG_REGISTER:
			memcpy(&OV5647tMIPI_sensor.eng.reg, pFeaturePara, sizeof(OV5647tMIPI_sensor.eng.reg));
			break;
		case SENSOR_FEATURE_GET_ENG_REGISTER:	/* 14 */
			if (*pFeatureParaLen >= sizeof(OV5647tMIPI_sensor.eng.reg) + sizeof(kal_uint32))
			{
			  *((kal_uint32 *)pFeaturePara++) = sizeof(OV5647tMIPI_sensor.eng.reg);
			  memcpy(pFeaturePara, &OV5647tMIPI_sensor.eng.reg, sizeof(OV5647tMIPI_sensor.eng.reg));
			}
		case SENSOR_FEATURE_GET_REGISTER_DEFAULT:
			((PNVRAM_SENSOR_DATA_STRUCT)pFeaturePara)->Version = NVRAM_CAMERA_SENSOR_FILE_VERSION;
#if defined(OV5647_CLONING)
			((PNVRAM_SENSOR_DATA_STRUCT)pFeaturePara)->SensorId = OV5647TRULY_SENSOR_ID;
#else
			((PNVRAM_SENSOR_DATA_STRUCT)pFeaturePara)->SensorId = OV5647MIPI_SENSOR_ID;
#endif
			memcpy(((PNVRAM_SENSOR_DATA_STRUCT)pFeaturePara)->SensorEngReg, &OV5647tMIPI_sensor.eng.reg, sizeof(OV5647tMIPI_sensor.eng.reg));
			memcpy(((PNVRAM_SENSOR_DATA_STRUCT)pFeaturePara)->SensorCCTReg, &OV5647tMIPI_sensor.eng.cct, sizeof(OV5647tMIPI_sensor.eng.cct));
			*pFeatureParaLen = sizeof(NVRAM_SENSOR_DATA_STRUCT);
			break;
		case SENSOR_FEATURE_GET_CONFIG_PARA:
			memcpy(pFeaturePara, &OV5647tMIPI_sensor.cfg_data, sizeof(OV5647tMIPI_sensor.cfg_data));
			*pFeatureParaLen = sizeof(OV5647tMIPI_sensor.cfg_data);
			break;
		case SENSOR_FEATURE_CAMERA_PARA_TO_SENSOR:
		     OV5647tMIPI_camera_para_to_sensor();
		break;
		case SENSOR_FEATURE_SENSOR_TO_CAMERA_PARA:
			OV5647tMIPI_sensor_to_camera_para();
		break;							
		case SENSOR_FEATURE_GET_GROUP_COUNT:
			OV5647tMIPI_get_sensor_group_count((kal_uint32 *)pFeaturePara);
			*pFeatureParaLen = 4;
		break;										
		  OV5647tMIPI_get_sensor_group_info((MSDK_SENSOR_GROUP_INFO_STRUCT *)pFeaturePara);
		  *pFeatureParaLen = sizeof(MSDK_SENSOR_GROUP_INFO_STRUCT);
		  break;
		case SENSOR_FEATURE_GET_ITEM_INFO:
		  OV5647tMIPI_get_sensor_item_info((MSDK_SENSOR_ITEM_INFO_STRUCT *)pFeaturePara);
		  *pFeatureParaLen = sizeof(MSDK_SENSOR_ITEM_INFO_STRUCT);
		  break;
		case SENSOR_FEATURE_SET_ITEM_INFO:
		  OV5647tMIPI_set_sensor_item_info((MSDK_SENSOR_ITEM_INFO_STRUCT *)pFeaturePara);
		  *pFeatureParaLen = sizeof(MSDK_SENSOR_ITEM_INFO_STRUCT);
		  break;
		case SENSOR_FEATURE_GET_ENG_INFO:
     		memcpy(pFeaturePara, &OV5647tMIPI_sensor.eng_info, sizeof(OV5647tMIPI_sensor.eng_info));
     		*pFeatureParaLen = sizeof(OV5647tMIPI_sensor.eng_info);
     		break;
		case SENSOR_FEATURE_GET_LENS_DRIVER_ID:
			// get the lens driver ID from EEPROM or just return LENS_DRIVER_ID_DO_NOT_CARE
			// if EEPROM does not exist in camera module.
			*pFeatureReturnPara32=LENS_DRIVER_ID_DO_NOT_CARE;
			*pFeatureParaLen=4;
		break;
		case SENSOR_FEATURE_SET_VIDEO_MODE:
	       OV5647tMIPISetVideoMode(*pFeatureData16);
        break; 
        case SENSOR_FEATURE_CHECK_SENSOR_ID:
            OV5647tGetSensorID(pFeatureReturnPara32); 
            break; 
		case SENSOR_FEATURE_SET_AUTO_FLICKER_MODE:
			 OV5647tMIPISetAutoFlickerMode((BOOL)*pFeatureData16,*(pFeatureData16+1));
			break;
		default:
			break;
	}
	return ERROR_NONE;
}	/* OV5647tMIPIFeatureControl() */
SENSOR_FUNCTION_STRUCT	SensorFuncOV5647tMIPI=
{
	OV5647tMIPIOpen,
	OV5647tMIPIGetInfo,
	OV5647tMIPIGetResolution,
	OV5647tMIPIFeatureControl,
	OV5647tMIPIControl,
	OV5647tMIPIClose
};

UINT32 OV5647TRULYSensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc)
{
	/* To Do : Check Sensor status here */
	if (pfFunc!=NULL)
		*pfFunc=&SensorFuncOV5647tMIPI;

	return ERROR_NONE;
}	/* SensorInit() */
