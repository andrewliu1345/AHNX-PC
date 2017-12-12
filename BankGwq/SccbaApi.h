#pragma once

#include "BankGwq.h"

#ifdef __cplusplus
extern "C" {
#endif

//02. Push Information
BANKGWQ_API int __stdcall SCCBA_StartInfoHtml(int iPortNo, int iTimeOut, int modex, char* strVoice, char *Info, int *iResult);

//03. Public Interface
BANKGWQ_API int __stdcall SCCBA_inputNumber(int iPortNo, int iLength, int iControlType, int iTimeOut, char *strVoice, char *iResult);
BANKGWQ_API int __stdcall SCCBA_PlayVoice(int iPortNo, char *strVoice);

//04. Electronic Card
BANKGWQ_API int __stdcall SCCBA_DownHeadFile(int iPortNo, char *pFilePath, char* pFilename);
BANKGWQ_API int __stdcall SCCBA_FindHeadPhoto(int iPortNo, char * pFilename, int pFilenameLenght);
BANKGWQ_API int __stdcall SCCBA_DelHeadFile(int iPortNo, char * pFilename);
BANKGWQ_API int __stdcall SCCBA_StartElectronicCard(int iPortNo, /*char extendPort, int iBaudRate,*/ char *tellerName, char *tellerNo, int nStarLevel, char *headFile, int iTimeOut);
BANKGWQ_API int __stdcall SCCBA_ReadDeviceId(int iPortNo, char *pDeviceId, int iSize);

//05. Evaluate
BANKGWQ_API int __stdcall SCCBA_StartEvaluate(int iPortNo, char * tellerID, char * headFile, char *tellerName, char *strOperData, char *strDispData, char *strVoice, int strDispTimeout, int strTimeout, char *iResult);

//06. Security Keyboard
BANKGWQ_API int __stdcall SCCBA_ReadPin(int iPortNo, int iEncryType, int iTimes, int iLength, int iTimeout, char *strVoice, char * strInfo, int EndType, char *iResult);
BANKGWQ_API int __stdcall SCCBA_InitPinPad(int iPortNo);
BANKGWQ_API int __stdcall SCCBA_UpdateMKey(int iPortNo, int ZmkIndex, int ZmkLength, char *Zmk, char* CheckValue);
BANKGWQ_API int __stdcall SCCBA_DownLoadWKey(int iPortNo, int MKeyIndex, int WKeyIndex, int WKeyLength, char *Key, char*CheckValue);
BANKGWQ_API int __stdcall SCCBA_ActiveWKey(int iPortNo, int MKeyIndex, int WKeyIndex);

//07. Electronic Sign
BANKGWQ_API int __stdcall SCCBA_signPDF(int iPortNo, unsigned char *pdfdata, int pdfdatalen, int page, int x, int y, int w, int h, int iTimeout, char *picdata, int *picdatalen, char *signeddata, int *signedLen);
BANKGWQ_API int __stdcall SCCBA_startSignPDF(int iPortNo, unsigned char *pdfdata, int pdfdatalen, int page, int x, int y, int w, int h, int iTimeout);
BANKGWQ_API int __stdcall SCCBA_getSignPDFState(int iPortNo, int *signState, char * picdata, int *picdataLen, char *signeddata, int* signedLen);
BANKGWQ_API int __stdcall SCCBA_cancelSignPDF(int iPortNo);
BANKGWQ_API int __stdcall SCCBA_showPDF(int iPortNo, unsigned char* pdfdata, int pdfdataLen, int iTimeout, char *buttonText);
BANKGWQ_API int __stdcall SCCBA_confirmPDF(int iPortNo, int waitResult, unsigned char* pdfdata, int pdfdatalen, int iTimeout, char * leftbtn, char * rightbtn, char * strVoice, char* iResult);
BANKGWQ_API int __stdcall SCCBA_signPDFEx(int iPortNo, int signtype, unsigned char * pdfdata, int pdfdataLen, int page,
	int x, int y, int w, int h, int iTimeout, char* strVoice, int * rettype,
	char * picdata, int *picdataLen, char *signdata, int* signdataLen,
	char * fingerPicdata, int* fingerPicdataLen, char * fingerdata, int * fingerdataLen);
//08. Finger
BANKGWQ_API int __stdcall SCCBA_ReadFinger(int iPortNo, int iTimeout, char * strVoice, char * fingerPicdata, int*
	fingerPicdataLen, char * fingerdata, int* fingerdataLen, int* fingerType);
//09. Question
BANKGWQ_API int __stdcall SCCBA_OperateQuestion(int iPortNo, char *strData, char *strItem, int iType, char *strSeq, char *strVoice, int iTimeout, char * iResult1, char * iResult2);
BANKGWQ_API int __stdcall SCCBA_GetUserOperateRstHtml(int iPortNo, char * strVoice, char * strDispData, int iTimeout, char * iResult1, char * iResult2);

//10. ListSelection 
BANKGWQ_API int __stdcall SCCBA_ActiveSignQryInfo(int iPortNo, char * strVoice, int iTimeout, char *	title, char * info, int col_num, char * leftbtn, char * rightbtn, char * gridtitle, char * gridinfo, int gridinfo_num, char * colwidth, int iType, char * iResult1, char * iResult2);


// 安徽农信
BANKGWQ_API int __stdcall DisplayInfo(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, int Infotype, char *Info, int *iDisplayResult, char * psErrInfo);
BANKGWQ_API int __stdcall ReadDeviceId(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, char *pDeviceId, char * psErrInfo);
BANKGWQ_API int __stdcall WriteDeviceId(int iPortNo, char extendPort, int iBaudRatee, int iTimeOut, char *pDeviceId, char * psErrInfo);
BANKGWQ_API int __stdcall DeleteFiles(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, int filetype, char * psErrInfo);
BANKGWQ_API int __stdcall SetTabletPictruePlay(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, int nSleepTime, char * psErrInfo);
BANKGWQ_API int __stdcall DownloadFiles(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, char* path, int filetype, char * psErrInfo);
BANKGWQ_API int __stdcall ShowPDF(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, char *pdfPath, char *location, char * signpicSize, char *signPdfPath, char *signImgPath, int * signType, char *signData, long* signDataLen, char * psErrInfo);
BANKGWQ_API int __stdcall Evaluate(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, char* tellerName, char* tellerNo, int nStarLevel, char* photoPath, int *evalValue, char * psErrInfo);
BANKGWQ_API int __stdcall ClosePinPad(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, char * psErrInfo);
BANKGWQ_API int __stdcall GetPin(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, int KeyIndex, char *AccNo, int VoiceType, int EndType, int *PinLength, char *PinCrypt, char * psErrInfo);
BANKGWQ_API int __stdcall LoadClearZMK(int  iPortNo, char extendPort, int iBaudRate, int iTimeOut, int ZmkIndex, int ZmkLength, char *Zmk, char *CheckValues, char * psErrInfo);
BANKGWQ_API int __stdcall CheckKey(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, int KeyIndex, int KeyType, int *KeyLength, char *CheckValue, char * psErrInfo);
BANKGWQ_API int __stdcall LoadWorkKey(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, int KeyIndex, int KeyType, int KeyLength, char *Key, char *CheckValue1, char *CheckValue2, char * psErrInfo);
BANKGWQ_API int __stdcall LoadZMK(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, int ZmkIndex, int ZmkLength, char *Zmk, char *CheckValue1, char *CheckValue2, char * psErrInfo);
BANKGWQ_API int __stdcall GetPlainText(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, int VoiceType, char *DisplayText,int DisPlayType, int EndType, int *PlainTextLength, char *PLainText, char * psErrInfo);
BANKGWQ_API int __stdcall CheckTellerPhoto(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, char *tellerPhotoName, char* psErrInfo);
BANKGWQ_API int __stdcall SetTabletVideoPlay(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, char * psErrInfo);
BANKGWQ_API int __stdcall SetTabletAllPlay(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, char * psErrInfo);
BANKGWQ_API int __stdcall FingerEnable(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, int* result, char * psErrInfo);



//指纹仪
BANKGWQ_API int __stdcall FPGetFeature(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, char *psFeature, char * psErrInfo);
BANKGWQ_API int __stdcall FPGetTemplate(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, char *psTemplate, int iLength, char * psErrInfo);
BANKGWQ_API int __stdcall FPGetImgData(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, int iIndex, char *psImgData, char * psErrInfo);
#ifdef __cplusplus
}
#endif
