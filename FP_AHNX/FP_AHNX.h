// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� FP_AHNX_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// FP_AHNX_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#ifdef FP_AHNX_EXPORTS
#define FP_AHNX_API __declspec(dllexport)
#else
#define FP_AHNX_API __declspec(dllimport)
#endif

// �����Ǵ� FP_AHNX.dll ������
class FP_AHNX_API CFP_AHNX {
public:
	CFP_AHNX(void);
	// TODO:  �ڴ�������ķ�����
};
//ָ����
FP_AHNX_API int __stdcall FPGetFeature(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, char *psFeature, char * psErrInfo);
FP_AHNX_API int __stdcall FPGetTemplate(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, char *psTemplate, int iLength, char * psErrInfo);
FP_AHNX_API int __stdcall FPGetImgData(int iPortNo, char extendPort, int iBaudRate, int iTimeOut, int iIndex, char *psImgData, char * psErrInfo);