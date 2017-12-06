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
	int strlen = GetTempPathA(MAX_PATH,tmpPath);
	
	//sprintf((char*)tmpPath, "TempPath=%ls", tmpPath);
	return strlen;
}
