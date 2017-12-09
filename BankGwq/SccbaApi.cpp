#include "stdafx.h"
#include "GwqDllComm.h"
#include "ApiStrCodec.h"
#include "SccbaApi.h"
#include "Include/YNDLL.h"
#include "ToolFun.h"
#include "base64.h"
#include <iostream>  
#include <fstream>  
#include <atlstr.h>


#define SIGNPDF_DEBUG

#ifdef BANKGWQ_SCCBA_API
CString ToVoiceStr(int VoiceType);

int WriteLog(char *log)
{
	FILE *pFile;

	if ((log == NULL) || (!strlen(log)))
		return 1;

	pFile = fopen("d:\\gwq.log", "a+");
	if (pFile)
	{
		fwrite(log, strlen(log), 1, pFile);
		fwrite("\r\n", 2, 1, pFile);
		fclose(pFile);
	}
	return 0;
}

//1. Common API
BANKGWQ_API int __stdcall SCCBA_GB18030ToUTF8(char *pStrGB18030, unsigned char **pStrUtf8)
{
	int iLen, iLenUnicode;
	unsigned char *pUnicodeBuf;

	if ((pStrGB18030 == NULL) || (pStrUtf8 == NULL))
		return ERRCODE_INVALID_PARAMETER;

	//Convert string from GB18030 to Unicode
	iLen = strlen(pStrGB18030);
	iLenUnicode = MultiByteToWideChar(54936, 0, pStrGB18030, -1, NULL, 0);
	pUnicodeBuf = new BYTE[iLenUnicode * sizeof(wchar_t)];
	memset(pUnicodeBuf, 0, iLenUnicode * sizeof(wchar_t));
	MultiByteToWideChar(54936, 0, pStrGB18030, iLen, (wchar_t *)pUnicodeBuf, iLenUnicode * sizeof(wchar_t));

	//Convert string from Unicode to UTF8
	iLen = WideCharToMultiByte(CP_UTF8, 0, (wchar_t *)pUnicodeBuf, -1, NULL, 0, NULL, NULL);
	*pStrUtf8 = new BYTE[iLen + 1];
	memset(*pStrUtf8, 0, iLen + 1);
	WideCharToMultiByte(CP_UTF8, 0, (wchar_t *)pUnicodeBuf, iLenUnicode * sizeof(wchar_t), (char *)*pStrUtf8, iLen, NULL, NULL);

	delete[] pUnicodeBuf;

	return ERRCODE_SUCCESS;
}

//2. Push Information  启动交互信息
BANKGWQ_API int __stdcall SCCBA_StartInfoHtml(int iPortNo, int iTimeOut, int modex, char* strVoice, char *Info, int *iResult)
{
	int iRet, iIndex, iHeaderSize, iWriteSize, iReadSize;
	int iSegmentSize, iTotalSize, iCurrentlSegment, iTotalSegment;
	int iVoiceLen, iVoiceSize, iInfoLen, iInfoSize;
	unsigned char szBuffer[4096];
	unsigned char *pUtf8Voice, *pUtf8Info;

	pUtf8Voice = pUtf8Info = NULL;

	iVoiceSize = iVoiceLen = 0;
	if (strVoice)
	{
		if (!SCCBA_GB18030ToUTF8(strVoice, &pUtf8Voice))
			iVoiceSize = iVoiceLen = strlen((char *)pUtf8Voice);
	}
	iInfoSize = iInfoLen = 0;
	if (Info)
	{
		if (!SCCBA_GB18030ToUTF8(Info, &pUtf8Info))
			iInfoSize = iInfoLen = strlen((char *)pUtf8Info);
	}

	if (iInfoSize <= 0)
	{
		if (pUtf8Voice)
			delete[] pUtf8Voice;
		if (pUtf8Info)
			delete[] pUtf8Info;
		return ERRCODE_INVALID_PARAMETER;
	}

	iRet = comm_open(iPortNo, iPortNo, 9600);
	if (iRet)
		return iRet;

	iTotalSize = iVoiceSize + iInfoSize;
	iTotalSegment = iTotalSize / 4000;
	if (iTotalSize % 4000)
		iTotalSegment += 1;
	iCurrentlSegment = iTotalSegment;

	do {
		iIndex = 0;
		memset(szBuffer, 0, sizeof(szBuffer));
		szBuffer[iIndex++] = 'S';                                            //Instruction code
		szBuffer[iIndex++] = 'I';
		sprintf((char *)&szBuffer[iIndex], "%03d", iTimeOut);                //Timeout
		iIndex += 3;
		szBuffer[iIndex++] = modex;                                          //Type
		sprintf((char *)&szBuffer[iIndex], "%04d", iVoiceLen);               //VoiceLen
		iIndex += 4;
		sprintf((char *)&szBuffer[iIndex], "%04d", iInfoLen);                //InfoLen
		iIndex += 4;
		sprintf((char *)&szBuffer[iIndex], "%04d", iCurrentlSegment);        //Current Segment
		iIndex += 4;
		sprintf((char *)&szBuffer[iIndex], "%04d", iTotalSegment);           //Total Segment
		iIndex += 4;

		if (iTotalSize >= 4000)
			iSegmentSize = 4000;
		else
			iSegmentSize = iTotalSize;
		iTotalSize -= iSegmentSize;
		sprintf((char *)&szBuffer[iIndex], "%04d", iSegmentSize);            //Segment Size
		iIndex += 4;
		iHeaderSize = iIndex;

		iWriteSize = 0;                                                      //Segment Data
		if (iVoiceSize)
		{
			if (iVoiceSize > iSegmentSize)
			{
				memcpy(&szBuffer[iIndex], &pUtf8Voice[iVoiceLen - iVoiceSize], iSegmentSize);
				iVoiceSize -= iSegmentSize;
				iWriteSize = iSegmentSize;
			}
			else
			{
				memcpy(&szBuffer[iIndex], &pUtf8Voice[iVoiceLen - iVoiceSize], iVoiceSize);
				iWriteSize = iVoiceSize;
				iVoiceSize = 0;
			}
			iIndex += iWriteSize;
		}

		if (iInfoSize && (iWriteSize < iSegmentSize))
		{
			if (iInfoSize >= (iSegmentSize - iWriteSize))
			{
				memcpy(&szBuffer[iIndex], &pUtf8Info[iInfoLen - iInfoSize], (iSegmentSize - iWriteSize));
				iInfoSize -= (iSegmentSize - iWriteSize);
				iIndex += (iSegmentSize - iWriteSize);
				iWriteSize = iSegmentSize;
			}
			else
			{
				memcpy(&szBuffer[iIndex], &pUtf8Info[iInfoLen - iInfoSize], iInfoSize);
				iWriteSize += iInfoSize;
				iIndex += iInfoSize;
				iInfoSize = 0;
			}
		}

		iReadSize = sizeof(szBuffer);
		iRet = comm_frame_write(szBuffer, iHeaderSize + iSegmentSize, szBuffer, &iReadSize, 5000);
		if (iRet)
			break;
		iCurrentlSegment--;
	} while (iTotalSize);

	if ((!iRet) && iResult && (iTimeOut > 0) && modex)
	{
		iReadSize = sizeof(szBuffer);
		memset(szBuffer, 0, sizeof(szBuffer));
		iRet = comm_frame_receive(szBuffer, &iReadSize, iTimeOut * 1000);
		if ((szBuffer[0] == 'S') && (szBuffer[1] == 'I') && (szBuffer[2] == '2') && (szBuffer[3] == '0'))
			*iResult = szBuffer[4];
	}

	comm_close();

	if (pUtf8Voice)
		delete[] pUtf8Voice;
	if (pUtf8Info)
		delete[] pUtf8Info;

	return iRet;
}

//3. Public Interface
BANKGWQ_API int __stdcall SCCBA_inputNumber(int iPortNo, int iLength, int iControlType, int iTimeOut, char *strVoice, char *iResult)
{
	int iRet, iIndex, iVoiceLen, iReadSize;
	unsigned char szBuffer[4096];
	unsigned char *pUtf8Voice;

	iRet = comm_open(iPortNo, iPortNo, 9600);
	if (iRet)
		return iRet;

	iIndex = 0;
	memset(szBuffer, 0, sizeof(szBuffer));
	szBuffer[iIndex++] = 'I';                                             //Instruction code
	szBuffer[iIndex++] = 'N';
	sprintf((char *)&szBuffer[iIndex], "%02d", iLength);                  //Input Length
	iIndex += 2;
	szBuffer[iIndex++] = (unsigned char)iControlType + 0x30;              //Control Type
	sprintf((char *)&szBuffer[iIndex], "%03d", iTimeOut);                 //Input Length
	iIndex += 3;

	iVoiceLen = 0;
	if (strVoice)
	{
		if (!SCCBA_GB18030ToUTF8(strVoice, &pUtf8Voice))
			iVoiceLen = strlen((char *)pUtf8Voice);
	}
	if (iVoiceLen > 4000)
		iVoiceLen = 4000;
	sprintf((char *)&szBuffer[iIndex], "%04d", iVoiceLen);
	iIndex += 4;
	memcpy(&szBuffer[iIndex], pUtf8Voice, iVoiceLen);
	iIndex += iVoiceLen;

	iReadSize = sizeof(szBuffer);
	iRet = comm_frame_write(szBuffer, iIndex, szBuffer, &iReadSize, 5000);
	if ((!iRet) && iResult && (iTimeOut > 0))
	{
		char szLen[5];
		int iInputLen;
		iReadSize = sizeof(szBuffer);
		memset(szBuffer, 0, sizeof(szBuffer));
		iRet = comm_frame_receive(szBuffer, &iReadSize, iTimeOut * 1000);
		if ((szBuffer[0] == 'I') && (szBuffer[1] == 'N'))
		{
			memset(szLen, 0, sizeof(szLen));
			szLen[0] = szBuffer[4];
			szLen[1] = szBuffer[5];

			iInputLen = atoi(szLen);
			if (iInputLen > 99)
				iInputLen = 99;
			memcpy(iResult, &szBuffer[6], iInputLen);
		}
	}
	comm_close();

	if (pUtf8Voice)
		delete[] pUtf8Voice;

	return iRet;
}
//播放语音
BANKGWQ_API int __stdcall SCCBA_PlayVoice(int iPortNo, char *strVoice)
{
	int iRet, iIndex, iVoiceLen, iReadSize;
	unsigned char szBuffer[4096];
	unsigned char *pUtf8Voice;

	if (strVoice == NULL)
		return ERRCODE_INVALID_PARAMETER;

	iRet = comm_open(iPortNo, iPortNo, 9600);
	if (iRet)
		return iRet;

	iVoiceLen = 0;
	if (strVoice)
		if (!SCCBA_GB18030ToUTF8(strVoice, &pUtf8Voice))
			iVoiceLen = strlen((char *)pUtf8Voice);

	if (iVoiceLen > 4000)
		iVoiceLen = 4000;

	iIndex = 0;
	memset(szBuffer, 0, sizeof(szBuffer));
	szBuffer[iIndex++] = 'P';                                             //Instruction code
	szBuffer[iIndex++] = 'V';
	sprintf((char *)&szBuffer[iIndex], "%04d", iVoiceLen);
	iIndex += 4;
	memcpy(&szBuffer[iIndex], pUtf8Voice, iVoiceLen);
	iIndex += iVoiceLen;

	iReadSize = sizeof(szBuffer);
	iRet = comm_frame_write(szBuffer, iIndex, szBuffer, &iReadSize, 5000);

	comm_close();

	if (pUtf8Voice)
		delete[] pUtf8Voice;

	return iRet;
}

int  DownOneFile(int iPortNo, char * pFilePath, int iFileType)
{
	string filename = PathFindFileName(pFilePath);
	char* pFilename = (char *)filename.c_str();
	FILE *pFile;
	int iRet, iIndex;
	int iFilenameLen;
	int iFileSize, iSegmentSize, iReadSize, iCurrentlSegment, iTotalSegment;
	unsigned char *pUtf8Filename;

	unsigned char szBuffer[FRAME_MAX_SIZE];

	if ((pFilePath == NULL) || (pFilename == NULL))
		return ERRCODE_INVALID_PARAMETER;

	pFile = fopen(pFilePath, "rb");
	if (pFile == NULL)
		return ERRCODE_OPENFILE_FAILURE;

	iRet = comm_open(iPortNo, iPortNo, 9600);
	if (iRet)
	{
		fclose(pFile);
		return iRet;
	}

	fseek(pFile, 0, SEEK_END);
	iFileSize = ftell(pFile);
	iTotalSegment = iFileSize / 4000;
	if (iFileSize % 4000)
		iTotalSegment += 1;
	iCurrentlSegment = iTotalSegment;
	fseek(pFile, 0, SEEK_SET);

	iFilenameLen = 0;
	pUtf8Filename = NULL;
	if (pFilename)
		if (!SCCBA_GB18030ToUTF8(pFilename, &pUtf8Filename))
			iFilenameLen = strlen((char *)pUtf8Filename);



	while (iFileSize)
	{
		iIndex = 0;
		memset(szBuffer, 0, sizeof(szBuffer));
		szBuffer[iIndex++] = 'A';                                      //Instruction code
		szBuffer[iIndex++] = 'F';
		sprintf((char *)&szBuffer[iIndex], "%02d", iFileType);         //File type
		iIndex += 2;
		sprintf((char *)&szBuffer[iIndex], "%02d", iFilenameLen);      //Filename Length
		iIndex += 2;
		memcpy((char *)&szBuffer[iIndex], pUtf8Filename, iFilenameLen);    //Filename
		iIndex += iFilenameLen;
		sprintf((char *)&szBuffer[iIndex], "%04d", iCurrentlSegment);  //Current Segment
		iIndex += 4;
		sprintf((char *)&szBuffer[iIndex], "%04d", iTotalSegment);     //Total Segment
		iIndex += 4;
		if (iFileSize >= 4000)                                         //Segment Size
			iSegmentSize = 4000;
		else
			iSegmentSize = iFileSize;
		iFileSize -= iSegmentSize;
		sprintf((char *)&szBuffer[iIndex], "%04d", iSegmentSize);
		iIndex += 4;
		fread(&szBuffer[iIndex], 1, iSegmentSize, pFile);              //Segment Data

		iReadSize = sizeof(szBuffer);
		iRet = comm_frame_write(szBuffer, iIndex + iSegmentSize, szBuffer, &iReadSize, 5000);
		if (iRet)
			break;
		iCurrentlSegment--;
	}

	fclose(pFile);
	comm_close();

	if (pUtf8Filename)
		delete[] pUtf8Filename;

	return iRet;
}

