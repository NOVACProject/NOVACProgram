// -*- mode: c++; c-basic-offset: 4; -*-
//
// BSpline.cxx: implementation of the BSplineBase class.
//
//////////////////////////////////////////////////////////////////////
/*
 * Copyright (c) 1998,1999
 * University Corporation for Atmospheric Research, UCAR
 *
 * Permission to use, copy, modify, distribute and sell this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation.  UCAR makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 * 
 * Note from the author:
 *
 * Where possible, you are encouraged to follow the GNU General Public
 * License, or at least the spirit of the license, for the distribution and
 * licensing of this software and any derived works.  See
 * http://www.gnu.org/copyleft/gpl.html.
 */

/*
 * If we're on WIN32 and the DLL macro has not been set yet, which
 * implies the interface file (BSpline.h) has not been included yet,
 * assume only the implementation is wanted and empty the DLL macro
 * before including the interface.
 */
#pragma warning(push)
#pragma warning(disable:4244)

#if WIN32
# ifndef BSPLINE_DLL_
#  define BSPLINE_DLL_
# endif
#endif /* WIN32 */

#include "BSpline.h"
#include "BandedMatrix.h"

#include <vector>
#include <algorithm>
#include <iterator>
#include <iostream>

#include <assert.h>

template <class T>
class Matrix : public BandedMatrix<T>
{
public:
    Matrix &operator += (const Matrix &B)
    {
		Matrix &A = *this;
        Matrix::size_type M = A.num_rows();
		Matrix::size_type N = A.num_cols();
		
		assert(M==B.num_rows());
		assert(N==B.num_cols());
		
		Matrix::size_type i,j;
		for (i=0; i<M; i++)
			for (j=0; j<N; j++)
				A[i][j] += B[i][j];
			return A;
    }
	
    inline Matrix & operator= (const Matrix &b) 
    {
		return Copy (*this, b);
    }
	
    inline Matrix & operator= (const T &e)
    {
		BandedMatrix<T>::operator= (e);
		return *this;
    }
	
};


// Our private state structure, which hides our use of some matrix
// template classes.

template <class T>
struct BSplineBaseP 
{
    typedef Matrix<T> MatrixT;
    
    MatrixT Q;				// Holds P+Q and its factorization
    std::vector<T> X;
    std::vector<T> Nodes;
};

// For now, hardcoding type 1 boundary conditions, 
// which constrains the derivative to zero at the endpoints.
template <class T>
const double BSplineBase<T>::BoundaryConditions[3][4] =
{ 
    //	0		1		M-1		M
    {	-4,		-1,		-1,		-4 },
    {	0,		1,		1,		0 },
    {	2,		-1,		-1,		2 }
};

template <class T>
inline bool BSplineBase<T>::Debug (int on)
{
    static bool debug = false;
    if (on >= 0)
		debug = (on > 0);
    return debug;
}

template <class T>
const double BSplineBase<T>::PI = 3.1415927;

template <class T>
const char* BSplineBase<T>::ImplVersion()
{
    return "$Id: BSpline.cxx,v 1.26 1999/05/06 19:43:00 granger Exp $";
}

template <class T>
const char* BSplineBase<T>::IfaceVersion()
{
    return _BSPLINEBASE_IFACE_ID;
}

	
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

template <class T>
BSplineBase<T>::~BSplineBase()
{
    delete base;
}


// This is a member-wise copy except for replacing our
// private base structure with the source's, rather than just copying
// the pointer.  But we use the compiler's default copy constructor for
// constructing our BSplineBaseP.
template <class T>
BSplineBase<T>::BSplineBase (const BSplineBase<T> &bb) : 
    K(bb.K), BC(bb.BC), OK(bb.OK), base(new BSplineBaseP<T>(*bb.base))
{
    xmin = bb.xmin;
    xmax = bb.xmax;
    alpha = bb.alpha;
    waveLength = bb.waveLength;
    DX = bb.DX;
    M = bb.M;
    NX = base->X.size();
}


template <class T>
BSplineBase<T>::BSplineBase (const T *x, int nx, double wl, int bc) : 
    K(2), OK(false), base(new BSplineBaseP<T>)
{
    setDomain (x, nx, wl, bc);
}


// Methods

