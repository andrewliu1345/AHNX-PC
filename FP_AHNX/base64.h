#pragma once
#include <string.h>
class base64
{
public:
	base64();
	~base64();
public:
	static std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len);
	static int base64_decode(std::string const& encoded_string, unsigned char *decodedata,int *out_len);
};