//4. Electronic Card  下载员工头像
BANKGWQ_API int __stdcall SCCBA_DownHeadFile(int iPortNo, char *pFilePath, char* pFilename)
{
	return DownOneFile(iPortNo, pFilePath, 1);

}
//查询员工头像
BANKGWQ_API int __stdcall SCCBA_FindHeadPhoto(int iPortNo, char * pFilename, int pFilenameLength)
{
	int iRet, iFileListLen;
	unsigned char szBuffer[FRAME_MAX_SIZE];

	iRet = comm_open(iPortNo, iPortNo, 9600);
	if (iRet)
		return iRet;

	szBuffer[0] = 'Q';
	szBuffer[1] = 'F';
	sprintf((char *)&szBuffer[2], "%02d", 1);
	iFileListLen = sizeof(szBuffer);
	iRet = comm_frame_write(szBuffer, 4, szBuffer, &iFileListLen, 5000);
	if (!iRet)
	{
		if ((szBuffer[0] == 'Q') && (szBuffer[1] == 'F'))
		{
			char temp[32];
			int index;

			memset(temp, 0, sizeof(temp));
			memcpy(temp, &szBuffer[2], 4);
			iFileListLen = atoi(temp);
			for (index = 0; index < iFileListLen; index++)
				if (szBuffer[6 + index] == '&')
					szBuffer[6 + index] = ';';

			if (pFilenameLength > iFileListLen)
				memcpy(pFilename, &szBuffer[6], iFileListLen);
			else
				memcpy(pFilename, &szBuffer[6], pFilenameLength);
		}
	}
	comm_close();

	return iRet;
}
//删除员工头像
BANKGWQ_API int __stdcall SCCBA_DelHeadFile(int iPortNo, char * pFilename)
{
	int iRet, iFilenameLen, iBufLen;
	char szBuffer[FRAME_MAX_SIZE];
	unsigned char *pUtf8Filename;

	if (pFilename == NULL)
		return ERRCODE_INVALID_PARAMETER;

	iRet = comm_open(iPortNo, iPortNo, 9600);
	if (iRet)
		return iRet;

	iFilenameLen = 0;
	pUtf8Filename = NULL;
	if (!SCCBA_GB18030ToUTF8(pFilename, &pUtf8Filename))
		iFilenameLen = strlen((char *)pUtf8Filename);
	szBuffer[0] = 'D';
	szBuffer[1] = 'F';
	sprintf(&szBuffer[2], "%02d", 1);
	sprintf(&szBuffer[4], "%04d", iFilenameLen);
	memcpy(&szBuffer[8], pUtf8Filename, iFilenameLen);
	iBufLen = sizeof(szBuffer);
	iRet = comm_frame_write((unsigned char *)szBuffer, 8 + strlen(pFilename), (unsigned char *)szBuffer, &iBufLen, 5000);

	comm_close();
	if (pUtf8Filename)
		delete[] pUtf8Filename;

	return iRet;
}
//启动电子工牌
BANKGWQ_API int __stdcall SCCBA_StartElectronicCard(int iPortNo, /*char extendPort, int iBaudRate,*/ char *tellerName, char *tellerNo, int nStarLevel, char *headFile, int iTimeOut)
{
	int iRet, iLen, iIndex;
	char szBuffer[FRAME_MAX_SIZE];
	unsigned char *pUtf8TellerName, *pUtf8TellerNo, *pUtf8HeadFile;

	iRet = comm_open(iPortNo, iPortNo, 9600);
	if (iRet)
		return iRet;

	pUtf8TellerName = pUtf8TellerNo = pUtf8HeadFile = NULL;
	iIndex = 0;
	szBuffer[iIndex++] = 'S';
	szBuffer[iIndex++] = 'C';

	iLen = 0;
	if (tellerName)
		if (!SCCBA_GB18030ToUTF8(tellerName, &pUtf8TellerName))
			iLen = strlen((char *)pUtf8TellerName);
	sprintf(&szBuffer[iIndex], "%02d", iLen);
	iIndex += 2;
	memcpy(&szBuffer[iIndex], pUtf8TellerName, iLen);
	iIndex += iLen;

	iLen = 0;
	if (tellerNo)
		if (!SCCBA_GB18030ToUTF8(tellerNo, &pUtf8TellerNo))
			iLen = strlen((char *)pUtf8TellerNo);
	sprintf(&szBuffer[iIndex], "%02d", iLen);
	iIndex += 2;
	memcpy(&szBuffer[iIndex], pUtf8TellerNo, iLen);
	iIndex += iLen;

	sprintf(&szBuffer[iIndex++], "%1d", nStarLevel);

	iLen = 0;
	if (headFile)
		if (!SCCBA_GB18030ToUTF8(headFile, &pUtf8HeadFile))
			iLen = strlen((char *)pUtf8HeadFile);
	sprintf(&szBuffer[iIndex], "%03d", iLen);
	iIndex += 3;
	memcpy(&szBuffer[iIndex], pUtf8HeadFile, iLen);
	iIndex += iLen;

	sprintf(&szBuffer[iIndex], "%03d", iTimeOut);
	iIndex += 3;

	iLen = sizeof(szBuffer);
	iRet = comm_frame_write((unsigned char *)szBuffer, iIndex, (unsigned char *)szBuffer, &iLen, 5000);

	comm_close();

	if (pUtf8TellerName)
		delete[] pUtf8TellerName;
	if (pUtf8TellerNo)
		delete[] pUtf8TellerNo;
	if (pUtf8HeadFile)
		delete[] pUtf8HeadFile;

	return iRet;
}

BANKGWQ_API int __stdcall SCCBA_ReadDeviceId(int iPortNo, char *pDeviceId, int iSize)
{
	int iRet, iLen;
	unsigned char szBuffer[FRAME_MAX_SIZE];

	if ((pDeviceId == NULL) || (iSize <= 0))
		return ERRCODE_INVALID_PARAMETER;

	memset(pDeviceId, 0, iSize);

	iRet = comm_open(iPortNo, iPortNo, 9600);
	if (iRet)
		return iRet;


	szBuffer[0] = 'R';
	szBuffer[1] = 'D';
	iLen = sizeof(szBuffer);
	iRet = comm_frame_write(szBuffer, 4, szBuffer, &iLen, 5000);
	if (!iRet)
	{
		if ((szBuffer[0] == 'R') && (szBuffer[1] == 'D') && (iLen >= 8))
		{
			char temp[32];

			memset(temp, 0, sizeof(temp));
			memcpy(temp, &szBuffer[4], 4);
			iLen = atoi(temp);
			if (iLen > iSize)
				memcpy(pDeviceId, &szBuffer[8], iSize);
			else
				memcpy(pDeviceId, &szBuffer[8], iLen);
		}
	}
	comm_close();

	return iRet;
}

//5. Evaluate  启用评价
BANKGWQ_API int __stdcall SCCBA_StartEvaluate(int iPortNo, char * tellerID, char * headFile, char *tellerName, char *strOperData, char *strDispData, char *strVoice, int strDispTimeout, int strTimeout, char *iResult)
{
	int iRet, iLen, iIndex;
	unsigned char szBuffer[FRAME_MAX_SIZE];
	unsigned char *pUtf8TellerID, *pUtf8HeadFile, *pUtf8TellerName, *pUtf8OperData, *pUtf8DispData, *pUtf8Voice;

	iRet = comm_open(iPortNo, iPortNo, 9600);
	if (iRet)
		return iRet;

	pUtf8TellerID = pUtf8HeadFile = pUtf8TellerName = pUtf8OperData = pUtf8DispData = pUtf8Voice = NULL;

	iIndex = 0;
	szBuffer[iIndex++] = 'S';
	szBuffer[iIndex++] = 'E';

	iLen = 0;
	if (tellerID)
		if (!SCCBA_GB18030ToUTF8(tellerID, &pUtf8TellerID))
			iLen = strlen((char *)pUtf8TellerID);
	sprintf((char *)&szBuffer[iIndex], "%02d", iLen);
	iIndex += 2;
	memcpy(&szBuffer[iIndex], pUtf8TellerID, iLen);
	iIndex += iLen;

	iLen = 0;
	if (headFile)
		if (!SCCBA_GB18030ToUTF8(headFile, &pUtf8HeadFile))
			iLen = strlen((char *)pUtf8HeadFile);
	sprintf((char *)&szBuffer[iIndex], "%03d", iLen);
	iIndex += 3;
	memcpy(&szBuffer[iIndex], pUtf8HeadFile, iLen);
	iIndex += iLen;

	iLen = 0;
	if (tellerName)
		if (!SCCBA_GB18030ToUTF8(tellerName, &pUtf8TellerName))
			iLen = strlen((char *)pUtf8TellerName);
	sprintf((char *)&szBuffer[iIndex], "%02d", iLen);
	iIndex += 2;
	memcpy(&szBuffer[iIndex], pUtf8TellerName, iLen);
	iIndex += iLen;

	iLen = 0;
	if (strOperData)
		if (!SCCBA_GB18030ToUTF8(strOperData, &pUtf8OperData))
			iLen = strlen((char *)pUtf8OperData);
	sprintf((char *)&szBuffer[iIndex], "%03d", iLen);
	iIndex += 3;
	memcpy(&szBuffer[iIndex], pUtf8OperData, iLen);
	iIndex += iLen;

	iLen = 0;
	if (strDispData)
		if (!SCCBA_GB18030ToUTF8(strDispData, &pUtf8DispData))
			iLen = strlen((char *)pUtf8DispData);
	sprintf((char *)&szBuffer[iIndex], "%03d", iLen);
	iIndex += 3;
	memcpy(&szBuffer[iIndex], pUtf8DispData, iLen);
	iIndex += iLen;

	iLen = 0;
	if (strVoice)
		if (!SCCBA_GB18030ToUTF8(strVoice, &pUtf8Voice))
			iLen = strlen((char *)pUtf8Voice);
	sprintf((char *)&szBuffer[iIndex], "%03d", iLen);
	iIndex += 3;
	memcpy(&szBuffer[iIndex], pUtf8Voice, iLen);
	iIndex += iLen;

	sprintf((char *)&szBuffer[iIndex], "%03d", strDispTimeout);
	iIndex += 3;
	sprintf((char *)&szBuffer[iIndex], "%03d", strTimeout);
	iIndex += 3;

	iLen = sizeof(szBuffer);
	iRet = comm_frame_write(szBuffer, iIndex, szBuffer, &iLen, 5000);
	if (!iRet)
	{
		iLen = sizeof(szBuffer);
		memset(szBuffer, 0, sizeof(szBuffer));
		iRet = comm_frame_receive(szBuffer, &iLen, strTimeout * 1000);
		if ((szBuffer[0] == 'S') && (szBuffer[1] == 'E') && (szBuffer[2] == '2') && (szBuffer[3] == '0'))
			if (iResult)
				*iResult = szBuffer[4];
	}
	comm_close();

	if (pUtf8TellerID)
		delete[] pUtf8TellerID;
	if (pUtf8HeadFile)
		delete[] pUtf8HeadFile;
	if (pUtf8TellerName)
		delete[] pUtf8TellerName;
	if (pUtf8OperData)
		delete[] pUtf8OperData;
	if (pUtf8DispData)
		delete[] pUtf8DispData;
	if (pUtf8Voice)
		delete[] pUtf8Voice;

	return iRet;
}

//#define READPIN_DEBUG
//6. Security Keyboard  获取密码
BANKGWQ_API int __stdcall SCCBA_ReadPin(int iPortNo, int iEncryType, int iTimes, int iLength, int iTimeout, char *strVoice, char * strInfo, int EndType, char *iResult)
{
	int iRet, iIndex, iVoiceLen, iDispLen, iReadSize;
	char *pInfo;
	unsigned char szBuffer[FRAME_MAX_SIZE];
	unsigned char *pUtf8Voice, *pUtf8Info;


	if ((iResult == NULL) || (iTimeout <= 0))
		return ERRCODE_INVALID_PARAMETER;

	iRet = comm_open(iPortNo, iPortNo, 9600);
	if (iRet)
		return iRet;

	if (iLength < 1)
		iLength = 1;
	else if (iLength > 12)
		iLength = 12;
	pUtf8Voice = pUtf8Info = NULL;
	iIndex = 0;
	memset(szBuffer, 0, sizeof(szBuffer));
	szBuffer[iIndex++] = 'R';                                             //Instruction code
	szBuffer[iIndex++] = 'P';
	sprintf((char *)&szBuffer[iIndex++], "%1d", iEncryType);              //iEncryType
	sprintf((char *)&szBuffer[iIndex++], "%1d", iTimes);                  //iTimes
	sprintf((char *)&szBuffer[iIndex], "%02d", iLength);                  //iLength
	iIndex += 2;
	sprintf((char *)&szBuffer[iIndex], "%03d", iTimeout);                 //iTimeout
	iIndex += 3;

	iVoiceLen = 0;
	if (strVoice)
		if (!SCCBA_GB18030ToUTF8(strVoice, &pUtf8Voice))
			iVoiceLen = strlen((char *)pUtf8Voice);
	if (iVoiceLen > 2000)
		iVoiceLen = 2000;
	sprintf((char *)&szBuffer[iIndex], "%04d", iVoiceLen);
	iIndex += 4;
	memcpy(&szBuffer[iIndex], pUtf8Voice, iVoiceLen);
	iIndex += iVoiceLen;

	iDispLen = 0;
	if (strInfo)
	{
#ifdef READPIN_DEBUG
		FILE *pInfoFile;
		pInfoFile = fopen("d:\\dump.txt", "wb");
		if (pInfoFile != NULL)
		{
			fwrite(strInfo, strlen(strInfo), 1, pInfoFile);
			fclose(pInfoFile);
		}
#endif
		for (pInfo = strInfo; pInfo;)
		{
			pInfo = strstr(pInfo, "36px");
			if (!pInfo)
				break;
			pInfo[0] = '2';
			pInfo[1] = '4';
			pInfo += 2;
		}

		if (!SCCBA_GB18030ToUTF8(strInfo, &pUtf8Info))
			iDispLen = strlen((char *)pUtf8Info);
	}
	if (iDispLen > 2000)
		iDispLen = 2000;
	sprintf((char *)&szBuffer[iIndex], "%04d", iDispLen);
	iIndex += 4;
	memcpy(&szBuffer[iIndex], pUtf8Info, iDispLen);
	iIndex += iDispLen;

	sprintf((char *)&szBuffer[iIndex++], "%1d", EndType);              //EndType

	iReadSize = sizeof(szBuffer);
	iRet = comm_frame_write(szBuffer, iIndex, szBuffer, &iReadSize, 5000);
	if ((!iRet) && iResult && (iTimeout > 0))
	{
		char szLen[5];
		int iInputLen;
		iReadSize = sizeof(szBuffer);
		memset(szBuffer, 0, sizeof(szBuffer));
		iRet = comm_frame_receive(szBuffer, &iReadSize, iTimeout * 1000);
		if ((szBuffer[0] == 'R') && (szBuffer[1] == 'P') && (iReadSize > 8))
		{
			memset(szLen, 0, sizeof(szLen));
			szLen[0] = szBuffer[4];
			szLen[1] = szBuffer[5];

			iInputLen = atoi(szLen);
			if (iInputLen > 99)
				iInputLen = 99;
			memcpy(iResult, &szBuffer[8], iInputLen);
		}
		else
		{
			memset(iResult, 0x30, 32);
			iResult[32] = 0;
		}
	}
	comm_close();
	if (pUtf8Voice)
		delete[] pUtf8Voice;
	if (pUtf8Info)
		delete[] pUtf8Info;
	return iRet;
}
//初始化密码键盘
BANKGWQ_API int __stdcall SCCBA_InitPinPad(int iPortNo)
{
	int iRet, iLen;
	unsigned char szBuffer[FRAME_MAX_SIZE];

	iRet = comm_open(iPortNo, iPortNo, 9600);
	if (!iRet)
	{
		szBuffer[0] = 'I';
		szBuffer[1] = 'P';
		iLen = sizeof(szBuffer);
		iRet = comm_frame_write(szBuffer, 2, szBuffer, &iLen, 5000);
		comm_close();
	}
	return iRet;
}
//更新主秘钥
BANKGWQ_API int __stdcall SCCBA_UpdateMKey(int iPortNo, int ZmkIndex, int ZmkLength, char *Zmk, char* CheckValue)
{
	int iRet, iIndex, iLen;
	unsigned char szBuffer[FRAME_MAX_SIZE];

	iRet = comm_open(iPortNo, iPortNo, 9600);
	if (iRet)
		return iRet;

	iIndex = 0;
	szBuffer[iIndex++] = 'U';
	szBuffer[iIndex++] = 'M';
	szBuffer[iIndex++] = (unsigned char)ZmkIndex;
	szBuffer[iIndex++] = (unsigned char)ZmkLength;

	sprintf((char *)&szBuffer[iIndex], "%02d", strlen(Zmk));
	iIndex += 2;
	memcpy((char *)&szBuffer[iIndex], Zmk, strlen(Zmk));
	iIndex += strlen(Zmk);

	sprintf((char *)&szBuffer[iIndex], "%02d", strlen(CheckValue));
	iIndex += 2;
	memcpy((char *)&szBuffer[iIndex], CheckValue, strlen(CheckValue));
	iIndex += strlen(CheckValue);

	iLen = sizeof(szBuffer);
	iRet = comm_frame_write(szBuffer, iIndex, szBuffer, &iLen, 5000);
	comm_close();

	return iRet;
}
//下载工作秘钥
BANKGWQ_API int __stdcall SCCBA_DownLoadWKey(int iPortNo, int MKeyIndex, int WKeyIndex, int WKeyLength, char *Key, char*CheckValue)
{
	int iRet, iIndex, iLen;
	unsigned char szBuffer[FRAME_MAX_SIZE];

	iRet = comm_open(iPortNo, iPortNo, 9600);
	if (iRet)
		return iRet;

	iIndex = 0;
	szBuffer[iIndex++] = 'U';
	szBuffer[iIndex++] = 'W';
	szBuffer[iIndex++] = (unsigned char)MKeyIndex;
	szBuffer[iIndex++] = (unsigned char)WKeyIndex;
	szBuffer[iIndex++] = (unsigned char)WKeyLength;

	sprintf((char *)&szBuffer[iIndex], "%02d", strlen(Key));
	iIndex += 2;
	memcpy((char *)&szBuffer[iIndex], Key, strlen(Key));
	iIndex += strlen(Key);

	sprintf((char *)&szBuffer[iIndex], "%02d", strlen(CheckValue));
	iIndex += 2;
	memcpy((char *)&szBuffer[iIndex], CheckValue, strlen(CheckValue));
	iIndex += strlen(CheckValue);

	iLen = sizeof(szBuffer);
	iRet = comm_frame_write(szBuffer, iIndex, szBuffer, &iLen, 5000);
	comm_close();

	return iRet;
}
//激活工作秘钥
BANKGWQ_API int __stdcall SCCBA_ActiveWKey(int iPortNo, int MKeyIndex, int WKeyIndex)
{
	int iRet, iIndex, iLen;
	unsigned char szBuffer[FRAME_MAX_SIZE];

	iRet = comm_open(iPortNo, iPortNo, 9600);
	if (iRet)
		return iRet;

	iIndex = 0;
	szBuffer[iIndex++] = 'A';
	szBuffer[iIndex++] = 'W';
	szBuffer[iIndex++] = (unsigned char)MKeyIndex;
	szBuffer[iIndex++] = (unsigned char)WKeyIndex;

	iLen = sizeof(szBuffer);
	iRet = comm_frame_write(szBuffer, iIndex, szBuffer, &iLen, 5000);
	comm_close();

	return iRet;
}