template <class T>
bool BSplineBase<T>::setDomain (const T *x, int nx, double wl, int bc)
{
    if (nx <= 0 || x == 0 || wl < 0 || bc < 0 || bc > 2)
    {
		return false;
    }
    OK = false;
    waveLength = wl;
    BC = bc;
	
    // Copy the x array into our storage.
    base->X.resize (nx);
    std::copy (x, x+nx, base->X.begin());
    NX = base->X.size();
	
    // The Setup() method determines the number and size of node intervals.
    if (Setup())
    {
		if (Debug()) 
		{
			std::cerr << "Using M node intervals: " << M << " of length DX: "
				<< DX << std::endl;
			std::cerr << "X min: " << xmin << " ; X max: " << xmax 
				<< std::endl;
			std::cerr << "Data points per interval: " << (float)NX/(float)M 
				<< std::endl;
			std::cerr << "Derivative constraint degree: " << K << std::endl;
		}
		
		// Now we can calculate alpha and our Q matrix
		alpha = Alpha (waveLength);
		if (Debug())
		{
			std::cerr << "Cutoff wavelength: " << waveLength << " ; "
				<< "Alpha: " << alpha << std::endl;
			std::cerr << "Calculating Q..." << std::endl;
		}
		calculateQ ();
		if (Debug() && M < 30)
		{
			std::cerr.fill(' ');
			std::cerr.precision(2);
			std::cerr.width(5);
			std::cerr << base->Q << std::endl;
		}
		
		if (Debug()) std::cerr << "Calculating P..." << std::endl;
		addP ();
		if (Debug())
		{
			std::cerr << "Done." << std::endl;
			if (M < 30)
			{
				std::cerr << "Array Q after addition of P." << std::endl;
				std::cerr << base->Q;
			}
		}
		
		// Now perform the LU factorization on Q
		if (Debug()) std::cerr << "Beginning LU factoring of P+Q..." 
			<< std::endl;
		if (! factor ())
		{
			if (Debug()) std::cerr << "Factoring failed." << std::endl;
		}
		else
		{
			if (Debug()) std::cerr << "Done." << std::endl;
			OK = true;
		}
    }
    return OK;
}

/*
 * Calculate the alpha parameter given a wavelength.
 */
template <class T>
double BSplineBase<T>::Alpha (double wl)
{
    // K is the degree of the derivative constraint: 1, 2, or 3
    double a = (double) (wl / (2 * PI * DX));
    a *= a;			// a^2
    if (K == 2)
		a = a * a;		// a^4
    else if (K == 3)
		a = a * a * a;		// a^6
    return a;
}

/*
 * Return the correct beta value given the node index.  The value depends
 * on the node index and the current boundary condition type.
 */
template <class T>
inline double BSplineBase<T>::Beta (int m)
{
    if (m > 1 && m < M-1)
		return 0.0;
    if (m >= M-1)
		m -= M-3;
    assert (0 <= BC && BC <= 2);
    assert (0 <= m && m <= 3);
    return BoundaryConditions[BC][m];
}

/*
 * Given an array of y data points defined over the domain
 * of x data points in this BSplineBase, create a BSpline
 * object which contains the smoothed curve for the y array.
 */
template <class T>
BSpline<T>* BSplineBase<T>::apply (const T *y)
{
    return new BSpline<T> (*this, y);
}

/*
 * Evaluate the closed basis function at node m for value x,
 * using the parameters for the current boundary conditions.
 */
template <class T>
double BSplineBase<T>::Basis (int m, T x)
{
    double y = 0;
    double xm = xmin + (m * DX);
    double z = fabs((double)(x - xm) / (double)DX);
    if (z < 2.0)
    {
		z = 2 - z;
		y = 0.25 * (z*z*z);
		z -= 1.0;
		if (z > 0)
			y -= (z*z*z);
    }
	
    // Boundary conditions, if any, are an additional addend.
    if (m == 0 || m == 1)
		y += Beta(m) * Basis (-1, x);
    else if (m == M-1 || m == M)
		y += Beta(m) * Basis (M+1, x);
	
    return y;
}

/*
 * Evaluate the deriviative of the closed basis function at node m for
 * value x, using the parameters for the current boundary conditions.
 */
