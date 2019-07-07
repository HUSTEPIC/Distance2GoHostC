/*
 ===================================================================================
 Name        : extract_raw_data.c
 Author      : Thomas Finke
 Version     :
 Copyright   : 2014-2017, Infineon Technologies AG
 Description : Example of how to extract raw data using the C communication library
 ===================================================================================
 */

/*
 * Copyright (c) 2014-2017, Infineon Technologies AG
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,are permitted provided that the
 * following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following
 * disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following
 * disclaimer in the documentation and/or other materials provided with the distribution.
 *
 * Neither the name of the copyright holders nor the names of its contributors may be used to endorse or promote
 * products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE  FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY,OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <string.h>
#include "library/Protocol.h"
#include "library/COMPort.h"
#include "library/EndpointRadarBase.h"

#define AUTOMATIC_DATA_FRAME_TRIGGER (1)		// define if automatic trigger is active or not

//#define AUTOMATIC_DATA_TRIGER_TIME_US (1000000)	// get ADC data each 1ms in automatic trigger mode
#define AUTOMATIC_DATA_TRIGER_TIME_US (1)

//保存输出数据的文件名，可以改
char outputFileNameI[100] = { "outputI.log" };
char outputFileNameQ[100] = { "outputQ.log" };

//每次读取数据后都会调用的回调函数
// called every time ep_radar_base_get_frame_data method is called to return measured time domain signals
void received_frame_data(void* context,
						int32_t protocol_handle,
		                uint8_t endpoint,
						const Frame_Info_t* frame_info)
{
	// Print the sampled data which can be found in frame_info->sample_data
	//for (uint32_t i = 0; i < frame_info->num_samples_per_chirp*2; i++)
	for (uint32_t i = 0; i < frame_info->num_samples_per_chirp; i++)
	{

	    if(i == 0)
	    {
	        printf("Frame_number = %d\n", frame_info->frame_number);
	        printf("Num_chirps = %d\n", frame_info->num_chirps);
	        printf("Num_rx_antennas = %d\n", frame_info->num_rx_antennas);
	    }

		//printf("ADC sample %d:     %f\n", i, frame_info->sample_data[i]);


        //这里将数据读到文件中
		//根据冷同学的论文，0-249是I信号数据，250-499是Q信号数据
		/*根据冷同学的论文：
		“通过GUI和提取出来的数据都可以发现，
		整个I信号的250个samples点呈现一种驻波形式，
		虽然能够看到起伏，但全部拿来使用很不方便。
		因此本文又单独选取了第250个信号点，用它的上下移动来作为呼吸信号的表征即可。”

		因此我们这里I信号数据暂时选用第249号信号点，Q选499号信号的数据
        */
	    else if (i == 249)
		{
			FILE* p = fopen(outputFileNameI, "a+");
			char msg[100];
			char msgTmp[50];
			if (p == NULL)
			{
				printf("file open error \n");
			}
			else
			{
				//sprintf(msgTmp, "%d           ", i);
				//strcpy(msg, msgTmp);
				sprintf(msgTmp, "%f", frame_info->sample_data[i]);
				strcpy(msg, msgTmp);
				strcat(msg, "\n");
				fputs(msg, p);
				fclose(p);
			}
        }
		else if (i == 499)
		{
			FILE* p = fopen(outputFileNameQ, "a+");
			char msg[100];
			char msgTmp[50];
			if (p == NULL)
			{
				printf("file open error \n");
			}
			else
			{
				//sprintf(msgTmp, "%d           ", i);
				//strcpy(msg, msgTmp);
				sprintf(msgTmp, "%f", frame_info->sample_data[i]);
				strcpy(msg, msgTmp);
				strcat(msg, "\n");
				fputs(msg, p);
				fclose(p);
			}
		}
        //*/
	}
}

int radar_auto_connect(void)
{
	int radar_handle = 0;
	int num_of_ports = 0;
	char comp_port_list[256];
	char* comport;
	const char *delim = ";";

	//----------------------------------------------------------------------------

	num_of_ports = com_get_port_list(comp_port_list, (size_t)256);

	if (num_of_ports == 0)
	{
		return -1;
	}
	else
	{
		comport = strtok(comp_port_list, delim);

		while (num_of_ports > 0)
		{
			num_of_ports--;

			// open COM port
			radar_handle = protocol_connect(comport);

			if (radar_handle >= 0)
			{
				break;
			}

			comport = strtok(NULL, delim);
		}

		return radar_handle;
	}

}

int main(void)
{
	int res = -1;
	int protocolHandle = 0;
	int endpointRadarBase = 0;

	// open COM port
	protocolHandle = radar_auto_connect();

	// get endpoint ids
	if (protocolHandle >= 0)
	{
		for (int i = 1; i <= protocol_get_num_endpoints(protocolHandle); ++i) {
			// current endpoint is radar base endpoint
			if (ep_radar_base_is_compatible_endpoint(protocolHandle, i) == 0) {
				endpointRadarBase = i;
				continue;
			}
		}
	}


	if (endpointRadarBase >= 0)
	{
		// register call back functions for adc data
		ep_radar_base_set_callback_data_frame(received_frame_data, NULL);

		// enable/disable automatic trigger
		if (AUTOMATIC_DATA_FRAME_TRIGGER)
		{
			res = ep_radar_base_set_automatic_frame_trigger(protocolHandle,
															endpointRadarBase,
															AUTOMATIC_DATA_TRIGER_TIME_US);
		}
		else
		{
			res = ep_radar_base_set_automatic_frame_trigger(protocolHandle,
															endpointRadarBase,
															0);
		}
		/////////清空文件内容
		FILE* p = fopen(outputFileNameI, "w");
		fclose(p);
		p = fopen(outputFileNameQ, "w");
		fclose(p);
		//////////////////
		while (1)
		{
			// get raw data
			res = ep_radar_base_get_frame_data(protocolHandle,
											   endpointRadarBase,
											   1);
		}
	}

	return res;





}
