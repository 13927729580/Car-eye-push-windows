/*
* Car eye ��������ƽ̨: www.car-eye.cn
* Car eye ��Դ��ַ: https://github.com/Car-eye-team
* CarEyeRtmpAPI.h
*
* Author: Wgj
* Date: 2018-03-19 19:11
* Copyright 2018
*
* CarEye RTSP/RTMP������ӿ�ͷ�ļ�
* ʵʱ��������ʱ��֧�����8��ͨ������
*/

#ifndef __CAREYE_PUSHER_H__
#define __CAREYE_PUSHER_H__

#ifdef _WIN32
#define CE_API  __declspec(dllexport)
#define CE_APICALL  __stdcall
#else
#define CE_API
#define CE_APICALL 
#endif

// Ŀǰ���֧��8��ͨ��ͬʱ����
#define MAX_CHN			8

// ���ӿڿ�����붨��
typedef enum __CarEye_Error__
{
	// �޴���, �ɹ���
	CAREYE_NOERROR = 0,
	// ��֧�ֵĹ���, δʵ�ֵ�
	CAREYE_UNSUPPORTED = -1,
	// ϵͳ��ʼ��ʧ��
	CAREYE_INITFAIL = -2,
	// ������ͨ����������
	CAREYE_NOCHANNEL = -3,
	// ��������������������ʧ��
	CAREYE_CREATE_FAIL = -4,
	// ��Ч���������
	CAREYE_INVALID_PARAM = -5,
	// ��Ȩ����
	CAREYE_AUTH = -6,
	// ����ʧ��
	CAREYE_SEND_FAILE = -7,
}CarEyeError;

// ���������Ͷ���
typedef enum __PUSHER_TYPE__
{
	// ֱ������RTSP��������������
	PUSHER_RTSP = 0,
	// ���ͱ����ļ�RTSP����������
	PUSHER_NATIVEFILE_RTSP,
	// ����RTMP��������������
	PUSHER_RTMP,
	// ���ͱ����ļ�RTMP����������
	PUSHER_NATIVEFILE_RTMP,
}CarEyePusherType;

// ��Ƶ�������Ͷ���
typedef enum __VIDEO_CODE_TYPE__
{
	// H264����
	CAREYE_VCODE_H264 = 0x1C,
	// H265����
	CAREYE_VCODE_H265 = 0x48323635,
	// MJPEG����
	CAREYE_VCODE_MJPEG = 0x08,
	// MPEG4����
	CAREYE_VCODE_MPEG4 = 0x0D,
}CarEye_VCodeType;

// ��Ƶ�������Ͷ���
typedef enum __AUDIO_CODE_TYPE__
{
	// AAC����
	CAREYE_ACODE_AAC = 0x15002,
	// G711 Ulaw����
	CAREYE_ACODE_G711U = 0x10006,
	// G711 Alaw����
	CAREYE_ACODE_G711A = 0x10007,
	// G726����
	CAREYE_ACODE_G726 = 0x1100B,
}CarEye_ACodeType;

// Ҫ������ý����Ϣ�ṹ��
typedef struct __MEDIA_INFO_T
{
	// ��Ƶ��������
	CarEye_VCodeType VideoCodec;
	// ��Ƶ֡��, һ��Ϊ25
	unsigned int VideoFps;

	// ��Ƶ��������
	CarEye_ACodeType AudioCodec;
	// ��Ƶ������, ¼������һ��Ϊ8000
	unsigned int AudioSamplerate;
	// ��Ƶͨ����, һ��ѡ��Ϊ1
	unsigned int AudioChannel;
	// ��Ƶ������, ��������
	unsigned int AudioBitsPerSample;
}CarEye_MediaInfo;

// ��Ƶ֡���Ͷ���
typedef enum __VIDEO_FRAME_TYPE__
{
	// I֡
	VIDEO_FRAME_I = 0x01,
	// P֡
	VIDEO_FRAME_P = 0x02,
	// B֡
	VIDEO_FRAME_B = 0x03,
	// JPEG
	VIDEO_FRAME_J = 0x04,
}CarEyeVideoFrameType;

