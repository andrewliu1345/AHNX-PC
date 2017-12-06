#pragma once

enum CODEC_FORMAT
{
	CODEC_FORMAT_GB2312 = 1,
	CODEC_FORMAT_UTF8 = 2,
};

int _stdcall urlencode(int format, char *pIn, int iInLen, char *pOut, int iOutLen, char cSpecialCharFilter = 0);
int _stdcall urldecode(int format, char *pIn, int iInLen, char *pOut, int iOutLen);

int _stdcall base64encode(char *pIn, int iInLen, char *pOut, int iOutLen);
int _stdcall base64decode(char *pIn, int iInLen, char *pOut, int iOutLen);