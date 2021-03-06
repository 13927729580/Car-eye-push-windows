/*
* Car eye 车辆管理平台: www.car-eye.cn
* Car eye 开源网址: https://github.com/Car-eye-team
* CarEyeRTMP.cpp
*
* Author: Wgj
* Date: 2018-03-20 13:44
* Copyright 2018
*
* CarEye RTMP推流库测试程序
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "CarEyeRtmpApi.h"

#ifdef _WIN32
#include <windows.h>
char *key = "D18Kr(s2SKR2V(R%7%7K7hR%7%rJ6(r%qIS%8%rKr(s2SKR2V(RkSk6Ktk7JT(R2+(R6D";
#else
#include <pthread.h>
#include <ctype.h>
#include <sys/time.h>
char *key = "u1B27kSKs2{Kuk{jrjS3VhUjrj73qk7j61sj+j727kSKs2{Kuk{(s(q2T(r18";
#endif

using namespace std;

//#define TEST_NATIVE_FILE
#define TEST_RTMP
//#define TEST_MULTI_CHN

char ServerIP[] = "www.car-eye.cn"; // CarEye服务器
const unsigned short ServerPort = 10085;
char SdpName[] = "live/123";
char TestFile[] = "./test.mp4";
char TestH264[] = "./test.264";

bool threadIsWork = false;
CarEye_MediaInfo mediaInfo;

#define ENUM_CHIP_TYPE_CASE(x)   case x: return(#x);

char* GetEnumString(CarEyePusherType aEnum)
{
	switch (aEnum)
	{
		ENUM_CHIP_TYPE_CASE(PUSHER_RTSP)
			ENUM_CHIP_TYPE_CASE(PUSHER_NATIVEFILE_RTSP)
			ENUM_CHIP_TYPE_CASE(PUSHER_RTMP)
			ENUM_CHIP_TYPE_CASE(PUSHER_NATIVEFILE_RTMP)
	}

	return "Already released";
}

/*
* Comments: 推流器状态更改事件
* Param :
* @Return int
*/
int CarEyePusher_StateChangedEvent(int channel, CarEyeStateType state, CarEyePusherType type)
{
	switch (state)
	{
		case CAREYE_STATE_CONNECTING:
			printf("Chn[%d] %s Connecting...\n", channel, GetEnumString(type));
			break;

		case CAREYE_STATE_CONNECTED:
			printf("Chn[%d] %s Connected\n", channel, GetEnumString(type));
			break;

		case CAREYE_STATE_CONNECT_FAILED:
			printf("Chn[%d] %s Connect failed\n", channel, GetEnumString(type));
			break;

		case CAREYE_STATE_CONNECT_ABORT:
			printf("Chn[%d] %s Connect abort\n", channel, GetEnumString(type));
			break;

		case CAREYE_STATE_PUSHING:

			break;

		case CAREYE_STATE_DISCONNECTED:
			printf("Chn[%d] %s Disconnect.\n", channel, GetEnumString(type));
			break;

		case CAREYE_STATE_FILE_FINISHED:
			printf("Chn[%d] %s Push native file finished.\n", channel, GetEnumString(type));
			CarEyeRTMP_StopNativeFile(channel);
			break;

		default:
			break;
	}

	return 0;
}

