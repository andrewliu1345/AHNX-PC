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
#include <io.h>
#include <atlimage.h>


#define SIGNPDF_DEBUG

#ifdef BANKGWQ_SCCBA_API
CString ToVoiceStr(int VoiceType);
void getFolderDayFile(CString pathStr, vector<CString>& arrStrFile, int layer);// 遍历文件夹  获取文件名
int getErrorInfo(int errCode, char * errInfo);
int getDeviceErrInfo(int errCode, char * errInfo);
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

void getFolderDayFile(CString pathStr, vector<CString>& arrStrFile, int layer)
{
	_finddata_t file;
	string pathstr = pathStr + "*.*";
	long HANDLE;
	HANDLE = _findfirst(pathstr.c_str(), &file);
	do {
		//cout << file.name << endl;

		if (file.attrib == _A_SUBDIR)
		{
			int layer_tmp = layer;
			if (strcmp(file.name, "..") != 0 && strcmp(file.name, ".") != 0)
				getFolderDayFile(pathStr + file.name + '\\', arrStrFile, layer_tmp + 1);
		}
		else
		{
			arrStrFile.push_back(pathStr + file.name);
		}

	} while (!_findnext(HANDLE, &file));
	_findclose(HANDLE);
}

#pragma region 日志
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
#pragma endregion

