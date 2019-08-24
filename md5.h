#ifndef MD5_H_2019_08_19
#define MD5_H_2019_08_19

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <io.h>
#endif

#include "Header.h"

class MD5_CTX {
public:
	MD5_CTX();
	virtual ~MD5_CTX();
	bool GetFileMd5(char *pMd5,  const char *pFileName);

private:
	//WARNING 2019-08-23 ubuntu X64 下unsigned long int 长度为8字节，而windows下为4字节，为了统一改成了unsigned int
	unsigned int state[4];			/* state (ABCD) */
	unsigned int count[2];			/* number of bits, modulo 2^64 (lsb first) */
	unsigned char buffer[64];       /* input buffer */
	unsigned char PADDING[64];		/* What? */

private:
	void MD5Init ();
	void MD5Update( unsigned char *input, unsigned int inputLen);
	void MD5Final (unsigned char digest[16]);
	void MD5Transform (unsigned int state[4], unsigned char block[64]);
	void MD5_memcpy (unsigned char* output, unsigned char* input,unsigned int len);
	void Encode (unsigned char *output, unsigned int *input,unsigned int len);
	void Decode (unsigned int *output, unsigned char *input, unsigned int len);
	void MD5_memset (unsigned char* output,int value,unsigned int len);
};

#endif //MD5_H