int gSignPdf_TimeoutTick = 0;
int gSignPdf_Cancel = 0;


//7. Electronic Sign
int __stdcall SCCBA_signPDFByType(int type, int iPortNo, char* pButtonInfo, unsigned char *pdfdata, int pdfdatalen, int page, int x, int y, int w, int h, int iTimeout, char *picdata, int *picdatalen, char *signeddata, int *signedLen)
{
#ifdef SIGNPDF_DEBUG
	FILE *pSignFile, *pSignDataFile;
#endif
	int iRet, iIndex;
	int iTotalSize, iSegmentSize, iWriteSize, iReadSize, iCurrentlSegment, iTotalSegment;
	int iHeaderSize, iStatusInfoSize, iFilenameSize, iMainpageInfoSize;
	int iStatusInfoLength, iFilenameLength, iMainpageInfoLength, iSignDataLength;
	char szPdfFilename[] = "esign.pdf";
	unsigned char szBuffer[FRAME_MAX_SIZE], *pPdfData, *pPicData, *pSignData, *pUtf8BtnInfo;
	char cSignPngFilename[MAX_PATH], cSignDataFilename[MAX_PATH];

	int iRetPicBufLen, iRetSignDataBufLen, iRetPicBufSize, iRetSignDataBufSize;

	if ((pdfdata == NULL) || (pdfdatalen <= 0))
		return ERRCODE_INVALID_PARAMETER;
	if ((type == 0) && ((picdata == NULL) || (picdatalen == NULL) || (signeddata == NULL) || (signedLen == NULL)))
		return ERRCODE_INVALID_PARAMETER;

	gSignPdf_TimeoutTick = GetTickCount();
	gSignPdf_TimeoutTick += (iTimeout * 1000);

	if (type == 0)
	{
		pPicData = (unsigned char *)picdata;
		pSignData = (unsigned char *)signeddata;
		iRetPicBufSize = iRetPicBufLen = *picdatalen;
		iRetSignDataBufSize = iRetSignDataBufLen = *signedLen;
		memset(pPicData, 0x0, iRetPicBufLen);
		memset(pSignData, 0x0, iRetSignDataBufLen);
		*picdatalen = 0;
		*signedLen = 0;
	}

	iRet = comm_open(iPortNo, iPortNo, 9600);
	if (iRet)
		return iRet;

	memset(cSignPngFilename, 0, sizeof(cSignPngFilename));
	memset(cSignDataFilename, 0, sizeof(cSignDataFilename));
#ifdef SIGNPDF_DEBUG
	pSignFile = NULL;
	pSignDataFile = NULL;
#endif

	iTotalSize = 0;
	//iStatusInfoSize
	pUtf8BtnInfo = NULL;
	iStatusInfoLength = iStatusInfoSize = 0;
	if (pButtonInfo)
	{
		if (!SCCBA_GB18030ToUTF8(pButtonInfo, &pUtf8BtnInfo))
			iStatusInfoSize = iStatusInfoLength = strlen((char *)pUtf8BtnInfo);
		iTotalSize += iStatusInfoSize;
	}

	//iMainpageInfoSize
	iMainpageInfoLength = iMainpageInfoSize = pdfdatalen;
	iTotalSize += iMainpageInfoSize;
	pPdfData = pdfdata;

	iFilenameLength = iFilenameSize = strlen(szPdfFilename);
	iTotalSize += iFilenameLength;

	iTotalSegment = iTotalSize / 4000;
	if (iTotalSize % 4000)
		iTotalSegment += 1;
	iCurrentlSegment = iTotalSegment;

	while (iTotalSize)
	{
		iIndex = 0;
		iHeaderSize = 0;
		memset(szBuffer, 0, sizeof(szBuffer));
		szBuffer[iIndex++] = 'S';                                            //Instruction code
		szBuffer[iIndex++] = 'G';
		szBuffer[iIndex++] = type;                                           //Type  0:Sync 1:Async
		sprintf((char *)&szBuffer[iIndex], "%04d", iStatusInfoLength);       //StatusBarInfo Length
		iIndex += 4;
		sprintf((char *)&szBuffer[iIndex], "%04d", iFilenameLength);         //Mainpage Filename Length
		iIndex += 4;
		sprintf((char *)&szBuffer[iIndex], "%012d", iMainpageInfoLength);    //Mainpage Length
		iIndex += 12;
		sprintf((char *)&szBuffer[iIndex], "%04d", 0);                       //VoiceText Length
		iIndex += 4;
		sprintf((char *)&szBuffer[iIndex], "%03d", iTimeout);                 //Timeout
		iIndex += 3;
		sprintf((char *)&szBuffer[iIndex], "%04d", page);                     //Sign parameter
		iIndex += 4;
		sprintf((char *)&szBuffer[iIndex], "%04d", x);
		iIndex += 4;
		sprintf((char *)&szBuffer[iIndex], "%04d", y);
		iIndex += 4;
		sprintf((char *)&szBuffer[iIndex], "%04d", w);
		iIndex += 4;
		sprintf((char *)&szBuffer[iIndex], "%04d", h);
		iIndex += 4;
		sprintf((char *)&szBuffer[iIndex], "%04d", iCurrentlSegment);        //Current Segment
		iIndex += 4;
		sprintf((char *)&szBuffer[iIndex], "%04d", iTotalSegment);           //Total Segment
		iIndex += 4;

		if (iTotalSize >= 4000)                                              //Segment Size
			iSegmentSize = 4000;
		else
			iSegmentSize = iTotalSize;
		iTotalSize -= iSegmentSize;
		sprintf((char *)&szBuffer[iIndex], "%04d", iSegmentSize);
		iIndex += 4;
		iHeaderSize = iIndex;

		iWriteSize = 0;                                                      //Segment Data
		if (iStatusInfoSize)
		{
			if (iStatusInfoSize > iSegmentSize)
			{
				memcpy(&szBuffer[iIndex], &pUtf8BtnInfo[strlen((char *)pUtf8BtnInfo) - iStatusInfoSize], iSegmentSize);
				iStatusInfoSize -= iSegmentSize;
				iWriteSize = iSegmentSize;
			}
			else
			{
				memcpy(&szBuffer[iIndex], &pUtf8BtnInfo[strlen((char *)pUtf8BtnInfo) - iStatusInfoSize], iStatusInfoSize);
				iWriteSize = iStatusInfoSize;
				iStatusInfoSize = 0;
			}
			iIndex += iWriteSize;
		}

		if (iFilenameSize && (iWriteSize < iSegmentSize))
		{
			if (iFilenameSize >= (iSegmentSize - iWriteSize))
			{
				memcpy(&szBuffer[iIndex], &szPdfFilename[strlen(szPdfFilename) - iFilenameSize], (iSegmentSize - iWriteSize));
				iFilenameSize -= (iSegmentSize - iWriteSize);
				iIndex += (iSegmentSize - iWriteSize);
				iWriteSize = iSegmentSize;
			}
			else
			{
				memcpy(&szBuffer[iIndex], &szPdfFilename[strlen(szPdfFilename) - iFilenameSize], iFilenameSize);
				iWriteSize += iFilenameSize;
				iIndex += iFilenameSize;
				iFilenameSize = 0;
			}
		}

		if (iMainpageInfoSize && (iWriteSize < iSegmentSize))
		{
			if (iMainpageInfoSize >= (iSegmentSize - iWriteSize))
			{
				memcpy(&szBuffer[iIndex], pPdfData, iSegmentSize - iWriteSize);
				pPdfData += (iSegmentSize - iWriteSize);

				iMainpageInfoSize -= (iSegmentSize - iWriteSize);
				iIndex += (iSegmentSize - iWriteSize);
				iWriteSize = iSegmentSize;
			}
			else
			{
				memcpy(&szBuffer[iIndex], pPdfData, iMainpageInfoSize);
				pPdfData += iMainpageInfoSize;

				iWriteSize += iMainpageInfoSize;
				iIndex += iMainpageInfoSize;
				iMainpageInfoSize = 0;
			}
		}

		iReadSize = sizeof(szBuffer);
		iRet = comm_frame_write(szBuffer, iHeaderSize + iSegmentSize, szBuffer, &iReadSize, 5000);
		if (iRet)
			break;
		iCurrentlSegment--;
	}

	if (iRet)
	{
		comm_close();
		return iRet;
	}

	if (w && h && (type == 0))
	{
		char currSegment[5], totalSegment[5], currSegmentSize[5], signDataSize[7];
		int  iCurrSegment, iTotalSegment, iCurrSegmentSize;

		//Start to receive sign data
		//Wait first frame
		iReadSize = sizeof(szBuffer);
		iRet = comm_frame_receive(szBuffer, &iReadSize, iTimeout * 1000);
		if ((szBuffer[0] == 'C') && (szBuffer[1] == 'L'))
		{
			iRet = ERRCODE_CANCEL_OPERATION;
			return iRet;
		}
#ifdef SIGNPDF_DEBUG
		if (pSignFile == NULL)
		{
			GetCurrentDirectory(MAX_PATH, cSignPngFilename);
			memcpy(cSignDataFilename, cSignPngFilename, strlen(cSignPngFilename));
			sprintf(&cSignPngFilename[strlen(cSignPngFilename)], "%s", "\\sign.png");
			sprintf(&cSignDataFilename[strlen(cSignDataFilename)], "%s", "\\sign.txt");
			pSignFile = fopen(cSignPngFilename, "wb");
			if (pSignFile == NULL)
				iRet = ERRCODE_OPENFILE_FAILURE;
			pSignDataFile = fopen(cSignDataFilename, "wb");
			if (pSignDataFile == NULL)
				iRet = ERRCODE_OPENFILE_FAILURE;
		}
#endif
		iSignDataLength = -1;
		while (!iRet)
		{
			iIndex = 0;
			if ((szBuffer[0] != 'R') && (szBuffer[1] != 'G'))
			{
				iRet = ERRCODE_RECEIVE_DATA_FAIL;
				break;
			}

			iIndex += 3;

			memset(currSegment, 0, sizeof(currSegment));
			memset(totalSegment, 0, sizeof(totalSegment));
			memset(currSegmentSize, 0, sizeof(currSegmentSize));

			memcpy(currSegment, &szBuffer[3], 4);
			memcpy(totalSegment, &szBuffer[7], 4);
			memcpy(currSegmentSize, &szBuffer[11], 4);

			iCurrSegment = atoi(currSegment);
			iTotalSegment = atoi(totalSegment);
			iCurrSegmentSize = atoi(currSegmentSize);
			iIndex += 12;

			if (iSignDataLength < 0)
			{
				memset(signDataSize, 0, sizeof(signDataSize));
				memcpy(signDataSize, &szBuffer[iIndex], 6);
				iSignDataLength = atoi(signDataSize);
				if (iSignDataLength < 0)
					iSignDataLength = 0;
				iIndex += 6;
				iCurrSegmentSize -= 6;
			}
			if (iReadSize = !(currSegmentSize + iIndex))
			{
				iRet = ERRCODE_RECEIVE_DATA_FAIL;
				break;
			}

			if (iSignDataLength > 0)
			{
				if (iCurrSegmentSize < iSignDataLength)
				{
					if (iRetSignDataBufLen > iCurrSegmentSize)
					{
						memcpy(pSignData, &szBuffer[iIndex], iCurrSegmentSize);
						iRetSignDataBufLen -= iCurrSegmentSize;
						pSignData += iCurrSegmentSize;
						*signedLen += iCurrSegmentSize;
					}
					else if (iRetSignDataBufLen > 0)
					{
						memcpy(pSignData, &szBuffer[iIndex], iRetSignDataBufLen);
						*signedLen += iRetSignDataBufLen;
						pSignData += iRetSignDataBufLen;
						iRetSignDataBufLen = 0;
					}
#ifdef SIGNPDF_DEBUG
					fwrite(&szBuffer[iIndex], iCurrSegmentSize, 1, pSignDataFile);
#endif
					iSignDataLength -= iCurrSegmentSize;
					iIndex += iSignDataLength;
					iCurrSegmentSize = 0;
				}
				else
				{
					if (iRetSignDataBufLen > iSignDataLength)
					{
						memcpy(pSignData, &szBuffer[iIndex], iSignDataLength);
						iRetSignDataBufLen -= iSignDataLength;
						*signedLen += iSignDataLength;
						pSignData += iSignDataLength;
					}
					else if (iRetSignDataBufLen > 0)
					{
						memcpy(pSignData, &szBuffer[iIndex], iRetSignDataBufLen);
						*signedLen += iRetSignDataBufLen;
						pSignData += iRetSignDataBufLen;
						iRetSignDataBufLen = 0;
					}
#ifdef SIGNPDF_DEBUG
					fwrite(&szBuffer[iIndex], iSignDataLength, 1, pSignDataFile);
#endif
					iCurrSegmentSize -= iSignDataLength;
					iIndex += iSignDataLength;
					iSignDataLength = 0;
				}
			}

			if (iCurrSegmentSize)
			{
#ifdef SIGNPDF_DEBUG
				fwrite(&szBuffer[iIndex], iCurrSegmentSize, 1, pSignFile);
#endif
				if (iRetPicBufLen > iCurrSegmentSize)
				{
					memcpy(pPicData, &szBuffer[iIndex], iCurrSegmentSize);
					iRetPicBufLen -= iCurrSegmentSize;
					*picdatalen += iCurrSegmentSize;
					pPicData += iCurrSegmentSize;
				}
				else if (iRetPicBufLen > 0)
				{
					memcpy(pSignData, &szBuffer[iIndex], iRetPicBufLen);
					pPicData += iRetPicBufLen;
					*picdatalen += iRetPicBufLen;
					iRetPicBufLen = 0;
				}
			}
			szBuffer[0] = 'R';
			szBuffer[1] = 'R';
			sprintf((char *)&szBuffer[2], "%04d", iCurrSegment - 1);
			szBuffer[6] = 0;
			iRet = comm_frame_send(szBuffer, 6);
			if (iRet || (iCurrSegment == 1))
				break;
			iReadSize = sizeof(szBuffer);
			iRet = comm_frame_receive(szBuffer, &iReadSize, 6000);
		}
	}
#ifdef SIGNPDF_DEBUG
	if (pSignFile)
		fclose(pSignFile);
	if (pSignDataFile)
		fclose(pSignDataFile);
#endif
	comm_close();
	if (pUtf8BtnInfo)
		delete[] pUtf8BtnInfo;

	if (type == 0)
	{
		if (*picdatalen >= iRetPicBufSize)
			picdata[iRetPicBufSize - 1] = 0;
		else
			picdata[*picdatalen] = 0;

		if (*signedLen >= iRetSignDataBufSize)
			signeddata[iRetSignDataBufSize - 1] = 0;
		else
			signeddata[*signedLen] = 0;
	}
#if 0 //def SIGNPDF_DEBUG
	{
		FILE *pDebugFile;
		long DebugFileSize;
		pDebugFile = fopen("e:\\lsc_signdata.txt", "rb");
		if (pDebugFile)
		{
			fseek(pDebugFile, 0, SEEK_END);
			DebugFileSize = ftell(pDebugFile);
			fseek(pDebugFile, 0, SEEK_SET);
			if (DebugFileSize >= iRetSignDataBufSize)
			{
				fread(signeddata, 1, iRetSignDataBufSize - 1, pDebugFile);
				signeddata[iRetSignDataBufSize - 1] = 0;
				*signedLen = iRetSignDataBufSize - 1;
			}
			else
			{
				fread(signeddata, 1, DebugFileSize, pDebugFile);
				signeddata[DebugFileSize] = 0;
				*signedLen = DebugFileSize;
			}
			fclose(pDebugFile);
		}
	}
#endif
	return iRet;
}
//推送pdf签字功能
BANKGWQ_API int __stdcall SCCBA_signPDF(int iPortNo, unsigned char *pdfdata, int pdfdatalen, int page, int x, int y, int w, int h, int iTimeout, char *picdata, int *picdatalen, char *signeddata, int *signedLen)
{
	char dbgbuf[1024];

	memset(dbgbuf, 0, sizeof(dbgbuf));
	sprintf(dbgbuf, "SCCBA_signPDF -> type:0 x:%d y:%d w:%d h:%d\r\n", x, y, w, h);
	WriteLog(dbgbuf);

	return SCCBA_signPDFByType(0, iPortNo, NULL, pdfdata, pdfdatalen, page, x, y, w, h, iTimeout, picdata, picdatalen, signeddata, signedLen);
}

