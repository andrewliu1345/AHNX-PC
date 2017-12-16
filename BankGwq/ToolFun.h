#pragma once

int bmp2png( char *bmppath, unsigned char *png);

int getTempPath(char * path);

void HexStrToByte(const char* source, unsigned char* dest, int sourceLen);

void ByteToHexStr(const unsigned char* source, char* dest, int sourceLen);