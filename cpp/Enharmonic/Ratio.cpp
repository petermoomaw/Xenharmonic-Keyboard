
#include "pch.h"
#include "Ratio.h"




Ratio Ratio::inverse()
{
  Ratio r;
  r.num = this->denom;
  r.denom = this->num;
  return r;
}

bool operator==(const Ratio& r1, const Ratio& r2)
{
	return r1.num*r2.denom == r2.num*r1.denom;
}

bool operator==(const Int &n, const Ratio& r)
{
	return n*r.denom == r.num;
}

bool operator==(const Ratio& r, const Int & n)
{
	return n*r.denom == r.num;
}

bool operator!=(const Ratio& r1, const Ratio& r2)
{
	return r1.num*r2.denom != r2.num*r1.denom;
}

bool operator!=(const Int & n, const Ratio& r)
{
	return n*r.denom != r.num;
}

bool operator!=(const Ratio& r, const Int & n)
{
	return n*r.denom != r.num;
}

bool operator>(const Ratio& r1, const Ratio& r2)
{
	return r1.num*r2.denom > r2.num*r1.denom;
}

bool operator>(const Int & n, const Ratio& r)
{
	return n*r.denom > r.num;
}

bool operator>(const Ratio& r, const Int & n)
{
	return  r.num > n*r.denom;
}

bool operator<(const Ratio& r1, const Ratio& r2)
{
	return r1.num*r2.denom < r2.num*r1.denom;
}

bool operator<(const Int & n, const Ratio& r)
{
	return n*r.denom < r.num;
}

bool operator<(const Ratio& r, const Int & n)
{
	return r.num < n*r.denom;
}

Ratio operator+(const Ratio& r1, const Ratio& r2)
{
	Ratio result;
	result.denom = Ratio::lcm(r1.denom, r2.denom);
	result.num = r1.num * result.denom/r1.denom + r2.num * result.denom/r2.denom;
	result.reduce();
	return result;
}

Ratio operator+(const Int & n, const Ratio& r)
{
	Ratio result;
	result.num = r.num + n*r.denom;
	result.denom = r.denom;
	result.reduce();
	return result;
}

Ratio operator+(const Ratio& r, const Int & n)
{
	Ratio result;
	result.num = r.num + n*r.denom;
	result.denom = r.denom;
	result.reduce();
	return result;
}

Ratio operator-(const Ratio& r1, const Ratio& r2)
{
	Ratio result;
	result.denom = Ratio::lcm(r1.denom, r2.denom);
	result.num = r1.num * result.denom / r1.denom - r2.num * result.denom / r2.denom;
	result.reduce();
	return result;
}

Ratio operator-(const Int & n, const Ratio& r)
{
	Ratio result;
	result.num = n*r.denom - r.num;
	result.denom = r.denom;
	result.reduce();
	return result;
}

Ratio operator-(const Ratio& r, const Int & n)
{
	Ratio result;
	result.num = r.num - n*r.denom;
	result.denom = r.denom;
	result.reduce();
	return result;
}

Ratio operator*(const Ratio& r1, const Ratio& r2)
{
	Ratio result;
	Ratio temp;

	result.num = r1.num;
	result.denom = r2.denom;
	result.reduce();

	temp.num = r2.num;
	temp.denom = r1.denom;
	temp.reduce();

	result.num *= temp.num;
	result.denom *= temp.denom;
	return result;
}

Ratio operator*(const Int & n, const Ratio& r)
{
	Ratio result;
	result.num = n;
	result.denom = r.denom;
	result.reduce();
	result.num *= r.num;
	return result;
}

Ratio operator*(const Ratio& r, const Int & n)
{
	Ratio result;
	result.num = n;
	result.denom = r.denom;
	result.reduce();
	result.num *= r.num;
	return result;
}

Ratio operator/(const Ratio& r1, const Ratio& r2)
{
	Ratio result;
	Ratio temp;

	result.num = r1.num;
	result.denom = r1.denom;
	result.reduce();

	temp.num = r2.denom;
	temp.denom = r2.num;
	temp.reduce();

	result.num *= temp.num;
	result.denom *= temp.denom;
  result.reduce();
	return result;
}

Ratio operator/(const Int & n, const Ratio& r)
{
	Ratio result;
	result.num = n;
	result.denom = r.num;
	result.reduce();
	result.num *= r.denom;
	return result;
}

Ratio operator/(const Ratio& r, const Int & n)
{
	Ratio result;
	result.num = r.num;
	result.denom = n;
	result.reduce();
	result.denom *= r.denom;
	return result;
}

//////////////////////////////////////////////////////////////////


Ratio::Ratio()
{
	this->num = 1;
	this->denom = 1;
}

Ratio::Ratio(Int  num)
{
	this->num = num;
	this->denom = 1;
}

Ratio::Ratio(Int  num, Int  denom)
{
	this->num = num;
	this->denom = denom;
	this->reduce();
}

Ratio::Ratio(double x, Int  maxDenominator)
{
	int m = (int)x;
	x = x - m;
	Int  N = maxDenominator;
	Int  a = 0;
	Int  b = 1;
	Int  c = 1;
	Int  d = 1;

	while (b <= N && d <= N)
	{
		double mediant = ((double)(a + c)) / (b + d);
		if (x == mediant)
		{
			if (b + d <= N)
			{
				this->num = a + c;
				this->denom = b + d;
			}
			else if (d > b)
			{
				this->num = c;
				this->denom = d;
			}
			else
			{
				this->num = a;
				this->denom = b;
			}

			this->num += m * denom;
			this->reduce();
			return;
		}
		else if (x > mediant)
		{
			a = a + c;
			b = b + d;
		}
		else
		{
			c = a + c;
			d = b + d;
		}
	}

	if (b > N)
	{
		this->num = c;
		this->denom = d;
	}
	else
	{
		this->num = a;
		this->denom = b;
	}

	this->num += m * denom;
	this->reduce();
}