template <class T>
double BSplineBase<T>::DBasis (int m, T x)
{
    double dy = 0;
    double xm = xmin + (m * DX);
    double delta = (double)(x - xm) / (double)DX;
    double z = fabs(delta);
    if (z < 2.0)
    {
		z = 2.0 - z;
		dy = 0.25 * z * z;
		z -= 1.0;
		
		if (z > 0)
		{
			dy -= z * z;
		}
		dy *= ((delta > 0) ? -1.0 : 1.0) * 3.0 / DX;
    }
	
    // Boundary conditions, if any, are an additional addend.
    if (m == 0 || m == 1)
		dy += Beta(m) * DBasis (-1, x);
    else if (m == M-1 || m == M)
		dy += Beta(m) * DBasis (M+1, x);
	
    return dy;
}

/*
 * Return the integral of the product of the basis function derivative
 * restricted to the node domain, 0 to M.
 */
template <class T>
double BSplineBase<T>::qDelta (int m1, int m2)
{
    // At present Q is hardcoded for the first derivative
    // filter constraint and the type 1 boundary constraint.
	
    // These are the products of the first derivative of the
    // normalized basis functions
    // given a distance m nodes apart, qparts[m], 0 <= m <= 3
    // Each column is the integral over each unit domain, -2 to 2
    static const double qparts[3][4][4] = 
    {
		{
			{ 0.11250,   0.63750,   0.63750,   0.11250 },
			{ 0.00000,   0.13125,  -0.54375,   0.13125 },
			{ 0.00000,   0.00000,  -0.22500,  -0.22500 },
			{ 0.00000,   0.00000,   0.00000,  -0.01875 }
		},
		{
			{ 0.75000,   2.25000,   2.25000,   0.75000 },
			{ 0.00000,  -1.12500,  -1.12500,  -1.12500 },
			{ 0.00000,   0.00000,   0.00000,   0.00000 },
			{ 0.00000,   0.00000,   0.00000,   0.37500 }
		},
		{
			{ 2.25000,  20.25000,  20.25000,   2.25000 },
			{ 0.00000,  -6.75000, -20.25000,  -6.75000 },
			{ 0.00000,   0.00000,   6.75000,   6.75000 },
			{ 0.00000,   0.00000,   0.00000,  -2.25000 }
		}
    };
	
    if (m1 > m2)
		std::swap (m1, m2);
	
    if (m2 - m1 > 3)
		return 0.0;
	
    double q = 0;
    for (int m = max (m1-2,0); m < min (m1+2, M); ++m)
		q += qparts[K-1][m2-m1][m-m1+2];
    return q * alpha;
}

template <class T>
void BSplineBase<T>::calculateQ ()
{
    Matrix<T> &Q = base->Q;
    Q.setup (M+1, 3);
    Q = 0;
    if (alpha == 0)
		return;

    // First fill in the q values without the boundary constraints.
    int i, j;
    for (i = 0; i <= M; ++i)
    {
		Q[i][i] = qDelta(i,i);
		for (j = 1; j < 4 && i+j <= M; ++j)
		{
			Q[i][i+j] = Q[i+j][i] = qDelta (i, i+j);
		}
    }

    // Now add the boundary constraints:
    // First the upper left corner.
    // float b1, b2, q;
    double b1, b2, q; //UPD010818 Stefan: I think its better to use the precision of the base data
    for (i = 0; i <= 1; ++i)
    {
		b1 = Beta(i);
		for (j = i; j < i+4; ++j)
		{
			b2 = Beta(j);
			assert (j-i >= 0 && j - i < 4);
			q = 0.0;
			if (i+1 < 4)
				q += b2*qDelta(-1,i);
			if (j+1 < 4)
				q += b1*qDelta(-1,j);
			q += b1*b2*qDelta(-1,-1);
			Q[j][i] = (Q[i][j] += q);
		}
    }
	
    // Then the lower right
	const int iM = M - 1;
    for(i = iM; i <= M; ++i)
    {
		b1 = Beta(i);
		for (j = i - 3; j <= i; ++j)
		{
			b2 = Beta(j);
			q = 0.0;
			if (M+1-i < 4)
				q += b2*qDelta(i,M+1);
			if (M+1-j < 4)
				q += b1*qDelta(j,M+1);
			q += b1*b2*qDelta(M+1,M+1);
			Q[j][i] = (Q[i][j] += q);
		}
    }
}

