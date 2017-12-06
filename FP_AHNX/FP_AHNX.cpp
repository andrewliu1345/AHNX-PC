// FP_AHNX.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"
#include "FP_AHNX.h"
#include "base64.h"
#include "ToolFun.h"
#include "Include/YNDLL.h"
#include <iostream>  
#include <fstream>  

// �����ѵ�����Ĺ��캯����
// �й��ඨ�����Ϣ������� FP_AHNX.h
CFP_AHNX::CFP_AHNX()
{
    return;
}
FP_AHNX_API int __stdcall FPGetFeature(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, char * psFeature, char * psErrInfo)
{
	unsigned char psFeatureInfo[1024 * 64] = { 0 };
	unsigned char pngdata[1024 * 64] = { 0 };
	int Fingerlength = 0;

	int iret = TcGetFingerFeature(0, NULL, psFeatureInfo, &Fingerlength, iTimeOut);
	if (iret != 0)
	{
		psErrInfo = "-29|ָ�ƶ�ȡʧ��";
		return -29;
	}
	int iLength = 0;
	unsigned char  psfeature[1024*64] = { 0 };
	string feature(( char *)psFeatureInfo);
	base64::base64_decode(feature, psfeature, &iLength);
	memcpy(psFeature, psfeature, iLength);
	return 0;
}
FP_AHNX_API int __stdcall FPGetTemplate(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, char * psTemplate, int iLength, char * psErrInfo)
{
	unsigned char psFeatureInfo[1024] = { 0 };
	//unsigned char pngdata[1024 * 64] = { 0 };
	int Fingerlength = 0;
	
	int iret = TcGetFingerTemplate(0, NULL, psFeatureInfo, &Fingerlength, iTimeOut);
	if (iret != 0)
	{
		psErrInfo = "-29|ָ�ƶ�ȡʧ��";
		return -29;
	}
	unsigned char  psfeature[1024] = { 0 };
	string feature((char *)psFeatureInfo);
	base64::base64_decode(feature, psfeature, &iLength);
	memcpy(psTemplate, psfeature, iLength);
	return 0;
}
FP_AHNX_API int __stdcall FPGetImgData(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, int iIndex, char * psImgData, char * psErrInfo)
{
	char bmppath[MAX_PATH] = { 0 };
	string bmpfile;
	getTempPath(bmppath);
	bmpfile = bmppath;
	bmpfile += "tmp.bmp";
	memset(bmppath, 0, MAX_PATH);
	strcpy(bmppath, bmpfile.c_str());
	unsigned char psFeatureInfo[1024 * 64] = { 0 };
	unsigned char pngdata[1024 * 64] = { 0 };
	char bmpdata[31478] = { 0 };
	int Fingerlength = 0;
	if (psImgData==NULL)
	{
		psErrInfo = "-1|��������";
		return -1;
	}
	int iret = TcGetFingerFeature(0, bmppath, psFeatureInfo, &Fingerlength, iTimeOut);
	if (iret != 0)
	{
		psErrInfo = "-29|ָ�ƶ�ȡʧ��";
		return -29;
	}
	ifstream in(bmppath,ios::binary);
	if (!in.is_open())
	{
		psErrInfo = "-30|ָ��ͼ������ʧ��";
		return -30;
	}
	in.read(bmpdata, 31478);
	string s = base64::base64_encode((unsigned char *)bmpdata + 1078, 152 * 200);
	const char * p=s.c_str();
	memcpy(psImgData, p, s.length());
	return 0;
}