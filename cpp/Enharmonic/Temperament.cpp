#include "pch.h"
#include "Common.h"
#include "Temperament.h"
#include <math.h>

#include <sstream>
#include <iostream>
using std::cout;
using std::endl;

#include <fstream>
using std::ifstream;

#include <cstring>

//#define TENNY_MAX  DBL_MAX
#define TENNY_MAX 100000000000

using namespace Windows::Foundation::Collections;

wstring Temperament::temperamentsDir = _T("../Temperaments");

struct greaterThan
{
	template<class T>
	bool operator()(T const &a, T const &b) const { return a > b; }
};

void Print(Index i, LPCTSTR lpszFormat, ...)
{
	for (Index j = 0; j < i; j++)
		Print(_T("  "));

	va_list args;
	va_start(args, lpszFormat);
	int nBuf;
	TCHAR szBuffer[512 * 4]; // get rid of this hard-coded buffer
	nBuf = _vsntprintf_s(szBuffer, 512 * 4 - 1, lpszFormat, args);
	::OutputDebugString(szBuffer);
	va_end(args);
}


void Print(Index i, MatrixXI m)
{
	for (Index j = 0; j < i; j++)
		Print(_T("  "));

	TCHAR szBuffer[512 * 4]; // get rid of this hard-coded buffer
	for (Index row = 0; row < m.rows(); row++)
	{
		for (Index col = 0; col < m.cols(); col++)
		{
			Print(_T("%d "), m(row, col));
		}
		Print(_T("\n"));
	}
}

void Print(MatrixXI m)
{
	TCHAR szBuffer[512 * 4]; // get rid of this hard-coded buffer
	for (Index row = 0; row < m.rows(); row++)
	{
		for (Index col = 0; col < m.cols(); col++)
		{
			Print(_T("%d "), m(row,col));
		}
		Print(_T("\n"));
	}
}

void Temperament::addToNamingRatioMap(wstring asciiName, Ratio r, wstring sharp, wstring flat)
{
	NamingRatio& n = namingRatioMap[asciiName];
	n.asciiName = asciiName;
	n.ratios.push_back(r);
	n.sharps.push_back(sharp);
	n.flats.push_back(flat);
  n.cents.push_back(1200*(log2((double)r.num/r.denom)));
}

void Temperament::addToNamingRatioMap(wstring asciiName, Ratio r)
{
	NamingRatio& n = namingRatioMap[asciiName];
	n.asciiName = asciiName;
	n.ratios.push_back(r);
  n.cents.push_back(1200 * (log2((double)r.num / r.denom)));
}

