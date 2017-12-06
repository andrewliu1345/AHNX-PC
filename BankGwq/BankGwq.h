// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the BANKGWQ_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// BANKGWQ_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifndef BANKGWQ_H
#define BANKGWQ_H

#define BANKGWQ_API extern "C" __declspec(dllexport)

//BANKGWQ_API int fnBankGwq(void);

#endif //BANKGWQ_H