template <class T>
void BSplineBase<T>::addP ()
{
    // Add directly to Q's elements
    Matrix<T> &P = base->Q;
    std::vector<T> &X = base->X;
	
    // For each data point, sum the product of the nearest, non-zero Basis
    // nodes
    int mx, m, n, i;
    for (i = 0; i < NX; ++i)
    {
		// Which node does this put us in?
		T &x = X[i];
		mx = (int)((x - xmin) / DX);
		
		// Loop over the upper triangle of nonzero basis functions,
		// and add in the products on each side of the diagonal.
		for (m = max(0, mx-1); m <= min(M, mx+2); ++m)
		{
			//float pn;
			//float pm = Basis (m, x);
			//float sum = pm * pm;
			// UPD010818 Stefan: I think its better to use a higher precission
			double pn;
			double pm = Basis (m, x);
			double sum = pm * pm;
			P[m][m] += sum;
			for (n = m+1; n <= min(M, mx+2); ++n)
			{
				pn = Basis (n, x);
				sum = pm * pn;
				P[m][n] += sum;
				P[n][m] += sum;
			}
		}
    }
}

template <class T>
bool BSplineBase<T>::factor ()
{	
    Matrix<T> &LU = base->Q;
	
    if (LU_factor_banded (LU, 3) != 0)
    {
        if (Debug()) std::cerr << "LU_factor_banded() failed." << std::endl;
		return false;
    }
    if (Debug() && M < 30)
		std::cerr << "LU decomposition: " << std::endl << LU << std::endl;
    return true;
}

template <class T>
inline int BSplineBase<T>::Ratio (int &ni, double &deltax, double &ratiof, double *ratiod)
{
    deltax = (xmax - xmin) / ni;
    ratiof = waveLength / deltax;
    double rd = (double) NX / (double) (ni + 1);
    if (ratiod)
		*ratiod = rd;
    return rd >= 1.0;
}

/*
 * Return zero if this fails, non-zero otherwise.
 */
template <class T>
bool BSplineBase<T>::Setup()
{
    std::vector<T> &X = base->X;
	
    // Find the min and max of the x domain
    xmin = X[0];
    xmax = X[0];
	
    int i;
    for (i = 1; i < NX; ++i)
    {
		if (X[i] < xmin)
			xmin = X[i];
		else if (X[i] > xmax)
			xmax = X[i];
    }
	
    if (waveLength > xmax - xmin)
    {
		return false;
    }
	
    int ni = 9;		// Number of node intervals (NX - 1)
    double deltax;
	
    if (waveLength == 0)	// Allows turning off frequency constraint
    {
		ni = NX;
		// UPD001211 Stefan: I'm not sure wheter we have NX intervals or NX - 1 intervals?? But i think its better to use NX - 1
		//deltax = (xmax - xmin) / (double)(NX); 
		deltax = (xmax - xmin) / (double)(NX - 1); 
    }
    else
    {
		// Minimum acceptable number of node intervals per cutoff wavelength.
		static const double fmin = 2.0;
		
		double ratiof;	// Nodes per wavelength for current deltax
		double ratiod;	// Points per node interval
		
		do {
			if (! Ratio (++ni, deltax, ratiof))
				return false;
		}
		while (ratiof < fmin);
		
		// Tweak the estimates obtained above
		do {
			if (! Ratio (++ni, deltax, ratiof, &ratiod) || 
				ratiof > 15.0)
			{
				Ratio (--ni, deltax, ratiof);
				break;
			}
		}
		while (ratiof < 4 || ratiod > 2.0);
    }
	
    // Store the calculations in our state
    M = ni;
    DX = deltax;
	
    return true;
}

template <class T>
const T* BSplineBase<T>::nodes (int *nn)
{
    if (base->Nodes.size() == 0)
    {
		base->Nodes.reserve (M+1);
		for (int i = 0; i <= M; ++i)
		{
			base->Nodes.push_back (xmin + (i * DX));
		}
    }
	
    if (nn)
		*nn = base->Nodes.size();
	
    assert (base->Nodes.size() == (unsigned)(M+1));
    return base->Nodes.begin();
}

template <class T> std::ostream &operator<< (std::ostream &out, const std::vector<T> &c)
{
    for (std::vector<T>::const_iterator it = c.begin(); it < c.end(); ++it)
		out << *it << ", ";
    out << std::endl;
    return out;
}