Temperament::Temperament()
{
	//Usual Names
	addToNamingRatioMap(L"#", Ratio((Int)3, (Int)1));
	NamingRatio& n = namingRatioMap[L"#"];
//	wstring defaultSharps1[] = { L"\xFD", L"\xFF" };
  wstring defaultSharps1[] = { L"\xFD" };
	n.sharps.assign(defaultSharps1, defaultSharps1 + sizeof(defaultSharps1) / sizeof(defaultSharps1[0]));
//	wstring defaultFlats1[] = { L"\x23", L"\x21" };
  wstring defaultFlats1[] = { L"\x23"};
	n.flats.assign(defaultFlats1, defaultFlats1 + sizeof(defaultFlats1) / sizeof(defaultFlats1[0]));

/////////////////////////// Spartin Set
//  5:7 - kleisma   n
//  |(
  addToNamingRatioMap(L"|(", Ratio((Int)5120, (Int)5103), L"\x92", L"\x8E");

  // 5 comma  /|    p    ~21.506¢
  addToNamingRatioMap(L"/|", Ratio((Int)81, (Int)80), L"\x9A", L"\x86");

  // 7 comma  |)   t
  addToNamingRatioMap(L"|)", Ratio((Int)64, (Int)63), L"\x9C", L"\x84");

  // 25-S-diesis  pp -> ph or f
  addToNamingRatioMap(L"//|", Ratio((Int)6561, (Int)6400), L"\xA4", L"\x7C");

  //35-M-diesis   pat or g
  addToNamingRatioMap(L"/|)", Ratio((Int)36, (Int)35), L"\xA6", L"\x7A");
  //	addToNamingRatioMap(L"/|)", Ratio((Int)1053, (Int)1024));
  //	addToNamingRatioMap(L"/|)", Ratio((Int)250, (Int)243));

  // 11 M-diesis  /|\    pak or v
  addToNamingRatioMap(L"/|\\", Ratio((Int)33, (Int)32), L"\xA8", L"\x78");

  //11 - L - diesis      jat or w
  addToNamingRatioMap(L"(|)", Ratio((Int)729, (Int)704), L"\xAC", L"\x74");
 
  //35-L-diesis   jak or d
  addToNamingRatioMap(L"(|\\", Ratio((Int)8505, (Int)8192), L"\xAE", L"\x72");

///////////////////////////////  Non-spartan single-flag symbols:
  // 19 - schisma      r
  addToNamingRatioMap(L")|", Ratio((Int)513, (Int)512), L"\x91", L"\x8F");

  // 23-comma          z (may be pronounced ts)
  addToNamingRatioMap(L"|~", Ratio((Int)736, (Int)729), L"\x97", L"\x89");

  // 55-comma         k
  addToNamingRatioMap(L"|\\", Ratio((Int)55, (Int)54), L"\x9E", L"\x82");

  // 7:11-comma      j (may be pronounced as in jaw, yaw, haw, or zhaw)
  addToNamingRatioMap(L"(|", Ratio((Int)45927, (Int)45056), L"\x9F", L"\x81");
  
  // 17-kleisma      s
  addToNamingRatioMap(L"~|", Ratio((Int)2187, (Int)2176), L"\x93", L"\x8D");

//////////////////////////////  Remaining athenian symbols:
  // 7:11-kleisma     ran
  addToNamingRatioMap(L")|(", Ratio((Int)896, (Int)891), L"\x94", L"\x8C");

  // 17 - comma  (1/24th tone)   san                          
  addToNamingRatioMap(L"~|(", Ratio((Int)4131, (Int)4096), L"\x96", L"\x8A");
  //	addToNamingRatioMap(L"~|(", Ratio((Int)120, (Int)119));

  // 5:11-S-diesis up    jan
  addToNamingRatioMap(L"(|(", Ratio((Int)45, (Int)44), L"\xA2", L"\x7E");
  //	addToNamingRatioMap(L"(|(", Ratio((Int)1701, (Int)1664), L"\xA2", L"\x7E");  //7:13 S-diesis
  //	addToNamingRatioMap(L"(|(", Ratio((Int)1408, (Int)1377), L"\xA2", L"\x7E"); //1:17 S-diesis


/////////////////////////////   Non-athenian blended symbols:
  // 11:49-comma   ss -> sh     17.576
  addToNamingRatioMap(L"~~|", Ratio((Int)99, (Int)98), L"\x98", L"\x88");

  //11:19-L-diesis   kk -> kh or ch (pronounced tsh)    63.790
  addToNamingRatioMap(L"|\\\\", Ratio((Int)373977, (Int)360448), L"\xAD", L"\x73");

  //143-comma   sr or sl   12.064
  addToNamingRatioMap(L")~|", Ratio((Int)144, (Int)143), L"\x95", L"\x8B");

  //5:19-comma  pr  24.884
  addToNamingRatioMap(L")/|", Ratio((Int)41553, (Int)40960), L"\x9B", L"\x85");

  // 5:13-M-diesis  phr  46.394
  addToNamingRatioMap(L")//|", Ratio((Int)416, (Int)405), L"\xA5", L"\x7B");

  // 5:49-M-diesis  prak or vr  56.482
  addToNamingRatioMap(L")/|\\", Ratio((Int)405, (Int)392), L"\xAA", L"\x76");

/////////////////////////////  Remaining(non - athenian) compound symbols :
  // 19 - comma     raz   20.082
  addToNamingRatioMap(L")|~", Ratio((Int)19683, (Int)19456), L"\x99", L"\x87");

  // 7:19-comma     rat   30.642
  addToNamingRatioMap(L")|)", Ratio((Int)57, (Int)56), L"\x9D", L"\x83");

  // 49-S-diesis    sat   35.697
  addToNamingRatioMap(L"~|)", Ratio((Int)49, (Int)48), L"\xA0", L"\x80");

  //5:23-S-diesis   paz   38.051
  addToNamingRatioMap(L"/|~", Ratio((Int)46, (Int)45), L"\xA1", L"\x7F");

  //23-S-diesis     sak     40.004
  addToNamingRatioMap(L"~|\\", Ratio((Int)16767, (Int)16384), L"\xA3", L"\x7D");

  //11:19-M-diesis  jaz    49.895
  addToNamingRatioMap(L"(|~", Ratio((Int)176, (Int)171), L"\xA7", L"\x79");

  //49-M-diesis   jp (pronounced jap)    54.528
  addToNamingRatioMap(L"(/|", Ratio((Int)4096, (Int)3969), L"\xA9", L"\x77");

  //49-L-diesis   kt (pronounced kat)   59.157
  addToNamingRatioMap(L"|\\)", Ratio((Int)8680203, (Int)8388608), L"\xAB", L"\x75");

  //5:13-L-diesis  rakh or rach   67.291
  addToNamingRatioMap(L")|\\\\", Ratio((Int)885735, (Int)851968), L"\xAF", L"\x71");

 //  ///////////////////////////   Schisma diacritics:
	//// 5 schisma  '|   ~1.954¢
	//// Note that 32768/32805*(81/80)^2 = 128/125 which is a fairly large interval of about 42¢
	//// which exists in mean tone temperaments.
	//// This indicates that simply using symbols willy nilly isn't a good practice.
	//// The issue is that multiplying a small number by a ratio which is tempered out can lead to a 
	//// number.
	//// Perhapse the thing to do is to use the symbol which is closest in cents, or put a threshold on
	//// whether a symbol can be used.
	//// Maybe we can take all candidate names, and then test for which name is the closest.
	//addToNamingRatioMap(L"'|", Ratio((Int)32805, (Int)32768), L"úù", L"&'");

	//Octave
	addToNamingRatioMap(L"2", Ratio((Int)2, (Int)1));



  names12TET.push_back(L"A");
  names12TET.push_back(L"B");
  names12TET.push_back(L"B");
  names12TET.push_back(L"C");
  names12TET.push_back(L"C");
  names12TET.push_back(L"D");
  names12TET.push_back(L"E");
  names12TET.push_back(L"E");
  names12TET.push_back(L"F");
  names12TET.push_back(L"F");
  names12TET.push_back(L"G");
  names12TET.push_back(L"G");

  accidentals12TET.push_back(L"");     // A
  accidentals12TET.push_back(L"\x23"); //Bb
  accidentals12TET.push_back(L"");     //B
  accidentals12TET.push_back(L"");     //C
  accidentals12TET.push_back(L"\xFD"); //C#
  accidentals12TET.push_back(L"");     //D
  accidentals12TET.push_back(L"\x23"); //Eb
  accidentals12TET.push_back(L"");     //E
  accidentals12TET.push_back(L"");     //F
  accidentals12TET.push_back(L"\xFD"); //F#
  accidentals12TET.push_back(L"");     //G
  accidentals12TET.push_back(L"\xFD"); //G#

  //addToNamingRatioMap(L"/|", Ratio((Int)81, (Int)80), L"\x9A", L"\x86");

  names53TET.push_back(L"a");
  accidentals53TET.push_back(L"");
  names53TET.push_back(L"a");
  accidentals53TET.push_back(L"\x9A");
  names53TET.push_back(L"a");
  accidentals53TET.push_back(L"\x9A\x9A");
  names53TET.push_back(L"b");
  accidentals53TET.push_back(L"\x23\x86");
  names53TET.push_back(L"b");
  accidentals53TET.push_back(L"\x23");
  names53TET.push_back(L"a");
  accidentals53TET.push_back(L"\xFD");
  names53TET.push_back(L"a");
  accidentals53TET.push_back(L"\xFD\x9A");
  names53TET.push_back(L"b");
  accidentals53TET.push_back(L"\x86\x86");
  names53TET.push_back(L"b");
  accidentals53TET.push_back(L"\x86");
  names53TET.push_back(L"b");
  accidentals53TET.push_back(L"");
  names53TET.push_back(L"b");
  accidentals53TET.push_back(L"\x9A");
  names53TET.push_back(L"c");
  accidentals53TET.push_back(L"\x86\x86");
  names53TET.push_back(L"c");
  accidentals53TET.push_back(L"\x86");
  names53TET.push_back(L"c");
  accidentals53TET.push_back(L"");
  names53TET.push_back(L"c");
  accidentals53TET.push_back(L"\x9A");
  names53TET.push_back(L"c");
  accidentals53TET.push_back(L"\x9A\x9A");
  names53TET.push_back(L"d");
  accidentals53TET.push_back(L"\x23\x86");
  names53TET.push_back(L"d");
  accidentals53TET.push_back(L"\x23");
  names53TET.push_back(L"c");
  accidentals53TET.push_back(L"\xFD");
  names53TET.push_back(L"c");
  accidentals53TET.push_back(L"\xFD\x9A");
  names53TET.push_back(L"d");
  accidentals53TET.push_back(L"\x86\x86");
  names53TET.push_back(L"d");
  accidentals53TET.push_back(L"\x86");
  names53TET.push_back(L"d");
  accidentals53TET.push_back(L"");
  names53TET.push_back(L"d");
  accidentals53TET.push_back(L"\x9A");
  names53TET.push_back(L"d");
  accidentals53TET.push_back(L"\x9A\x9A");
  names53TET.push_back(L"e");
  accidentals53TET.push_back(L"\x23\x86");
  names53TET.push_back(L"e");
  accidentals53TET.push_back(L"\x23");
  names53TET.push_back(L"d");
  accidentals53TET.push_back(L"\xFD");
  names53TET.push_back(L"d");
  accidentals53TET.push_back(L"\xFD\x9A");
  names53TET.push_back(L"e");
  accidentals53TET.push_back(L"\x86\x86");
  names53TET.push_back(L"e");
  accidentals53TET.push_back(L"\x86");
  names53TET.push_back(L"e");
  accidentals53TET.push_back(L"");
  names53TET.push_back(L"e");
  accidentals53TET.push_back(L"\x9A");
  names53TET.push_back(L"f");
  accidentals53TET.push_back(L"\x86\x86");
  names53TET.push_back(L"f");
  accidentals53TET.push_back(L"\x86");
  names53TET.push_back(L"f");
  accidentals53TET.push_back(L"");
  names53TET.push_back(L"f");
  accidentals53TET.push_back(L"\x9A");
  names53TET.push_back(L"f");
  accidentals53TET.push_back(L"\x9A");
  names53TET.push_back(L"g");
  accidentals53TET.push_back(L"\x23\x86");
  names53TET.push_back(L"g");
  accidentals53TET.push_back(L"\x23");
  names53TET.push_back(L"f");
  accidentals53TET.push_back(L"\xFD");
  names53TET.push_back(L"f");
  accidentals53TET.push_back(L"\xFD\x9A");
  names53TET.push_back(L"g");
  accidentals53TET.push_back(L"\x86\x86");
  names53TET.push_back(L"g");
  accidentals53TET.push_back(L"\x86");
  names53TET.push_back(L"g");
  accidentals53TET.push_back(L"");
  names53TET.push_back(L"g");
  accidentals53TET.push_back(L"\x9A");
  names53TET.push_back(L"g");
  accidentals53TET.push_back(L"\x9A\x9A");
  names53TET.push_back(L"a");
  accidentals53TET.push_back(L"\x23\x86");
  names53TET.push_back(L"a");
  accidentals53TET.push_back(L"\x23");
  names53TET.push_back(L"g");
  accidentals53TET.push_back(L"\xFD");
  names53TET.push_back(L"g");
  accidentals53TET.push_back(L"\xFD\x9A");
  names53TET.push_back(L"a");
  accidentals53TET.push_back(L"\x86\x86");
  names53TET.push_back(L"a");
  accidentals53TET.push_back(L"\x86");


  //names12TET.push_back(L"E");
  //names12TET.push_back(L"B");
  //names12TET.push_back(L"F");
  //names12TET.push_back(L"C");
  //names12TET.push_back(L"G");
  //names12TET.push_back(L"D");
  //names12TET.push_back(L"A");
  //names12TET.push_back(L"E");
  //names12TET.push_back(L"B");
  //names12TET.push_back(L"F");
  //names12TET.push_back(L"C");
  //names12TET.push_back(L"D");

  //accidentals12TET.push_back(L"\x23");     
  //accidentals12TET.push_back(L"\x23"); 
  //accidentals12TET.push_back(L"");     
  //accidentals12TET.push_back(L"");     
  //accidentals12TET.push_back(L""); 
  //accidentals12TET.push_back(L""); 
  //accidentals12TET.push_back(L""); 
  //accidentals12TET.push_back(L"");     
  //accidentals12TET.push_back(L"");     
  //accidentals12TET.push_back(L"\xFD"); 
  //accidentals12TET.push_back(L"\xFD"); 
  //accidentals12TET.push_back(L"\xFD"); 


  accidentals144TET.push_back(&namingRatioMap[L"~|("]);
  accidentals144TET.push_back(&namingRatioMap[L"/|"]);
  accidentals144TET.push_back(&namingRatioMap[L"|~"]);
  accidentals144TET.push_back(&namingRatioMap[L"|)"]);
  accidentals144TET.push_back(&namingRatioMap[L"/|~"]);
  accidentals144TET.push_back(&namingRatioMap[L"/|\\"]);

  accidentals72TET.push_back(&namingRatioMap[L"/|"]);
  accidentals72TET.push_back(&namingRatioMap[L"|)"]);
  accidentals72TET.push_back(&namingRatioMap[L"/|\\"]);


	nameViaGenerator = false;
	justIntonation = false;
}


//void Temperament::setJustMapping(const Val& persIn, const Val& gensIn)
//{
//	if (persIn.size() != gensIn.size())
//      Throw(L"diferent number of periods and generators specified.");
//
//	this->persIn = persIn;
//	this->gensIn = gensIn;
//	this->enabledIn = vector<bool>(persIn.size(), true);
//}

