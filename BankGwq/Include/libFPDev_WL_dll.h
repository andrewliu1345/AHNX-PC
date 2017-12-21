/****************************************************************************** 
 * ��������: libFPDev_WL_dll.h                                                        *
 * ������: �㽭ά��ָ�ƿ��ļ�                                               *
 * ����汾: 1.000                                                            *
 * ������Ա: kf                                                               *
 * �������: 2011.05.03                                                       *
 * �޸���Ա:                                                                  *
 * �޸�����:                                                                  *
 * �޸�����:                                                                  *
 ******************************************************************************/

#ifndef __LIBFPDEVWL_DLL_H__
#define __LIBFPDEVWL_DLL_H__

#ifdef __cplusplus 
extern "C" { 
#endif

	// ����ũ�Ű汾
	//int  __declspec(dllexport)  __stdcall   FPIGetFeature(int nPortNo, int nTimeOut, char *lpFeature);
	int  __declspec(dllexport)  __stdcall   FPGetFeature (int iPortNo, char extendPort ,int iBaudRate, int iTimeOut, char *psFeature, char * psErrInfo);
	int  __declspec(dllexport)  __stdcall   FPGetTemplate(int iPortNo, char extendPort ,int iBaudRate, int iTimeOut, char *psTemplate,int *iLength, char * psErrInfo);
	int  __declspec(dllexport)  __stdcall   FPGetImgData (int iPortNo, char extendPort ,int iBaudRate, int iTimeOut,int iIndex,char *psImgData, char * psErrInfo);
	int	 __declspec(dllexport)  __stdcall	FPIEnrollX(unsigned char *psTZ1, unsigned char *psTZ2, unsigned char *psTZ3, unsigned char *psMB, int *lpLength);
	int  __declspec(dllexport)  __stdcall	FPIMatch(unsigned char *psRegBuf, unsigned char *psVerBuf, int iLevel);
	int __declspec(dllexport)  __stdcall FPIGetVersion(int nPort, char extendPort ,int iBaudRate, unsigned char *psOutversion, int *lpLength);
	int  __declspec(dllexport)  __stdcall   FPIFeatureAndImgeData (int nPort, char extendPort ,int iBaudRate, int iTimeOut, char *psFeature, char *psImgData, int *lpLength,char * psErrInfo);
	//__declspec(dllexport)   char*  __stdcall FPIGetVendorInfo();
		
#ifdef __cplusplus
}
#endif

#endif