void Ratio::operator+=(const Ratio &r)
{
	Int newDenom = Ratio::lcm(this->denom, r.denom);
	this->num = this->num * newDenom / this->denom + r.num * newDenom / r.denom;
	this->denom = newDenom;
	this->reduce();
}

void Ratio::operator+=(const Int  &n)
{
	this->num = this->num + this->denom*n;
	this->reduce();
}

void Ratio::operator-=(const Ratio &r)
{
	Int newDenom = Ratio::lcm(this->denom, r.denom);
	this->num = this->num * newDenom / this->denom - r.num * newDenom / r.denom;
	this->denom = newDenom;
	this->reduce();
}

void Ratio::operator-=(const Int  &n)
{
	this->num = this->num - this->denom*n;
	this->reduce();
}

void Ratio::operator*=(const Ratio &r)
{
	Ratio temp;
	temp.num = r.num;
	temp.denom = this->denom;
	temp.reduce();

	this->denom = r.denom;
	this->reduce();

	this->num *= temp.num;
	this->denom *= temp.denom;
}

void Ratio::operator*=(const Int  &n)
{
	Int temp = this->num;
	this->num = n;
	this->reduce();
	this->num *= temp;
}

void Ratio::operator/=(const Ratio &r)
{
	this->num *= r.denom;
	this->denom *= r.num;
	this->reduce();

/*	Ratio temp;
	temp.num = this->denom;
	temp.denom = r.denom;
	temp.reduce();

	this->denom = r.num;
	this->reduce();

	this->num *= temp.num;
	this->denom *= temp.denom;*/
}

void Ratio::operator/=(const Int  &n)
{
	Int temp = this->denom;
	this->denom = n;
	this->reduce();
	this->denom *= temp;
}

void Ratio::collectPrimes( vector<Int>& primes)
{
	collectPrimes(this->num, primes);
	collectPrimes(this->denom, primes);
}

void Ratio::collectPrimes(Int n, vector<Int>& primes)
{
	Int z = 2;
	while (z * z <= n)
	{
		if (n % z == 0)
		{
			unsigned int i = 0;
			for (; i < primes.size(); i++)
			{
				if (z == primes[i])
					break; // prime is already in list
				else if (z > primes[i])
					continue;
				else
				{
					primes.insert(primes.begin() + i, z);
					break;
				}
			}

			if (i == primes.size())
				primes.push_back(z);

			//PRINT(L"%d\n", z);
			n /= z;
		}
		else
		{
			z++;
		}
	}

	if (n > 1)
	{
		unsigned int i = 0;
		for (; i < primes.size(); i++)
		{
			if (n == primes[i])
				break; // prime is already in list
			else if (n > primes[i])
				continue;
			else
			{
				primes.insert(primes.begin() + i, n);
				break;
			}
		}

		if (i == primes.size())
			primes.push_back(n);
	}
}

void Ratio::primeFactors(Index col, MatrixXI& P, vector<Int>& primes)
{
	primeFactors(col, this->num, P, primes);
	primeFactors(col, -this->denom, P, primes);
}

// nevative interges have powers subtracted.
void Ratio::primeFactors(Index col, Int n, MatrixXI& P, vector<Int>& primes)
{
	int inc = 1;
	if (n < 0)
	{
		n = -n;
		inc = -1;
	}

	Int z = 2;
	while (z * z <= n)
	{
		if (n % z == 0)
		{
			for (Index row = 0; row < primes.size(); row++)
			{
				if (primes[row] == z)
				{
					P(row,col) += inc;
					break;
				}
			}

			//PRINT(L"%d\n", z);
			n /= z;
		}
		else
		{
			z++;
		}
	}

	if (n > 1)
	{
		for (Index row = 0; row < primes.size(); row++)
		{
			if (primes[row] == n)
			{
				P(row,col)+= inc;
				break;
			}
		}
	}
}

//Int  Ratio::gcd(Int m, Int  n)     	// function definition
//{                         	// block begin
//	Int  r;                	// declaration of remainder
//
//	m = m < 0 ? -m : m;
//	n = n < 0 ? -n : n;
//
//	while (n != 0) {       	// not equal
//		r = m % n;          	// modulus operator
//		m = n;              	// assignment
//		n = r;
//	}                      	// end while loop
//	return m;              	// exit gcd with value m
//}

Int  Ratio::gcd(Int a, Int  b) 
{                         	
	a = a < 0 ? -a : a;
	b = b < 0 ? -b : b;

	for (;;)
	{
		if (a == 0) return b;
		b %= a;
		if (b == 0) return a;
		a %= b;
	}
}

Int Ratio::lcm(Int a, Int b)
{
	a = a < 0 ? -a : a;
	b = b < 0 ? -b : b;

	int temp = gcd(a, b);

	return temp ? (a / temp * b) : 0;
}

int Ratio::sign()
{
	if (num == 0)
		return 1;

	int signNum = num > 0 ? 1 : -1;
	int signDenom = denom > 0 ? 1 : -1;
	return signNum*signDenom;
}

void Ratio::reduce()
{
	int s = sign();
	denom = denom > 0 ? denom : -denom;
	num = num > 0 ? num : -num;
	Int g = gcd(num, denom);
	num /= g;
	denom /= g;
	num *= s;

}

//static void factor(Int n)
//{
//	Int z = 2;
//	while (z * z <= n)
//	{
//		if (n % z == 0)
//		{
//			PRINT(L"%d\n", z);
//			n /= z;
//		}
//		else
//		{
//			z++;
//		}
//	}
//	if (n > 1) {
//		PRINT(L"%d\n", n);
//	}
//}
