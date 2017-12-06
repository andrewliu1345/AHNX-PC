#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define FRAME_MAX_SIZE  8184

enum COMMUNICATIONTYPE
{
	COMM_TYPE_MIN = 0,
	COMM_TYPE_HID = 0,
	COMM_TYPE_SERIAL = 1,
	COMM_TYPE_ADB = 256,
	COMM_TYPE_MAX = 256
};

#define ERRCODE_SUCCESS                  0
#define ERRCODE_FIND_USBDEVICE_FAILURE   10
#define ERRCODE_DEVICE_HAS_OPENED        11
#define ERRCODE_INVALID_PARAMETER        12
#define ERRCODE_TIMEOUT                  13
#define ERRCODE_OPENFILE_FAILURE         14
#define ERRCODE_RECEIVE_DATA_FAIL        15
#define ERRCODE_OPERATION_IS_IN_PROCESS  16
#define ERRCODE_SYSCALL_FAILURE          17
#define ERRCODE_RESPONSECODE_WRONG       18
#define ERRCODE_CANCEL_OPERATION         -23


int _stdcall comm_open(int iType, int iPortNum, int iBaudRate);
int _stdcall comm_cancelio(void);
int _stdcall comm_close();

int _stdcall comm_frame_send(unsigned char *pBuffer, int iBufferSize);
int _stdcall comm_frame_receive(unsigned char *pBuffer, int *pBufferSize, int iTimeout);
int _stdcall comm_frame_write(unsigned char *pInBuffer, int iInBufferSize, unsigned char *pOutBuffer, int *pOutBufferSize, int iTimeout);

#ifdef __cplusplus
}
#endif