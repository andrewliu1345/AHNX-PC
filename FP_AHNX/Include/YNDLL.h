// YNDLL.h : main header file for the YNDLL DLL
//

bool _stdcall abc_FINGER_TEMPLATE(unsigned char * pFingerTemplate,unsigned short  iFingerTemplateLen);
bool _stdcall abc_FINGER_FEATURE (unsigned char * pFingerFeature ,unsigned short  iFingerFeatureLen );
bool _stdcall abc_FINGER_MATCH   (unsigned char * pFingerFeature ,unsigned char * pFingerTemplate   );

// ==================================================================
// 20131211 TCCZY ��������� 6 ���ӿں���������ũ��ָ��
// ==================================================================

// ==================================================================
// Function : ��ȡָ�����豸��Ϣ
// Argument : int iFlag            -> �豸���� 0-USB 1��2��3��8����
// Argument : char *psFactory      -> ����Ӣ����д����дTECHSHINO), 10�ֽ�
// Argument : char *psFirmVersion  -> �̼��汾��20�ֽ�
// Return   : 0-�ɹ�����0-ʧ��
// ==================================================================
int WINAPI TcGetDeviceInfo(int iFlag, char *psFactory, char *psFirmVersion);

// ==================================================================
// Function : ��ȡע��ָ��ͼ��ע������
// Argument : int iFlag                       -> �豸���� 0-USB 1��2��3��8����
// Argument : char *ImgPath                   -> ͼƬ�����·������c:\\Teso.bmp(���·��Ϊ�ռ�ΪNULL,�򲻻���ͼ�����,ֻ�ᴫ������ֵ)
// Argument : unsigned char *psTemplateInfo   -> ע��ָ������(BASE64����)
// Argument : int *TemplateLen                -> ע��ָ�������ĳ���
// Argument : int TimeOut                     -> ��ʱʱ��(��)
// Return   : 0-�ɹ�����0-ʧ��
// ==================================================================
int WINAPI TcGetFingerTemplate(int iFlag, char *ImgPath, unsigned char *psTemplateInfo, int *TemplateLen, int TimeOut);

// ==================================================================
// Function : ��ȡ��ָ֤��ͼ����֤����
// Argument : int iFlag                     -> �豸���� 0-USB 1��2��3��8����
// Argument : char *ImgPath                 -> ͼƬ�����·������c:\\Teso.bmp(���·��Ϊ�ռ�ΪNULL,�򲻻���ͼ�����,ֻ�ᴫ������ֵ)
// Argument : unsigned char *psFeatureInfo  -> ��ָ֤������(BASE64����)
// Argument : int *FeatureLen				-> ��ָ֤�������ĳ���
// Argument : int TimeOut                   -> ��ʱʱ��(��)
// Return   : 0-�ɹ�����0-ʧ��
// ==================================================================
int WINAPI TcGetFingerFeature(int iFlag, char *ImgPath, unsigned char *psFeatureInfo, int *FeatureLen, int TimeOut);

// ==================================================================
// Function : ָ�������Ա�
// Argument : unsigned char *psTemplateInfo  -> ע��ָ������(BASE64����)
// Argument : unsigned char *psFeatureInfo   -> ��ָ֤������(BASE64����)
// Argument : int nLevel                     -> �ȶԵȼ�(1-5����Ĭ��Ϊ3������5�ĵȼ����)
// Return   : 0-�ɹ�����0-ʧ��
// ==================================================================
int WINAPI TcMatchFinger(unsigned char *psTemplateInfo, unsigned char *psFeatureInfo, int nLevel = 3);

// ==================================================================
// Function : ȡ������
// Argument : ��
// Return   : 0-�ɹ�����0-ʧ��
// ==================================================================
int WINAPI TcCancel();

// ==================================================================
// Function : ��ʼ����λ����
// Argument : ��
// Return   : 0-�ɹ�����0-ʧ��
// ==================================================================
int WINAPI TcInit();


/////////////////////////////////////////////////////////////////////////////


