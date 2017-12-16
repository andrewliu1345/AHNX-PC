#include "stdafx.h"
#include "ToolFun.h"
#include "atlimage.h"
#include <fstream>


int bmp2png(char * bmppath, unsigned char * png)
{
	CImage image;
	char  tmpPath[MAX_PATH] = { 0 };
 	getTempPath(tmpPath);
	string pngPath;
	string bmpPath = bmppath;
	pngPath = tmpPath;
	pngPath +="temp.png";
	image.Load((LPCTSTR)bmpPath.c_str());
	image.Save((LPCTSTR)pngPath.c_str());
	ifstream pngfile(pngPath);
	if (pngfile.is_open())
	{
		pngfile.seekg(0, std::ios::end);
		int length = pngfile.tellg();
		pngfile.seekg(0, std::ios::beg);
		char* buffer = new char[length];
		pngfile.read(buffer, length);
		memcpy(png, buffer, length);
		delete[] buffer;
		return length;
	}
	return -1;
}

int getTempPath(char * tmpPath)
{
	int strlen = GetTempPath(MAX_PATH,tmpPath);
	//sprintf((char*)tmpPath, "TempPath=%ls", tmpPath);
	return strlen;
}
void HexStrToByte(const char* source, unsigned char* dest, int sourceLen)
{
	short i;
	unsigned char highByte, lowByte;

	for (i = 0; i < sourceLen; i += 2)
	{
		highByte = toupper(source[i]);
		lowByte = toupper(source[i + 1]);

		if (highByte > 0x39)
			highByte -= 0x37;
		else
			highByte -= 0x30;

		if (lowByte > 0x39)
			lowByte -= 0x37;
		else
			lowByte -= 0x30;

		dest[i / 2] = (highByte << 4) | lowByte;
	}
	return;
}

void ByteToHexStr(const unsigned char* source, char* dest, int sourceLen)
{
	short i;
	unsigned char highByte, lowByte;

	for (i = 0; i < sourceLen; i++)
	{
		highByte = source[i] >> 4;
		lowByte = source[i] & 0x0f;

		highByte += 0x30;

		if (highByte > 0x39)
			dest[i * 2] = highByte + 0x07;
		else
			dest[i * 2] = highByte;

		lowByte += 0x30;
		if (lowByte > 0x39)
			dest[i * 2 + 1] = lowByte + 0x07;
		else
			dest[i * 2 + 1] = lowByte;
	}
	return;
}