#pragma once

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
#define ERRCODE_CANCEL_OPERATION         19

#define ERROR_MSG_MAX_SIZE               4096

int _stdcall GetLastErrorCode();
int _stdcall SetLastErrorCode(int iErrCode);
int _stdcall GetLastErrorMsg(int iLen, char *pError);
int _stdcall SetLastErrorMsg(int iLen, char *pError);
int _stdcall ClearLastError();
