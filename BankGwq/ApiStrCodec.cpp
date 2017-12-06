#include "stdafx.h"
#include "ApiError.h"
#include "ApiStrCodec.h"

int _stdcall urlencode(int format, char *pIn, int iInLen, char *pOut, int iOutLen, char cSpecialCharFilter)
{
	int index, len, wlen, clen, outindex;
	wchar_t wBuf[4096];
	char cBuf[4096], tmpBuf[4], *pBuf;
	unsigned char ch;

	if ((format != CODEC_FORMAT_GB2312) && (format != CODEC_FORMAT_UTF8))
		return -1;
	if ((pIn == NULL) || (iInLen == 0) || (pOut == NULL) || (iOutLen == 0))
		return -1;

	if (format == CODEC_FORMAT_UTF8)
	{
		//Convert ansi to unicode...
		memset(wBuf, 0, sizeof(wchar_t) * 4096);
		wlen = MultiByteToWideChar(CP_OEMCP, 0, pIn, iInLen, wBuf, 0);
		MultiByteToWideChar(CP_OEMCP, 0, pIn, iInLen, wBuf, wlen);

		//Convert unicode to utf8...
		memset(cBuf, 0, 4096);
		clen = ::WideCharToMultiByte(CP_UTF8, 0, wBuf, wlen, cBuf, 0, NULL, NULL);
		WideCharToMultiByte(CP_UTF8, 0, wBuf, wlen, cBuf, clen, NULL, NULL);

		pBuf = cBuf;
	}
	else
	{
		pBuf = pIn;
		clen = iInLen;
	}

	//UrlEncode...
	outindex = 0;
	for (index = 0; index < clen; index++)
	{
		memset(tmpBuf, 0, 4);
		ch = (unsigned char)pBuf[index];
		if (((ch >= '0') && (ch <= '9')) || ((ch >= 'a') && (ch <= 'z')) || ((ch >= 'A') && (ch <= 'Z')))
			tmpBuf[0] = ch;
		else if ((ch == '.') || (ch == '-') || (ch == '*') || (ch == '_'))
			tmpBuf[0] = ch;
		else if(ch == cSpecialCharFilter)
			tmpBuf[0] = ch;
		else if (ch == ' ')
			tmpBuf[0] = '+';
		else
		{
			tmpBuf[0] = '%';
			//sprintf(&tmpBuf[1], "%02X", ch);
		}
		len = strlen(tmpBuf);
		memcpy(&pOut[outindex], tmpBuf, len);
		outindex += len;
	}
	return 0;
}

int _stdcall urldecode(int format, char *pIn, int iInLen, char *pOut, int iOutLen)
{
	int index, bufindex;
	int wlen, clen;
	char buffer[4096], hex[5];
	wchar_t wBuf[4096];

	//Decode to utf8...
	bufindex = 0;
	memset(buffer, 0, sizeof(buffer));
	for (index = 0; index < iInLen; index++)
	{
		if (pIn[index] == '%')
		{
			memset(hex, 0, sizeof(hex));
			hex[0] = '0';
			hex[1] = 'x';
			hex[2] = pIn[index + 1];
			hex[3] = pIn[index + 2];
			buffer[bufindex++] = (char)strtol(hex, NULL, 16);
			index += 2;
		}
		else if (pIn[index] == '+')
			buffer[bufindex++] = ' ';
		else
		{
			buffer[bufindex++] = pIn[index];
		}
	}

	//Convert utf8 to unicode
	memset(wBuf, 0, sizeof(wchar_t) * 4096);
	wlen = MultiByteToWideChar(CP_UTF8, 0, buffer, bufindex, wBuf, 0);
	MultiByteToWideChar(CP_UTF8, 0, buffer, bufindex, wBuf, wlen);

	//Convert unicode to ansi...
	memset(buffer, 0, 4096);
	clen = ::WideCharToMultiByte(CP_UTF8, 0, wBuf, wlen, buffer, 0, NULL, NULL);
	WideCharToMultiByte(CP_OEMCP, 0, wBuf, wlen, buffer, clen, NULL, NULL);

	if (iOutLen > clen)
		memcpy(pOut, buffer, clen);
	else
		memcpy(pOut, buffer, iOutLen);

	return 0;
}

