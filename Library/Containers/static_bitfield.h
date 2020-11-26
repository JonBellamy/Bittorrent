#ifndef BITFIELD_H
#define BITFIELD_H

#include "Debug/CompileTimeAssert.h"

template <u32 numberOfValues, u32 bitsPerValue>
class cBitField
{
private:
	// Could be templatised parameter but 32 bit is probably fastest.
	typedef u32 tStorage;

	enum eIntrinsics
	{
		STORAGE_BITS = sizeof(tStorage) * 8,
		VALUES_PER_STORAGE = STORAGE_BITS/bitsPerValue,
		STORAGE_REQUIRED = ((numberOfValues-1)/VALUES_PER_STORAGE)+1,
		VALUE_MASK = (1 << bitsPerValue)-1
	};

	// You've changed the storage size so it isn't 32 bit aligned... you'll need
	// to change Reset probably.
	//COMPILE_TIME_ASSERT( sizeof(tStorage) == 4 );

	tStorage mStorage[STORAGE_REQUIRED];

	u32 StorageIndex(u32 id) const { ASSERT( id < numberOfValues); return id / VALUES_PER_STORAGE; }
	u32 StorageShift(u32 id) const { ASSERT( id < numberOfValues); return (id % VALUES_PER_STORAGE)*bitsPerValue; }


public:
	// Compile time assert moved here to prevent scope clash... 
	// bitsPerValue is 1 - you'd be better off using a conventional method.
	cBitField()					{ Zero(); }
	inline void Zero()			{ FastClear(&mStorage, 0, sizeof(mStorage)); }
	inline void MaxAll()		{ FastClear(&mStorage, 0xff, sizeof(mStorage)); }
	inline void Set( u32 index, u32 value );
	inline u32  Get( u32 index ) const;
	inline void Inc( u32 index );
	inline void Dec( u32 index );
	// Add our values together (in parallel)
	void Accumulate( const cBitField &other );
	// Set a bitfield with the values of another bitfield
	void Set( const cBitField &other );
};

template <u32 numberOfValues, u32 bitsPerValue>
inline void cBitField<numberOfValues, bitsPerValue>::Set( u32 index, u32 value )
{ 
	ASSERT( value <= VALUE_MASK );
	mStorage[StorageIndex(index)] &= ~(VALUE_MASK << StorageShift(index));
	mStorage[StorageIndex(index)] |= ((value & VALUE_MASK) << StorageShift(index));
}

template <u32 numberOfValues, u32 bitsPerValue>
inline u32 cBitField<numberOfValues, bitsPerValue>::Get( u32 index ) const
{
	return (mStorage[StorageIndex(index)] >> StorageShift(index))&VALUE_MASK;
}

template <u32 numberOfValues, u32 bitsPerValue>
inline void cBitField<numberOfValues, bitsPerValue>::Inc( u32 index )
{
	ASSERT( Get(index) < VALUE_MASK);
	mStorage[StorageIndex(index)] += 1 << StorageShift(index);
}

template <u32 numberOfValues, u32 bitsPerValue>
inline void cBitField<numberOfValues, bitsPerValue>::Dec( u32 index )
{
	ASSERT( Get(index) > 0);
	mStorage[StorageIndex(index)] -= 1 << StorageShift(index);
}

template <u32 numberOfValues, u32 bitsPerValue>
inline void cBitField<numberOfValues, bitsPerValue>::Accumulate( const cBitField &other )
{
	for ( int i = 0; i < STORAGE_REQUIRED; ++i )
	{
		mStorage[i] += other.mStorage[i];
	}
}

template <u32 numberOfValues, u32 bitsPerValue>
inline void cBitField<numberOfValues, bitsPerValue>::Set( const cBitField &other )
{
	memcpy(&mStorage, &other, STORAGE_REQUIRED * STORAGE_BITS / 8);
}



#endif BITFIELD_H