BANKGWQ_API int __stdcall SCCBA_startSignPDF(int iPortNo, unsigned char *pdfdata, int pdfdatalen, int page, int x, int y, int w, int h, int iTimeout)
{
	char dbgbuf[1024];

	memset(dbgbuf, 0, sizeof(dbgbuf));
	sprintf(dbgbuf, "SCCBA_startSignPDF -> type:1 x:%d y:%d w:%d h:%d\r\n", x, y, w, h);
	WriteLog(dbgbuf);

	return SCCBA_signPDFByType(1, iPortNo, NULL, pdfdata, pdfdatalen, page, x, y, w, h, iTimeout, NULL, 0, NULL, 0);
}
//推送PDF显示功能
BANKGWQ_API int __stdcall SCCBA_showPDF(int iPortNo, unsigned char* pdfdata, int pdfdataLen, int iTimeout, char *buttonText)
{
	char dbgbuf[1024];

	memset(dbgbuf, 0, sizeof(dbgbuf));
	sprintf(dbgbuf, "SCCBA_showPDF\r\n");
	WriteLog(dbgbuf);

	return SCCBA_signPDFByType(1, iPortNo, buttonText, pdfdata, pdfdataLen, 1, 0, 0, 0, 0, iTimeout, NULL, 0, NULL, 0);

}



BANKGWQ_API int __stdcall SCCBA_getSignPDFState(int iPortNo, int *signState, char * picdata, int *picdataLen, char *signeddata, int* signedLen)
{
	int iRet, iIndex;
	int iReadSize, iSignDataLength;
	int iRetPicBufLen, iRetSignDataBufLen;
	unsigned char szBuffer[FRAME_MAX_SIZE], *pPicData, *pSignData;

	char dbgbuf[1024];

	memset(dbgbuf, 0, sizeof(dbgbuf));
	sprintf(dbgbuf, "SCCBA_getSignPDFState -> start\r\n");
	WriteLog(dbgbuf);

	if ((signState == NULL) || (picdata == NULL) || (picdataLen == NULL) || (*picdataLen <= 0) || (signeddata == NULL) || (signedLen == NULL) || (*signedLen <= 0))
	{
		WriteLog("SCCBA_getSignPDFState -> Parameter error\r\n");
		return ERRCODE_INVALID_PARAMETER;
	}
	iRetPicBufLen = *picdataLen;
	iRetSignDataBufLen = *signedLen;
	pPicData = (unsigned char *)picdata;
	pSignData = (unsigned char *)signeddata;

	memset(picdata, 0, *picdataLen);
	memset(signeddata, 0, *signedLen);
	*signState = 0;
	*picdataLen = 0;
	*signedLen = 0;

	iRet = comm_open(iPortNo, iPortNo, 9600);
	if (iRet)
	{
		WriteLog("SCCBA_getSignPDFState -> Fail to open USB\r\n");
		return iRet;
	}
	iReadSize = sizeof(szBuffer);
	memset(szBuffer, 0, sizeof(szBuffer));
	szBuffer[0] = 'E';
	szBuffer[1] = 'S';
	iRet = comm_frame_write(szBuffer, 2, szBuffer, &iReadSize, 5000);
	if (!iRet)
	{
		char currSegment[5], totalSegment[5], currSegmentSize[5], signDataSize[7];
		int  iCurrSegment, iTotalSegment, iCurrSegmentSize;

		iSignDataLength = -1;
		while (!iRet)
		{
			iIndex = 0;
			if ((szBuffer[0] != 'R') && (szBuffer[1] != 'G'))
			{
				iRet = ERRCODE_RECEIVE_DATA_FAIL;
				break;
			}
			if (signState && (szBuffer[2] >= 0x30) && (szBuffer[2] <= 0x34))
				*signState = szBuffer[2] - 0x30;
			if ((szBuffer[2] != 0x31) && (szBuffer[2] != 0x32))
				break;
			iIndex += 3;

			memset(currSegment, 0, sizeof(currSegment));
			memset(totalSegment, 0, sizeof(totalSegment));
			memset(currSegmentSize, 0, sizeof(currSegmentSize));

			memcpy(currSegment, &szBuffer[3], 4);
			memcpy(totalSegment, &szBuffer[7], 4);
			memcpy(currSegmentSize, &szBuffer[11], 4);

			iCurrSegment = atoi(currSegment);
			iTotalSegment = atoi(totalSegment);
			iCurrSegmentSize = atoi(currSegmentSize);
			iIndex += 12;

			if (iSignDataLength < 0)
			{
				memset(signDataSize, 0, sizeof(signDataSize));
				memcpy(signDataSize, &szBuffer[iIndex], 6);
				iSignDataLength = atoi(signDataSize);
				if (iSignDataLength < 0)
					iSignDataLength = 0;
				iIndex += 6;
				iCurrSegmentSize -= 6;
			}
			if (iReadSize = !(currSegmentSize + iIndex))
			{
				iRet = ERRCODE_RECEIVE_DATA_FAIL;
				break;
			}

			if (iSignDataLength > 0)
			{
				if (iCurrSegmentSize < iSignDataLength)
				{
					if (iRetSignDataBufLen > iCurrSegmentSize)
					{
						memcpy(pSignData, &szBuffer[iIndex], iCurrSegmentSize);
						iRetSignDataBufLen -= iCurrSegmentSize;
						pSignData += iCurrSegmentSize;
						*signedLen += iCurrSegmentSize;
					}
					else if (iRetSignDataBufLen > 0)
					{
						memcpy(pSignData, &szBuffer[iIndex], iRetSignDataBufLen);
						*signedLen += iRetSignDataBufLen;
						pSignData += iRetSignDataBufLen;
						iRetSignDataBufLen = 0;
					}
					iSignDataLength -= iCurrSegmentSize;
					iIndex += iSignDataLength;
					iCurrSegmentSize = 0;
				}
				else
				{
					if (iRetSignDataBufLen > iSignDataLength)
					{
						memcpy(pSignData, &szBuffer[iIndex], iSignDataLength);
						iRetSignDataBufLen -= iSignDataLength;
						*signedLen += iSignDataLength;
						pSignData += iSignDataLength;
					}
					else if (iRetSignDataBufLen > 0)
					{
						memcpy(pSignData, &szBuffer[iIndex], iRetSignDataBufLen);
						pSignData += iRetSignDataBufLen;
						*signedLen += iRetSignDataBufLen;
						iRetSignDataBufLen = 0;
					}

					iCurrSegmentSize -= iSignDataLength;
					iIndex += iSignDataLength;
					iSignDataLength = 0;
				}
			}

			if (iCurrSegmentSize)
			{
				if (iRetPicBufLen > iCurrSegmentSize)
				{
					memcpy(pPicData, &szBuffer[iIndex], iCurrSegmentSize);
					iRetPicBufLen -= iCurrSegmentSize;
					*picdataLen += iCurrSegmentSize;
					pPicData += iCurrSegmentSize;
				}
				else if (iRetPicBufLen > 0)
				{
					memcpy(pSignData, &szBuffer[iIndex], iRetPicBufLen);
					pPicData += iRetPicBufLen;
					*picdataLen += iRetPicBufLen;
					iRetPicBufLen = 0;
				}
			}
			szBuffer[0] = 'R';
			szBuffer[1] = 'R';
			sprintf((char *)&szBuffer[2], "%04d", iCurrSegment - 1);
			szBuffer[6] = 0;
			iRet = comm_frame_send(szBuffer, 6);
			if (iRet || (iCurrSegment == 1))
				break;
			iReadSize = sizeof(szBuffer);
			iRet = comm_frame_receive(szBuffer, &iReadSize, 6000);
		}
	}

	if (signState && (*signState != 1) && (*signState != 2))
	{
		char szSignData[] = "937,550,P1024,(272, 94, 458; 271, 94, 476; 267, 94, 468; 265, 94, 336; 267, 94, 0; 267, 94, 0)";
		unsigned char szPicData[] =
		{ 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0x00, 0x00, 0x00, 0x0D, 0x49, 0x48, 0x44, 0x52,
			0x00, 0x00, 0x01, 0xC2, 0x00, 0x00, 0x01, 0xC2, 0x08, 0x06, 0x00, 0x00, 0x00, 0x7C, 0x18, 0xC9,
			0x45, 0x00, 0x00, 0x00, 0x04, 0x73, 0x42, 0x49, 0x54, 0x08, 0x08, 0x08, 0x08, 0x7C, 0x08, 0x64,
			0x88, 0x00, 0x00, 0x03, 0xA5, 0x49, 0x44, 0x41, 0x54, 0x78, 0x9C, 0xED, 0xDA, 0xB1, 0x4D, 0x03,
			0x41, 0x14, 0x45, 0xD1, 0x8B, 0x2B, 0x70, 0x4A, 0x86, 0x3B, 0xD8, 0x92, 0x1C, 0x50, 0x08, 0xA5,
			0xD0, 0x01, 0x05, 0x38, 0x21, 0x70, 0x01, 0xC4, 0x54, 0x61, 0x52, 0x12, 0x13, 0x18, 0x09, 0x64,
			0xC9, 0x68, 0xA5, 0x15, 0x5A, 0xBC, 0x9C, 0x23, 0x7D, 0x4D, 0x32, 0xC1, 0xCB, 0x9E, 0xBE, 0x66,
			0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x98,
			0xE0, 0x66, 0xEE, 0x00, 0xC0, 0x55, 0x59, 0x7F, 0x4E, 0xD5, 0xE1, 0xEC, 0x04, 0x80, 0x45, 0xBA,
			0xAF, 0x5E, 0xAB, 0xE3, 0x85, 0x79, 0xEA, 0xAB, 0x1C, 0x01, 0x60, 0x51, 0x9E, 0xBB, 0x5C, 0x80,
			0xDF, 0xE7, 0x61, 0x9E, 0x78, 0x00, 0xF0, 0x7B, 0x86, 0xC6, 0x95, 0xE0, 0xB1, 0x53, 0x61, 0xC2,
			0x55, 0x5A, 0xCD, 0x1D, 0x00, 0xF8, 0xB3, 0x5E, 0xAA, 0xB7, 0x91, 0x77, 0xBD, 0x13, 0x02, 0xB0,
			0x48, 0x43, 0xB5, 0xEF, 0xE7, 0x6D, 0xF0, 0x50, 0xDD, 0xCD, 0x94, 0x0F, 0x26, 0xF3, 0x6B, 0x14,
			0x18, 0x63, 0xA8, 0xB6, 0xD5, 0x6D, 0xB5, 0xA9, 0xDE, 0x3B, 0x6D, 0x8B, 0xBB, 0xEA, 0x31, 0x1B,
			0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC, 0x1B, 0x1F, 0x8C, 0xB0, 0x2F, 0xC7, 0xBB, 0xD6,
			0x5D, 0x78, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4E, 0x44, 0xAE, 0x42, 0x60, 0x82
		};

		//Init Data...
		if (iRetPicBufLen > 1006)
		{
			*picdataLen = 1006;
			memcpy(picdata, szPicData, 1006);
		}
		if (iRetSignDataBufLen > (int)strlen(szSignData))
		{
			*signedLen = (int)strlen(szSignData);
			memcpy(signeddata, szSignData, strlen(szSignData));
		}

		WriteLog("SCCBA_getSignPDFState -> ...\r\n");
	}

	if (GetTickCount() > (DWORD)gSignPdf_TimeoutTick)
	{
		WriteLog("SCCBA_getSignPDFState -> 3 \r\n");
		*signState = 3;
	}
	else if (gSignPdf_Cancel)
	{
		WriteLog("SCCBA_getSignPDFState -> 4 \r\n");
		*signState = 4;
	}
	gSignPdf_Cancel = 0;

	comm_close();

	WriteLog("SCCBA_getSignPDFState -> end\r\n");

	return iRet;
}

extern int gCancelCurrOperation;