const unsigned char base64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
int _stdcall base64encode(char *pIn, int iInLen, char *pOut, int iOutLen)
{
	int iIndex, oIndex;
	if ((pIn == NULL) || (pOut == NULL) || (iInLen <= 0) || (iOutLen <= 0))
		return ERRCODE_INVALID_PARAMETER;
	
	memset(pOut, 0, iOutLen);
	for (iIndex = 0, oIndex = 0; iIndex <= (iInLen - 3); iIndex += 3, oIndex += 4)
	{
		pOut[oIndex + 0] = (pIn[iIndex] & 0xFC) >> 2;
		pOut[oIndex + 1] = ((pIn[iIndex] & 0x03) << 4) + ((pIn[iIndex + 1] & 0xF0) >> 4);
		pOut[oIndex + 2] = ((pIn[iIndex + 1] & 0x0F) << 2) + ((pIn[iIndex + 2] & 0xC0) >> 6);
		pOut[oIndex + 3] = (pIn[iIndex+2] & 0x3F);
		if (oIndex > (iOutLen - 5))
			break;
	}
	switch ((iInLen % 3))
	{
		case 1:
			pOut[oIndex + 0] = (pIn[iIndex] & 0xFC) >> 2;
			pOut[oIndex + 1] = ((pIn[iIndex] & 0x03) << 4);
			pOut[oIndex + 2] = 64;
			pOut[oIndex + 3] = 64;
			oIndex += 4;
			break;
		case 2:
			pOut[oIndex + 0] = (pIn[iIndex] & 0xFC) >> 2;
			pOut[oIndex + 1] = ((pIn[iIndex] & 0x03) << 4) + ((pIn[iIndex + 1] & 0xF0) >> 4);
			pOut[oIndex + 2] = ((pIn[iIndex + 1] & 0x0F) << 2);
			pOut[oIndex + 3] = 64;
			oIndex += 4;
			break;
	}

	for (iIndex = 0; iIndex < oIndex; iIndex++)
		pOut[iIndex] = base64[(int)pOut[iIndex]];
	pOut[iIndex] = 0;

	return 0;
}

int _stdcall base64decode(char *pIn, int iInLen, char *pOut, int iOutLen)
{
	int iIndex, oIndex;
	unsigned char *pTemp;
	if ((pIn == NULL) || (pOut == NULL) || (iInLen <= 0) || (iOutLen <= 0))
		return ERRCODE_INVALID_PARAMETER;

	while (iInLen > 0 && (pIn[iInLen - 1] == '='))
	{
		pIn[iInLen - 1] = 0;
		iInLen--;
	}
	//map base64 ASCII character to 6bit value
	for (iIndex = 0; iIndex < iInLen; iIndex++)
	{
		pTemp = (unsigned char *)strchr((const char *)base64, (int)pIn[iIndex]);
		if (!pTemp)
			break;
		pIn[iIndex] = pTemp - (unsigned char *)base64;
	}

	memset(pOut, 0, iOutLen);
	for (iIndex = 0, oIndex = 0; iIndex < iInLen; iIndex += 4, oIndex += 3)
	{
		pOut[oIndex] = ((pIn[iIndex] & 0x3F) << 2) + ((pIn[iIndex + 1] & 0x30) >> 4);
		pOut[oIndex + 1] = ((pIn[iIndex + 1] & 0x0F) << 4) + ((pIn[iIndex + 2] & 0x3C) >> 2);
		pOut[oIndex + 2] = ((pIn[iIndex + 2] & 0x03) << 6) + (pIn[iIndex + 3] & 0x3F);
	}

	return 0;
}