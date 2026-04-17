/**
 * @file Matrix.h
 * @brief Interface for the CMatrix template class — simple 3D array.
 */

#if !defined(AFX_MATRIX_H__E6397425_F122_44A4_A3A8_53CD4D2523E7__INCLUDED_)
#define AFX_MATRIX_H__E6397425_F122_44A4_A3A8_53CD4D2523E7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning(disable: 4786)	// Disable warning about long names

#include <vector>

#ifdef WIN_DEBUG
#define allocator A
#endif

/**
 * @brief Simple 3-dimensional dynamic array with zero-based indexing.
 *
 * CMatrix wraps a nested @c std::vector as a three-dimensional container
 * with operator() access. Used internally for multi-dimensional data
 * tables in the model-data and event layers. The template parameter T
 * should be a numeric type.
 *
 * @tparam T Element type (must be default-constructible and assignable).
 */
template <class T>
class CMatrix
{
public:
	/**
	 * @brief Get the size of dimension @p a (1, 2, or 3).
	 */
	int dim(int a);

	/**
	 * @brief Construct with specified dimensions.
	 * @param[in] a Size of dimension 1.
	 * @param[in] b Size of dimension 2.
	 * @param[in] c Size of dimension 3.
	 */
	CMatrix(int a, int b, int c);

	/// @brief Default-construct an empty matrix.
	CMatrix();
	virtual ~CMatrix();

	/// @brief Return true if (a, b, c) is within bounds.
	bool checkBounds(int a, int b, int c);

	/// @brief Grow the matrix to accommodate index (a, b, c).
	void add(int a, int b, int c);

	/// @brief Allocate storage for dimensions (a, b, c) and zero-fill.
	void init(int a, int b, int c);

	/// @brief Clear all elements.
	void clear();

	/// @brief Read-only element access by (x, y, z).
	T operator()(const int, const int, const int) const;

	/// @brief Read-write element access by (x, y, z).
	T& operator()(const int, const int, const int);

private:
	int d1;  ///< Size of dimension 1.
	int d2;  ///< Size of dimension 2.
	int d3;  ///< Size of dimension 3.
	typedef std::vector< std::vector<T> > M2D;
	const int base;	///< Index base (always 0).

	std::vector< M2D > m;  ///< Nested storage.
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

	std::vector<T> v3;
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
	std::vector<T> v3;
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
