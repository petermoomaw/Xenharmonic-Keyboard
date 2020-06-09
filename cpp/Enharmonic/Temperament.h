#pragma once
#include "Common.h"
#include "Lattice.h"
#include "Ratio.h"
#include "Val.h"
#include <vector>

using namespace std;

using namespace Microsoft::WRL;
using namespace Windows::Storage;

typedef long double Double;

class NamingRatio
{
public:
	wstring asciiName;
	vector<Ratio> ratios;
	vector<wstring> noteNames;
	vector<wstring> sharps;
	vector<wstring> flats;
  vector<double> cents;
};

class NoteName
{
public:
  NoteName()
  {
    ratio.num = 0;
  }

	~NoteName()
	{
		for (int i = 0; i < pTextLayout.size(); i++)
		{
			if (pTextLayout[i] != NULL)
			{
				(pTextLayout[i])->Release();
				(pTextLayout[i]) = NULL;
			}
		}
	}

  void clearNoteText()
  {
    for (int i = 0; i < pTextLayout.size(); i++)
    {
      if (pTextLayout[i] != NULL)
      {
        (pTextLayout[i])->Release();
        (pTextLayout[i]) = NULL;
      }
    }
  }

	vector<wstring> name;
  vector<wstring> accidental;
	vector<int> sharps;
  vector<double> accidentalCents;
	Ratio ratio;
	vector<IDWriteTextLayout*> pTextLayout;
};

class Temperament
{
public:
	bool nameViaGenerator;
	Val persIn;
	Val gensIn;
	vector<Ratio> ratioGeneratorsIn;
	vector<bool> enabledIn;
	Val pers;
	Val gens;
	int perFactor;
	int genFactor;
	vector<Int> primes;
	VectorXd logs;
  Ratio rootRatio;
  Ratio anchorFreqRatio;  
  Ratio anchorToRootRatio;
  int tuningGens;
  int tuningPers;
  int rootGens;
  int rootPers;
  wstring rootNote;
  int rootIndex;
  wstring tuningNote;
  double shiftCents = 0;
	std::vector<VectorXI, Eigen::aligned_allocator<VectorXI> > Yfactor;

  VectorXI a;
	MatrixXI aU;
	MatrixXI aV;
	VectorXI gcds;

	vector<vector<wstring>> noteNames;
	vector<vector<wstring>> sharps;
	vector<vector<wstring>> flats;
	vector<Ratio> namingRatios;
  vector<double> accidentalCents;
  vector<double> accidentalWeights;

	void addToNamingRatioMap(wstring asciiName, Ratio r, wstring sharp, wstring flat);
	void addToNamingRatioMap(wstring asciiName, Ratio r);
	map<wstring, NamingRatio> namingRatioMap;
	vector<NamingRatio> inNamingRatios;
	VectorXI n;
	MatrixXI nU;
	MatrixXI nV;
  VectorXI gcdNs;
  std::vector<VectorXI, Eigen::aligned_allocator<VectorXI> > YNfactor;
  VectorXI m;
  MatrixXI mU;
  MatrixXI mV;
  VectorXI gcdMs;
	std::vector<VectorXI, Eigen::aligned_allocator<VectorXI> > YMfactor;
	bool justIntonation;
//  bool scientificPitchNotation = false;
//  bool use53TETNotation = false;
  vector<wstring> names12TET;
  vector<wstring> accidentals12TET;
  vector<wstring> names53TET;
  vector<wstring> accidentals53TET;
  vector<NamingRatio*> accidentals144TET;
  vector<NamingRatio*> accidentals72TET;

	static wstring temperamentsDir;
	wstring name;
	
	static Int gcd(MatrixXI& m, Index row);

	static void diagonalize(MatrixXI& U, MatrixXI& A, MatrixXI& V);// Given matrix A, calculates diagonal matrix B such  such that B = U A T
	static void reduceRow(Index p, MatrixXI& A, MatrixXI& U);
	static void reduceCol(Index p, MatrixXI& A, MatrixXI& U);
	static bool checkRow(Index p, MatrixXI& A);
	static bool checkCol(Index p, MatrixXI& A);
	
  bool reduceRatio(Ratio& in, Ratio& out);
	bool calculateRatio(Int gen, Int per, VectorXI& X);
	void minTenney(Index i, VectorXI& Y, VectorXI& X, double& minTenney);

  bool calculateName(VectorXI& N, VectorXI& X, double& minAcc);
  int calculateAccidental(VectorXI& N, Index i, bool allowZeroAccidentals = false);
  bool minAccidentals(Index i, VectorXI& Y, VectorXI& N, VectorXI& X, double& minAcc);
  bool minGenPer(VectorXI& M, VectorXI& X);
  bool minGenPer(Index i, VectorXI& Y, VectorXI& N, int& dist);
  bool calculateGenPer(int& gen, int& per, Ratio& r);
	void calcMatricies();
  void refreshNames(vector<wstring>& names);
  void refreshTuningAndRoot();

  void generateTemperament(double periodCents, int Cols, double EDO, double keySlope);

	void buildFraction(Ratio& r, VectorXI& x);

	task<bool> loadTemperamentAsync(StorageFolder^ folder);
	task<void> saveTemperament();

private:
	map<int, map<int, NoteName>> noteInfo;

public:
	Temperament();
	Concurrency::critical_section* criticalSection;
//	void setJustMapping(const Val& pers, const Val& gens);
	NoteName createNoteName(int gen, int period, bool duplicateRatios, bool showName, bool scientificPitchNotation, bool use53TETNotation);
	NoteName* borrowNoteName(int gen, int period, bool duplicateRatios, bool showName, bool scientificPitchNotation, bool use53TETNotation);
	void clearNoteNames();
  void clearNoteText();

  double getRootCents();
	double getCents(double gens, double per, bool duplicateRatios);
  double mapGenPerToCents(double gens, double pers);
  double frequencyFromCents(double cents);
  double centsFromFrequance(double freq);

	double genCents;
	double periodCents;
	double baseFreq;

  wstring fontName;
  int fontSize;

	StorageFolder^ temperamentFolder;
	StorageFolder^ preloadedTemperamentFolder;
	StorageFolder^ localTemperamentFolder;

public:
	vector<FileInfo> tempFiles;
};