//取消PDF签字
BANKGWQ_API int __stdcall SCCBA_cancelSignPDF(int iPortNo)
{
	int iRet, iLen;
	unsigned char szBuffer[FRAME_MAX_SIZE];

	gCancelCurrOperation = 0;
	gSignPdf_Cancel = 1;
	iRet = comm_open(iPortNo, iPortNo, 9600);
	if (!iRet)
	{
		szBuffer[0] = 'C';
		szBuffer[1] = 'S';
		iLen = sizeof(szBuffer);
		iRet = comm_frame_write(szBuffer, 2, szBuffer, &iLen, 2000);
		comm_close();
	}
	return iRet;
}
//推送PDF显示功能，并获取用户确认结果
BANKGWQ_API int __stdcall SCCBA_confirmPDF(int iPortNo, int waitResult, unsigned char* pdfdata, int pdfdatalen, int iTimeout, char * leftbtn, char * rightbtn, char * strVoice, char* iResult)
{
	SCCBA_PlayVoice(iPortNo, strVoice);
	int iRet, iIndex;
	int iTotalSize, iSegmentSize, iWriteSize, iReadSize, iCurrentlSegment, iTotalSegment;
	int iHeaderSize, iFilenameSize, iMainpageInfoSize, iLeftBtnSize, iRightBtnSize, iVoiceSize;
	char szPdfFilename[] = "esign.pdf";
	int iFilenameLength, iMainpageInfoLength, iLeftBtnLength, iRightBtnLength, iVoiceLength;
	unsigned char szBuffer[FRAME_MAX_SIZE], *pPdfData, *pUtf8LeftBtnInfo, *pUtf8RightBtnInfo, *pUtf8VoiceInfo;


	char dbgbuf[1024];

	memset(dbgbuf, 0, sizeof(dbgbuf));
	sprintf(dbgbuf, "SCCBA_confirmPDF\r\n");
	WriteLog(dbgbuf);

	if ((pdfdata == NULL) || (pdfdatalen <= 0))
		return ERRCODE_INVALID_PARAMETER;

	gSignPdf_TimeoutTick = GetTickCount();
	gSignPdf_TimeoutTick += (iTimeout * 1000);

	iRet = comm_open(iPortNo, iPortNo, 9600);
	if (iRet)
		return iRet;

	iTotalSize = 0;
	pUtf8LeftBtnInfo = NULL;
	if (leftbtn)
	{
		if (!SCCBA_GB18030ToUTF8(leftbtn, &pUtf8LeftBtnInfo))
			iLeftBtnSize = iLeftBtnLength = strlen((char *)pUtf8LeftBtnInfo);
		iTotalSize += iLeftBtnSize;
	}

	pUtf8RightBtnInfo = NULL;
	if (rightbtn)
	{
		if (!SCCBA_GB18030ToUTF8(rightbtn, &pUtf8RightBtnInfo))
			iRightBtnSize = iRightBtnLength = strlen((char *)pUtf8RightBtnInfo);
		iTotalSize += iRightBtnSize;
	}

	pUtf8VoiceInfo = NULL;
	if (strVoice)
	{
		if (!SCCBA_GB18030ToUTF8(strVoice, &pUtf8VoiceInfo))
			iVoiceSize = iVoiceLength = strlen((char *)pUtf8VoiceInfo);
		iTotalSize += iVoiceSize;
	}

	//iMainpageInfoSize
	iMainpageInfoLength = iMainpageInfoSize = pdfdatalen;
	iTotalSize += iMainpageInfoSize;
	pPdfData = pdfdata;

	iFilenameLength = iFilenameSize = strlen(szPdfFilename);
	iTotalSize += iFilenameLength;

	iTotalSegment = iTotalSize / 4000;
	if (iTotalSize % 4000)
		iTotalSegment += 1;
	iCurrentlSegment = iTotalSegment;

	while (iTotalSize)
	{
		iIndex = 0;
		iHeaderSize = 0;
		memset(szBuffer, 0, sizeof(szBuffer));
		szBuffer[iIndex++] = 'C';                                            //Instruction code
		szBuffer[iIndex++] = 'P';
		szBuffer[iIndex++] = (!!waitResult);                                 //WaitType  0:Sync 1:Async
		sprintf((char *)&szBuffer[iIndex], "%04d", iFilenameLength);         //Mainpage Filename Length
		iIndex += 4;
		sprintf((char *)&szBuffer[iIndex], "%012d", iMainpageInfoLength);    //Mainpage Length
		iIndex += 12;
		sprintf((char *)&szBuffer[iIndex], "%04d", iVoiceLength);            //VoiceText Length
		iIndex += 4;
		sprintf((char *)&szBuffer[iIndex], "%04d", iLeftBtnLength);          //Left Button
		iIndex += 4;
		sprintf((char *)&szBuffer[iIndex], "%04d", iRightBtnLength);         //Right Button
		iIndex += 4;
		sprintf((char *)&szBuffer[iIndex], "%03d", iTimeout);                 //Timeout
		iIndex += 3;
		sprintf((char *)&szBuffer[iIndex], "%04d", iCurrentlSegment);        //Current Segment
		iIndex += 4;
		sprintf((char *)&szBuffer[iIndex], "%04d", iTotalSegment);           //Total Segment
		iIndex += 4;

		if (iTotalSize >= 4000)                                              //Segment Size
			iSegmentSize = 4000;
		else
			iSegmentSize = iTotalSize;
		iTotalSize -= iSegmentSize;
		sprintf((char *)&szBuffer[iIndex], "%04d", iSegmentSize);
		iIndex += 4;
		iHeaderSize = iIndex;

		iWriteSize = 0;                                                      //Segment Data
		if (iFilenameSize && (iWriteSize < iSegmentSize))
		{
			if (iFilenameSize >= (iSegmentSize - iWriteSize))
			{
				memcpy(&szBuffer[iIndex], &szPdfFilename[strlen(szPdfFilename) - iFilenameSize], (iSegmentSize - iWriteSize));
				iFilenameSize -= (iSegmentSize - iWriteSize);
				iIndex += (iSegmentSize - iWriteSize);
				iWriteSize = iSegmentSize;
			}
			else
			{
				memcpy(&szBuffer[iIndex], &szPdfFilename[strlen(szPdfFilename) - iFilenameSize], iFilenameSize);
				iWriteSize += iFilenameSize;
				iIndex += iFilenameSize;
				iFilenameSize = 0;
			}
		}

		if (iMainpageInfoSize && (iWriteSize < iSegmentSize))
		{
			if (iMainpageInfoSize >= (iSegmentSize - iWriteSize))
			{
				memcpy(&szBuffer[iIndex], pPdfData, iSegmentSize - iWriteSize);
				pPdfData += (iSegmentSize - iWriteSize);

				iMainpageInfoSize -= (iSegmentSize - iWriteSize);
				iIndex += (iSegmentSize - iWriteSize);
				iWriteSize = iSegmentSize;
			}
			else
			{
				memcpy(&szBuffer[iIndex], pPdfData, iMainpageInfoSize);
				pPdfData += iMainpageInfoSize;

				iWriteSize += iMainpageInfoSize;
				iIndex += iMainpageInfoSize;
				iMainpageInfoSize = 0;
			}
		}

		if (iVoiceSize && (iWriteSize < iSegmentSize))
		{
			if (iVoiceSize >= (iSegmentSize - iWriteSize))
			{
				memcpy(&szBuffer[iIndex], &pUtf8VoiceInfo[iVoiceLength - iVoiceSize], iSegmentSize - iWriteSize);
				iVoiceSize -= (iSegmentSize - iWriteSize);
				iIndex += (iSegmentSize - iWriteSize);
				iWriteSize = iSegmentSize;
			}
			else
			{
				memcpy(&szBuffer[iIndex], &pUtf8VoiceInfo[iVoiceLength - iVoiceSize], iVoiceSize);
				iWriteSize += iVoiceSize;
				iIndex += iVoiceSize;
				iVoiceSize = 0;
			}
		}

		if (iRightBtnSize && (iWriteSize < iSegmentSize))
		{
			if (iRightBtnSize >= (iSegmentSize - iWriteSize))
			{
				memcpy(&szBuffer[iIndex], &pUtf8RightBtnInfo[iRightBtnLength - iRightBtnSize], iSegmentSize - iWriteSize);
				iRightBtnSize -= (iSegmentSize - iWriteSize);
				iIndex += (iSegmentSize - iWriteSize);
				iWriteSize = iSegmentSize;
			}
			else
			{
				memcpy(&szBuffer[iIndex], &pUtf8RightBtnInfo[iRightBtnLength - iRightBtnSize], iRightBtnSize);
				iWriteSize += iRightBtnSize;
				iIndex += iRightBtnSize;
				iRightBtnSize = 0;
			}
		}

		if (iLeftBtnSize && (iWriteSize < iSegmentSize))
		{
			if (iLeftBtnSize >= (iSegmentSize - iWriteSize))
			{
				memcpy(&szBuffer[iIndex], &pUtf8LeftBtnInfo[iLeftBtnLength - iLeftBtnSize], iSegmentSize - iWriteSize);
				iLeftBtnSize -= (iSegmentSize - iWriteSize);
				iIndex += (iSegmentSize - iWriteSize);
				iWriteSize = iSegmentSize;
			}
			else
			{
				memcpy(&szBuffer[iIndex], &pUtf8LeftBtnInfo[iLeftBtnLength - iLeftBtnSize], iLeftBtnSize);
				iWriteSize += iLeftBtnSize;
				iIndex += iLeftBtnSize;
				iLeftBtnSize = 0;
			}
		}

		iReadSize = sizeof(szBuffer);
		iRet = comm_frame_write(szBuffer, iHeaderSize + iSegmentSize, szBuffer, &iReadSize, 5000);
		if (iRet)
			break;
		iCurrentlSegment--;
	}

	if (!iRet)
	{
		iReadSize = sizeof(szBuffer);
		memset(szBuffer, 0, sizeof(szBuffer));
		iRet = comm_frame_receive(szBuffer, &iReadSize, iTimeout * 1000);
		if ((szBuffer[0] == 'C') && (szBuffer[1] == 'P') && (iReadSize >= 5))
			*iResult = szBuffer[4];
	}

	comm_close();

	if (pUtf8LeftBtnInfo)
		delete[] pUtf8LeftBtnInfo;
	if (pUtf8RightBtnInfo)
		delete[] pUtf8RightBtnInfo;
	if (pUtf8VoiceInfo)
		delete[] pUtf8VoiceInfo;

	return iRet;
}
// 7.7   推送 pdf到柜外清签字或加盖指纹


BANKGWQ_API int __stdcall SCCBA_signPDFEx(int iPortNo, int signtype, unsigned char *pdfdata, int pdfdataLen, int page,
	int x, int y, int w, int h, int iTimeout, char* strVoice, int * rettype,
	char * picdata, int *picdataLen, char *signdata, int* signdataLen,
	char * fingerPicdata, int* fingerPicdataLen, char * fingerdata, int * fingerdataLen)
{

	int iRet = 0;

	char dbgbuf[1024];

	// 	memset(dbgbuf, 0, sizeof(dbgbuf));
	// 	sprintf(dbgbuf, "SCCBA_signPDFEx -> signtype:%d page:%d x:%d y:%d w:%d h:%d\r\n", signtype, page, x, y, w, h);
	// 	WriteLog(dbgbuf);

	switch (signtype)
	{
	case 1:
	{
		iRet = SCCBA_signPDF(iPortNo, pdfdata, pdfdataLen, page, x, y, w, h, iTimeout, picdata, picdataLen, signdata, signdataLen);
		//iRet = SCCBA_signPDFByType(0, iPortNo, NULL, pdfdata, pdfdataLen, page, x, y, w, h, iTimeout, picdata, picdataLen, signdata, signdataLen);

		if (iRet == 0)
		{
			*rettype = 1;
		}

		break;
	}

	case 2:
	{
		//iRet = SCCBA_signPDFByType(1, iPortNo, NULL, pdfdata, pdfdataLen, page, 1, 0, 0, 0, iTimeout, NULL, 0, NULL, 0);
		SCCBA_PlayVoice(iPortNo, "请按指纹");
		//iRet = SCCBA_showPDF(iPortNo, pdfdata, pdfdataLen, iTimeout, NULL);
		iRet = SCCBA_signPDFByType(1, iPortNo, NULL, pdfdata, pdfdataLen, page, 0, 0, 0, 0, iTimeout, NULL, 0, NULL, 0);
		iRet = SCCBA_ReadFinger(iPortNo, 3000, NULL, fingerPicdata, fingerPicdataLen, fingerdata, fingerdataLen, rettype);
		if (iRet == 0)
		{
			*rettype = 2;
		}

		SCCBA_cancelSignPDF(iPortNo);
		break;
	}

	case 4:
		iRet = SCCBA_signPDFByType(0, iPortNo, NULL, pdfdata, pdfdataLen, page, x, y, w, h, iTimeout, picdata, picdataLen, signdata, signdataLen);
		if (iRet != 0)
		{
			SCCBA_PlayVoice(iPortNo, "请按指纹");

			//iRet = SCCBA_showPDF(iPortNo, pdfdata, pdfdataLen, iTimeout, NULL);
			iRet = SCCBA_signPDFByType(1, iPortNo, NULL, pdfdata, pdfdataLen, page, 0, 0, 0, 0, iTimeout, NULL, 0, NULL, 0);
			iRet = SCCBA_ReadFinger(iPortNo, 3000, NULL, fingerPicdata, fingerPicdataLen, fingerdata, fingerdataLen, rettype);
			if (iRet == 0)
			{
				*rettype = 2;

			}
			SCCBA_cancelSignPDF(iPortNo);
			break;
		}
		*rettype = 1;
		break;
	case 3:
		iRet = SCCBA_signPDFByType(0, iPortNo, NULL, pdfdata, pdfdataLen, page, x, y, w, h, iTimeout, picdata, picdataLen, signdata, signdataLen);
		if (iRet == 0)
		{
			SCCBA_PlayVoice(iPortNo, "请按指纹");
			//iRet = SCCBA_showPDF(iPortNo, pdfdata, pdfdataLen, iTimeout, NULL);
			iRet = SCCBA_signPDFByType(1, iPortNo, NULL, pdfdata, pdfdataLen, page, 0, 0, 0, 0, iTimeout, NULL, 0, NULL, 0);
			iRet = SCCBA_ReadFinger(iPortNo, 3000, NULL, fingerPicdata, fingerPicdataLen, fingerdata, fingerdataLen, rettype);
			if (iRet == 0)
			{
				*rettype = 3;
			}
			SCCBA_cancelSignPDF(iPortNo);
		}
		break;
	default:
		break;
	}





	// 	memset(dbgbuf, 0, sizeof(dbgbuf));
	// 	sprintf(dbgbuf, "SCCBA_signPDFEx -> return value:%d\r\n", iRet);
	// 	WriteLog(dbgbuf);

	return iRet;
}





//08. Finger  获取指纹
BANKGWQ_API int __stdcall SCCBA_ReadFinger(int iPortNo, int iTimeout, char * strVoice, char * fingerPicdata, int* fingerPicdataLen, char * fingerdata, int* fingerdataLen, int* fingerType)
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
	int Fingerlength = 0;
	SCCBA_PlayVoice(iPortNo, strVoice);
	int iret = TcGetFingerFeature(0, bmppath, psFeatureInfo, &Fingerlength, iTimeout);
	if (iret != 0)
	{
		return -29;
	}
	int pnglength = bmp2png(bmppath, pngdata);
	if (pnglength < 0)
	{
		return -29;
	}
	memcpy(fingerPicdata, pngdata, pnglength);
	memcpy(fingerdata, psFeatureInfo, Fingerlength);
	*fingerPicdataLen = pnglength;
	return 0;
}
/*
BANKGWQ_API int __stdcall SCCBA_ReadFinger(int iPortNo, int iTimeout, char * strVoice, char * fingerPicdata, int* fingerPicdataLen, char * fingerdata, int* fingerdataLen, int* fingerType)
{
	char bmppath[MAX_PATH] = { 0 };
	string bmpfile;
	getTempPath(bmppath);
	bmpfile = bmppath;
	bmpfile += "tmp.bmp";

	cout<<"文件格式"<<bmpfile.c_str() << endl;
	memset(bmppath, 0, MAX_PATH);
	strcpy(bmppath, bmpfile.c_str());
	unsigned char psFeatureInfo[1024 * 64] = { 0 };
	unsigned char pngdata[1024 * 64] = { 0 };
	int Fingerlength = 0;
	SCCBA_PlayVoice(iPortNo, strVoice);
	int iret = TcGetFingerFeature(0, bmppath, psFeatureInfo, &Fingerlength, iTimeout);
	if (iret != 0)
	{
		return iret;
	}
	int pnglength = bmp2png(bmppath, pngdata);
	if (pnglength < 0)
	{
		return pnglength;
	}
	memcpy(fingerPicdata, pngdata, pnglength);
	memcpy(fingerdata, psFeatureInfo, Fingerlength);
	*fingerPicdataLen = pnglength;
	return iret;
}

*/