void Temperament::diagonalize(MatrixXI& U, MatrixXI& A, MatrixXI& V)// Given matrix A, diagonalized A  such that A' = U A V is diagonal
{
#ifdef _DEBUG
	MatrixXI origA = A;
#endif

	U = MatrixXI::Identity(A.rows(), A.rows());
	V = MatrixXI::Identity(A.cols(), A.cols());

	// Choose a pivot
	Index p = 0;

	Index maxP = A.rows() < A.cols() ? A.rows() : A.cols();

    // diagonalize matrix
	for (Index p = 0; p < maxP; p++)
	{
		bool rowGood = false;
		bool colGood = false;

		do
		{
			rowGood = checkRow(p, A);
			if(!rowGood)
				reduceRow(p, A, V);

			colGood = checkCol(p, A);
			if(! colGood)
				reduceCol(p, A, U);

		} while (!(rowGood && colGood));
	}

	// The matrix is now diagonal. Now we need to move all zero rows and columns to the end.
	Index zeroIndex = 0;
	for (Index p = 0; p < maxP; p++)
	{
		if (A(p, p) != 0)
		{
			if (zeroIndex < p)
			{
				MatrixXI temp = A.col(zeroIndex);
				A.col(zeroIndex) = A.col(p);
				A.col(p) = temp;

				temp = V.col(zeroIndex);
				V.col(zeroIndex) = V.col(p);
				V.col(p) = temp;

				temp = A.row(zeroIndex);
				A.row(zeroIndex) = A.row(p);
				A.row(p) = temp;

				temp = U.row(zeroIndex);
				U.row(zeroIndex) = U.row(p);
				U.row(p) = temp;
			}
			
			zeroIndex++;
		}
	}

#ifdef _DEBUG
	//PRINT(_T("a = \n"));
	//PRINT(A);
	//PRINT(_T("U*A*V = \n"));
	//PRINT(U*origA*V);

	//PRINT(_T("A = \n"));
	//PRINT(origA);
	//PRINT(_T("U^-1*a*V^-1 = \n"));
	//PRINT(U.inverse()*A*V.inverse());

	if (A != U*origA*V)
		PRINT(_T("CRAP!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"));

	//if (origA != U.inverse()*A*V.inverse())
	//	PRINT(_T("CRAP^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"));
#endif

}

void Temperament::reduceCol(Index p, MatrixXI& A, MatrixXI& U)
{
	while (true)
	{
		// find the cell in this column inclusively below row with
		// the smallest absolute value.
		Int smallest = -1;
		unsigned int smallestIndex = 0;
		for (unsigned int i = p; i < A.rows(); i++)
		{
			Int cell = A(i,p);
			if (cell < 0)   // make cell positive by multiplying its row by -1;
			{
				cell *= -1;
				A.row(i) *= -1;
				U.row(i) *= -1;
			}

			if (cell != 0 && (cell < smallest || smallest < 0))
			{
				smallest = cell;
				smallestIndex = i;
			}
		}

		if (smallest < 0)
		{
			// all values inclusively below r are zero so there is nothing to do.
			break;
		}

		for (unsigned int i = p; i < A.rows(); i++)
		{
			if (i != smallestIndex)
			{
				Int cell = A(i,p);
				if (cell != 0) // no need to do extra calculations
				{
					Int k = cell / smallest;

					// subtract k times row smallestAbsIndex from row j
					A.row(i) -= k*A.row(smallestIndex);
					U.row(i) -= k*U.row(smallestIndex);
				}
			}
		}

		// Move the smallest row to the top
		if (smallestIndex != p)
		{
			MatrixXI temp = A.row(smallestIndex);
			A.row(smallestIndex) = A.row(p);
			A.row(p) = temp;

			temp = U.row(smallestIndex);
			U.row(smallestIndex) = U.row(p);
			U.row(p) = temp;
		}

		if (checkCol(p, A))
			break;
	}
}

void Temperament::reduceRow(Index p, MatrixXI& A, MatrixXI& U)
{
	while (true)
	{
		// find the cell in this column inclusively below row with
		// the smallest absolute value.
		Int smallest = -1;
		unsigned int smallestIndex = 0;
		for (unsigned int i = p; i < A.cols(); i++)
		{
			Int cell = A(p, i);
			if (cell < 0)   // make cell positive by multiplying its col by -1;
			{
				cell *= -1;
				A.col(i) *= -1;
				U.col(i) *= -1;
			}

			if (cell != 0 && (cell < smallest || smallest < 0))
			{
				smallest = cell;
				smallestIndex = i;
			}
		}

		if (smallest < 0)
		{
			// all values inclusively below r are zero so there is nothing to do.
			break;
		}

		for (unsigned int i = p; i < A.cols(); i++)
		{
			if (i != smallestIndex)
			{
				Int cell = A(p, i);
				if (cell != 0) // no need to do extra calculations
				{
					Int k = cell / smallest;

					// subtract k times row smallestAbsIndex from row j
					A.col(i) -= k*A.col(smallestIndex);
					U.col(i) -= k*U.col(smallestIndex);
				}
			}
		}

		// Move the smallest row to the top
		if (smallestIndex != p)
		{
			MatrixXI temp = A.col(smallestIndex);
			A.col(smallestIndex) = A.col(p);
			A.col(p) = temp;

			temp = U.col(smallestIndex);
			U.col(smallestIndex) = U.col(p);
			U.col(p) = temp;
		}

		if (checkRow(p, A))
			break;
	}
}

bool Temperament::checkCol(Index p, MatrixXI& A)
{
	for (Index i = p + 1; i < A.rows(); i++)
	{
		if (A(i, p) != 0)
			return false;
	}

	return true;
}

bool Temperament::checkRow(Index p, MatrixXI& A)
{
	for (Index i = p + 1; i < A.cols(); i++)
	{
		if (A(p, i) != 0)
			return false;
	}

	return true;
}

Int Temperament::gcd(MatrixXI& m, Index row)
{
	Index cols = m.cols();
	if (cols == 0)
	{
		return 0;
	}
	else
	{
		Int d = m(row,0);
		for (Index i = 1; i < cols; i++)
		{
			d = Ratio::gcd(d, m(row, i));
		}

		return d;
	}
}

