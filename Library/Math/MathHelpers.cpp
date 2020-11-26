



bool IsPow2(int i)
{
	// & the negative 2's complement number with the passed param
	// a power of 2 number will slip through, nice :) 
	return ( (i & -i) == i );
}// END IsPow2
