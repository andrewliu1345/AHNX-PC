// stdafx.cpp : source file that includes just the standard includes
// BankGwq.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file
#pragma comment ( lib,"kernel32.lib" )
#pragma comment ( lib,"user32.lib" )
#pragma comment ( lib,"gdi32.lib" )
#pragma comment ( lib,"hid.lib" )
#pragma comment ( lib,"SetupAPI.lib" )
#ifdef _DEBUG
#pragma comment ( lib,"lib/debug/GwqDllComm.lib" )
#pragma comment ( lib,"lib/debug/TESO_NH_HID.lib" )
// #pragma comment ( lib,"lib/debug/cximage.lib" )
// #pragma comment ( lib,"lib/debug/jasper.lib" )
// #pragma comment ( lib,"lib/debug/jbig.lib" )
// #pragma comment ( lib,"lib/debug/jpeg.lib" )
// #pragma comment ( lib,"lib/debug/libdcr.lib" )
// #pragma comment ( lib,"lib/debug/libpsd.lib" )
// #pragma comment ( lib,"lib/debug/mng.lib" )
// #pragma comment ( lib,"lib/debug/png.lib" )
// #pragma comment ( lib,"lib/debug/tiff.lib" )
// #pragma comment ( lib,"lib/debug/zlib.lib" )
#else
#pragma comment ( lib,"lib/release/GwqDllComm.lib" )
// #pragma comment ( lib,"lib/release/cximage.lib" )
// #pragma comment ( lib,"lib/release/jasper.lib" )
// #pragma comment ( lib,"lib/release/jbig.lib" )
// #pragma comment ( lib,"lib/release/jpeg.lib" )
// #pragma comment ( lib,"lib/release/libdcr.lib" )
// #pragma comment ( lib,"lib/release/libpsd.lib" )
// #pragma comment ( lib,"lib/release/mng.lib" )
// #pragma comment ( lib,"lib/release/png.lib" )
// #pragma comment ( lib,"lib/release/tiff.lib" )
// #pragma comment ( lib,"lib/release/zlib.lib" )
#pragma comment ( lib,"lib/release/TESO_NH_HID.lib" )
#endif