void Temperament::calcMatricies()
{
	pers = persIn;
	gens = gensIn;
	vector<Ratio> ratioGenerators = ratioGeneratorsIn;
	primes.clear();
//	primes.push_back(2);primes.push_back(3);primes.push_back(5);  // we need these three for naming, so make sure they are there.

  accidentalCents.clear();
	namingRatios.clear();
	noteNames.clear();
	sharps.clear();
	flats.clear();
  accidentalWeights.clear();

	for (unsigned int i = 0; i < inNamingRatios.size(); i++)
	{
		NamingRatio& n = inNamingRatios[i];
		for (unsigned int j = 0; j < n.ratios.size(); j++)
		{
			n.ratios[j].collectPrimes(primes);
			namingRatios.push_back(n.ratios[j]);
      accidentalCents.push_back(n.cents[j]);
			sharps.push_back(n.sharps);
			flats.push_back(n.flats);
			noteNames.push_back(n.noteNames);
		}
	}

  for (int i = namingRatios.size() - 1; i >=0; i--)
  {
    // we need to move all naming ratios with no accidentals (like the period) to the end of the list. 
    if (sharps[i].size() == 0 || flats[i].size() == 0)
    {
      namingRatios.push_back(namingRatios[i]);
      namingRatios.erase(namingRatios.begin() + i);

      accidentalCents.push_back(accidentalCents[i]);
      accidentalCents.erase(accidentalCents.begin() + i);

      sharps.push_back(sharps[i]);
      sharps.erase(sharps.begin() + i);

      flats.push_back(flats[i]);
      flats.erase(flats.begin() + i);

      noteNames.push_back(noteNames[i]);
      noteNames.erase(noteNames.begin() + i);
    }
  }

  for (int i = 0; i < namingRatios.size(); i++)
  {
    accidentalWeights.push_back(1-0.001*i);
  }

	for (Index i = ratioGeneratorsIn.size() - 1; i >= 0; i--)
	{
		if (enabledIn[i])
		{
			ratioGeneratorsIn[i].collectPrimes(primes);
		}
		else
		{
			pers.erase(pers.begin() + i);
			gens.erase(gens.begin() + i);
			ratioGenerators.erase(ratioGenerators.begin() + i);
		}
	}

//For testing.
//	primes.push_back(17);primes.push_back(19);primes.push_back(23);primes.push_back(29);primes.push_back(31);primes.push_back(37);primes.push_back(41);

	std::sort(primes.begin(), primes.end(), greaterThan()); //sort from greatest to least.

	logs.resize(primes.size());
	for (unsigned int i = 0; i < primes.size(); i++)
	{
		logs(i) = log2(primes[i]);
	}

	MatrixXI P = MatrixXI::Zero(primes.size(), ratioGenerators.size());
	MatrixXI G = MatrixXI::Zero(2, ratioGenerators.size());
	for (unsigned int i = 0; i < ratioGenerators.size(); i++)
	{
		ratioGenerators[i].primeFactors(i, P, primes);
		G(0, i) = pers[i];
		G(1, i) = gens[i];
	}

  {
	  MatrixXI A = MatrixXI::Zero(primes.size() + 2, ratioGenerators.size() + primes.size());
	  A.topLeftCorner(primes.size(), primes.size()) = -MatrixXI::Identity(primes.size(), primes.size());
	  A.topRightCorner(primes.size(), ratioGenerators.size()) = P;
	  A.bottomRightCorner(2, ratioGenerators.size()) = G;

	  PRINT(_T("A = \n"));
	  PRINT(A);
	  PRINT(_T("\n"));

	  MatrixXI U, V;
	  diagonalize(U, A, V);
	  MatrixXI& B = A;   

	  PRINT(_T("a = \n"));
	  PRINT(A);
	  PRINT(_T("\n"));

	  PRINT(_T("U = \n"));
	  PRINT(U);
	  PRINT(_T("\n"));

	  PRINT(_T("V = \n"));
	  PRINT(V);
	  PRINT(_T("\n"));

  //	/////////////////
  //	// Just a note on Eigen. You can't assign a block of a matrix to itself. For example the following ends up with juberish in U and aU:
  //	U = U.rightCols(2);
  //	aU = U;
  //	// However multiplying the block by another matrix and reassigning it to the origonal variable is fine: The following works
  //	U = U.rightCols(2)*MatrixXI::Identity(2,2);
  //	aU = U;
  //	/////////////////

	  aU = U.rightCols(2);  //used to calculate d from g.  d = aU*g.

	  MatrixXI W;
		Index rank = 0;
		while (rank < B.rows() && rank < B.cols() && B(rank, rank) != 0)
			rank++;

		a.resize(rank);
		for (Index i = 0; i < rank; i++)
		{
			a(i) = B(i, i);
		}

		W = V.topRightCorner(primes.size(), V.cols() - rank);
		aV = V.topLeftCorner(primes.size(), rank);               // used for calculating frquancies of primes.
	  
	  PRINT(_T("P = \n"));
	  PRINT(P);
	  PRINT(_T("\n"));

	  PRINT(_T("aV = \n"));
	  PRINT(aV);
	  PRINT(_T("\n"));

	  PRINT(_T("W = \n"));
	  PRINT(W);
	  PRINT(_T("\n"));

	  MatrixXI Wprime = W;
	  Index i = 0;
	  Yfactor.clear();
	  gcds = VectorXI(W.rows() > Wprime.cols() ? W.rows(): Wprime.cols());

	  while (Wprime.cols() > 0 && Wprime.rows() > 0)
	  {
		  MatrixXI row = Wprime.row(0);
		  Int d = gcd(Wprime, 0);
		  gcds(i) = d;
		  MatrixXI U, V;
		  diagonalize(U, row, V);
		  PRINT(_T("Wprime = \n"));
		  PRINT(Wprime);
		  PRINT(_T("U = "));
		  PRINT(U);
		  PRINT(_T("V = "));
		  PRINT(V.transpose());
		  if (d != 0)
		  {
			  Yfactor.push_back(Wprime.bottomRows(Wprime.rows() - 1)*V.col(0)*U(0, 0)*d / row(0));
			  Wprime = Wprime.bottomRows(Wprime.rows() - 1)*V.rightCols(V.cols() - 1);
		  }
		  else
		  {
			  Yfactor.push_back(VectorXI::Zero(Wprime.rows() - 1));
			  Wprime = Wprime.bottomRows(Wprime.rows() - 1)*V;
		  }

		  i++;

		  PRINT(_T("d = %d\n"), d);
		  PRINT(_T("Yfactor = "));
		  PRINT(Yfactor.back().transpose());
		  PRINT(_T("\n"));
	  }
  }

  {
	  MatrixXI N = MatrixXI::Zero(primes.size() + 2, ratioGenerators.size() + namingRatios.size());

	  for (unsigned int i = 0; i < namingRatios.size(); i++)
	  {
		  namingRatios[i].primeFactors(i, N, primes);
	  }

	  N.topRightCorner(primes.size(), ratioGenerators.size()) = P;
	  N.bottomRightCorner(2, ratioGenerators.size()) = G;

	  PRINT(_T("N = \n"));
	  PRINT(N);

		MatrixXI U, V;
		diagonalize(U, N, V);
        nU = U.leftCols(primes.size());   //used to calculate d from X.  d = nU*X.

		Index rank = 0;
		while (rank < N.rows() && rank < N.cols() && N(rank, rank) != 0)
			rank++;

		n.resize(rank); 
		for (Index i = 0; i < rank; i++)
		{
			n(i) = N(i, i);
		}    

    MatrixXI W = V.topRightCorner(namingRatios.size(), V.cols() - rank);
    nV = V.topLeftCorner(namingRatios.size(), rank); //used to calucate Y_i from d_i and n_i.  Y_i = d_i/n_i. 


    MatrixXI Wprime = W;
    Index i = 0;
    YNfactor.clear();
    gcdNs = VectorXI(W.rows() > Wprime.cols() ? W.rows() : Wprime.cols());

    while (Wprime.cols() > 0 && Wprime.rows() > 0)
    {
      MatrixXI row = Wprime.row(0);
      Int d = gcd(Wprime, 0);
      gcdNs(i) = d;
      MatrixXI U, V;
      diagonalize(U, row, V);
      PRINT(_T("Wprime = \n"));
      PRINT(Wprime);
      PRINT(_T("U = "));
      PRINT(U);
      PRINT(_T("V = "));
      PRINT(V.transpose());
      if (d != 0)
      {
        YNfactor.push_back(Wprime.bottomRows(Wprime.rows() - 1)*V.col(0)*U(0, 0)*d / row(0));
        Wprime = Wprime.bottomRows(Wprime.rows() - 1)*V.rightCols(V.cols() - 1);
      }
      else
      {
        YNfactor.push_back(VectorXI::Zero(Wprime.rows() - 1));
        Wprime = Wprime.bottomRows(Wprime.rows() - 1)*V;
      }

      i++;

      PRINT(_T("d = %d\n"), d);
      PRINT(_T("YNfactor = "));
      PRINT(YNfactor.back().transpose());
      PRINT(_T("\n"));
    }
  }

  {
    MatrixXI M = MatrixXI::Zero(primes.size() + 2, ratioGenerators.size() + 2);

    M.topRightCorner(primes.size(), ratioGenerators.size()) = P;
    M.bottomRightCorner(2, ratioGenerators.size()) = G;
    M(M.rows() - 2, 0) = -1;
    M(M.rows()-1,1) = -1;

    PRINT(_T("M = \n"));
    PRINT(M);

    MatrixXI U, V;
    diagonalize(U, M, V);
    mU = U.leftCols(U.cols()-2);   //used to calculate d from X.  d = mU*X.

    Index rank = 0;
    while (rank < M.rows() && rank < M.cols() && M(rank, rank) != 0)
      rank++;

    m.resize(rank);
    for (Index i = 0; i < rank; i++)
    {
      m(i) = M(i, i);
    }

    MatrixXI W = V.topRightCorner(2, V.cols() - rank); 
    mV = V.topLeftCorner(2, rank); //used to calucate Y_i from d_i and n_i.  Y_i = d_i/n_i.   


    MatrixXI Wprime = W;
    Index i = 0;
    YMfactor.clear();
    gcdMs = VectorXI(W.rows() > Wprime.cols() ? W.rows() : Wprime.cols());

    while (Wprime.cols() > 0 && Wprime.rows() > 0)
    {
      MatrixXI row = Wprime.row(0);
      Int d = gcd(Wprime, 0);
      gcdMs(i) = d;
      MatrixXI U, V;
      diagonalize(U, row, V);
      PRINT(_T("Wprime = \n"));
      PRINT(Wprime);
      PRINT(_T("U = "));
      PRINT(U);
      PRINT(_T("V = "));
      PRINT(V.transpose());
      if (d != 0)
      {
        YMfactor.push_back(Wprime.bottomRows(Wprime.rows() - 1)*V.col(0)*U(0, 0)*d / row(0));
        Wprime = Wprime.bottomRows(Wprime.rows() - 1)*V.rightCols(V.cols() - 1);
      }
      else
      {
        YMfactor.push_back(VectorXI::Zero(Wprime.rows() - 1));
        Wprime = Wprime.bottomRows(Wprime.rows() - 1)*V;
      }

      i++;

      PRINT(_T("d = %d\n"), d);
      PRINT(_T("YMfactor = "));
      PRINT(YMfactor.back().transpose());
      PRINT(_T("\n"));
    }
  }
}

bool Temperament::reduceRatio(Ratio& in, Ratio& out)
{
  int gen;
  int per;
  if(!calculateGenPer(gen, per, in))
     return false;

  VectorXI X = VectorXI::Zero(primes.size());
  if(!calculateRatio(gen, per, X))
     return false;

//  PRINT(L"~~ X = ");
//  PRINT(X.transpose());
  out = Ratio();
  buildFraction(out, X);  // reduce Anchor freq ratio
  return true;
}

bool Temperament::calculateRatio(Int gen, Int per, VectorXI& X)
{
	//PRINT(_T("!!!!!!!!!! per = %d, gen = %d\n"), per, gen);

	X = VectorXI::Zero(primes.size());

	VectorXI d(2);
	d << per, gen;
	d = aU*d;

//	PRINT(_T("d = "));
//	PRINT(d.transpose());

	VectorXI y(a.rows());
	Index i = 0;
	for (; i < a.rows(); i++)
	{
		if (d(i) % a(i))
			return false;
		else
			y(i) = d(i) / a(i);
	}

	for (; i < d.rows(); i++)
	{
		if (d(i) != 0)
		  return false;
	}

	double t = TENNY_MAX;
	VectorXI Y = aV*y;

	//PRINT(_T("Y = "));
	//PRINT(Y.transpose());

	minTenney(0, Y, X, t);


	//PRINT(_T("~~~ X = "));
	//PRINT(X.transpose());
 //   PRINT(_T("T = %f\n\n"), t);
	//PRINT(_T("\n"));

	return true;
}