// ��Ƶ֡��ʶ
#define CAREYE_VFRAME_FLAG	0x00000001
// ��Ƶ֡��ʶ
#define CAREYE_AFRAME_FLAG	0x00000002

// ý��֡��Ϣ�ṹ����
typedef struct __CAREYE_AV_Frame_T
{
	// ֡���ͱ�ʶ CAREYE_VFRAME_FLAG����CAREYE_AFRAME_FLAG
	unsigned int	FrameFlag;
	// ֡�����ݳ���
	unsigned int	FrameLen;
	// ��Ƶ֡����, �ο�CarEyeVideoFrameType����
	unsigned int	VFrameType;
	// ���ݻ�����
	unsigned char	*Buffer;
	// ʱ���������
	unsigned int	Second;
	// ʱ��������޷���ȷ��΢����, ����΢����, ��Second����γɾ�ȷʱ��
	unsigned int	USecond;
}CarEye_AV_Frame;

// ������״̬����
typedef enum __CAREYE_STATE_TYPE__
{
	// �������ӷ�������
	CAREYE_STATE_CONNECTING = 1,
	// �ѳɹ����ӵ�������
	CAREYE_STATE_CONNECTED = 2,
	// ����������ʧ��
	CAREYE_STATE_CONNECT_FAILED = 3,
	// �����쳣�ж�, ��ʱӦ��ֹͣ������������������������...
	CAREYE_STATE_CONNECT_ABORT = 4,
	// ����������
	CAREYE_STATE_PUSHING = 5,
	// ��������Ͽ�����
	CAREYE_STATE_DISCONNECTED = 6,
	// �����ļ����������
	CAREYE_STATE_FILE_FINISHED = 7,
	// δ֪����
	CAREYE_STATE_ERROR = 0xFF,
}CarEyeStateType;

/*
* Comments: ������״̬�仯֪ͨ�ص���������
* Param channel: ��������ͨ����
* Param state: ʵʱ״̬
* Param type: ����������
* @Return int
*/
typedef int(*CarEyePusher_StateChanged)(int channel, CarEyeStateType state, CarEyePusherType type);