// 推流测试线程
#ifdef _WIN32
DWORD WINAPI PushThreadEntry(LPVOID arg)
#else
void *PushThreadEntry(void *arg)
#endif
{
	int chn = *((int *)arg);

	int buf_size = 1024 * 512;
	char *pbuf = (char *)malloc(buf_size);
	FILE *fES = NULL;
	int position = 0;
	int iFrameNo = 0;
	int timestamp = 0;
	int i = 0;

	fES = fopen(TestH264, "rb");
	if (NULL == fES)
	{
		printf("Test push file has not found.\n");
		return -1;
	}

	while (threadIsWork)
	{
		if (CarEyeRTMP_PusherIsReady(chn))
		{
			break;
#ifndef _WIN32
			usleep(1000);
#else
			Sleep(1);
#endif
		}
	}

	while (threadIsWork)
	{
		int nReadBytes = fread(pbuf + position, 1, 1, fES);
		if (nReadBytes < 1)
		{
			if (feof(fES))
			{
				position = 0;
				fseek(fES, 0, SEEK_SET);
				memset(pbuf, 0x00, buf_size);
				continue;
			}
			break;
		}

		position++;

		if (position > 5)
		{
			unsigned char naltype = ((unsigned char)pbuf[position - 1] & 0x1F);

			if ((unsigned char)pbuf[position - 5] == 0x00 &&
				(unsigned char)pbuf[position - 4] == 0x00 &&
				(unsigned char)pbuf[position - 3] == 0x00 &&
				(unsigned char)pbuf[position - 2] == 0x01 &&
				//(((unsigned char)pbuf[position-1] == 0x61) ||
				//((unsigned char)pbuf[position-1] == 0x67) ) )
				(naltype == 0x07 || naltype == 0x01))
			{
				int framesize = position - 5;
				CarEye_AV_Frame avFrame;

				naltype = (unsigned char)pbuf[4] & 0x1F;

				memset(&avFrame, 0x00, sizeof(CarEye_AV_Frame));
				avFrame.FrameFlag = CAREYE_VFRAME_FLAG;
				avFrame.Second = timestamp / 1000;
				avFrame.USecond = (timestamp % 1000) * 1000;
				avFrame.VFrameType = (naltype == 0x07) ? VIDEO_FRAME_I : VIDEO_FRAME_P;
				avFrame.FrameLen = framesize;
				avFrame.Buffer = (unsigned char*)pbuf;

				while (!CarEyeRTMP_PusherIsReady(chn))
				{
					// 网络未准备好等待重连
#ifndef _WIN32
						usleep(40 * 1000);
#else
						Sleep(30);
#endif
				}
				CarEyeRTMP_PushData(chn, &avFrame);
				timestamp += 1000 / mediaInfo.VideoFps;
#ifndef _WIN32
				usleep(40 * 1000);
#else
				Sleep(30);
#endif

				memmove(pbuf, pbuf + framesize, 5);
				position = 5;

				iFrameNo++;

// 				if (iFrameNo == 100 || iFrameNo == 200)
// 				{
// 					for (i = 0; i < 20 && threadIsWork; i++)
// 					{
// #ifndef _WIN32
// 						usleep(50000);
// #else
// 						Sleep(50);
// #endif
// 					}
// 				}
			}
		}
	}
	fclose(fES);

	return NULL;
}

#ifdef TEST_NATIVE_FILE

// 本地MP4文件推流测试程序
int main()
{
	if (CarEyeRTMP_Register(key) != CAREYE_NOERROR)
	{
		printf("Register pusher failed.\n");
		return -1;
	}

	CarEyeRTMP_RegisterStateChangedEvent(CarEyePusher_StateChangedEvent);

	int chn = CarEyeRTMP_StartNativeFile(ServerIP, ServerPort, SdpName, TestFile, 0, 0);
	if (chn < 0)
	{
		printf("Start native file rtmp failed %d.\n", chn);
		return -1;
	}

	printf("Wait key stop channel[%d] rtmp...\n", chn);
	getchar();

	CarEyeRTMP_StopNativeFile(chn);
	printf("Wait key exit program...\n");
	getchar();

	return 0;
}

#elif defined TEST_RTMP
// 媒体流推送测试程序
int main()
{
#ifdef _WIN32
	// 解码视频并推送的线程句柄
	HANDLE		thread_id;
#else
	pthread_t	thread_id;
#endif

	if (CarEyeRTMP_Register(key) != CAREYE_NOERROR)
	{
		printf("Register pusher failed.\n");
		return -1;
	}

	CarEyeRTMP_RegisterStateChangedEvent(CarEyePusher_StateChangedEvent);

	mediaInfo.VideoCodec = CAREYE_VCODE_H264;
	mediaInfo.VideoFps = 25;
	mediaInfo.AudioCodec = CAREYE_ACODE_AAC;
	mediaInfo.Channels = 1;
	mediaInfo.Samplerate = 8000;
	int chn = CarEyeRTMP_StartPusher(ServerIP, ServerPort, SdpName, mediaInfo);
	if (chn < 0)
	{
		printf("Start push rtmp failed %d.\n", chn);
		return -1;
	}

	threadIsWork = true;
#ifdef _WIN32
	thread_id = CreateThread(NULL, 0, PushThreadEntry, &chn, 0, NULL);
	if (thread_id == NULL)
	{
		printf("Create push thread failed.\n");
		return -1;
	}
#else
	if (pthread_create(&thread_id, NULL, PushThreadEntry, &chn) != 0)
	{
		printf("Create push thread failed: %d.\n", errno);
		return -1;
	}
	pthread_detach(thread_id);
#endif

	printf("Wait key exit program...\n");
	getchar();
	threadIsWork = false;

	CarEyeRTMP_StopPusher(chn);
	return 0;
}