void Temperament::minTenney(Index i, VectorXI& Y, VectorXI& X, double& minT)
{
	//PRINT(i, _T("X top ="));
	//PRINT(X.topRows(i).transpose());
	//PRINT(i, _T("Y="));
	//PRINT(Y.transpose());

	if (i >= Yfactor.size())
	{
		minT = 0;
		Index j = 0;
		for (; i < X.rows(); i++)
		{
			X(i) = Y(j);
			minT += logs(i)*abs(X(i));
			j++;
		}

		//PRINT(i, _T("------------ minT=%f"), minT);
		//PRINT(i, _T("------------ Y="));
		//PRINT(Y.transpose());
		//PRINT(i, _T("------------ X="));
		//PRINT(X.transpose());
		return;
	}

	Int d = gcds(i);
	MatrixXI Ybottom = Y.bottomRows(Y.rows() - 1);
	VectorXI Xmin = X;

	// X(0) = d*k + Y(0) ==> k=(X(0)-Y(0))/d   ==> minimum X(0) is at k = -Y(0)/d;
	int k = 0;
	if (d != 0)
		k = lrint(-(double)Y(0) / d + .00000001);

	X(i) = d*k + Y(0);
	int sign = X(i) >= 0 ? -1:1;
	int inc = 0;
	while (true)
	{
		double t = logs(i)*abs(X(i));
		if (t > minT + .0001) // The .0001 is a threshold to deal with small rounding errors.
		{
			X = Xmin;

			//PRINT(_T("prime = %d\n"), primes[i]);
			//PRINT(_T("log = %f\n"), logs(i));
			//PRINT(_T("inc = %d\n"), inc);
			//PRINT(_T("X = "));
			//PRINT(X.transpose());
			return;
		}

		VectorXI Yprime = Ybottom + Yfactor[i] * k;

		double t2 = minT - t;
		minTenney(i + 1, Yprime, X, t2);
		t2 += t;

		//PRINT(i,_T("minT = %f\n"), minT);
		//PRINT(i,_T("t2 = %f\n"), t2);
		//PRINT(i,_T("t = %f\n"), t);
		//PRINT(i, _T("k = %d, inc = %d\n"), k, inc);
		//PRINT(i,_T("YFactor = "));
		//PRINT(Yfactor[i].transpose());
		//PRINT(i,_T("Yprime = "));
		//PRINT(Yprime.transpose());
		//PRINT(i,_T("Ybottom = "));
		//PRINT(Ybottom.transpose());

		//PRINT(i,_T("prime = %d\n"), primes[i]);
		////	PRINT(i,_T("log = %f\n"), logs(i));
		//PRINT(i,_T("X = "));
		//PRINT(X.transpose());

		if (t2 < minT - .0001)  // The .0001 is a threshold to deal with small rounding errors.
		{
			minT = t2;
			Xmin = X;

			//PRINT(i, _T("******************************** T = %f"), minT);
			//PRINT(_T(" X bottom = "));
			//PRINT(Xmin.bottomRows(Xmin.rows() - i).transpose());
		}

		if (d == 0)  // If d = 0, there is only one possible value for X(i). So we are done.
		{
			return;
		}
		else
		{
			//	k = k0 + inc
			//	X(i) = d*k + Y(0)
	        //
			//k0 -> k0-1 -> k0+1 -> k0-2 -> k0+2
			//    -1      +2      -3      +4
			//
			// d*k0+Y(0) -> d*(k0-1)+Y(0) -> d*(k0+1)+Y(0) -> d*(k0-2)+Y(0) -> d*(k0-2)+Y(0)
			//           -d              +2d               -3d              +4d

			inc++;
			X(i) -= sign*d*inc;
			k -= sign*inc;
			sign *= -1;
		}

	}
}

bool Temperament::calculateName(VectorXI& N, VectorXI& X, double& minAcc)
{
  //PRINT(_T("!!!!!!!!!! per = %d, gen = %d\n"), per, gen);

  N = VectorXI::Zero(namingRatios.size());
  VectorXI d = nU*X;

  //	PRINT(_T("d = "));
  //	PRINT(d.transpose());

  VectorXI y(n.rows());
  Index i = 0;
  for (; i < n.rows(); i++)
  {
    if (d(i) % n(i))
      return false;
    else
      y(i) = d(i) / n(i);
  }

  for (; i < d.rows(); i++)
  {
    if (d(i) != 0)
      return false;
  }

  VectorXI Y = nV*y;

  //PRINT(_T("Y = "));
  //PRINT(Y.transpose());

  minAcc = 100;
  return minAccidentals(0, Y, N, X, minAcc);
}

int Temperament::calculateAccidental(VectorXI& N, Index i, bool allowZeroAccidentals)
{
  int numNames = 0;
  if (noteNames.size() > i)
    numNames = noteNames[i].size();

  int noteIndex = N(i);
  int acc;
  if (numNames > 0)
  {
    if (noteIndex >= 0)
      acc = noteIndex / numNames;
    else
      acc = (noteIndex+1) / numNames - 1;
  }
  else
  {
    acc = noteIndex;
  }

  // this is nessisary for the period which dosn't contribute accidentals
  if ((acc < 0 && (flats.size() <= i || flats[i].size() == 0)) || (acc > 0 && (sharps.size() <= i || sharps[i].size() == 0)) && allowZeroAccidentals)
    acc = 0; 

  return acc;
}

bool Temperament::minAccidentals(Index i, VectorXI& Y, VectorXI& N, VectorXI& X, double& minAcc)
{
//  PRINT(L"~~~~~~~~i  = %d\n", i);

  int numNames = 0;
  if (noteNames.size() > i)
    numNames = noteNames[i].size();


  if (i >= YNfactor.size())
  {
    double minAcc2 = 0;
    Index j = 0;
    for (; i < N.rows(); i++)
    {
      N(i) = Y(j);
      j++;

           
      int acc = calculateAccidental(N,i, true);
      if (abs(acc) > 100)
      {
        PRINT(L"abs(acc) > 100");
        return false; // bailout for absurd conditions   
      }

      minAcc2 += abs(acc*accidentalWeights[i]);
    }
    
    if(minAcc2 > minAcc)
    {
      PRINT(L"minAcc2 > minAcc");
      return false;
    }

    minAcc = minAcc2;
    return true;
  }

  Int d = gcdNs(i);
  MatrixXI Ybottom = Y.bottomRows(Y.rows() - 1);

  int k = 0;
  if (d != 0)
    k = lrint((numNames / 2 - (double)Y(0)) / d);

  N(i) = d*k + Y(0);
  VectorXI Nmin = VectorXI::Zero(namingRatios.size());
//  PRINT(_T("~ Nmin  = "));
//  PRINT(Nmin.transpose());

  int accInit = calculateAccidental(N, i);
  int sign = accInit >= 0 ? -1 : 1;
  int inc = 0;
  bool minFound = false;
  while (true)
  {
 //   PRINT(L"!!!!!! i  = %d\n", i);
    double acc = abs(calculateAccidental(N, i)*accidentalWeights[i]);

    if (acc > minAcc && inc > 1)
   // if ((acc > minAcc  || inc > minAcc) && inc > 1) //Why the inc check?
    {
//      if(!minFound) PRINT(L"acc > minAcc");

      N = Nmin;
      return minFound;
    }

    VectorXI Yprime = Ybottom + YNfactor[i] * k;
    VectorXI N2 = N;
    double acc2 = minAcc - acc;
    bool tf = minAccidentals(i + 1, Yprime, N2, X, acc2);
    acc2 += acc;

    //wstring indent; for(int q = 0; q < i; q++) indent+= L" ";
    //PRINT(L"\n%stf = %d", indent.c_str(), (int) tf);
    //PRINT(L"\n%si  = %d", indent.c_str(), i);
    //PRINT(L"\n%sminAcc = %d", indent.c_str(), minAcc);
    //PRINT(L"\n%sacc2 = %d", indent.c_str(), acc2);
    //PRINT(L"\n%sX = ", indent.c_str());
    //PRINT(X.transpose());
    //PRINT(_T("%sNmin  = "), indent.c_str() );
    //PRINT(Nmin.topRows(Nmin.rows() - 1).transpose());
    //PRINT(_T("%sN2  = "), indent.c_str());
    //PRINT(N2.topRows(N2.rows() - 1).transpose());


    if(tf)
    { 
      if (acc2 < minAcc)
      {
  //      PRINT(i, _T("**** minAcc = %d : acc2 = %d\n"), minAcc, acc2);
        minAcc = acc2;
        Nmin = N2;
        minFound = true;
      
        if (d == 0)
        { 
          N = Nmin;
          return true;
        }

//        PRINT(_T("~~~ Nmin  = "));
//        PRINT(Nmin.transpose());

        //PRINT(i, _T("******************************** minAcc = %f"), minAcc);
        //PRINT(_T(" N bottom = "));
        //PRINT(Nmin.bottomRows(Nmin.rows() - i).transpose());
      }
      else if (acc2 == minAcc)
      {
//        PRINT(L"This actolly happens!!!\n");      
        
        for(int k = N.rows()-1; k >= 0; k--)
        { 
          int a1 = abs(calculateAccidental(N2, k)*accidentalWeights[i]);
          int a2 = abs(calculateAccidental(Nmin, k)*accidentalWeights[i]);
//          PRINT(_T("~~~ %d,%d\n"), a1, a2);
          
          if(a1 > a2)
          { 
            Nmin = N2;
            minFound = true;

//            PRINT(_T("~~~ Nmin  = "));
//            PRINT(Nmin.transpose());
//            PRINT(_T("\n"));

            if (d == 0)
            {
              N = Nmin;
              return true;
            }

            break;
          }
          else if (a1 < a2)
          {
            break;
          }
        }
      }
    }
 
    if (d == 0)  // If d = 0, there is only one possible value for N(i). So we are done.
    {
//      PRINT(L"d == 0");
      return false;
    }

    inc++;
    k -= sign*inc;
    N(i) -= sign*d*inc;
    sign *= -1;
  }
}

