#ifndef _SINGLETON_H
#define _SINGLETON_H


template<class T>
class cSingleton
{
protected:
	cSingleton(void) { }
	~cSingleton(void) { }
	cSingleton( const cSingleton& rhs );
	cSingleton& operator=( const cSingleton& rhs);
public:
	static T& Instance() 
	{
		static T m_Instance;
		return m_Instance;
	}
};

#endif // _SINGLETON_H