//09. Question 9.1问卷调查一
BANKGWQ_API int __stdcall SCCBA_OperateQuestion(int iPortNo, char *strData, char *strItem, int iType, char *strSeq, char *strVoice, int iTimeout, char *iResult1, char *iResult2)
{
	int iRet, iIndex;
	int iStrDataLen, iStrDataSize, iStrItemLen, iStrItemSize, iStrSeqLen, iStrSeqSize, iStrVoiceLen, iStrVoiceSize;
	int iTotalSize, iCurrentlSegment, iTotalSegment, iSegmentSize, iReadSize, iWriteSize;
	unsigned char szBuffer[FRAME_MAX_SIZE];
	unsigned char *pUtf8Data, *pUtf8Item, *pUtf8Seq, *pUtf8Voice;

	if ((strlen(strData) <= 0) || (strlen(strItem) <= 0))
		return ERRCODE_INVALID_PARAMETER;

	iRet = comm_open(iPortNo, iPortNo, 9600);
	if (iRet)
		return iRet;

	pUtf8Data = pUtf8Item = pUtf8Seq = pUtf8Voice = NULL;
	iStrDataLen = iStrDataSize = 0;
	if (strData)
		if (!SCCBA_GB18030ToUTF8(strData, &pUtf8Data))
			iStrDataLen = iStrDataSize = strlen((char *)pUtf8Data);
	iStrItemLen = iStrItemSize = 0;
	if (strItem)
		if (!SCCBA_GB18030ToUTF8(strItem, &pUtf8Item))
			iStrItemLen = iStrItemSize = strlen((char *)pUtf8Item);
	iStrSeqLen = iStrSeqSize = 0;
	if (strSeq)
		if (!SCCBA_GB18030ToUTF8(strSeq, &pUtf8Seq))
			iStrSeqLen = iStrSeqSize = strlen((char *)pUtf8Seq);
	iStrVoiceLen = iStrVoiceSize = 0;
	if (strVoice)
		if (!SCCBA_GB18030ToUTF8(strVoice, &pUtf8Voice))
			iStrVoiceLen = iStrVoiceSize = strlen((char *)pUtf8Voice);

	iTotalSize = iStrDataSize + iStrItemSize + iStrSeqSize + iStrVoiceSize;
	iTotalSegment = iTotalSize / 4000;
	if (iTotalSize % 4000)
		iTotalSegment += 1;
	iCurrentlSegment = iTotalSegment;

	do {
		iIndex = 0;
		memset(szBuffer, 0, sizeof(szBuffer));
		szBuffer[iIndex++] = 'O';                                            //Instruction code
		szBuffer[iIndex++] = 'Q';
		sprintf((char *)&szBuffer[iIndex], "%04d", iStrDataLen);             //strData
		iIndex += 4;
		sprintf((char *)&szBuffer[iIndex], "%04d", iStrItemLen);             //strItem
		iIndex += 4;
		sprintf((char *)&szBuffer[iIndex++], "%01d", iType);                 //Type
		sprintf((char *)&szBuffer[iIndex], "%04d", iStrSeqLen);              //strSeq
		iIndex += 4;
		sprintf((char *)&szBuffer[iIndex], "%04d", iStrVoiceLen);            //strVoice
		iIndex += 4;
		sprintf((char *)&szBuffer[iIndex], "%03d", iTimeout);                //Timeout
		iIndex += 3;

		sprintf((char *)&szBuffer[iIndex], "%04d", iCurrentlSegment);        //Current Segment
		iIndex += 4;
		sprintf((char *)&szBuffer[iIndex], "%04d", iTotalSegment);           //Total Segment
		iIndex += 4;
		if (iTotalSize >= 4000)
			iSegmentSize = 4000;
		else
			iSegmentSize = iTotalSize;
		iTotalSize -= iSegmentSize;
		sprintf((char *)&szBuffer[iIndex], "%04d", iSegmentSize);            //Segment Size
		iIndex += 4;

		iWriteSize = 0;                                                      //Segment Data
		if (iStrDataSize)
		{
			if (iStrDataSize > iSegmentSize)
			{
				memcpy(&szBuffer[iIndex], &pUtf8Data[iStrDataLen - iStrDataSize], iSegmentSize);
				iIndex += iSegmentSize;
				iStrDataSize -= iSegmentSize;
				iWriteSize += iSegmentSize;
			}
			else
			{
				memcpy(&szBuffer[iIndex], &pUtf8Data[iStrDataLen - iStrDataSize], iStrDataSize);
				iIndex += iStrDataSize;
				iWriteSize += iStrDataSize;
				iStrDataSize = 0;
			}
		}

		if (iStrItemSize && (iWriteSize < iSegmentSize))
		{
			if (iStrItemSize >= (iSegmentSize - iWriteSize))
			{
				memcpy(&szBuffer[iIndex], &pUtf8Item[iStrItemLen - iStrItemSize], (iSegmentSize - iWriteSize));
				iIndex += (iSegmentSize - iWriteSize);
				iStrItemSize -= (iSegmentSize - iWriteSize);
				iWriteSize = iSegmentSize;
			}
			else
			{
				memcpy(&szBuffer[iIndex], &pUtf8Item[iStrItemLen - iStrItemSize], iStrItemSize);
				iIndex += iStrItemSize;
				iWriteSize += iStrItemSize;
				iStrItemSize = 0;
			}
		}

		if (iStrSeqSize && (iWriteSize < iSegmentSize))
		{
			if (iStrSeqSize >= (iSegmentSize - iWriteSize))
			{
				memcpy(&szBuffer[iIndex], &pUtf8Seq[iStrSeqLen - iStrSeqSize], (iSegmentSize - iWriteSize));
				iIndex += (iSegmentSize - iWriteSize);
				iStrSeqSize -= (iSegmentSize - iWriteSize);
				iWriteSize = iSegmentSize;
			}
			else
			{
				memcpy(&szBuffer[iIndex], &pUtf8Seq[iStrSeqLen - iStrSeqSize], iStrSeqSize);
				iIndex += iStrSeqSize;
				iWriteSize += iStrSeqSize;
				iStrSeqSize = 0;
			}
		}

		if (iStrVoiceSize && (iWriteSize < iSegmentSize))
		{
			if (iStrVoiceSize >= (iSegmentSize - iWriteSize))
			{
				memcpy(&szBuffer[iIndex], &pUtf8Voice[iStrVoiceLen - iStrVoiceSize], (iSegmentSize - iWriteSize));
				iIndex += (iSegmentSize - iWriteSize);
				iStrVoiceSize -= (iSegmentSize - iWriteSize);
				iWriteSize = iSegmentSize;
			}
			else
			{
				memcpy(&szBuffer[iIndex], &pUtf8Voice[iStrVoiceLen - iStrVoiceSize], iStrVoiceSize);
				iIndex += iStrVoiceSize;
				iWriteSize += iStrVoiceSize;
				iStrVoiceSize = 0;
			}
		}

		iReadSize = sizeof(szBuffer);
		iRet = comm_frame_write(szBuffer, iIndex, szBuffer, &iReadSize, 5000);
		if (iRet)
			break;
		iCurrentlSegment--;
	} while (iTotalSize);

	if (!iRet)
	{
		iReadSize = sizeof(szBuffer);
		memset(szBuffer, 0, sizeof(szBuffer));
		iRet = comm_frame_receive(szBuffer, &iReadSize, iTimeout * 1000);
		if ((!iRet) && (iReadSize >= 9) && (szBuffer[0] == 'O') && (szBuffer[1] == 'Q'))
		{
			char szLen[10];
			if (iResult1)
			{
				iResult1[0] = (char)szBuffer[4];
				iResult1[1] = 0;
			}
			if (iResult2)
			{
				int iLen;
				char szSeq[100];

				memset(szLen, 0, sizeof(szLen));
				memcpy(szLen, &szBuffer[5], 4);
				iLen = atoi(szLen);
				if ((iLen + 9) > iReadSize)
					iLen = iReadSize - 9;
				memcpy(&iResult2[1], &szBuffer[9], iLen);

				memset(szSeq, 0, sizeof(szSeq));
				if (iStrSeqLen)
				{
					memcpy(szSeq, strSeq, iStrSeqLen);
					for (iIndex = 0; iIndex < iStrSeqLen; iIndex++)
					{
						if (szSeq[iIndex] == '/')
						{
							szSeq[iIndex] = 0;
							break;
						}
					}
				}
				iResult2[0] = (char)atoi(szSeq);
				iResult2[iLen + 1] = 0;
			}
		}
	}
	comm_close();

	if (pUtf8Data)
		delete[] pUtf8Data;
	if (pUtf8Item)
		delete[] pUtf8Item;
	if (pUtf8Seq)
		delete[] pUtf8Seq;
	if (pUtf8Voice)
		delete[] pUtf8Voice;

	return iRet;
}

BANKGWQ_API int __stdcall SCCBA_GetUserOperateRstHtml(int iPortNo, char *strVoice, char *strDispData, int iTimeout, char *iResult1, char *iResult2)
{
	int iRet, iIndex;
	int iStrVoiceLen, iStrVoiceSize, iStrDispDataLen, iStrDispDataSize;
	int iTotalSize, iCurrentlSegment, iTotalSegment, iSegmentSize, iReadSize, iWriteSize;
	unsigned char szBuffer[FRAME_MAX_SIZE];
	unsigned char *pUtf8Voice, *pUtf8DispData;

	if (strlen(strDispData) <= 0)
		return ERRCODE_INVALID_PARAMETER;

	iRet = comm_open(iPortNo, iPortNo, 9600);
	if (iRet)
		return iRet;

	pUtf8Voice = pUtf8DispData = NULL;
	iStrVoiceLen = iStrVoiceSize = 0;
	if (strVoice)
		if (!SCCBA_GB18030ToUTF8(strVoice, &pUtf8Voice))
			iStrVoiceLen = iStrVoiceSize = strlen((char *)pUtf8Voice);
	iStrDispDataLen = iStrDispDataSize = 0;
	if (strDispData)
		if (!SCCBA_GB18030ToUTF8(strDispData, &pUtf8DispData))
			iStrDispDataLen = iStrDispDataSize = strlen((char *)pUtf8DispData);

	iTotalSize = iStrVoiceSize + iStrDispDataSize;
	iTotalSegment = iTotalSize / 4000;
	if (iTotalSize % 4000)
		iTotalSegment += 1;
	iCurrentlSegment = iTotalSegment;

	do {
		iIndex = 0;
		memset(szBuffer, 0, sizeof(szBuffer));
		szBuffer[iIndex++] = 'O';                                            //Instruction code
		szBuffer[iIndex++] = 'R';
		sprintf((char *)&szBuffer[iIndex], "%04d", iStrVoiceLen);            //strVoice
		iIndex += 4;
		sprintf((char *)&szBuffer[iIndex], "%05d", iStrDispDataLen);         //strDispData
		iIndex += 5;
		sprintf((char *)&szBuffer[iIndex], "%03d", iTimeout);
		iIndex += 3;

		sprintf((char *)&szBuffer[iIndex], "%04d", iCurrentlSegment);        //Current Segment
		iIndex += 4;
		sprintf((char *)&szBuffer[iIndex], "%04d", iTotalSegment);           //Total Segment
		iIndex += 4;
		if (iTotalSize >= 4000)
			iSegmentSize = 4000;
		else
			iSegmentSize = iTotalSize;
		iTotalSize -= iSegmentSize;
		sprintf((char *)&szBuffer[iIndex], "%04d", iSegmentSize);            //Segment Size
		iIndex += 4;

		iWriteSize = 0;                                                      //Segment Data
		if (iStrVoiceSize)
		{
			if (iStrVoiceSize > iSegmentSize)
			{
				memcpy(&szBuffer[iIndex], &pUtf8Voice[iStrVoiceLen - iStrVoiceSize], iSegmentSize);
				iIndex += iSegmentSize;
				iStrVoiceSize -= iSegmentSize;
				iWriteSize += iSegmentSize;
			}
			else
			{
				memcpy(&szBuffer[iIndex], &pUtf8Voice[iStrVoiceLen - iStrVoiceSize], iStrVoiceSize);
				iIndex += iStrVoiceSize;
				iWriteSize += iStrVoiceSize;
				iStrVoiceSize = 0;
			}
		}

		if (iStrDispDataSize && (iWriteSize < iSegmentSize))
		{
			if (iStrDispDataSize >= (iSegmentSize - iWriteSize))
			{
				memcpy(&szBuffer[iIndex], &pUtf8DispData[iStrDispDataLen - iStrDispDataSize], (iSegmentSize - iWriteSize));
				iIndex += (iSegmentSize - iWriteSize);
				iStrDispDataSize -= (iSegmentSize - iWriteSize);
				iWriteSize = iSegmentSize;
			}
			else
			{
				memcpy(&szBuffer[iIndex], &pUtf8DispData[iStrDispDataLen - iStrDispDataSize], iStrDispDataSize);
				iIndex += iStrDispDataSize;
				iWriteSize += iStrDispDataSize;
				iStrDispDataSize = 0;
			}
		}

		iReadSize = sizeof(szBuffer);
		iRet = comm_frame_write(szBuffer, iIndex, szBuffer, &iReadSize, 5000);
		if (iRet)
			break;
		iCurrentlSegment--;
	} while (iTotalSize);

	if (!iRet)
	{
		iReadSize = sizeof(szBuffer);
		memset(szBuffer, 0, sizeof(szBuffer));
		iRet = comm_frame_receive(szBuffer, &iReadSize, iTimeout * 1000);
		if ((!iRet) && (iReadSize >= 9) && (szBuffer[0] == 'O') && (szBuffer[1] == 'R'))
		{
			char szLen[10];
			if (iResult1)
			{
				iResult1[0] = (char)szBuffer[4];
				iResult1[1] = 0;
			}
			if (iResult2)
			{
				int iLen;
				memset(szLen, 0, sizeof(szLen));
				memcpy(szLen, &szBuffer[5], 4);
				iLen = atoi(szLen);
				if ((iLen + 9) > iReadSize)
					iLen = iReadSize - 9;
				memcpy(iResult2, &szBuffer[9], iLen);
				iResult2[iLen] = 0;
			}
		}
	}

	comm_close();

	if (pUtf8Voice)
		delete[] pUtf8Voice;
	if (pUtf8DispData)
		delete[] pUtf8DispData;

	return iRet;
}