bool Temperament::minGenPer(VectorXI& M, VectorXI& X)
{
  //PRINT(_T("!!!!!!!!!! per = %d, gen = %d\n"), per, gen);

  M = VectorXI::Zero(2);
  VectorXI d = mU*X;

  //PRINT(_T("X = "));
  //PRINT(X.transpose());

  //PRINT(_T("mU = "));
  //PRINT(mU);

 	//PRINT(_T("d = "));
  //PRINT(d.transpose());

  VectorXI y(m.rows());
  Index i = 0;
  for (; i < m.rows(); i++)
  {
    if (d(i) % m(i))
      return false;
    else
      y(i) = d(i) / m(i);
  }

  for (; i < d.rows(); i++)
  {
    if (d(i) != 0)
      return false;
  }

  VectorXI Y = mV*y;

  //PRINT(_T("Y = "));
  //PRINT(Y.transpose());
  
  int minDist = 100000;
  return minGenPer(0, Y, M, minDist);


  //PRINT(_T("~~~ X = "));
  //PRINT(X.transpose());
  //   PRINT(_T("T = %f\n\n"), t);
  //PRINT(_T("\n"));
}


bool Temperament::minGenPer(Index i, VectorXI& Y, VectorXI& M, int& minDist)
{
  //  PRINT(L"~~~~~~~~i  = %d\n", i);

  if(minDist == 0)
    return false;

  if (i >= YMfactor.size())
  {
    minDist = 0;
    Index j = 0;
    for (; i < M.rows(); i++)
    {
      M(i) = Y(j);
      j++;

//      minDist += M(i)*M(i);
      minDist += abs(M(i));
    }

    return true;
  }

  Int d = gcdMs(i);
  MatrixXI Ybottom = Y.bottomRows(Y.rows() - 1);
  VectorXI Mmin = M;

  int k = 0;
  if (d != 0)
    k = lrint( -(double)Y(0) / d + .000001);

  M(i) = d*k + Y(0);

  int sign = M(i) >= 0 ? -1 : 1;
  int inc = 0;
  bool minFound = false;
  while (true)
  {
    //   PRINT(L"!!!!!! i  = %d\n", i);
//    double dist = M(i)*M(i);
    long dist = abs(M(i));

    if (dist > minDist)
    {
      M = Mmin;
      return minFound;
    }

    VectorXI Yprime = Ybottom + YMfactor[i] * k;

    int dist2 = minDist - dist;
    bool tf = minGenPer(i + 1, Yprime, M, dist2);
    dist2 += dist;

    if (tf)
    {
      if (dist2 < minDist)
      {
        //      PRINT(i, _T("**** minAcc = %d : acc2 = %d\n"), minAcc, acc2);
        minDist = dist2;
        Mmin = M;
        minFound = true;

        //PRINT(i, _T("******************************** minAcc = %f"), minAcc);
        //PRINT(_T(" N bottom = "));
        //PRINT(Nmin.bottomRows(Nmin.rows() - i).transpose());
      }
      //else if (dist2 == minDist && M(i+1) < Mmin(i+1))
      //{
      //  minDist = dist2;
      //  Mmin = M;
      //  minFound = true;
      //}
    }

    if (d == 0)  // If d = 0, there is only one possible value for N(i). So we are done.
    {
      return minFound;
    }

    inc++;
    k -= sign*inc;
    M(i) -= sign*d*inc;
    sign *= -1;
  }
}

void Temperament::clearNoteNames()
{
	noteInfo.clear();
}

void Temperament::clearNoteText()
{
  for (auto &ent1 : noteInfo)
  {
    for (auto  &ent2 : ent1.second)
    {
      ent2.second.clearNoteText();
    }
  }
}


NoteName* Temperament::borrowNoteName(int gen, int period, bool duplicateRatios, bool showName, bool scientificPitchNotation, bool use53TETNotation)
{
	if (noteInfo.count(gen))
	{
		if (noteInfo[gen].count(period))
		{
			return &noteInfo[gen][period];
		}
	}

	noteInfo[gen][period] = createNoteName(gen, period, duplicateRatios, showName,  scientificPitchNotation,  use53TETNotation);
	return &noteInfo[gen][period];
}

bool Temperament::calculateGenPer(int& gen, int& per, Ratio& r)
{
  gen = 0;
  per = 0;
  MatrixXI X = VectorXI::Zero(primes.size());
  r.primeFactors(0,  X, primes);

//  PRINT(L"X = ");
//  PRINT(X.transpose());

  VectorXI G;
  if (!minGenPer(G, (VectorXI)X.col(0)))
    return false;

  gen = G(1);
  per = G(0);
  return true;
}



NoteName Temperament::createNoteName(int gen, int period, bool duplicateRatios, bool showName, bool scientificPitchNotation, bool use53TETNotation)
{
	NoteName name;
  Ratio r;
	VectorXI X;

  period += tuningPers;
  gen += tuningGens;

  if (calculateRatio(gen, period, X))
	{
     if(!duplicateRatios)
     {
        VectorXI G;
        if(!minGenPer(G, X))
          return name;

        if(G(0) != period || G(1) != gen)
        {
    //      PRINT(_T("G = "));
    //      PRINT(G.transpose());
    //      PRINT(_T("per = %d : gen = %d\n\n"), period, gen);
          return name;
        }
	  }

  	buildFraction(r, X);

    if (r.num <= 0 || r.denom <= 0)  // This can happen if powers in X are absurdly large.
     return  name;

    r /= rootRatio;
		name.ratio = r;
    reduceRatio(r, name.ratio);

//		name.num = abs(period);
//		name.denom = abs(gen);
   
    VectorXI N;
    double  minAcc;
		if (showName && calculateName(N, X, minAcc))
		{

			//		PRINT(L"N = " );
			//		PRINT(N);
      name.name.push_back(L"");
      name.sharps.push_back(0);
      name.accidentalCents.push_back(0);
      name.accidental.push_back(L"");
			for (int i = 0; i < N.rows(); i++)
			{
				int numNames = 0;
				if (noteNames.size() > i)
					numNames = noteNames[i].size();

				if (numNames > 0)
				{
					int noteIndex = N(i) % numNames;
					name.name.back() = (noteNames[i][noteIndex < 0 ? noteIndex + numNames : noteIndex]);
				}

				int accidentals = calculateAccidental(N, i);

				if (numNames > 0)
					name.sharps.back() = accidentals;
   //     else if(accidentals > 0 && sharps.size() >0 && sharps[i].size() > 0) // We don;t want to add cents for the period.
   //       name.accidentalCents.back() += accidentals*accidentalCents[i];

				wstring accidentalStr;
				if (accidentals > 0 && sharps.size() > i && sharps[i].size() > 0)
				{
					int bigSharps = accidentals / sharps[i].size();
					for (int j = 0; j < bigSharps; j++)
						accidentalStr += sharps[i][sharps[i].size() - 1];

					int littleSharp = accidentals % sharps[i].size();
					if (littleSharp > 0)
						accidentalStr += sharps[i][littleSharp - 1];
				}
				else if (accidentals < 0 && flats.size() > i && flats[i].size() > 0)
				{
					accidentals = -accidentals;
					int bigFlats = accidentals / flats[i].size();
					for (int j = 0; j < bigFlats; j++)
						accidentalStr += flats[i][flats[i].size() - 1];

					int littleFlat = accidentals % flats[i].size();
					if (littleFlat > 0)
						accidentalStr += flats[i][littleFlat - 1];
				}

				if (numNames > 0)
					name.accidental.back() = accidentalStr + name.accidental.back();
				else
					name.accidental.back() += accidentalStr;
			}



			if (minAcc > 0 && name.accidental.size() == 0)
			{
				PRINT(L"minAcc = %d = ", minAcc);
				PRINT(L"N = ");
				PRINT(N.transpose());
				PRINT(L"\n");
			}

			//   wchar_t str[256]; swprintf_s(str, 256, L"%s%d", name.name.c_str(), minAcc); name.name = str;
		}
	}

  //if (use144ForNaming)
  //{
  //  double cents = mapGenPerToCents(gen - tuningGens, period - tuningPers);
  //  int num144Steps = (int)round(cents * 144 / 1200);
  //  int num12Steps;

  //  if (num144Steps < 0)
  //  {
  //    num12Steps = (num144Steps - 6) / 12;
  //  }
  //  else
  //  {
  //    num12Steps = (num144Steps + 5) / 12;
  //  }

  //  num12Steps = num12Steps % 12;

  //  if (num12Steps < 0)
  //    num12Steps += 12;
  //  else
  //    num12Steps;

  //  name.name = names12TET[num12Steps];
  //  name.accidental = accidentals12TET[num12Steps];
  //  if (accidentals12TET[num12Steps] == L"\xFD")
  //    name.sharps = 1;
  //  else if (accidentals12TET[num12Steps] == L"\x23")
  //    name.sharps = -1;
  //  else
  //    name.sharps = 0;

  //  int accidentals144 = (num144Steps + 5) % 144;
  //  if (accidentals144 < 0)
  //    accidentals144 += 144;

  //  accidentals144 = accidentals144 % 12;
  //  accidentals144 -= 5;
  //  if (accidentals144 > 0)
  //  {
  //    name.accidental += accidentals144TET[accidentals144 - 1]->sharps[0];
  //    name.accidentalCents += accidentals144TET[accidentals144 - 1]->cents[0];
  //  }
  //  else if (accidentals144 < 0)
  //  {
  //    name.accidental += accidentals144TET[-accidentals144 - 1]->flats[0];
  //    name.accidentalCents -= accidentals144TET[-accidentals144 - 1]->cents[0];
  //  }
  //}
//  if (use144ForNaming)
//  {
//    int edo = 144;
////    int edo = 130;
////    int edo = 24;
//    double cents = mapGenPerToCents(gen - tuningGens, period - tuningPers);
//    int numSteps = (int)round(cents * edo / 1200);
//
//    numSteps %= edo;
//    if(numSteps < 0)
//      numSteps += edo;
//    
//    double halfStepSteps = edo/12;
//    int numHalfSteps = round(numSteps/ halfStepSteps);
//
////    double stepSize = (double)1200/edo;
////   int stepsToFifth = 1200*log2(3.0/2)/stepSize;    
////    1200 * log2(3.0 / 2) / (1200 / edo) =  log2(3.0 / 2) / (1 / edo) = edo log2(3.0 / 2)
//
////    int stepsToFifth = round(edo*log2(3.0 / 2));
//
////    int numFifths = round(((double)numSteps)/stepsToFifth);
////    int nameIndex = numFifths%12;
//        
//    int nameIndex = numHalfSteps%12;
//    name.name = names12TET[nameIndex];
//    name.accidental = accidentals12TET[nameIndex];
//
//    if (accidentals12TET[nameIndex] == L"\xFD")
//      name.sharps = 1;
//    else if (accidentals12TET[nameIndex] == L"\x23")
//      name.sharps = -1;
//    else
//      name.sharps = 0;
//
//    //int accidentals144 = (num144Steps + 5) % 130;
//    //if (accidentals144 < 0)
//    //  accidentals144 += 130;
//
//    //accidentals144 = accidentals144 % 12;
//    //accidentals144 -= 5;
//    //if (accidentals144 > 0)
//    //{
//    //  name.accidental += accidentals144TET[accidentals144 - 1]->sharps[0];
//    //  name.accidentalCents += accidentals144TET[accidentals144 - 1]->cents[0];
//    //}
//    //else if (accidentals144 < 0)
//    //{
//    //  name.accidental += accidentals144TET[-accidentals144 - 1]->flats[0];
//    //  name.accidentalCents -= accidentals144TET[-accidentals144 - 1]->cents[0];
//    //}
//  }
//  else if (use72ForNaming)
  if(scientificPitchNotation)
  {
    double cents = mapGenPerToCents(gen - tuningGens, period - tuningPers);

    double freq = frequencyFromCents(cents);

    cents = 1200 * log2(freq / 440);

    double num12StepsD = cents/100;
    int num12Steps = round(num12StepsD);
    num12StepsD -= num12Steps;

    name.accidentalCents.push_back(round(num12StepsD*100));

    num12Steps = num12Steps % 12;

    if (num12Steps < 0)
      num12Steps += 12;

    if (name.accidentalCents.back() == -50)
    {
      name.accidentalCents.back() *= -1;
      num12Steps--;

      num12Steps = num12Steps % 12;
      if (num12Steps < 0)
        num12Steps += 12;
    }
 
    name.name.push_back(names12TET[num12Steps]);
    name.accidental.push_back(accidentals12TET[num12Steps]);
    if (accidentals12TET[num12Steps] == L"\xFD")
      name.sharps.push_back(1);
    else if (accidentals12TET[num12Steps] == L"\x23")
      name.sharps.push_back(-1);
    else
      name.sharps.push_back(0);
  }

  if (use53TETNotation)
  {
    double cents = mapGenPerToCents(gen - tuningGens, period - tuningPers);
    int edo = 53;
    double freq = frequencyFromCents(cents);

    cents = 1200 * log2(freq / 441.2);

    double numStepsD = cents / (1200.0/edo);
    int numSteps = round(numStepsD);
    numStepsD -= numSteps;

    name.accidentalCents.push_back(cents  - round(numSteps * (1200.0 /edo)));

    numSteps = numSteps % 53;

    if (numSteps < 0)
      numSteps += edo;

    name.name.push_back(names53TET[numSteps]);
    name.accidental.push_back(accidentals53TET[numSteps]);
    name.sharps.push_back(0);
  }


  for(int i = 0; i < name.accidental.size(); i++)
  {
    if(name.accidental[i].size()>0)
      name.accidental[i] = L" " + name.accidental[i];
  }

//  if(centsOff < 0)
//    name.accidental = L"+" + to_string()centsoff;
//  else if (centsOff > 0)
//    name.accidental = L"+" + ;

	return name;
}

