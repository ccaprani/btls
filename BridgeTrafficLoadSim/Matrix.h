// Matrix.h: interface for the CMatrix class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MATRIX_H__E6397425_F122_44A4_A3A8_53CD4D2523E7__INCLUDED_)
#define AFX_MATRIX_H__E6397425_F122_44A4_A3A8_53CD4D2523E7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning(disable: 4786)	// Disable warning about long names

#include <vector>

#ifdef _DEBUG
#define allocator A
#endif

using namespace std;

template <class T>
class CMatrix
{
public:
	int dim(int a);
	CMatrix(int a, int b, int c);
	CMatrix();
	virtual ~CMatrix();
	bool checkBounds(int a, int b, int c);
	void add(int a, int b, int c);
	void init(int a, int b, int c);
	void clear();

	T operator()(const int, const int, const int) const;
	// Get the value of one element.
	T& operator()(const int, const int, const int);
	// Get/Set the value of one element.

private:
	int d1;
	int d2;
	int d3;
	typedef std::vector< std::vector<T> > M2D;
	const int base;	// defines the base to use: zero or 1

	std::vector< M2D > m;
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

template<class T>  
CMatrix<T>::CMatrix(int a, int b, int c):base(0)	// Zero-based indices
{
	init(a,b,c);
}

template<class T>  
CMatrix<T>::CMatrix():base(0)	// Zero-based indices
{

}

template<class T>  
CMatrix<T>::~CMatrix()
{

}

template<class T>  
void
CMatrix<T>::init(int a, int b, int c)
{
	d1 = a;	d2 = b; d3 = c;

	vector<T> v3;
	v3.assign(d3,0);
	M2D v2;	v2.assign(d2,v3);
	m.assign(d1, v2);

}

template<class T>  
T 
CMatrix<T>::operator()(const int x, const int y, const int z) const
{
	if( checkBounds(x, y, z) )
		return m.at(x-base).at(y-base).at(z-base);
}


template<class T>  
T& 
CMatrix<T>::operator()(const int x, const int y, const int z)
{
	if(checkBounds(x, y, z))
		return m.at(x-base).at(y-base).at(z-base);
	else
	{
		add(x, y, z);
		return m.at(x-base).at(y-base).at(z-base);
	}
}

template<class T>  
int 
CMatrix<T>::dim(int a)
{
	if(a == 1)
		return d1;
	else if(a == 2)
		return d2;
	else
		return d3;
}

template<class T>  
void
CMatrix<T>::add(int a, int b, int c)
{
	vector<T> v3;
	v3.assign(d3,0);	
	M2D v2;	v2.assign(d2,v3);
	m.assign(d1, v2);
	
	if(a > d1)
	{
		d1 = a;
		m.resize(d1,v2);
	}
	if(b > d2)
	{
		d2 = b;
		for(int i = 0; i < d1; i++)
			m[i].resize(d2, v3);
	}
	if(c > d3)
	{
		d3 = c;
		for(int i = 0; i < d1; i++)
		{
			for(int j = 0; j < d2; j++)
				m[i][j].resize(d3, 0);
		}
	}
}

template<class T>  
void
CMatrix<T>::clear()
{
	for(int i = 0; i < d1; i++)
	{
		for(int j = 0; j < d2; j++)
			m[i][j].clear();
	}

}

template<class T>  
inline bool 
CMatrix<T>::checkBounds(int a, int b, int c)
{
	if(a-base < d1 || b-base < d2 || c-base < d3)
		return 1;
	else
		return 0;
}


#endif // !defined(AFX_MATRIX_H__E6397425_F122_44A4_A3A8_53CD4D2523E7__INCLUDED_)