#ifdef __cplusplus
extern "C"
{
#endif

	/*
	* Comments: ʹ����Ч��Key����CarEye��������ע���Ա�ʹ��, ʹ�ñ�ϵͳǰ�������ע���������ʹ��
	* Param key: ��Ч����Կ
	* Param packName: ���Androidϵͳ��Ӧ�ó������
	* @Return int CAREYE_NOERROR: �ɹ�, ���ؽ���ο�CarEyeError
	*/
#ifdef ANDROID
	CE_API int CE_APICALL CarEye_Register(char* key, char* packName);
#else
	CE_API int CE_APICALL CarEye_Register(char* key);
#endif

	/*
	* Comments: ע������״̬�¼�
	* Param event: ��ȡ����״̬�Ļص�����
	* @Return None
	*/
	CE_API void CE_APICALL CarEye_RegisterStateChangedEvent(CarEyePusher_StateChanged event);

	/*
	* Comments: ����RTSP����ͨ�� ������ַ�����������������: rtsp://svrip:port/name
	* Param svrip: ��ý�������IP��ַ������
	* Param port: ��ý��������˿ں�
	* Param name: ������sdp��
	* Param mediaInfo: Ҫ������ý����Ϣ
	* @Return int ���ڵ���0: ����������ͨ���� С��0�����Ųο�CarEyeError
	*/
	CE_API int CE_APICALL CarEye_StartRTSPPusher(char* svrip, unsigned short port, char* name, CarEye_MediaInfo mediaInfo);

	/*
	* Comments: �ر�ָ����RTSP����ͨ��
	* Param channel: ��������RTSP����ͨ����
	* @Return int �Ƿ�ɹ��ر�, ״̬��ο�CarEyeError
	*/
	CE_API int CE_APICALL CarEye_StopRTSPPusher(int channel);

	/*
	* Comments: ��ȡ��ǰͨ�����������Ƿ��Ѿ����ӵ�����������������׼��
	* Param channel: ͨ����
	* @Return int 0δ����׼��, ��0����׼��
	*/
	CE_API int CE_APICALL CarEye_PusherIsReady(int channel);

	/*
	* Comments: ����RTSP���������ļ�ͨ�� ������ַ�����������������: rtsp://svrip:port/name
	* Param svrip: ��ý�������IP��ַ������
	* Param port: ��ý��������˿ں�
	* Param name: ������sdp��
	* Param fileName: Ҫ�����ı����ļ�·�� Ŀǰ��ʱ֧��MP4�ļ�
	* Param startMs: ������ʼ�ĺ�����
	* Param endMs: ���������ĺ����� endMs�������startMs, ��������ʧ��, ������������Ϊ0ʱ����ȫ�ļ�
	* @Return int ���ڵ���0: ����������ͨ���� С��0�����Ųο�CarEyeError
	*/
	CE_API int CE_APICALL CarEye_StartNativeFileRTSP(char* svrip, unsigned short port, char* name, char* fileName, int startMs, int endMs);

	/*
	* Comments: �ر�ָ���ı����ļ�RTSP����ͨ��
	* Param channel: ��������RTSP����ͨ����
	* @Return int �Ƿ�ɹ��ر�, ״̬��ο�CarEyeError
	*/
	CE_API int CE_APICALL CarEye_StopNativeFileRTSP(int channel);

	/*
	* Comments: ����RTMP����ͨ��
	* Param svrip: ��ý�������IP��ַ������
	* Param port: ��ý��������˿ں�
	* Param name: ������ͨ����
	* Param mediaInfo: Ҫ������ý����Ϣ
	* @Return int ���ڵ���0: ����������ͨ���� С��0�����Ųο�CarEyeError
	*/
	CE_API int CE_APICALL CarEye_StartRTMPPusher(char* svrip, unsigned short port, char* name, CarEye_MediaInfo mediaInfo);

	/*
	* Comments: �ر�ָ����RTMP����ͨ��
	* Param channel: ��������RTMP����ͨ����
	* @Return int �Ƿ�ɹ��ر�, ״̬��ο�CarEyeError
	*/
	CE_API int CE_APICALL CarEye_StopRTMPPusher(int channel);

	/*
	* Comments: ����RTMP���������ļ�ͨ��
	* Param svrip: ��ý�������IP��ַ������
	* Param port: ��ý��������˿ں�
	* Param name: ������ͨ����
	* Param fileName: Ҫ�����ı����ļ�·�� Ŀǰ��ʱ֧��MP4�ļ�
	* Param startMs: ������ʼ�ĺ�����
	* Param endMs: ���������ĺ����� endMs�������startMs, ��������ʧ��, ������������Ϊ0ʱ����ȫ�ļ�
	* @Return int ���ڵ���0: ����������ͨ���� С��0�����Ųο�CarEyeError
	*/
	CE_API int CE_APICALL CarEye_StartNativeFileRTMP(char* svrip, unsigned short port, char* name, char* fileName, int startMs, int endMs);

	/*
	* Comments: �ر�ָ���ı����ļ�RTMP����ͨ��
	* Param channel: ��������RTMP����ͨ����
	* @Return int �Ƿ�ɹ��ر�, ״̬��ο�CarEyeError
	*/
	CE_API int CE_APICALL CarEye_StopNativeFileRTMP(int channel);

	/*
	* Comments: ������ý�����ݵ�ָ��ͨ����
	* Param channel: Ҫ���͵�ͨ����
	* Param frame: Ҫ���͵�֡����
	* @Return int �Ƿ����ͳɹ�, ״̬��ο�CarEyeError
	*/
	CE_API int CE_APICALL CarEye_PushData(int channel, CarEye_AV_Frame* frame);

#ifdef __cplusplus
}
#endif

#endif