//////////////////////////////////////////////////////////////////////
// BSpline Class
//////////////////////////////////////////////////////////////////////

template <class T>
struct BSplineP
{
    std::vector<T> spline;
    std::vector<T> A;
};


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


/*
 * This BSpline constructor constructs and sets up a new base and 
 * solves for the spline curve coeffiecients all at once.
 */
template <class T>
BSpline<T>::BSpline (const T *x, int nx, const T *y, double wl, int bc_type) :
    BSplineBase<T>(x, nx, wl, bc_type), s(new BSplineP<T>)
{
    solve (y);
}

/*
 * Create a new spline given a BSplineBase.
 */
template <class T>
BSpline<T>::BSpline (BSplineBase<T> &bb, const T *y) :
    BSplineBase<T>(bb), s(new BSplineP<T>)
{
    solve (y);
}

/*
 * (Re)calculate the spline for the given set of y values.
 */
template <class T>
bool BSpline<T>::solve (const T *y)
{
    if (! OK)
		return false;
	
    // Any previously calculated curve is now invalid.
    s->spline.clear ();
    OK = false;
	
    // Given an array of data points over x and its precalculated
    // P+Q matrix, calculate the b vector and solve for the coefficients.
    std::vector<T> &B = s->A;
    std::vector<T> &A = s->A;
    A.clear ();
    A.resize (M+1);
	
    if (Debug()) std::cerr << "Solving for B..." << std::endl;
	
    // Find the mean of these data
    mean = 0.0;
    int i;
    for (i = 0; i < NX; ++i)
    {
		mean += y[i];
    }
    mean = mean / (double)NX;
    if (Debug())
		std::cerr << "Mean for y: " << mean << std::endl;
	
    int mx, m, j;
    for (j = 0; j < NX; ++j)
    {
		// Which node does this put us in?
		T &xj = base->X[j];
		T yj = y[j] - mean;
		mx = (int)((xj - xmin) / DX);
		
		for (m = max(0,mx-1); m <= min(mx+2,M); ++m)
		{
			B[m] += yj * Basis (m, xj);
		}
    }
	
    if (Debug() && M < 30)
    {
		std::cerr << "Solution a for (P+Q)a = b" << std::endl;
		std::cerr << " b: " << B << std::endl;
    }
	
    // Now solve for the A vector in place.
    if (LU_solve_banded (base->Q, A, 3) != 0)
    {
		if (Debug())
			std::cerr << "LU_solve_banded() failed." << std::endl;
    }
    else
    {
		OK = true;
		if (Debug()) std::cerr << "Done." << std::endl;
		if (Debug() && M < 30)
		{
			std::cerr << " a: " << A << std::endl;
			std::cerr << "LU factor of (P+Q) = " << std::endl 
				<< base->Q << std::endl;
		}
    }
    return OK;
}

template <class T>
BSpline<T>::~BSpline()
{
    delete s;
}

template <class T>
T BSpline<T>::coefficient (int n)
{
    if (OK)
		if (0 <= n && n <= M)
			return s->A[n];
	return 0;
}

template <class T>
T BSpline<T>::evaluate (T x)
{
    double y = 0;
    if (OK)
    {
		int n = (int)((x - xmin)/DX);
		for (int i = max(0,n-1); i <= min(M,n+2); ++i)
		{
			y += s->A[i] * Basis (i, x);
		}
		y += mean;
    }
    return y;
}

template <class T>
T BSpline<T>::slope (T x)
{
    T dy = 0;
    if (OK)
    {
		int n = (int)((x - xmin)/DX);
		for (int i = max(0,n-1); i <= min(M,n+2); ++i)
		{
			dy += s->A[i] * DBasis (i, x);
		}
    }
    return dy;
}

template <class T>
const T *BSpline<T>::curve (int *nx)
{
    if (! OK)
		return 0;
	
    // If we already have the curve calculated, don't do it again.
    std::vector<T> &spline = s->spline;
    if (spline.size() == 0)
    {
		spline.reserve (M+1);
		for (int n = 0; n <= M; ++n)
		{
			T x = xmin + (n * DX);
			spline.push_back (evaluate (x));
		}
    }
	
    if (nx)
		*nx = spline.size();
    return spline.begin();
}

#pragma warning(pop)