//10. ListSelection 
BANKGWQ_API int __stdcall SCCBA_ActiveSignQryInfo(int iPortNo, char *strVoice, int iTimeout, char *title, char *info, int col_num, char *leftbtn, char * rightbtn, char * gridtitle, char * gridinfo, int gridinfo_num, char * colwidth, int iType, char *iResult1, char * iResult2)
{
	int iRet, iIndex;
	int iStrVoiceLen, iTitleLen, iInfoLen, iLeftBtnLen, iRightBtnLen, iGridTitleLen, iGridInfoLen, iColWidthLen;
	int iStrVoiceSize, iTitleSize, iInfoSize, iLeftBtnSize, iRightBtnSize, iGridTitleSize, iGridInfoSize, iColWidthSize;
	int iTotalSize, iCurrentlSegment, iTotalSegment, iSegmentSize, iReadSize, iWriteSize;
	unsigned char szBuffer[FRAME_MAX_SIZE];
	unsigned char *pUtf8Voice, *pUtf8Title, *pUtf8Info, *pUtf8leftbtn, *pUtf8Rightbtn, *pUtf8Gridtitle, *pUtf8Gridinfo, *pUtf8Colwidth;

	iRet = comm_open(iPortNo, iPortNo, 9600);
	if (iRet)
		return iRet;

	iStrVoiceLen = iTitleLen = iInfoLen = iLeftBtnLen = iRightBtnLen = iGridTitleLen = iGridInfoLen = iColWidthLen = 0;
	iStrVoiceSize = iTitleSize = iInfoSize = iLeftBtnSize = iRightBtnSize = iGridTitleSize = iGridInfoSize = iColWidthSize = 0;
	pUtf8Voice = pUtf8Title = pUtf8Info = pUtf8leftbtn = pUtf8Rightbtn = pUtf8Gridtitle = pUtf8Gridinfo = pUtf8Colwidth = NULL;

	if (strVoice)
		if (!SCCBA_GB18030ToUTF8(strVoice, &pUtf8Voice))
			iStrVoiceLen = iStrVoiceSize = strlen((char*)pUtf8Voice);
	if (title)
		if (!SCCBA_GB18030ToUTF8(title, &pUtf8Title))
			iTitleLen = iTitleSize = strlen((char*)pUtf8Title);
	if (info)
		if (!SCCBA_GB18030ToUTF8(info, &pUtf8Info))
			iInfoLen = iInfoSize = strlen((char*)pUtf8Info);
	if (leftbtn)
		if (!SCCBA_GB18030ToUTF8(leftbtn, &pUtf8leftbtn))
			iLeftBtnLen = iLeftBtnSize = strlen((char*)pUtf8leftbtn);
	if (rightbtn)
		if (!SCCBA_GB18030ToUTF8(rightbtn, &pUtf8Rightbtn))
			iRightBtnLen = iRightBtnSize = strlen((char*)pUtf8Rightbtn);
	if (gridtitle)
		if (!SCCBA_GB18030ToUTF8(gridtitle, &pUtf8Gridtitle))
			iGridTitleLen = iGridTitleSize = strlen((char*)pUtf8Gridtitle);
	if (gridinfo)
		if (!SCCBA_GB18030ToUTF8(gridinfo, &pUtf8Gridinfo))
			iGridInfoLen = iGridInfoSize = strlen((char*)pUtf8Gridinfo);
	if (colwidth)
		if (!SCCBA_GB18030ToUTF8(colwidth, &pUtf8Colwidth))
			iColWidthLen = iColWidthSize = strlen((char*)pUtf8Colwidth);

	iTotalSize = iStrVoiceSize + iTitleSize + iInfoSize + iLeftBtnSize + iRightBtnSize + iGridTitleSize + iGridInfoSize + iColWidthSize;
	iTotalSegment = iTotalSize / 4000;
	if (iTotalSize % 4000)
		iTotalSegment += 1;
	iCurrentlSegment = iTotalSegment;

	do {
		iIndex = 0;
		memset(szBuffer, 0, sizeof(szBuffer));
		szBuffer[iIndex++] = 'Q';                                            //Instruction code
		szBuffer[iIndex++] = 'I';
		sprintf((char *)&szBuffer[iIndex], "%04d", iStrVoiceLen);            //strData
		iIndex += 4;
		sprintf((char *)&szBuffer[iIndex], "%03d", iTimeout);
		iIndex += 3;
		sprintf((char *)&szBuffer[iIndex], "%04d", iTitleLen);
		iIndex += 4;
		sprintf((char *)&szBuffer[iIndex], "%04d", iInfoLen);
		iIndex += 4;
		sprintf((char *)&szBuffer[iIndex], "%03d", col_num);
		iIndex += 3;
		sprintf((char *)&szBuffer[iIndex], "%04d", iLeftBtnLen);
		iIndex += 4;
		sprintf((char *)&szBuffer[iIndex], "%04d", iRightBtnLen);
		iIndex += 4;
		sprintf((char *)&szBuffer[iIndex], "%04d", iGridTitleLen);
		iIndex += 4;
		sprintf((char *)&szBuffer[iIndex], "%04d", iGridInfoLen);
		iIndex += 4;
		sprintf((char *)&szBuffer[iIndex], "%04d", gridinfo_num);
		iIndex += 4;
		sprintf((char *)&szBuffer[iIndex], "%04d", iColWidthLen);
		iIndex += 4;
		sprintf((char *)&szBuffer[iIndex++], "%01d", iType);

		sprintf((char *)&szBuffer[iIndex], "%04d", iCurrentlSegment);        //Current Segment
		iIndex += 4;
		sprintf((char *)&szBuffer[iIndex], "%04d", iTotalSegment);           //Total Segment
		iIndex += 4;
		if (iTotalSize >= 4000)
			iSegmentSize = 4000;
		else
			iSegmentSize = iTotalSize;
		iTotalSize -= iSegmentSize;
		sprintf((char *)&szBuffer[iIndex], "%04d", iSegmentSize);            //Segment Size
		iIndex += 4;

		iWriteSize = 0;                                                      //Segment Data
		if (iStrVoiceSize)
		{
			if (iStrVoiceSize > iSegmentSize)
			{
				memcpy(&szBuffer[iIndex], &pUtf8Voice[iStrVoiceLen - iStrVoiceSize], iSegmentSize);
				iIndex += iSegmentSize;
				iStrVoiceSize -= iSegmentSize;
				iWriteSize += iSegmentSize;
			}
			else
			{
				memcpy(&szBuffer[iIndex], &pUtf8Voice[iStrVoiceLen - iStrVoiceSize], iStrVoiceSize);
				iIndex += iStrVoiceSize;
				iWriteSize += iStrVoiceSize;
				iStrVoiceSize = 0;
			}
		}

		if (iTitleSize && (iWriteSize < iSegmentSize))
		{
			if (iTitleSize >= (iSegmentSize - iWriteSize))
			{
				memcpy(&szBuffer[iIndex], &pUtf8Title[iTitleLen - iTitleSize], (iSegmentSize - iWriteSize));
				iIndex += (iSegmentSize - iWriteSize);
				iTitleSize -= (iSegmentSize - iWriteSize);
				iWriteSize = iSegmentSize;
			}
			else
			{
				memcpy(&szBuffer[iIndex], &pUtf8Title[iTitleLen - iTitleSize], iTitleSize);
				iIndex += iTitleSize;
				iWriteSize += iTitleSize;
				iTitleSize = 0;
			}
		}

		if (iInfoSize && (iWriteSize < iSegmentSize))
		{
			if (iInfoSize >= (iSegmentSize - iWriteSize))
			{
				memcpy(&szBuffer[iIndex], &pUtf8Info[iInfoLen - iInfoSize], (iSegmentSize - iWriteSize));
				iIndex += (iSegmentSize - iWriteSize);
				iInfoSize -= (iSegmentSize - iWriteSize);
				iWriteSize = iSegmentSize;
			}
			else
			{
				memcpy(&szBuffer[iIndex], &pUtf8Info[iInfoLen - iInfoSize], iInfoSize);
				iIndex += iInfoSize;
				iWriteSize += iInfoSize;
				iInfoSize = 0;
			}
		}

		if (iLeftBtnSize && (iWriteSize < iSegmentSize))
		{
			if (iLeftBtnSize >= (iSegmentSize - iWriteSize))
			{
				memcpy(&szBuffer[iIndex], &pUtf8leftbtn[iLeftBtnLen - iLeftBtnSize], (iSegmentSize - iWriteSize));
				iIndex += (iSegmentSize - iWriteSize);
				iLeftBtnSize -= (iSegmentSize - iWriteSize);
				iWriteSize = iSegmentSize;
			}
			else
			{
				memcpy(&szBuffer[iIndex], &pUtf8leftbtn[iLeftBtnLen - iLeftBtnSize], iLeftBtnSize);
				iIndex += iLeftBtnSize;
				iWriteSize += iLeftBtnSize;
				iLeftBtnSize = 0;
			}
		}

		if (iRightBtnSize && (iWriteSize < iSegmentSize))
		{
			if (iRightBtnSize >= (iSegmentSize - iWriteSize))
			{
				memcpy(&szBuffer[iIndex], &pUtf8Rightbtn[iRightBtnLen - iRightBtnSize], (iSegmentSize - iWriteSize));
				iIndex += (iSegmentSize - iWriteSize);
				iRightBtnSize -= (iSegmentSize - iWriteSize);
				iWriteSize = iSegmentSize;
			}
			else
			{
				memcpy(&szBuffer[iIndex], &pUtf8Rightbtn[iRightBtnLen - iRightBtnSize], iRightBtnSize);
				iIndex += iRightBtnSize;
				iWriteSize += iRightBtnSize;
				iRightBtnSize = 0;
			}
		}

		if (iGridTitleSize && (iWriteSize < iSegmentSize))
		{
			if (iGridTitleSize >= (iSegmentSize - iWriteSize))
			{
				memcpy(&szBuffer[iIndex], &pUtf8Gridtitle[iGridTitleLen - iGridTitleSize], (iSegmentSize - iWriteSize));
				iIndex += (iSegmentSize - iWriteSize);
				iGridTitleSize -= (iSegmentSize - iWriteSize);
				iWriteSize = iSegmentSize;
			}
			else
			{
				memcpy(&szBuffer[iIndex], &pUtf8Gridtitle[iGridTitleLen - iGridTitleSize], iGridTitleSize);
				iIndex += iGridTitleSize;
				iWriteSize += iGridTitleSize;
				iGridTitleSize = 0;
			}
		}

		if (iGridInfoSize && (iWriteSize < iSegmentSize))
		{
			if (iGridInfoSize >= (iSegmentSize - iWriteSize))
			{
				memcpy(&szBuffer[iIndex], &pUtf8Gridinfo[iGridInfoLen - iGridInfoSize], (iSegmentSize - iWriteSize));
				iIndex += (iSegmentSize - iWriteSize);
				iGridInfoSize -= (iSegmentSize - iWriteSize);
				iWriteSize = iSegmentSize;
			}
			else
			{
				memcpy(&szBuffer[iIndex], &pUtf8Gridinfo[iGridInfoLen - iGridInfoSize], iGridInfoSize);
				iIndex += iGridInfoSize;
				iWriteSize += iGridInfoSize;
				iGridInfoSize = 0;
			}
		}

		if (iColWidthSize && (iWriteSize < iSegmentSize))
		{
			if (iColWidthSize >= (iSegmentSize - iWriteSize))
			{
				memcpy(&szBuffer[iIndex], &pUtf8Colwidth[iColWidthLen - iColWidthSize], (iSegmentSize - iWriteSize));
				iIndex += (iSegmentSize - iWriteSize);
				iColWidthSize -= (iSegmentSize - iWriteSize);
				iWriteSize = iSegmentSize;
			}
			else
			{
				memcpy(&szBuffer[iIndex], &pUtf8Colwidth[iColWidthLen - iColWidthSize], iColWidthSize);
				iIndex += iColWidthSize;
				iWriteSize += iColWidthSize;
				iColWidthSize = 0;
			}
		}

		iReadSize = sizeof(szBuffer);
		iRet = comm_frame_write(szBuffer, iIndex, szBuffer, &iReadSize, 5000);
		if (iRet)
			break;
		iCurrentlSegment--;
	} while (iTotalSize);

	if (!iRet)
	{
		iReadSize = sizeof(szBuffer);
		memset(szBuffer, 0, sizeof(szBuffer));
		iRet = comm_frame_receive(szBuffer, &iReadSize, iTimeout * 1000);
		if ((!iRet) && (iReadSize >= 9) && (szBuffer[0] == 'Q') && (szBuffer[1] == 'I'))
		{
			char szLen[10];
			if (iResult1)
			{
				iResult1[0] = (char)szBuffer[4];
				iResult1[1] = 0;
			}
			if (iResult2)
			{
				int iLen;
				memset(szLen, 0, sizeof(szLen));
				memcpy(szLen, &szBuffer[5], 4);
				iLen = atoi(szLen);
				if ((iLen + 9) > iReadSize)
					iLen = iReadSize - 9;
				memcpy(iResult2, &szBuffer[9], iLen);
				iResult2[iLen] = 0;
			}
		}
	}
	comm_close();

	if (pUtf8Voice)
		delete[] pUtf8Voice;
	if (pUtf8Title)
		delete[] pUtf8Title;
	if (pUtf8Info)
		delete[] pUtf8Info;
	if (pUtf8leftbtn)
		delete[] pUtf8leftbtn;
	if (pUtf8Rightbtn)
		delete[] pUtf8Rightbtn;
	if (pUtf8Gridtitle)
		delete[] pUtf8Gridtitle;
	if (pUtf8Gridinfo)
		delete[] pUtf8Gridinfo;
	if (pUtf8Colwidth)
		delete[] pUtf8Colwidth;

	return iRet;
}

/***
6.1.1.1写入设备号
*/
BANKGWQ_API int __stdcall  WriteDeviceId(int iPortNo, char extendPort, int iBaudRatee, int iTimeOut, char *pDeviceId, char * psErrInfo) {
	int iRet = comm_open(iPortNo, iPortNo, 9600);
	if (iRet != 0)
	{
		sprintf(psErrInfo, "%d|%s", -1, "端口打开失败");
		return -1;
	}
	unsigned char szBuffer[FRAME_MAX_SIZE] = { 0 };
	int iIndex = 0;
	int iLen = sizeof(szBuffer);
	int str_len = strlen(pDeviceId);
	szBuffer[iIndex++] = 'I';
	szBuffer[iIndex++] = 'D';
	szBuffer[iIndex++] = 1;
	sprintf((char *)&szBuffer[iIndex], "%02d", str_len);
	iIndex += 2;
	memcpy(szBuffer + iIndex, pDeviceId, str_len);
	iRet = comm_frame_write(szBuffer, iIndex, szBuffer, &iLen, iTimeOut * 1000);

	return 0;
}
/***
6.1.1.2读取设备号
*/
BANKGWQ_API int __stdcall ReadDeviceId(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, char *pDeviceId, char * psErrInfo) {

	int iRet = comm_open(iPortNo, iPortNo, 9600);
	if (iRet != 0)
	{
		sprintf(psErrInfo, "%d|%s", -1, "端口打开失败");
		return -1;
	}
	unsigned char szBuffer[FRAME_MAX_SIZE] = { 0 };
	int iIndex = 0;
	int iLen = sizeof(szBuffer);
	szBuffer[iIndex++] = 'I';
	szBuffer[iIndex++] = 'D';
	szBuffer[iIndex++] = 0;
	iRet = comm_frame_write(szBuffer, iIndex, szBuffer, &iLen, iTimeOut * 1000);
	//if (iRet == 0)
	//{
	int iReadSize = sizeof(szBuffer);
	memset(szBuffer, 0, sizeof(szBuffer));
	iRet = comm_frame_receive(szBuffer, &iReadSize, iTimeOut * 1000);
	comm_close();
	if ((szBuffer[0] == 'I') && (szBuffer[1] == 'D') && (szBuffer[2] == '0'))
	{
		memcpy(pDeviceId, szBuffer, iReadSize);
		return 0;
	}

	//}
	comm_close();
	sprintf(psErrInfo, "%d|%s", iRet, "读失败");
	return iRet;
}
/***
iPortNo端口号  COM 1 COM2 0表示USB口
extendPort：传入值包括’A’, ‘B’, ‘C’, ‘K’，分别对应串口扩展器的A口、B口、C口和K口，如果没接串口扩展器，传入’9’
iBaudRate：波特率  传0表示默认9600。若为USB接口，该参数无用处。
iTimeOut：传0表示默认30秒；其他超时设定则一直等待，直到超时，单位秒
ZmkIndex：索引号 初始密钥主ZMK的索引号，取值范围为0~15,共16组。
ZmkLength：秘钥长度  取值为1 代表单倍长(64bits)密钥；取值为2 代表双倍长(128bits)密钥；取值为3 代表三倍长(192bits)密钥
Zmk：明文主密钥  字符串，以空字符结束，初始主密钥ZMK明文，长度为ZmkLength决定。(长度可以取16/32/48)

出参：
CheckValues： 校验值  密钥验证字符串，以空字符结束，16个字符的密钥校验值(初始主密钥ZMK明文加密64比特的0所得结果)
psErrInfo：输出信息 “错误代码|错误信息”，最大长度32字节
*
*/