void Temperament::buildFraction(Ratio& r, VectorXI& x)
{
	for (unsigned int i = 0; i < x.size(); i++)
	{
		Int power = x[i];
    if (abs(power*primes[i]) > 40)  // text for absurdly hight powers
    {
      r.num = 0;
      r.denom = 0;
      return;
    }

		if (power > 0)
		{
			r.num *= (Int)static_cast<unsigned int>(pow(primes[i], static_cast<unsigned int>(power)));
		}
		else
		{
			r.denom *= (Int)(pow(primes[i], static_cast<unsigned int>(-power)));
		}
	}
}


// returns 1 on sucess, 0 on failer
task<bool> Temperament::loadTemperamentAsync(StorageFolder^ folder)
{
  wstring fileName = convertStringToFileName(this->name.c_str());
  fileName += L".tpmt";

  return create_task(folder->GetFileAsync(ref new Platform::String(fileName.c_str()))).then([this, folder, fileName](StorageFile^ file)
  {
    {
      critical_section::scoped_lock lock(*criticalSection);  // This is probobly not nessisary
      temperamentFolder = folder;
    }

    return FileIO::ReadLinesAsync(file);
  }).then([this, fileName](IVector<Platform::String^>^ lines)
  {
    critical_section::scoped_lock lock(*criticalSection);

	  trimFileExtension(this->name);

    clearNoteNames();
	  persIn.clear();
	  gensIn.clear();
	  ratioGeneratorsIn.clear();
	  enabledIn.clear();
	  inNamingRatios.clear();

	  genCents = 700;
	  periodCents = 1200;
	  baseFreq = 440;

    tuningNote = L"A";
    rootNote = L"C";

    fontName = L"Verdana";
    fontSize = 20;

    vector<wstring> names;

	  // read each line of the file
	  for (unsigned int i = 0; i < lines->Size; i++)
	  {
		  Platform::String^ str = lines->GetAt(i);
		  if (str->Length() == 0)
			  continue;

		  // read an entire line into memory
		  wchar_t buf[MAX_CHARS_PER_LINE] = { 0 };
		  wcscpy_s(buf, str->Data());


		  // parse the line into blank-delimited tokens
		  int n = 0; // a for-loop index

		  LPTSTR next_token = NULL;
		  LPTSTR token = wcstok_s(buf, _T(" "), &next_token); // first token
		  if (!token)
		  {
			  // do nothing
		  }
		  else if ( !wcscmp(token, _T("name")))
		  {
			  wstring name;
			  for (n = 1; n < MAX_TOKENS_PER_LINE; n++)
			  {
				  token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
				  if (!token) break; // no more tokens

				  if (n > 1)
            name += _T(" ");

          name += token;
			  }
		  }
      else if (!wcscmp(token, _T("fontName")))
      {
        fontName = L"";
        for (n = 1; n < MAX_TOKENS_PER_LINE; n++)
        {
          token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
          if (!token) break; // no more tokens

          if (n > 1)
            fontName += _T(" ");

          fontName += token;
        }
      }
      else if (!wcscmp(token, _T("fontSize")))
      {
        token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
        if (!token) break; // no more tokens

        LPTSTR end = token;

        double num = wcstod(token, &end);
        if (end != token)
        {
          fontSize = num;
        }
      }
		  else if (!wcscmp(token, _T("ratio")))
		  {
			  while (true)
			  {
				  token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
				  if (!token) break; // no more tokens

				  LPTSTR start = token;
				  LPTSTR end = token;

				  Ratio ratio;
				  int num = wcstol(start, &end, 10);
				  if (*end == _T('\0'))
				  {
					  ratio.num = num;
				  }
				  else if (start != end && *end == _T(':'))
				  {
					  end++;
					  start = end;
					  int denom = wcstol(start, &end, 10);
					  if (*end == _T('\0'))
					  {
						  ratio.num = num;
						  ratio.denom = denom;
					  }
				  }

				  ratioGeneratorsIn.push_back(ratio);
			  }
		  }
		  else if (!wcscmp(token, _T("enabled")))
		  {
			  while (true)
			  {
				  token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
				  if (!token) break; // no more tokens

				  LPTSTR end = token;

				  int num = wcstol(token, &end, 10);
				  if (*end == _T('\0'))
				  {
					  enabledIn.push_back((bool)num);
				  }
			  }
		  }
		  else if (!wcscmp(token, _T("period")))
		  {
			  while (true)
			  {
				  token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
				  if (!token) break; // no more tokens

				  LPTSTR end = token;

				  int num = wcstol(token, &end, 10);
				  if (*end == _T('\0'))
				  {
					  persIn.push_back(num);
				  }
			  }
		  }
		  else if (!wcscmp(token, _T("generator")))
		  {
			  while (true)
			  {
				  token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
				  if (!token) break; // no more tokens

				  LPTSTR end = token;

				  int num = wcstol(token, &end, 10);
				  if (*end == _T('\0'))
				  {
					  gensIn.push_back(num);
				  }
			  }
		  }
		  else if (!wcscmp(token, _T("gen-cents")))
		  {
			  token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
			  if (!token) break; // no more tokens

			  LPTSTR end = token;

			  double num = wcstod(token, &end);
			  if (end != token)
			  {
				  genCents = num;
			  }
		  }
		  else if (!wcscmp(token, _T("per-cents")))
		  {
			  token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
			  if (!token) break; // no more tokens

			  LPTSTR end = token;

			  double num = wcstod(token, &end);
			  if (end != token)
			  {
				  periodCents = num;
			  }
		  }
		  else if (!wcscmp(token, _T("rootNote")))
		  {
			  token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
			  if (!token) break; // no more tokens

        rootNote = token;
		  }
      else if (!wcscmp(token, _T("tuningNote")))
      {
        token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
        if (!token) break; // no more tokens

        tuningNote = token;

        token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
        if (!token) break; // no more tokens

        LPTSTR end = token;

        double num = wcstod(token, &end);
        if (end != token)
        {
          baseFreq = num;
        }
      }
      else if (!wcscmp(token, _T("notes")))
      {
        while (true)
        {
          token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
          if (!token) break; // no more tokens       
          names.push_back(token);
        }
      }
	    else if (!wcscmp(token, _T("accidentals")))
	    {
		    while (true)
		    {
			    token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
			    if (!token) break; // no more tokens       

			    if (namingRatioMap.count(token))
			    {
				    inNamingRatios.push_back(namingRatioMap[token]);
			    }
		    }
	    }
	  }

	  if (persIn.size() > gensIn.size())
	  {
		  gensIn.resize(persIn.size(), 0);
	  }
	  else if (persIn.size() < gensIn.size())
	  {
		  persIn.resize(gensIn.size(), 0);
	  }

	  gensIn.resize(ratioGeneratorsIn.size(), 0);
	  persIn.resize(ratioGeneratorsIn.size(), 0);
	  enabledIn.resize(ratioGeneratorsIn.size(), true);

    namingRatios.clear();
    noteNames.clear();
    sharps.clear();
    flats.clear();

    if (inNamingRatios.size() == 0)
    {
	    inNamingRatios.push_back(namingRatioMap[L"#"]);
      inNamingRatios.push_back(namingRatioMap[L"2"]);
    }

    Temperament::refreshNames(names);
  }).then([this](task<void> t)
  {
    critical_section::scoped_lock lock(*criticalSection);
    try
    {
      t.get();
      return true;
    }
    catch (Platform::Exception^ e)
    {
      // file not found
      return false;
    }
  });
}

