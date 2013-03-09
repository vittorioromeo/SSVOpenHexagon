#ifndef HG_MD5
#define HG_MD5

/* Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
rights reserved.

License to copy and use this software is granted provided that it
is identified as the "RSA Data Security, Inc. MD5 Message-Digest
Algorithm" in all material mentioning or referencing this software
or this function.

License is also granted to make and use derivative works provided
that such works are identified as "derived from the RSA Data
Security, Inc. MD5 Message-Digest Algorithm" in all material
mentioning or referencing the derived work.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

These notices must be retained in any copies of any part of this
documentation and/or software.
 */

#include <string>
#include <fstream>
#define uint32 unsigned int

#include<stdint.h>
using namespace std;

class MD5
{
	public:
        MD5();
        MD5(const std::string& source);
        MD5(std::ifstream& file);
        MD5(const unsigned char* source, uint32 len);

        std::string Calculate(const std::string& source);
        std::string Calculate(std::ifstream& file);
        std::string Calculate(const unsigned char* source, uint32_t len);

        std::string GetHash() const;
        const unsigned char* GetRawHash() const { return m_rawHash; }

	private:
        std::string     m_sHash;
        unsigned char m_rawHash[16];
};

#endif