//初始化主秘钥明文
BANKGWQ_API int __stdcall  LoadClearZMK(int  iPortNo, char extendPort, int iBaudRate, int iTimeOut, int ZmkIndex, int ZmkLength, char *Zmk, char *CheckValues, char * psErrInfo) {
	return	SCCBA_InitPinPad(iPortNo);
}
/***
6.1.1.4密文注入主密钥ZMK



*/
BANKGWQ_API int __stdcall  LoadZMK(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, int ZmkIndex, int ZmkLength, char *Zmk, char *CheckValue1, char *CheckValue2, char * psErrInfo) {
	return	SCCBA_UpdateMKey(iPortNo, ZmkIndex, ZmkLength, Zmk, CheckValue1);
}

#pragma region 6.1.1.7	读取明文输入

CString ToVoiceStr(int VoiceType)
{
	CString voicestr = "";
	switch (VoiceType)
	{
	case 0:
		voicestr = "您好，请输入密码";
		break;
	case 1:
		voicestr = "请再输入一次";
		break;
	case 2:
		voicestr = "您好，请输入查询密码";
		break;
	case 3:
		voicestr = "您好，请输入交易密码";
		break;
	case 4:
		voicestr = "您好，请输入（号码）";
		break;
	case 5:
		voicestr = "您好，请输入验证码";
		break;
	case 6:
		voicestr = "您好，请输入手机号";
		break;
	case 7:
		voicestr = "您好，请输入手机号";
		break;
	case 8:
		break;
	case 9:
		break;
	case 10:
		break;
	case 11:
		break;
	case 12:
		break;
	case 13:
		break;
	default:
		break;
	}
	return voicestr;
}

BANKGWQ_API int __stdcall GetPlainText(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, int VoiceType, char *DisplayText, int DisPlayType, int EndType, int * PlainTextLength, char * PLainText, char * psErrInfo)
{
	CString voicestr = ToVoiceStr(VoiceType);

	int ret = SCCBA_ReadPin(iPortNo, 5, 1, 0, iTimeOut, voicestr.GetBuffer(), psErrInfo, EndType, PLainText);
	return 0;
}

#pragma endregion

#pragma region 6.1.2.5	检测柜员头像是否存在
BANKGWQ_API int __stdcall CheckTellerPhoto(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, char * tellerPhotoName, char * psErrInfo)
{
	CString photoname(tellerPhotoName);
	int ret = SCCBA_FindHeadPhoto(iPortNo, tellerPhotoName, photoname.GetLength());
	if (ret != 0)
	{
		CString errstr;
		errstr.Format("%d|%s", ret, "查找失败");
		memcpy(psErrInfo, errstr.GetBuffer(), errstr.GetLength());
	}
	return ret;
}

#pragma endregion

#pragma region 6.1.2.7	设置全部视频播放
BANKGWQ_API int __stdcall SetTabletVideoPlay(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, char * psErrInfo)
{
	int iRet, iLen;
	unsigned char szBuffer[FRAME_MAX_SIZE] = { 0 };
	int iTimeout = iTimeOut * 1000;
	iRet = comm_open(iPortNo, iPortNo, 9600);
	if (!iRet)
	{
		szBuffer[0] = 'P';
		szBuffer[1] = 'T';
		szBuffer[2] = 1;//视频
		iLen = sizeof(szBuffer);
		iRet = comm_frame_write(szBuffer, 3, szBuffer, &iLen, iTimeout);
		comm_close();
	}
	return iRet;
}

#pragma endregion

#pragma region 6.1.2.8	设置全部图片和视频混合播放
BANKGWQ_API int __stdcall SetTabletAllPlay(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, char * psErrInfo)
{
	return 0;
}

#pragma endregion

#pragma region 6.1.3.1	指纹模块确认
BANKGWQ_API int __stdcall FingerEnable(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, int * result, char * psErrInfo)
{
	char psFactory[1024] = { 0 };
	char psFirmVersion[1024] = { 0 };
	int ret = TcGetDeviceInfo(iPortNo, psFactory, psFirmVersion);
	if (ret != 0)
	{
		CString errstr;
		errstr.Format("%d|%s", ret, "获取指纹信息失败");
		memcpy(psErrInfo, errstr.GetBuffer(), errstr.GetLength());
	}
	*result = ret;
	return ret;

}
#pragma endregion

BANKGWQ_API int __stdcall FPGetFeature(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, char * psFeature, char * psErrInfo)
{
	unsigned char psFeatureInfo[1024 * 64] = { 0 };
	unsigned char pngdata[1024 * 64] = { 0 };
	int Fingerlength = 0;

	int iret = TcGetFingerFeature(0, NULL, psFeatureInfo, &Fingerlength, iTimeOut);
	if (iret != 0)
	{
		psErrInfo = "-29|指纹读取失败";
		return -29;
	}
	int iLength = 0;
	unsigned char  psfeature[1024 * 64] = { 0 };
	string feature((char *)psFeatureInfo);
	base64::base64_decode(feature, psfeature, &iLength);
	memcpy(psFeature, psfeature, iLength);
	return 0;
}
BANKGWQ_API int __stdcall FPGetTemplate(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, char * psTemplate, int iLength, char * psErrInfo)
{
	unsigned char psFeatureInfo[1024] = { 0 };
	//unsigned char pngdata[1024 * 64] = { 0 };
	int Fingerlength = 0;

	int iret = TcGetFingerTemplate(0, NULL, psFeatureInfo, &Fingerlength, iTimeOut);
	if (iret != 0)
	{
		psErrInfo = "-29|指纹读取失败";
		return -29;
	}
	unsigned char  psfeature[1024] = { 0 };
	string feature((char *)psFeatureInfo);
	base64::base64_decode(feature, psfeature, &iLength);
	memcpy(psTemplate, psfeature, iLength);
	return 0;
}
BANKGWQ_API int __stdcall FPGetImgData(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, int iIndex, char * psImgData, char * psErrInfo)
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
	if (psImgData == NULL)
	{
		psErrInfo = "-1|参数错误";
		return -1;
	}
	int iret = TcGetFingerFeature(0, bmppath, psFeatureInfo, &Fingerlength, iTimeOut);
	if (iret != 0)
	{
		psErrInfo = "-29|指纹读取失败";
		return -29;
	}
	ifstream in(bmppath, ios::binary);
	if (!in.is_open())
	{
		psErrInfo = "-30|指纹图像生成失败";
		return -30;
	}
	in.read(bmpdata, 31478);
	string s = base64::base64_encode((unsigned char *)bmpdata + 1078, 152 * 200);
	const char * p = s.c_str();
	memcpy(psImgData, p, s.length());
	return 0;
}
/***

6.1.1.5密文注入工作密钥ZPK

**/
BANKGWQ_API int __stdcall  LoadWorkKey(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, int KeyIndex, int KeyType, int KeyLength, char *Key, char *CheckValue1, char *CheckValue2, char * psErrInfo) {
	return	SCCBA_DownLoadWKey(iPortNo, KeyIndex, KeyIndex, KeyLength, Key, CheckValue1);

}
/***

检查密钥

**/

BANKGWQ_API int __stdcall  CheckKey(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, int KeyIndex, int KeyType, int *KeyLength, char *CheckValue, char * psErrInfo) {
	return	0;

}
/***
6.1.1.7读取客户密码PinBlock密文
**/

BANKGWQ_API int __stdcall  GetPin(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, int KeyIndex, char *AccNo, int VoiceType, int EndType, int *PinLength, char *PinCrypt, char * psErrInfo) {
	int iTimes = 1;
	char*strInfo = "";
	CString voicestr = ToVoiceStr(VoiceType);
	return	SCCBA_ReadPin(iPortNo, 5, iTimes, *PinLength, iTimeOut, voicestr.GetBuffer(), strInfo, EndType, PinCrypt);
}
/****
6.1.1.8初始化密码键盘
**/
BANKGWQ_API int __stdcall InitPinPad(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, char * psErrInfo) {
	return	SCCBA_InitPinPad(iPortNo);

}
/****
6.1.1.8关闭密码键盘
**/
BANKGWQ_API int __stdcall  ClosePinPad(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, char * psErrInfo) {
	return comm_close();
}

/****
6.1.2.1业务评价
**/
BANKGWQ_API int __stdcall  Evaluate(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, char* tellerName, char* tellerNo, int nStarLevel, char* photoPath, int *evalValue, char * psErrInfo) {

	char*strOpenData = NULL;
	char* strDispData = NULL;
	char*strVoice = "请对我们的服务进行评价";
	return SCCBA_StartEvaluate(iPortNo, tellerNo, photoPath, tellerName, strOpenData, strDispData, strVoice, iTimeOut, iTimeOut, (char*)evalValue);

}
/****
6.1.2.2显示交互信息
**/
BANKGWQ_API int __stdcall  DisplayInfo(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, char * Info, int *iDisplayResult, char * psErrInfo) {
	char*strVoice = "请确认信息是否正确";

	return	SCCBA_StartInfoHtml(iPortNo, iTimeOut, 1, strVoice, Info, iDisplayResult);
}
/****
6.1.2.3电子签名
**/
BANKGWQ_API int __stdcall ShowPDF(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, char *pdfPath, char *location, char *signPdfPath, char *signImgPath, int * signType, char *signData, char * psErrInfo) {
	unsigned char *pdfdata = NULL;
	int pdfdatalen = NULL;
	int page = 1;
	int x = NULL;
	int y = NULL; int w = NULL; int h = NULL;
	char *picdata = NULL; int *picdatalen = NULL; char *signeddata = NULL; int *signedLen = NULL;
	return 	SCCBA_signPDF(iPortNo, pdfdata, pdfdatalen, page, x, y, w, h, iTimeOut, picdata, picdatalen, signeddata, signedLen);
}

int  DownOneFile(char* path, char * &psErrInfo, int &ret, int iPortNo, int iBaudRate, unsigned char * sendbuff, unsigned char * retbuff, int retlen, int iTimeOut)
{
	int iTimeout = iTimeOut * 1000;
	string filename = PathFindFileName(path);

	if (filename == "")
	{
		psErrInfo = "-1|路径错误";
		return -1;
	}
	ifstream in(path, ios::binary);
	if (!in.is_open())
	{
		psErrInfo = "-30|文件读取失败";
		return -30;
	}
	in.seekg(0, ios::end);    // go to the end  
	int filelength = in.tellg();           //num   report location (this is the length)
	in.clear();
	in.seekg(0, ios::beg);
	char * buffer = new  char[filelength];
	if (in.eof())
	{
		return -2;
	}
	in.read(buffer, filelength);
	int x = filelength / 4000;//num2
	int m = filelength % 4;
	if (m != 0l)
	{
		x++;
	}
	int x1 = x;//num3
	int tmplength = filelength;
	ret = comm_open(iPortNo, iPortNo, iBaudRate);
	while (ret == 0 && tmplength > 0)
	{
		CString s;
		int bufferlen = 0;//num5
		int bufferindex = 0;
		if (tmplength > 4000)
		{
			bufferlen = 4000;
		}
		else
		{
			bufferlen = tmplength;
		}
		if (x1 == x)
		{
			int namelen = filename.length();//num7
			int len2 = 2 + namelen;//num8
			if (len2 + bufferlen > 4000)
			{
				bufferlen -= bufferlen + len2 - 4000;
			}

			s.Format("AF%.6d%.6d%.4d%.2d%s", x, x1, len2 + bufferlen, namelen, filename.c_str());
		}
		else
		{
			s.Format("AF%.6d%.6d%.4d", x, x1, bufferlen);
		}
		int slen = s.GetAllocLength();
		tmplength -= bufferlen;
		memset(sendbuff, 0, 4096);
		memset(retbuff, 0, 4096);
		memcpy(sendbuff, s.GetBuffer(), slen);
		memcpy(sendbuff + slen, buffer + bufferindex, bufferlen);
		bufferindex += bufferlen;
		ret = comm_frame_write(sendbuff, 4096, retbuff, &retlen, iTimeout);
		x1--;
	}
	delete[]buffer;
	in.close();
	comm_close();
	if (ret != 0)
	{
		CString errstr;
		errstr.Format("%d|%s", ret, "文件下载失败！");
		memcpy(psErrInfo, errstr.GetBuffer(), errstr.GetAllocLength());
	}
	return ret;
}

/****
批量下传图片及视频文件
**/
BANKGWQ_API int __stdcall DownloadFiles(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, char* path, int filetype, char * psErrInfo) {

	unsigned char  sendbuff[4096] = { 0 };
	unsigned char  retbuff[4096] = { 0 };
	int retlen = 0;
	int ret = -1;
	switch (filetype)
	{
	case 2: {

		ret = DownOneFile(iPortNo, path, 0);
		//return DownOneFile(path, psErrInfo, ret, iPortNo, iBaudRate, sendbuff, retbuff, retlen, iTimeOut);
		break;
	}
	default:
		break;
	}

	return ret;
}
/****
6.1.2.5设置全部图片播放
**/
BANKGWQ_API int __stdcall	SetTabletPictruePlay(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, int nSleepTime, char * psErrInfo) {
	int iRet, iLen;
	unsigned char szBuffer[FRAME_MAX_SIZE] = { 0 };
	int iTimeout = iTimeOut * 1000;
	iRet = comm_open(iPortNo, iPortNo, 9600);
	if (!iRet)
	{
		szBuffer[0] = 'P';
		szBuffer[1] = 'T';
		szBuffer[2] = 0;//图片
		iLen = sizeof(szBuffer);
		iRet = comm_frame_write(szBuffer, 3, szBuffer, &iLen, iTimeout);
		comm_close();
	}
	return iRet;
}
/****
6.1.2.7清除交互屏存储
**/
BANKGWQ_API int __stdcall DeleteFiles(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, int filetype, char * psErrInfo) {
	int iRet, iFilenameLen, iBufLen;
	char szBuffer[FRAME_MAX_SIZE];
	unsigned char *pUtf8Filename;


	iRet = comm_open(iPortNo, iPortNo, 9600);
	if (iRet)
		return iRet;

	iFilenameLen = 0;

	szBuffer[0] = 'D';
	szBuffer[1] = 'F';
	sprintf(&szBuffer[2], "%02d", 0);
	sprintf(&szBuffer[4], "%04d", 0);
	//memcpy(&szBuffer[8], pUtf8Filename, iFilenameLen);
	iBufLen = sizeof(szBuffer);
	iRet = comm_frame_write((unsigned char *)szBuffer, 8, (unsigned char *)szBuffer, &iBufLen, 5000);

	comm_close();


	return iRet;
}

#endif
