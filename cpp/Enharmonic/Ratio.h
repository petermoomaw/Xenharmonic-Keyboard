#pragma once
#include "Common.h"
#include <vector>

#include <eigen/Eigen/Dense>
using  Eigen::VectorXd;
using  Eigen::Matrix;
using  Eigen::Index;
//using namespace Eigen;

//typedef long long  Int;
typedef long   Int;

typedef Matrix<Int, Eigen::Dynamic, Eigen::Dynamic> MatrixXI;
typedef Matrix<Int, Eigen::Dynamic, 1> VectorXI;
typedef Matrix<Int, 1, Eigen::Dynamic> RowVectorXI;


using namespace std;


class Ratio
{
public:
	Ratio();
	Ratio(Int  num);
	Ratio(Int  num, Int  denom);
	Ratio(double x, Int maxDenominator);

	int sign();
  Ratio inverse();
	void operator+=(const Ratio &R);
	void operator+=(const Int  &n);
	void Ratio::operator-=(const Ratio &r);
	void Ratio::operator-=(const Int  &n);
	void Ratio::operator*=(const Ratio &r);
	void Ratio::operator*=(const Int  &n);
	void Ratio::operator/=(const Ratio &r);
	void Ratio::operator/=(const Int  &n);

	void collectPrimes(vector<Int>& primes);
	static void collectPrimes(Int n, vector<Int>& primes);
	void primeFactors(Index col, MatrixXI& P, vector<Int>& primes);
	static void primeFactors(Index col, Int n, MatrixXI& P, vector<Int>& primes);    // nevative interges how powers subtracted.

	static Int  gcd(Int  m, Int  n);
	static Int  lcm(Int a, Int b);
	void reduce();

	Int num;
	Int denom;
};

bool operator==(const Ratio& r1, const Ratio& r2);
bool operator==(const Int & n, const Ratio& r);
bool operator==(const Ratio& r, const Int & n);

bool operator!=(const Ratio& r1, const Ratio& r2);
bool operator!=(const Int & n, const Ratio& r);
bool operator!=(const Ratio& r, const Int & n);

bool operator>(const Ratio& r1, const Ratio& r2);
bool operator>(const Int & n, const Ratio& r);
bool operator>(const Ratio& r, const Int & n);

bool operator<(const Ratio& r1, const Ratio& r2);
bool operator<(const Int & n, const Ratio& r);
bool operator<(const Ratio& r, const Int & n);

Ratio operator+(const Ratio& r1, const Ratio& r2);
Ratio operator+(const Int & n, const Ratio& r);
Ratio operator+(const Ratio& r, const Int & n);

Ratio operator-(const Ratio& r1, const Ratio& r2);
Ratio operator-(const Int & n, const Ratio& r);
Ratio operator-(const Ratio& r, const Int & n);

Ratio operator*(const Ratio& r1, const Ratio& r2);
Ratio operator*(const Int & n, const Ratio& r);
Ratio operator*(const Ratio& r, const Int & n);

Ratio operator/(const Ratio& r1, const Ratio& r2);
Ratio operator/(const Int & n, const Ratio& r);
Ratio operator/(const Ratio& r, const Int & n);