#pragma region 返回码列表
int getErrorInfo(int errCode, char * errInfo)
{
	switch (errCode)
	{
	case 0:
		sprintf(errInfo, "%d|%s", errCode, "执行成功");
		break;
	case -1:
		sprintf(errInfo, "%d|%s", errCode, "执行失败");
		break;
	case -2:
		sprintf(errInfo, "%d|%s", errCode, "超时");
		break;
	case -3:
		sprintf(errInfo, "%d|%s", errCode, "打开端口失败");
		break;
	case -4:
		sprintf(errInfo, "%d|%s", errCode, "设备无应答");
		break;
	case -5:
		sprintf(errInfo, "%d|%s", errCode, "数据传输错误");
		break;
	case -6:
		sprintf(errInfo, "%d|%s", errCode, "消息长度过短");
		break;
	case -7:
		sprintf(errInfo, "%d|%s", errCode, "消息长度过长");
		break;
	case -8:
		sprintf(errInfo, "%d|%s", errCode, "EEPROM读写错");
		break;
	case -9:
		sprintf(errInfo, "%d|%s", errCode, "密钥组号错");
		break;
	case -10:
		sprintf(errInfo, "%d|%s", errCode, "密钥长度错");
		break;
	case -11:
		sprintf(errInfo, "%d|%s", errCode, "输入数据不合法");
		break;
	case -12:
		sprintf(errInfo, "%d|%s", errCode, "ZMK密钥不存在");
		break;
	case -13:
		sprintf(errInfo, "%d|%s", errCode, "工作密钥(ZPK、ZAK)不存在");
		break;
	case -14:
		sprintf(errInfo, "%d|%s", errCode, "密钥不存在(ZMK、ZPK、ZAK)");
		break;
	case -15:
		sprintf(errInfo, "%d|%s", errCode, "密钥校验值不一致");
		break;
	case -16:
		sprintf(errInfo, "%d|%s", errCode, "帐号域语法检查错");
		break;
	case -17:
		sprintf(errInfo, "%d|%s", errCode, "数据长度域语法检查错");
		break;
	case -18:
		sprintf(errInfo, "%d|%s", errCode, "取消操作");
		break;
	default:
		sprintf(errInfo, "%d|%s", errCode, "未知错误");
		break;
	}
	return errCode;
}
int getDeviceErrInfo(int errCode, char * errInfo)
{
	switch (errCode)
	{
	case  ERRCODE_SUCCESS:
		return getErrorInfo(0, errInfo);

	case ERRCODE_TIMEOUT:
		return getErrorInfo(-2, errInfo);

	case ERRCODE_FIND_USBDEVICE_FAILURE:
		getErrorInfo(-3, errInfo);
		return -3;
	case ERRCODE_OPERATION_IS_IN_PROCESS:
		getErrorInfo(-5, errInfo);
		return -5;
	case ERRCODE_CANCEL_OPERATION:
		getErrorInfo(-18, errInfo);
		return -18;
	default:
		getErrorInfo(-1, errInfo);
		return -1;
	}
}
#pragma endregion

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

	if (!iRet)
	{
		iReadSize = sizeof(szBuffer);
		memset(szBuffer, 0, sizeof(szBuffer));
		iRet = comm_frame_receive(szBuffer, &iReadSize, iTimeOut * 1000);
		if ((szBuffer[0] == 'S') && (szBuffer[1] == 'I') && (szBuffer[2] == '2') && (szBuffer[3] == '0'))
		{
			char tmp[] = { szBuffer[4] };
			*iResult = atoi(tmp);
		}

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

int  DownOneFile(int iPortNo, char * pFilePath, char *pFilename, int iFileType)
{
	// 	string filepath(pFilePath);
	// 	string suffixStr = filepath.substr(filepath.find_last_of('.') + 1);//获取文件后缀 
	// 	char pFilename[MAX_PATH] = {0} ;
	// 	sprintf(pFilename, "%s.%s", filename, suffixStr.c_str());
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
	return DownOneFile(iPortNo, pFilePath, pFilename, 1);

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
			if (iFileListLen == 0)
			{
				return 1;
			}
			else
			{
				return 0;
			}
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
			{
				*iResult = szBuffer[4];
			}
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
	FILE *pSignFile, *pSignDataFile, *pSignPdfFile;
#endif
	int iRet, iIndex;
	int iTotalSize, iSegmentSize, iWriteSize, iReadSize, iCurrentlSegment, iTotalSegment;
	int iHeaderSize, iStatusInfoSize, iFilenameSize, iMainpageInfoSize;
	int iStatusInfoLength, iFilenameLength, iMainpageInfoLength, iSignDataLength, iSignPngLength, iSignPdfFileLength;
	char szPdfFilename[] = "esign.pdf";
	unsigned char szBuffer[FRAME_MAX_SIZE], *pPdfData, *pPicData, *pSignData, *pSignPdfDate, *pUtf8BtnInfo;
	char cSignPngFilename[MAX_PATH] = { 0 }, cSignDataFilename[MAX_PATH] = { 0 }, cSignPdfFilename[MAX_PATH] = { 0 };

	int iRetPicBufLen, iRetSignDataBufLen, iRetSignPdfBufLen, iRetPicBufSize, iRetSignDataBufSize, iRetSignPdfBufSize;

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

	// 	memset(cSignPngFilename, 0, sizeof(cSignPngFilename));
	// 	memset(cSignDataFilename, 0, sizeof(cSignDataFilename));
#ifdef SIGNPDF_DEBUG
	pSignFile = NULL;
	pSignDataFile = NULL;
	pSignPdfFile = NULL;
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
		char currSegment[5] = {0}, totalSegment[5] = { 0 }, currSegmentSize[5] = { 0 }, signDataSize[7] = { 0 }, signPngSize[7] = { 0 }, signPdfSize[7] = { 0 };
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
			memcpy(cSignPdfFilename, cSignPngFilename, strlen(cSignPngFilename));
			sprintf(&cSignPngFilename[strlen(cSignPngFilename)], "%s", "\\sign.png");
			sprintf(&cSignDataFilename[strlen(cSignDataFilename)], "%s", "\\sign.txt");
			sprintf(&cSignPdfFilename[strlen(cSignPdfFilename)], "%s", "\\sign.pdf");

			pSignFile = fopen(cSignPngFilename, "wb");
			if (pSignFile == NULL)
				iRet = ERRCODE_OPENFILE_FAILURE;
			pSignDataFile = fopen(cSignDataFilename, "wb");
			if (pSignDataFile == NULL)
				iRet = ERRCODE_OPENFILE_FAILURE;
			pSignPdfFile = fopen(cSignPdfFilename, "wb");
			if (pSignPdfFile == NULL)
				iRet = ERRCODE_OPENFILE_FAILURE;
		}
#endif
		iSignDataLength = -1;
		iSignPngLength = -1;
		iSignPdfFileLength = -1;
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
			if (iSignPngLength < 0)
			{
				memset(signPngSize, 0, sizeof(signPngSize));
				memcpy(signPngSize, &szBuffer[iIndex], 6);
				iSignPngLength = atoi(signPngSize);
				if (iSignDataLength < 0)
					iSignPngLength = 0;
				iIndex += 6;
				iCurrSegmentSize -= 6;
			}
			if (iSignPdfFileLength < 0)
			{
				memset(signPdfSize, 0, sizeof(signPdfSize));
				memcpy(signPdfSize, &szBuffer[iIndex], 6);
				iSignPdfFileLength = atoi(signPdfSize);
				if (iSignPdfFileLength < 0)
					iSignPdfFileLength = 0;
				iIndex += 6;
				iCurrSegmentSize -= 6;
			}
			if (iReadSize = !(currSegmentSize + iIndex))
			{
				iRet = ERRCODE_RECEIVE_DATA_FAIL;
				break;
			}
#pragma region 签名数据
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
#pragma endregion

#pragma region 签名图片
			if (iSignPngLength > 0&& iCurrSegmentSize)
			{
				if (iCurrSegmentSize < iSignPngLength)
				{
					if (iRetPicBufLen > iCurrSegmentSize)
					{
						memcpy(pPicData, &szBuffer[iIndex], iCurrSegmentSize);
						iRetPicBufLen -= iCurrSegmentSize;
						pPicData += iCurrSegmentSize;
						*picdatalen += iCurrSegmentSize;
					}
					else if (iRetPicBufLen > 0)
					{
						memcpy(pPicData, &szBuffer[iIndex], iRetPicBufLen);
						*picdatalen += iRetPicBufLen;
						pPicData += iRetPicBufLen;
						iRetPicBufLen = 0;
					}
#ifdef SIGNPDF_DEBUG
					fwrite(&szBuffer[iIndex], iCurrSegmentSize, 1, pSignFile);
#endif
					iSignPngLength -= iCurrSegmentSize;
					iIndex += iSignPngLength;
					iCurrSegmentSize = 0;
				}
				else
				{
					if (iRetPicBufLen > iSignPngLength)
					{
						memcpy(pSignData, &szBuffer[iIndex], iSignPngLength);
						iRetPicBufLen -= iSignPngLength;
						*picdatalen += iSignPngLength;
						pPicData += iSignPngLength;
					}
					else if (iRetSignPdfBufLen > 0)
					{
						memcpy(pSignData, &szBuffer[iIndex], iRetPicBufLen);
						*picdatalen += iRetPicBufLen;
						pPicData += iRetPicBufLen;
						iRetPicBufLen = 0;
					}
#ifdef SIGNPDF_DEBUG
					fwrite(&szBuffer[iIndex], iSignPngLength, 1, pSignFile);
#endif
					iCurrSegmentSize -= iSignPngLength;
					iIndex += iSignPngLength;
					iSignPngLength = 0;
				}
			}
#pragma endregion

#pragma region 签名PDF
			if (iSignPdfFileLength > 0&&iCurrSegmentSize)
			{
				if (iCurrSegmentSize < iSignPdfFileLength)
				{
					if (iRetSignPdfBufLen > iCurrSegmentSize)
					{
						memcpy(pSignPdfDate, &szBuffer[iIndex], iCurrSegmentSize);
						iRetSignPdfBufLen -= iCurrSegmentSize;
						pSignPdfDate += iCurrSegmentSize;
						//*signPdfSize += iCurrSegmentSize;
					}
					else if (iRetSignPdfBufLen > 0)
					{
						memcpy(pSignPdfDate, &szBuffer[iIndex], iRetSignPdfBufLen);
						//*picdatalen += iRetSignPdfBufLen;
						pSignPdfDate += iRetSignPdfBufLen;
						iRetSignPdfBufLen = 0;
					}
#ifdef SIGNPDF_DEBUG
					fwrite(&szBuffer[iIndex], iCurrSegmentSize, 1, pSignPdfFile);
#endif
					iSignPdfFileLength -= iCurrSegmentSize;
					iIndex += iSignPdfFileLength;
					iCurrSegmentSize = 0;
				}
				else
				{
					if (iRetSignPdfBufLen > iSignPngLength)
					{
						memcpy(pSignData, &szBuffer[iIndex], iSignPdfFileLength);
						iRetSignPdfBufLen -= iSignPdfFileLength;
						//*signedLen += iSignPngLength;
						pPicData += iSignPdfFileLength;
					}
					else if (iRetSignPdfBufLen > 0)
					{
						memcpy(pSignPdfDate, &szBuffer[iIndex], iRetSignPdfBufLen);
						//*signedLen += iRetSignPdfBufLen;
						pSignPdfDate += iRetSignPdfBufLen;
						iRetSignPdfBufLen = 0;
					}
#ifdef SIGNPDF_DEBUG
					fwrite(&szBuffer[iIndex], iSignPdfFileLength, 1, pSignPdfFile);
#endif
					iCurrSegmentSize -= iSignPdfFileLength;
					iIndex += iSignPdfFileLength;
					iSignPdfFileLength = 0;
				}
			}
#pragma endregion

// 			if (iCurrSegmentSize)
// 			{
// #ifdef SIGNPDF_DEBUG
// 				fwrite(&szBuffer[iIndex], iCurrSegmentSize, 1, pSignFile);
// #endif
// 				if (iRetPicBufLen > iCurrSegmentSize)
// 				{
// 					memcpy(pPicData, &szBuffer[iIndex], iCurrSegmentSize);
// 					iRetPicBufLen -= iCurrSegmentSize;
// 					*picdatalen += iCurrSegmentSize;
// 					pPicData += iCurrSegmentSize;
// 				}
// 				else if (iRetPicBufLen > 0)
// 				{
// 					memcpy(pSignData, &szBuffer[iIndex], iRetPicBufLen);
// 					pPicData += iRetPicBufLen;
// 					*picdatalen += iRetPicBufLen;
// 					iRetPicBufLen = 0;
// 				}
// 			}
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
	if (pSignPdfFile)
		fclose(pSignPdfFile);
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
#pragma region 键盘操作
#pragma region 6.1.1.1写入设备号
/***
6.1.1.1写入设备号
*/
BANKGWQ_API int __stdcall  WriteDeviceId(int iPortNo, char extendPort, int iBaudRatee, int iTimeOut, char *pDeviceId, char * psErrInfo) {
	int iRet = comm_open(iPortNo, iPortNo, 9600);
	if (iRet != 0)
	{
		return getErrorInfo(-3, psErrInfo);
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
	iIndex += str_len;
	iRet = comm_frame_write(szBuffer, iIndex, szBuffer, &iLen, iTimeOut * 1000);
	comm_close();
	return	getDeviceErrInfo(iRet, psErrInfo);

}
#pragma endregion

#pragma region 6.1.1.2读取设备号
/***
6.1.1.2读取设备号
*/
BANKGWQ_API int __stdcall ReadDeviceId(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, char *pDeviceId, char * psErrInfo) {

	int iRet = comm_open(iPortNo, iPortNo, 9600);
	if (iRet != 0)
	{
		return getErrorInfo(-3, psErrInfo);
	}
	unsigned char szBuffer[FRAME_MAX_SIZE] = { 0 };
	int iIndex = 0;
	int iLen = sizeof(szBuffer);
	szBuffer[iIndex++] = 'I';
	szBuffer[iIndex++] = 'D';
	szBuffer[iIndex++] = 0;
	iRet = comm_frame_write(szBuffer, iIndex, szBuffer, &iLen, iTimeOut * 1000);
	comm_close();

	if (iRet == 0 && (szBuffer[0] == 'I') && (szBuffer[1] == 'D') && iLen > 8 && (szBuffer[2] == '0') && szBuffer[3] == '0')
	{
		char ilen[4] = { 0 };
		ilen[0] = szBuffer[4];
		ilen[1] = szBuffer[5];
		ilen[2] = szBuffer[6];
		ilen[3] = szBuffer[7];

		int iReadSize = atoi(ilen);
		memcpy(pDeviceId, szBuffer + 8, iReadSize);
		return	getDeviceErrInfo(iRet, psErrInfo);
	}

	return getErrorInfo(-1, psErrInfo);
}
#pragma endregion

#pragma region 6.1.1.3	明文注入初始主密钥ZMK
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
	int iRet = SCCBA_UpdateMKey(iPortNo, ZmkIndex, ZmkLength, Zmk, CheckValues);
	return getDeviceErrInfo(iRet, psErrInfo);
}
#pragma endregion

#pragma region 6.1.1.4密文注入主密钥ZMK
/***
6.1.1.4密文注入主密钥ZMK
*/
BANKGWQ_API int __stdcall  LoadZMK(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, int ZmkIndex, int ZmkLength, char *Zmk, char *CheckValue1, char *CheckValue2, char * psErrInfo) {
	int iRet = SCCBA_UpdateMKey(iPortNo, ZmkIndex, ZmkLength, Zmk, CheckValue1);
	return getDeviceErrInfo(iRet, psErrInfo);
}
#pragma endregion

#pragma region 6.1.1.5密文注入工作密钥ZPK
/***

6.1.1.5密文注入工作密钥ZPK

**/
BANKGWQ_API int __stdcall  LoadWorkKey(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, int KeyIndex, int KeyType, int KeyLength, char *Key, char *CheckValue1, char *CheckValue2, char * psErrInfo) {
	int iRet = SCCBA_DownLoadWKey(iPortNo, KeyIndex, KeyIndex, KeyLength, Key, CheckValue1);
	return getDeviceErrInfo(iRet, psErrInfo);
}
#pragma endregion

#pragma region 6.1.1.6	检查密钥
/***

检查密钥

**/

BANKGWQ_API int __stdcall  CheckKey(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, int KeyIndex, int KeyType, int *KeyLength, char *CheckValue, char * psErrInfo) {
	int iRet = comm_open(iPortNo, iPortNo, 9600);

	return getDeviceErrInfo(iRet, psErrInfo);

}
#pragma endregion

#pragma region 6.1.1.7	读取明文输入
BANKGWQ_API int __stdcall GetPlainText(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, int VoiceType, char *DisplayText, int DisPlayType, int EndType, int * PlainTextLength, char * PLainText, char * psErrInfo)
{
	CString voicestr = ToVoiceStr(VoiceType);

	int ret = SCCBA_ReadPin(iPortNo, 1, 1, *PlainTextLength, iTimeOut, voicestr.GetBuffer(), psErrInfo, EndType, PLainText);
	return getDeviceErrInfo(ret, psErrInfo);
}
#pragma endregion

#pragma region 6.1.1.8读取客户密码PinBlock密文
/***
6.1.1.8读取客户密码PinBlock密文
**/

BANKGWQ_API int __stdcall  GetPin(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, int KeyIndex, char *AccNo, int VoiceType, int EndType, int *PinLength, char *PinCrypt, char * psErrInfo) {
	int iTimes = 1;
	char*strInfo = "";
	CString voicestr = ToVoiceStr(VoiceType);
	int iRet = SCCBA_ReadPin(iPortNo, 5, iTimes, *PinLength, iTimeOut, voicestr.GetBuffer(), strInfo, EndType, PinCrypt);
	return getDeviceErrInfo(iRet, psErrInfo);
}
#pragma endregion

#pragma region 6.1.1.9初始化密码键盘
/****
6.1.1.9初始化密码键盘
**/
BANKGWQ_API int __stdcall InitPinPad(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, char * psErrInfo) {
	int iRet = SCCBA_InitPinPad(iPortNo);
	return getDeviceErrInfo(iRet, psErrInfo);

}
#pragma endregion

#pragma region 6.1.1.10关闭密码键盘
/****
6.1.1.10关闭密码键盘
**/
BANKGWQ_API int __stdcall  ClosePinPad(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, char * psErrInfo) {
	int iRet, iIndex, iReadSize;

	unsigned char szBuffer[FRAME_MAX_SIZE];

	iRet = comm_open(iPortNo, iPortNo, 9600);
	if (iRet)
		return getDeviceErrInfo(iRet, psErrInfo);

	iIndex = 0;
	memset(szBuffer, 0, sizeof(szBuffer));
	szBuffer[iIndex++] = 'R';                                     //Instruction code
	szBuffer[iIndex++] = 'P';
	sprintf((char *)&szBuffer[iIndex++], "%1d", 0);              //iEncryType

	iReadSize = sizeof(szBuffer);
	iRet = comm_frame_write(szBuffer, iIndex, szBuffer, &iReadSize, 5000);
	return getDeviceErrInfo(iRet, psErrInfo);

}
#pragma endregion
#pragma endregion

#pragma region 6.1.2.5	检测柜员头像是否存在
BANKGWQ_API int __stdcall CheckTellerPhoto(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, char * tellerPhotoName, char * psErrInfo)
{
	int iRet, iFileListLen, iNameLen, iIndex;
	unsigned char szBuffer[FRAME_MAX_SIZE] = { 0 };
	iNameLen = strlen(tellerPhotoName);
	iRet = comm_open(iPortNo, iPortNo, 9600);
	if (iRet)
		return iRet;
	iIndex = 0;
	szBuffer[iIndex++] = 'E';
	szBuffer[iIndex++] = 'F';
	sprintf((char *)&szBuffer[iIndex], "%02d", 1);
	iIndex += 2;
	sprintf((char *)&szBuffer[iIndex], "%02d", iNameLen);
	iIndex += 2;
	sprintf((char *)&szBuffer[iIndex], "%s", tellerPhotoName);
	iIndex += iNameLen;
	iFileListLen = sizeof(szBuffer);
	iRet = comm_frame_write(szBuffer, iIndex, szBuffer, &iFileListLen, 5000);
	comm_close();
	if (!iRet)
	{
		if ((szBuffer[0] == 'E') && (szBuffer[1] == 'F') && szBuffer[2] == '0'&&szBuffer[3] == '0')
		{
			return getErrorInfo(0, psErrInfo);
		}
		else
		{
			return getErrorInfo(1, psErrInfo);
		}
	}
	return getDeviceErrInfo(iRet, psErrInfo);
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
	int iRet, iLen;
	unsigned char szBuffer[FRAME_MAX_SIZE] = { 0 };
	int iTimeout = iTimeOut * 1000;
	iRet = comm_open(iPortNo, iPortNo, 9600);
	if (!iRet)
	{
		szBuffer[0] = 'P';
		szBuffer[1] = 'T';
		szBuffer[2] = 2;//播放全部
		iLen = sizeof(szBuffer);
		iRet = comm_frame_write(szBuffer, 3, szBuffer, &iLen, iTimeout);
		comm_close();
	}
	return iRet;
}

#pragma endregion


#pragma region 6.1.3	指纹仪模块
#pragma region 6.1.3.1	指纹模块确认
BANKGWQ_API int __stdcall FingerEnable(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, int * result, char * psErrInfo)
{
	char psFactory[1024] = { 0 };
	char psFirmVersion[1024] = { 0 };
	int ret = TcGetDeviceInfo(iPortNo, psFactory, psFirmVersion);
	if (ret != 0)
	{

		*result = 2;
		return getErrorInfo(-1, psErrInfo);
	}
	else
	{
		*result = 1;
		return getErrorInfo(0, psErrInfo);
	}
}
#pragma endregion

#pragma region 6.1.3.2	获取指纹特征值
BANKGWQ_API int __stdcall FPGetFeature(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, char * psFeature, char * psErrInfo)
{
	SCCBA_PlayVoice(iPortNo, "请按指纹");
	unsigned char psFeatureInfo[1024 * 64] = { 0 };
	unsigned char pngdata[1024 * 64] = { 0 };
	int Fingerlength = 0;

	int iret = TcGetFingerFeature(0, NULL, psFeatureInfo, &Fingerlength, iTimeOut);
	if (iret != 0)
	{
		return getErrorInfo(-1, psErrInfo);
	}
	int iLength = 0;
	unsigned char  psfeature[1024 * 64] = { 0 };
	string feature((char *)psFeatureInfo);
	base64::base64_decode(feature, psfeature, &iLength);
	memcpy(psFeature, psfeature, iLength);
	return getErrorInfo(0, psErrInfo);
}
#pragma endregion

#pragma region 6.1.3.3	获取指纹模板
BANKGWQ_API int __stdcall FPGetTemplate(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, char * psTemplate, int iLength, char * psErrInfo)
{
	SCCBA_PlayVoice(iPortNo, "请按指纹");
	unsigned char psFeatureInfo[1024] = { 0 };
	unsigned char psFeatureInfo2[1024] = { 0 };
	//unsigned char pngdata[1024 * 64] = { 0 };
	int Fingerlength = 0;
	int Fingerlength2 = 0;
	char bmppath[MAX_PATH] = { 0 };
	string bmpfile;
	getTempPath(bmppath);
	bmpfile = bmppath;
	bmpfile += "tmp.bmp";
	strcpy(bmppath, bmpfile.c_str());

	int iret = TcGetFingerTemplate(0, bmppath, psFeatureInfo, &Fingerlength, iTimeOut);
	if (iret != 0)
	{
		return getErrorInfo(-1, psErrInfo);
	}
	iret = TcGetFingerFeature(iPortNo, NULL, psFeatureInfo2, &Fingerlength2, iTimeOut);
	if (iret != 0)
	{
		return getErrorInfo(-1, psErrInfo);
	}
	iret = TcMatchFinger(psFeatureInfo, psFeatureInfo2, 3);
	if (iret != 0)
	{
		return getErrorInfo(-1, psErrInfo);
	}
	unsigned char  psfeature[1024] = { 0 };
	string feature((char *)psFeatureInfo);
	base64::base64_decode(feature, psfeature, &iLength);
	memcpy(psTemplate, psfeature, iLength);
	return getErrorInfo(0, psErrInfo);
}
#pragma endregion

#pragma region 6.1.3.4	获取指纹图像
BANKGWQ_API int __stdcall FPGetImgData(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, int iIndex, char * psImgData, char * psErrInfo)
{
	char bmpdata[31478] = { 0 };
	char bmppath[MAX_PATH] = { 0 };
	string bmpfile;
	getTempPath(bmppath);
	bmpfile = bmppath;
	bmpfile += "tmp.bmp";
	strcpy(bmppath, bmpfile.c_str());
	// 	unsigned char psFeatureInfo[1024 * 64] = { 0 };
	// 	unsigned char pngdata[1024 * 64] = { 0 };

	// 	int Fingerlength = 0;
	// 	if (psImgData == NULL)
	// 	{
	// 		return getErrorInfo(-1, psErrInfo);
	// 	}
	// 	int iret = TcGetFingerFeature(0, bmppath, psFeatureInfo, &Fingerlength, iTimeOut);
	// 	if (iret != 0)
	// 	{
	// 		return getErrorInfo(-1, psErrInfo);
	// 	}
	ifstream in(bmppath, ios::binary);
	if (!in.is_open())
	{
		return getErrorInfo(-1, psErrInfo);
	}
	in.read(bmpdata, 31478);
	string s = base64::base64_encode((unsigned char *)bmpdata + 1078, 152 * 200);
	const char * p = s.c_str();
	memcpy(psImgData, p, s.length());
	return getErrorInfo(0, psErrInfo);
}
#pragma endregion
#pragma endregion








#pragma region 6.1.2.1业务评价
/****
6.1.2.1业务评价
**/
BANKGWQ_API int __stdcall  Evaluate(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, char* tellerName, char* tellerNo, int nStarLevel, char* photoPath, int *evalValue, char * psErrInfo) {

	char retValue[1] = { 0 };
	char*strOpenData = "请你进行评价";
	char* strDispData = "感谢你的参与";
	char*strVoice = "请对我们的服务进行评价";
	int iret = SCCBA_StartEvaluate(iPortNo, tellerNo, photoPath, tellerName, strOpenData, strDispData, strVoice, iTimeOut, iTimeOut, retValue);
	if (iret != 0)
	{
		if (iret == ERRCODE_TIMEOUT)
		{
			*evalValue = 4;
		}
		return getDeviceErrInfo(iret, psErrInfo);
	}
	*evalValue = atoi(retValue);
	return  getDeviceErrInfo(iret, psErrInfo);

}
#pragma endregion


#pragma region 6.1.2.2显示交互信息
/****
6.1.2.2显示交互信息
**/
BANKGWQ_API int __stdcall DisplayInfo(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, int Infotype, char *Info, int *iDisplayResult, char * psErrInfo) {
	char*strVoice = "请确认您的交易信息";
	int type = 0, iResult = 0;
	if (Infotype == 2)
	{
		type = 1;
	}
	int iRet = SCCBA_StartInfoHtml(iPortNo, iTimeOut, type, strVoice, Info, &iResult);
	*iDisplayResult = iResult;
	return getDeviceErrInfo(iRet, psErrInfo);
}
#pragma endregion


/****
6.1.2.3电子签名
**/
BANKGWQ_API int __stdcall ShowPDF(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, char *pdfPath, char *location, char * signpicSize, char *signPdfPath, char *signImgPath, int * signType, char *signData, long* signDataLen, char * psErrInfo) {
	char *pdfdata;
	char  *picdata;
	int pdfdatalen = 0;
	int page = 1;
	int iFileSize = 0;
	int piclen = 0;
	char cSignPngFilename[MAX_PATH] = { 0 };
	char * x_char = strtok(location, ",");
	char *y_char = strtok(NULL, ",");

	char *w_char = strtok(signpicSize, ":");
	char *h_char = strtok(NULL, ":");

	int x = atoi(x_char);
	int y = atoi(y_char);
	int w = atoi(w_char);
	int h = atoi(h_char);
	piclen = w*h + 2000;
	picdata = new char[piclen];
	memset(picdata, 0, piclen);
	ifstream pFile(pdfPath, ios::binary);
	if (!pFile.is_open())
	{
		return getErrorInfo(-1, psErrInfo);
	}

	pFile.seekg(0, ios::end);
	iFileSize = pFile.tellg();
	pFile.clear();
	pFile.seekg(0, ios::beg);
	pdfdata = new char[iFileSize];
	memset(pdfdata, 0, iFileSize);
	pFile.read(pdfdata, iFileSize);
	pFile.close();
	int iret = SCCBA_signPDF(iPortNo, (unsigned char *)pdfdata, iFileSize, page, x, y, w, h, iTimeOut, picdata, &piclen, signData, (int *)signDataLen);
	delete[]pdfdata;
	if (iret != 0)
	{
		sprintf(psErrInfo, "%d|%s", iret, "签名失败");
		return iret;
	}

	GetCurrentDirectory(MAX_PATH, cSignPngFilename);
	sprintf(&cSignPngFilename[strlen(cSignPngFilename)], "%s", "\\sign.png");
	CImage image;
	image.Load(cSignPngFilename);
	image.Save(signImgPath);//保存BMP

// 	ofstream picFile(signImgPath, ios::binary);//保存BMP
// 	if (!picFile.is_open())
// 	{
// 		return getErrorInfo(-1, psErrInfo);
// 	}
// 	picFile.write(picdata, iFileSize);
// 	picFile.close();
	char exe_path[MAX_PATH] = { 0 };
	char cSignPdfFilename[MAX_PATH] = { 0 };
	GetCurrentDirectory(MAX_PATH, exe_path);
	memcpy(cSignPdfFilename, exe_path, strlen(exe_path));
	sprintf(&cSignPdfFilename[strlen(cSignPdfFilename)], "%s", "\\sign.pdf");

	ifstream inPdfFile(cSignPdfFilename, ios::binary);
	if (!inPdfFile.is_open())
	{
		return getErrorInfo(-1, psErrInfo);
	}
	inPdfFile.seekg(0, ios::end);
	iFileSize = inPdfFile.tellg();
	
	pdfdata = new char[iFileSize];
	inPdfFile.clear();
	inPdfFile.seekg(0, ios::beg);
	inPdfFile.read(pdfdata,iFileSize);

	ofstream saPdfFile(signPdfPath, ios::binary);//保存PDF
	if (!saPdfFile.is_open())
	{
		return getErrorInfo(-1, psErrInfo);
	}
	saPdfFile.write(pdfdata, iFileSize);
	saPdfFile.close();
	delete[]picdata;
	delete[]pdfdata;
	return iret;
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
BANKGWQ_API int __stdcall DownloadFiles(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, char* path, int filetype, char* tellerPhotoName, char * psErrInfo) {

	unsigned char  sendbuff[4096] = { 0 };
	unsigned char  retbuff[4096] = { 0 };
	int retlen = 0;
	int ret = -1;
	int type = -1;
	switch (filetype)
	{
	case 0:
		type = 4;
		break;
	case 1:
		type = 2;
		break;
	case 2:
		type = 1;
		ret = DownOneFile(iPortNo, path, tellerPhotoName, type);
		return getDeviceErrInfo(ret, psErrInfo);

	case 3:
		type = 3;
		break;
	default:
		break;
	}
	ret = GetFileAttributes(path);
	if (ret == FILE_ATTRIBUTE_DIRECTORY)
	{

		vector<CString> filearry;
		getFolderDayFile(path, filearry, 0);
		for each (CString file_path in filearry)
		{
			char * file_name = PathFindFileName(file_path);
			ret = DownOneFile(iPortNo, file_path.GetBuffer(), file_name, type);
		}
	}
	else
	{
		char * file_name = PathFindFileName(path);
		ret = DownOneFile(iPortNo, path, file_name, type);
	}



	//return DownOneFile(path, psErrInfo, ret, iPortNo, iBaudRate, sendbuff, retbuff, retlen, iTimeOut);


	return getDeviceErrInfo(ret, psErrInfo);
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
	int iRet, iFilenameLen, iBufLen, iIndex;
	char szBuffer[FRAME_MAX_SIZE];
	unsigned char *pUtf8Filename;


	iRet = comm_open(iPortNo, iPortNo, 9600);
	if (iRet)
		return iRet;

	int type = -1;
	switch (filetype)
	{
	case 0:
		type = 4;
		break;
	case 1:
		type = 2;
		break;
	case 2:
		type = 1;
		break;
	case 3:
		type = 3;
		break;
	default:
		break;
	}
	iFilenameLen = 0;
	iIndex = 0;
	szBuffer[iIndex++] = 'D';
	szBuffer[iIndex++] = 'F';
	sprintf(&szBuffer[iIndex], "%02d", type);
	iIndex += 2;
	sprintf(&szBuffer[iIndex], "%04d", 0);
	iIndex += 4;
	//memcpy(&szBuffer[8], pUtf8Filename, iFilenameLen);
	iBufLen = sizeof(szBuffer);
	iRet = comm_frame_write((unsigned char *)szBuffer, iIndex, (unsigned char *)szBuffer, &iBufLen, 5000);

	comm_close();


	return getDeviceErrInfo(iRet, psErrInfo);
}

#endif
