// YNDLL.h : main header file for the YNDLL DLL
//

bool _stdcall abc_FINGER_TEMPLATE(unsigned char * pFingerTemplate,unsigned short  iFingerTemplateLen);
bool _stdcall abc_FINGER_FEATURE (unsigned char * pFingerFeature ,unsigned short  iFingerFeatureLen );
bool _stdcall abc_FINGER_MATCH   (unsigned char * pFingerFeature ,unsigned char * pFingerTemplate   );

// ==================================================================
// 20131211 TCCZY 新增下面的 6 个接口函数，用于农行指纹
// ==================================================================

// ==================================================================
// Function : 获取指纹仪设备信息
// Argument : int iFlag            -> 设备类型 0-USB 1、2、3…8串口
// Argument : char *psFactory      -> 厂商英文缩写（大写TECHSHINO), 10字节
// Argument : char *psFirmVersion  -> 固件版本，20字节
// Return   : 0-成功，非0-失败
// ==================================================================
int WINAPI TcGetDeviceInfo(int iFlag, char *psFactory, char *psFirmVersion);

// ==================================================================
// Function : 获取注册指纹图像、注册特征
// Argument : int iFlag                       -> 设备类型 0-USB 1、2、3…8串口
// Argument : char *ImgPath                   -> 图片保存的路径例如c:\\Teso.bmp(如果路径为空即为NULL,则不会有图像输出,只会传出特征值)
// Argument : unsigned char *psTemplateInfo   -> 注册指纹特征(BASE64编码)
// Argument : int *TemplateLen                -> 注册指纹特征的长度
// Argument : int TimeOut                     -> 超时时间(秒)
// Return   : 0-成功，非0-失败
// ==================================================================
int WINAPI TcGetFingerTemplate(int iFlag, char *ImgPath, unsigned char *psTemplateInfo, int *TemplateLen, int TimeOut);

// ==================================================================
// Function : 获取验证指纹图像、验证特征
// Argument : int iFlag                     -> 设备类型 0-USB 1、2、3…8串口
// Argument : char *ImgPath                 -> 图片保存的路径例如c:\\Teso.bmp(如果路径为空即为NULL,则不会有图像输出,只会传出特征值)
// Argument : unsigned char *psFeatureInfo  -> 验证指纹特征(BASE64编码)
// Argument : int *FeatureLen				-> 验证指纹特征的长度
// Argument : int TimeOut                   -> 超时时间(秒)
// Return   : 0-成功，非0-失败
// ==================================================================
int WINAPI TcGetFingerFeature(int iFlag, char *ImgPath, unsigned char *psFeatureInfo, int *FeatureLen, int TimeOut);

// ==================================================================
// Function : 指纹特征对比
// Argument : unsigned char *psTemplateInfo  -> 注册指纹特征(BASE64编码)
// Argument : unsigned char *psFeatureInfo   -> 验证指纹特征(BASE64编码)
// Argument : int nLevel                     -> 比对等级(1-5级，默认为3，其中5的等级最高)
// Return   : 0-成功，非0-失败
// ==================================================================
int WINAPI TcMatchFinger(unsigned char *psTemplateInfo, unsigned char *psFeatureInfo, int nLevel = 3);

// ==================================================================
// Function : 取消操作
// Argument : 无
// Return   : 0-成功，非0-失败
// ==================================================================
int WINAPI TcCancel();

// ==================================================================
// Function : 初始化起复位作用
// Argument : 无
// Return   : 0-成功，非0-失败
// ==================================================================
int WINAPI TcInit();


/////////////////////////////////////////////////////////////////////////////