void Temperament::refreshNames(vector<wstring>& names)
{
  if (names.size() == 0)
  {
    wstring defaultNames[] = { L"F", L"C", L"G", L"D", L"A", L"E", L"B" };
    names.assign(defaultNames, defaultNames + sizeof(defaultNames) / sizeof(defaultNames[0]));
  }

  if(names.size() > 0)
  {
    for (int i = 0; i < inNamingRatios.size(); i++)
    {
      if (inNamingRatios[i].asciiName == L"#")
      {
        inNamingRatios[i].noteNames.clear();
        for (int j = 0; j < names.size(); j++)
          inNamingRatios[i].noteNames.push_back(names[j]);
      }
    }
  }

  Ratio rAnchor = Ratio();
  vector<wstring>::iterator it = std::find(names.begin(), names.end(), tuningNote.c_str());
  if (it != names.end())
  {
    int pos = distance(names.begin(), it);
    for (int i = 0; i < pos; i++)
      rAnchor *= Ratio((Int)3, (Int)2);

    while (rAnchor  > 4)
      rAnchor /= 2;
  }


  Ratio rRoot = Ratio();
  it = std::find(names.begin(), names.end(), rootNote.c_str());
  if (it != names.end())
  {
    rootIndex = distance(names.begin(), it);
    for (int i = 0; i < rootIndex; i++)
      rRoot *= Ratio((Int)3, (Int)2);

    while ((rRoot - rAnchor) > 2)
      rRoot /= 2;
  }

  calcMatricies();

  reduceRatio(rAnchor, anchorFreqRatio);  //try making A tuning note and G root and play A in just intonation. A will drop in pitch
  reduceRatio(rRoot, rootRatio);

  calculateGenPer(tuningGens, tuningPers, anchorFreqRatio);
  calculateGenPer(rootGens, rootPers, rootRatio);

  {
    Ratio r = anchorFreqRatio / rootRatio;
    anchorToRootRatio;
    reduceRatio(r, anchorToRootRatio);
  }
}

double Temperament::getRootCents()
{
  if (justIntonation)
  {
    return 1200 * log2(static_cast<double>(anchorToRootRatio.num) / anchorToRootRatio.denom);
  }
  else
  {
    return (tuningPers-rootPers)*periodCents + (tuningGens-rootGens)*genCents;
  }
}

double Temperament::getCents(double gens, double pers, bool duplicateRatios)
{
	if (justIntonation)
  {
    if (noteInfo.count(gens))
    {
      if (noteInfo[gens].count(pers))
      {
        noteInfo[gens][pers];
        return 1200 * log2(static_cast<double>(noteInfo[gens][pers].ratio.num) / noteInfo[gens][pers].ratio.denom) - getRootCents();
      }
    }

    Ratio r;
    VectorXI X;

    pers += tuningPers;
    gens += tuningGens;

    if (calculateRatio(gens, pers, X))
    {
      buildFraction(r, X);

      if (!duplicateRatios)
      {
        VectorXI G;
        if (!minGenPer(G, X))
          return -100000;

        if (G(0) != pers || G(1) != gens)
        {
          //      PRINT(_T("G = "));
          //      PRINT(G.transpose());
          //      PRINT(_T("per = %d : gen = %d\n\n"), period, gen);
          return -100000;
        }
      }

      if (r.num <= 0 || r.denom <= 0)  // This can happen if powers in X are absurdly large.
        return  -100000;

      r /= rootRatio;
      reduceRatio(r, r);

	    return 1200 * log2(static_cast<double>(r.num) / r.denom) - getRootCents();
    }
    else
    {
      return   -100000;
    }

	}
	else
	{
		return mapGenPerToCents(gens, pers);
	}
}


void Temperament::generateTemperament(double periodInCents, int Cols, double EDO, double keySlope)
{
  periodCents = periodInCents / EDO;

  double delta = periodInCents - periodCents * keySlope*Cols;
  genCents = -delta / Cols;
//  refreshNames(vector<wstring>());

{
  wstring defaultNames[] = { L"F", L"C", L"G", L"D", L"A", L"E", L"B" };
  vector<wstring> names;
  names.assign(defaultNames, defaultNames + sizeof(defaultNames) / sizeof(defaultNames[0]));

  Ratio rAnchor = Ratio();
  vector<wstring>::iterator it = std::find(names.begin(), names.end(), tuningNote.c_str());
  if (it != names.end())
  {
    int pos = distance(names.begin(), it);
    for (int i = 0; i < pos; i++)
      rAnchor *= Ratio((Int)3, (Int)2);

    while (rAnchor  > 4)
      rAnchor /= 2;
  }


  Ratio rRoot = Ratio();
  it = std::find(names.begin(), names.end(), rootNote.c_str());
  if (it != names.end())
  {
    rootIndex = distance(names.begin(), it);
    for (int i = 0; i < rootIndex; i++)
      rRoot *= Ratio((Int)3, (Int)2);

    while ((rRoot - rAnchor) > 2)
      rRoot /= 2;
  }

  reduceRatio(rAnchor, anchorFreqRatio);  //try making A tuning note and G root and play A in just intonation. A will drop in pitch
  reduceRatio(rRoot, rootRatio);

  calculateGenPer(tuningGens, tuningPers, anchorFreqRatio);
  calculateGenPer(rootGens, rootPers, rootRatio);

  {
    Ratio r = anchorFreqRatio / rootRatio;
    anchorToRootRatio;
    reduceRatio(r, anchorToRootRatio);
  }
}

  clearNoteNames();
}

double Temperament::mapGenPerToCents(double gens, double pers)
{
//  pers -= tuningPers;
//  gens -= tuningGens;
  return periodCents * pers + genCents * gens + shiftCents;
}

double Temperament::frequencyFromCents(double cents)
{
  return baseFreq*pow(2, cents / 1200);
}

double Temperament::centsFromFrequance(double freq)
{
  return 1200 * log2(freq / baseFreq);
}

task<void> Temperament::saveTemperament()
{
	wstring fileNameBase = convertStringToFileName(name.c_str());
	wstring fileName = fileNameBase + _T(".tpmt");
  return create_task(localTemperamentFolder->CreateFileAsync(ref new Platform::String(fileName.c_str()), CreationCollisionOption::ReplaceExisting))
    .then([this](StorageFile^ file)
  {
	  wstringstream content;

	  content << "name " << name.c_str() << endl;

	  content << "enabled";
	  for (unsigned int i = 0; i < enabledIn.size(); i++)
		  content << " " << (int)enabledIn[i];
	  content << endl;


	  content << "ratio";
	  for (unsigned int i = 0; i < ratioGeneratorsIn.size(); i++)
		  content << " " << ratioGeneratorsIn[i].num << ":" << ratioGeneratorsIn[i].denom;
	  content << endl;

	  content << "period";
	  for (unsigned int i = 0; i < persIn.size(); i++)
		  content << " " << persIn[i];
	  content << endl;

	  content << "generator";
	  for (unsigned int i = 0; i < gensIn.size(); i++)
		  content << " " << gensIn[i];
	  content << endl;

	  content << "gen-cents " << genCents << endl;
	  content << "per-cents " << periodCents << endl;

    for (unsigned int i = 0; i < inNamingRatios.size(); i++)
    { 
      if (inNamingRatios[i].noteNames.size() > 0)
      {
        content << "notes";
        for (unsigned int j = 0; j < inNamingRatios[i].noteNames.size(); j++)
          content << " " << inNamingRatios[i].noteNames[j];
        content << endl;
      }
    }

    content << "accidentals";
    for (unsigned int i = 0; i < inNamingRatios.size(); i++)
      content << " " << inNamingRatios[i].asciiName;
    content << endl;


    content << "tuningNote " << tuningNote << " " << baseFreq << endl;
    content << "rootNote " << rootNote << endl;

	  wstring str = content.str();

    return FileIO::WriteTextAsync(file, ref new Platform::String(str.c_str()));
  });
}