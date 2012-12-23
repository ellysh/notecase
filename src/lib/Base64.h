////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Base64 encoding/decoding
////////////////////////////////////////////////////////////////////////////
//
// Base64.h: interface for the CBase64 class.
// Author: Wes Clyburn (clyburnw@enmu.edu)
//////////////////////////////////////////////////////////////////////
// WARNING: this class DOES NOT pad  with '=' as per RFC 1521
//    to the 4byte border (when encoding).
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BASE64_H__FD6A25D1_EE0E_11D1_870E_444553540001__INCLUDED_)
#define AFX_BASE64_H__FD6A25D1_EE0E_11D1_870E_444553540001__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <string>
#define BYTES_PER_LINE  72

// CBase64
// An encoding agent that handles Base64
//
class CBase64
{
public:
	CBase64();
	virtual ~CBase64();

	// Override the base class mandatory functions
	virtual int Decode( const char* szDecoding, char* szOutput );
	virtual std::string Encode( const char* szEncoding, int nSize );

protected:
	inline void write_bits(unsigned int nBits, int nNumBts, char* szOutput, int& lp);
	inline unsigned int read_bits(int nNumBits, int* pBitsRead, int& lp);

	int m_nInputSize;
	int m_nBitsRemaining;
	unsigned long m_lBitStorage;
	const char* m_szInput;

	static int m_nMask[];
	static std::string m_sBase64Alphabet;
};

#endif // !defined(AFX_BASE64_H__FD6A25D1_EE0E_11D1_870E_444553540001__INCLUDED_)