#elif defined TEST_MULTI_CHN

// 本地MP4文件与H264多通道推流测试程序
int main()
{
	int chns[4]{ -1, -1, -1, 1 };
	int i;

#ifdef _WIN32
	// 解码视频并推送的线程句柄
	HANDLE		thread_id;
#else
	pthread_t	thread_id;
#endif

	if (CarEyeRTMP_Register(key) != CAREYE_NOERROR)
	{
		printf("Register pusher failed.\n");
		return -1;
	}

	CarEyeRTMP_RegisterStateChangedEvent(CarEyePusher_StateChangedEvent);
	int chn = CarEyeRTMP_StartNativeFile(ServerIP, ServerPort, "live/1230", TestFile, 12000, 32000);
	if (chn < 0)
	{
		printf("Start chn0 native file rtmp failed %d.\n", chn);
	}
	chns[0] = chn;

	chn = CarEyeRTMP_StartNativeFile(ServerIP, ServerPort, "live/1231", TestFile, 32000, 0);
	if (chn < 0)
	{
		printf("Start chn1 native file rtmp failed %d.\n", chn);
	}
	chns[1] = chn;

#if 1
	mediaInfo.VideoCodec = CAREYE_VCODE_H264;
	mediaInfo.VideoFps = 25;
	mediaInfo.AudioCodec = CAREYE_ACODE_G711U;
	mediaInfo.AudioChannel = 1;
	mediaInfo.AudioSamplerate = 8000;
	chn = CarEyeRTMP_StartPusher(ServerIP, ServerPort, "live/1232", mediaInfo);
	if (chn < 0)
	{
		printf("Start chn2 push rtmp failed %d.\n", chn);
	}
	chns[2] = chn;

	threadIsWork = true;
	if (chn >= 0)
	{
#ifdef _WIN32
		thread_id = CreateThread(NULL, 0, PushThreadEntry, &chn, 0, NULL);
		if (thread_id == NULL)
		{
			printf("Create chn2 push thread failed.\n");
		}
#else
		if (pthread_create(&thread_id, NULL, PushThreadEntry, &chn) != 0)
		{
			printf("Create chn2 push thread failed: %d.\n", errno);
		}
		else
		{
			pthread_detach(thread_id);
		}
#endif
	}

	chn = CarEyeRTMP_StartPusher(ServerIP, ServerPort, "live/1233", mediaInfo);
	if (chn < 0)
	{
		printf("Start chn3 push rtmp failed %d.\n", chn);
	}
	chns[3] = chn;

	if (chn >= 0)
	{
#ifdef _WIN32
		thread_id = CreateThread(NULL, 0, PushThreadEntry, &chn, 0, NULL);
		if (thread_id == NULL)
		{
			printf("Create chn3 push thread failed.\n");
		}
#else
		if (pthread_create(&thread_id, NULL, PushThreadEntry, &chn) != 0)
		{
			printf("Create chn3 push thread failed: %d.\n", errno);
		}
		else
		{
			pthread_detach(thread_id);
		}
#endif
	}
#endif
	printf("Wait key stop pushing...\n");
	getchar();
	threadIsWork = false;

	for (i = 0; i < 4; i++)
	{
		if (i < 2 && chns[i] >= 0)
		{
			CarEyeRTMP_StopNativeFile(chns[i]);
		}
		else if (chns[i] >= 0)
		{
			CarEyeRTMP_StopPusher(chn);
		}
	}
	printf("Wait key exit program...\n");
	getchar();

	return 0;
}

#endif
