#ifndef _ENDIANNESS_H
#define _ENDIANNESS_H



inline void endian_swap(u16& x)
{
	x = (x>>8) | 
		(x<<8);
}

inline void endian_swap(u32& x)
{
	x = (x>>24) | 
		((x<<8) & 0x00FF0000) |
		((x>>8) & 0x0000FF00) |
		(x<<24);
}

// __int64 for MSVC, "long long" for gcc
inline void endian_swap(u64& x)
{
	x = (x>>56) | 
		((x<<40) & 0x00FF000000000000) |
		((x<<24) & 0x0000FF0000000000) |
		((x<<8)  & 0x000000FF00000000) |
		((x>>8)  & 0x00000000FF000000) |
		((x>>24) & 0x0000000000FF0000) |
		((x>>40) & 0x000000000000FF00) |
		(x<<56);
}


#endif // _RAND_H