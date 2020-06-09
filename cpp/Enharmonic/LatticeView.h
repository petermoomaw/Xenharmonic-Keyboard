#pragma once
#include "Lattice.h"
#include "Temperament.h"
#include "Instrument.h"
#include "Common.h"

#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <wchar.h>
#include <math.h>

#include <d2d1.h>
#include <d2d1_1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>
#include "..//MainPage.xaml.h"

////for ComPtr
//#include <wrl\client.h>


//#include <D3D11.h>

#include "..//DirectX/DirectXBase.h"

using namespace Microsoft::WRL;
using namespace Windows::Storage;

#define PI 3.14159265

//ref class SDKTemplate::MainPage;
class Enharmonic;
/*
//Declare additional functions for releasing interfaces and macros for error handling and retrieving the module's base address.
template<class Interface>
inline void SafeRelease(
	Interface **ppInterfaceToRelease
	)
{
	if (*ppInterfaceToRelease != NULL)
	{
		(*ppInterfaceToRelease)->Release();

		(*ppInterfaceToRelease) = NULL;
	}
}
*/

typedef enum PitchAxis
{
  BOTH,
  HORIZONTAL,
  VERTICAL

};

typedef enum AfterTouchMode
{
	SUSTAIN = 0,
  VIBRATO = 1,
  BEND = 2,
	GLISSANDO = 3,
};

typedef enum TouchMode
{
	TOUCH_DRAG,
	NONE,

	EDIT_ROTATE,
	EDIT_ROTATE_SCALE,
	EDIT_SCALE,
	EDIT_SQUEEZE,
	EDIT_SHEAR,
};

typedef enum TouchPhase
{
	TOUCH_DOWN,
	TOUCH_MOVE,
	TOUCH_UP,
	NOTHING
};

typedef enum ColorMode
{
  _12EDO_Black_White,
  _12EDO_Rainbow
};

class Note
{
public:
	double freq;
	int noteID;
	unsigned long long  startTime;
	unsigned long long  endTime;
	bool noteToMatch = true;
	double cents;
};

class TouchData
{
public:
	Eigen::Matrix<double, 2, 1, Eigen::DontAlign> latticeCoordsStart;
	Eigen::Matrix<double, 2, 1, Eigen::DontAlign> cellPos;
	Eigen::Matrix<double, 2, 1, Eigen::DontAlign> cellPosOrig;
	Eigen::Matrix<double, 2, 1, Eigen::DontAlign> bounderyPointStart;
	Eigen::Matrix<double, 2, 1, Eigen::DontAlign> dragVector;
	Vector2d* bounderyPoint = 0;
	Vector2d* intervalVector;

	unsigned int dwID;
	TouchPhase phase;
  Vector2d posStart;
  Vector2d pos;
  Vector2d prevMovePos;
  Vector2d prevVibratoPos;
	unsigned long long  prevTime;
	unsigned long long  time;
	unsigned long long killTime;
  bool permanent;

  NoteName* name;
	int midiNote;

	int noteID = -1;
	double cents;
	double origCents;
	double deltaCents;
  Vector2d deltaPitchPos;
	double trigMin;
	double trigMax;
//	bool dragHorizontal;
//	bool dragVertical;
//	bool dragConfirmed;
//	Vector velocity;
//	double speed;



	TouchData()
	{
		clear();
	};

	void clear()
	{
    permanent = false;
    midiNote = -1;
    name = 0;
		dwID = UINT_MAX;
		killTime = 0;
		phase = NOTHING;  //phase = 0 indicates that this touchData dosn't corrispond to any touch 
		latticeCoordsStart = Vector2d(0,0);
		cellPos = Vector2d(0,0);
		cellPosOrig = Vector2d(0, 0);
		posStart = Vector2d(0, 0);
		pos = Vector2d(0, 0);
		prevMovePos = Vector2d(0, 0);
		prevVibratoPos = Vector2d(0, 0);
    deltaPitchPos = Vector2d(0, 0);
		trigMin = 0;
		trigMax = 0;

		prevTime = 0;
		time = 0;

		intervalVector = 0;

		noteID = -1;
		origCents = -666666666666;
		cents = -666666666666;
		deltaCents = 0;

		//velocity = Vector(0, 0);
		//speed = 0;
		//dragHorizontal = false;
		//dragVertical = false;
		//dragConfirmed = false;
	};

	void updateTouchData(Windows::UI::Core::PointerEventArgs^ args, double scale)
	{
		prevMovePos = pos;
		prevTime = time;
		time = args->CurrentPoint->Timestamp;
		//time = GetTickCount64();

    pos(0) = args->CurrentPoint->Position.X/scale;
		pos(1) = args->CurrentPoint->Position.Y/scale;
	}
};


struct Intersects
{
	// coords of points where raster line intersects oversized view rectangle.
	double x1;
	double y1;
	double x2;
	double y2;
	int numberIntersections;  // number of intersection raster line makes with oversized view rectangle
};

ref class LatticeView sealed : public DirectXBase
{
public:
	LatticeView();
	virtual ~LatticeView();

	///////////////////////////////////////
	// These paramiters are to help deal with imperfect touch screens.
	// radius defines how big a touch point is.
	// If two touchpoints overlap, they are considered to be refering to the same note. This helps with situations with the thumb, for example,
	// where there is a large touch area which the touch screen might interpret incorectly as two finger touching.
	// Linger time is how long a touch point lingers after you release it. This helps when somtime a touch screen will incorectly think a finger has lifted off and then been pressed imediatly back down.
	// This can hppen in instances where the finger nail is inconcate with the touchscreen, somtime the touch screen in this case sees a "flikering" of up and down events.
	// radiusEscapeTime defines the minimum time a touch movment can move a distance of 2*radius and still be considered the same touch. THis is to deal with times when a a finger up folloed
	// by a finger down nearby is incorrectly interpreted as a touch move event, instead of a touch up and touch doum event.
	// If a touch screen is working perfectly, then all these parameters could be set to zerowith no ill effect, unfortonatly I have yet to see a perfect touchscreen.
internal:
	DWORD touchDiameter = 20;
	unsigned long long touchLingerDuration = 20;
	DWORD touchDiameterEscapeTime = 10;
	Concurrency::critical_section criticalSection;
	double scale = 1;
	///////////////////////////////////////////////
private:

	vector<Note> notes;
	vector<Note> notesToMatch;
  wstring missedNotes;
  int questions = 0;
  int correct = 0; 
internal:
  int octavesBelow = 2;
  int octavesAbove = 3;
  int duration = 500;
  int sequenceLength = 0;
  vector<int> instruments;
  wstring exerciseName;
  void clearEarTrainingExercise();
  void restartTrainingExercise();
  task<void> loadEarTrainingExerciseAsync(StorageFile^ file);
  int saveEarTrainingExercise();

private:
	double maxSpeed;
  bool initialized = false;

	//// There should be at least as many colors
	//// as there can be touch points so that you
	//// can have different colors for each point
	//COLORREF colors[] = { RGB(153,255,51),
	//	RGB(153,0,0),
	//	RGB(0,153,0),
	//	RGB(255,255,0),
	//	RGB(255,51,204),
	//	RGB(0,0,0),
	//	RGB(0,153,0),
	//	RGB(153, 255, 255),
	//	RGB(153,153,255),
	//	RGB(0,51,153)
	//};

	// For double buffering
	HDC memDC = 0;
	HBITMAP hMemBmp = 0;
	HBITMAP hOldBmp = 0;

	// For drawing / fills
	HDC hdc;

public:	// Initialize device-independent resources.
	virtual void CreateDeviceIndependentResources() override;

	// Initialize device-dependent resources.
	virtual void CreateDeviceDependentResources() override;
	 
	// Release device-dependent resource.
	void ReleaseDeviceDependentResources();

	virtual void CreateWindowSizeDependentResources() override;

public:
	// Draw content.
	void Render() override;
	// Resize the render target.
	void OnResize(UINT width, UINT height);

	unsigned int updateTouchTimes();

private:
	IDWriteTextFormat* m_pTextFormat;

	ID2D1SolidColorBrush* m_pWhiteBrush;
	ID2D1SolidColorBrush* m_pBlackBrush;
	ID2D1SolidColorBrush* m_pBorderBrush;
	ID2D1SolidColorBrush* m_pGeneratorBrush;
	ID2D1SolidColorBrush* m_pGlowBrush;
	ID2D1SolidColorBrush* m_pVerticiesBrush_1;
	ID2D1SolidColorBrush* m_pVerticiesBrush_2;
	ID2D1SolidColorBrush* m_pVerticiesBrush_3;
	ID2D1SolidColorBrush* m_pCornerBrush;
	ID2D1PathGeometry* m_pCellPathGeometry;  
	ID2D1PathGeometry* m_pCellVerticiesGeometry_1;
	ID2D1PathGeometry* m_pCellVerticiesGeometry_2;
	ID2D1PathGeometry* m_pCellVerticiesGeometry_3;
	ID2D1PathGeometry* m_pCellVerticiesGeometry_4;
	ID2D1PathGeometry* m_pCellVerticiesGeometry_5;
	ID2D1PathGeometry* m_pCellVerticiesGeometry_6;
	ID2D1PathGeometry* m_pArrowPathGeometry;
  IDWriteFontCollection *customFonts;

internal:
  SDKTemplate::MainPage^ mainPage;
	Lattice lattice;
	Temperament temperament;
	bool cellPathValid = false;
	Instrument instrument;

	TouchMode mode;
	bool playMusic = true;
  bool oneTouchEdit = false;
	bool twoTouchEdit;
	bool threeTouchEdit;
	Vector2d largeGen1;
	Vector2d largeGen2;

public:
	void invalidateCellPath();
  void invalidateBitmap();
private:
  D2D1::ColorF backgroundColor;
	void DrawLatticeBitmap();
	HRESULT CreateCellPath();
	HRESULT CreateArrowHead();
	HRESULT renderLattice(const RECT& invalidRect);
	HRESULT renderRows(double bInc, const RECT& bounds, const double& m, double& b);
	HRESULT renderRasterLines(double bInc, const RECT& bounds, const double& m, double& b);
	void renderArrows();
	void renderSmallGens();
	void calculateLineRectangleIntersection(const RECT& bounds, Intersects& intersects, const double& m, const double& b);
	int getGenPer(Vector2d& pos);
	HRESULT drawCell(double x, double y);
	double getCents(double x, double y);
	double getDeltaCents(double dx, double dy);
  ID2D1SolidColorBrush* LatticeView::BrushFromCents(double cents);
  ID2D1SolidColorBrush* getWhiteBrushFromCents(double cents);
  ID2D1SolidColorBrush* getBlackBrushFromCents(double cents);
  map<int, ID2D1SolidColorBrush*> whiteBrushes;
  map<int, ID2D1SolidColorBrush*> blackBrushes;

	void startPlayingNote(TouchData* td);
//	void updateFrequancy(TouchData* touchData);
	void updateVibratoFreq(TouchData* touchData);
  void updateBendFreq(TouchData* touchData);
  void updateBend(TouchData* touchData);
	void updateGlissando(TouchData* touchData);
	void stopPlayingNote(TouchData* touchData);
	void startKillTimer(TouchData* touchData);

	//	void ProcessTouch(HWND hwnd, WPARAM wParam, LPARAM lParam);
	TouchData* newTouchDown(Windows::UI::Core::PointerEventArgs^  args);
//	void updateTouchData(HWND hwnd, PTOUCHINPUT pInputs, UINT cInputs);

	public:
	void stopPlayingAllNotes();
	void touchUp(Windows::UI::Core::PointerEventArgs^  args);
	void touchDown(Windows::UI::Core::PointerEventArgs^  args);
	void touchMoved(Windows::UI::Core::PointerEventArgs^  args);
	
private:
	void drag(TouchData* touchData);
  bool maybeUpdateLattice(Vector2d& v1, Vector2d& v2);
	bool maybeUpdateLattice(double x1, double y1, double x2, double y2);

internal:
	double originX;
	double originY;
private:
	double originXDraw;
	double originYDraw;

	vector<TouchData> touchDataArray;
	TouchData* newTouchData(unsigned int dwID);
	TouchData* getTouchData(unsigned int dwID);

  //Eigen::Matrix<double, 2, 1, Eigen::DontAlign> selectedCellPos;
	Vector2d intervalOrigin;
internal:
	TouchData* intervalOriginTouch;
private:
	Vector2d touchBegin_latticeGen1;
	Vector2d touchBegin_latticeGen2;

	Enharmonic* enharmonic;

internal:
	Vector2d generatorVec;
	Vector2d periodVec;
	Vector2d keyDuplicateVec;

	wstring keyboardName;
	StorageFolder^ keyboardFolder;
	StorageFolder^ preloadedKeyboardFolder;
  StorageFolder^ localKeyboardFolder;
  StorageFolder^ localEarTrainingFolder;
  StorageFolder^ oldPreloadedFolder;
	bool dupKeys = true;

  bool scientificPitchNotation = false;
  bool use53TETNotation = false;
  bool showName = true;
  bool showCents = true;
  bool showRatio = true;
  bool showDelta = true;
  bool duplicateRatios = true;
  bool playUnamedNotes = false;
	private:

	Microsoft::WRL::ComPtr<ID2D1Bitmap1> pathBitMap;
	Microsoft::WRL::ComPtr<ID2D1Effect>  m_pathBlurEffect;
//	Microsoft::WRL::ComPtr<ID2D1Bitmap1> whiteCellBitMap;
//	Microsoft::WRL::ComPtr<ID2D1Bitmap1> blackCellBitMap;
	Microsoft::WRL::ComPtr<ID2D1Bitmap1> touchBitMap;
	Microsoft::WRL::ComPtr<ID2D1Effect>  m_touchBlurEffect;
	float touchBitMapWidth;
	float touchBitMapHeight;
	Microsoft::WRL::ComPtr<ID2D1Bitmap1> latticeBitMap;
  D2D1_SIZE_U prevWidowSizeInPixels = D2D1::SizeU(0,0);
	bool latticeBitMapValid;

internal:
  void deleteDuplicateFiles(StorageFolder^ folder1, StorageFolder^ folder2);
	task<void> loadKeyboardAsync(StorageFile^ file);
	void loadVectorList(LPTSTR& token, LPTSTR& next_token, Vector2dList& list);
  task<void> saveKeyboard();

	///pitchbent
	double vibratoCuttoff;  
  double bendCuttoff;
	double vibratoAmplitude;      // in cents;
	bool vibratoTrigger;
	double triggerResetRate;
	double triggerThreshold;
	bool roundFirstNote;
	bool usePitchAxis;
  AfterTouchMode afterTouchMode;
  bool dragKeyboard = false;
  PitchAxis pitchAxis;
  double textSize = 1;

	private:
		LARGE_INTEGER StartingTime, EndingTime, Freq;
		double deltaT = 0;

    vector<Note> currentSequance;
public:
    void playRandomNotes();
    void clearTouchData();
    double getBestKeySlope(double periodInCents, int Cols, double EDO);
    double getBestKeySlopeEDO(double periodInCents, int Cols, double EDO);
    void generateKeyboard(double newEDO, bool resizeKey = false);
    double getMaxY(double periodInCents, int Cols, double EDO, double slope);
    void mapRatios();
//    Windows::UI::Color ColorFromCents(double cents);
//    void UpdateColor(Windows::UI::Color c);

internal:
// Automatic keyboard stuff
   double keyHeight12EDO;
   double keySlope;
   int columns=7;
   int minEDO = 12;
   int maxEDO = 72;
   double EDO = 12;
   double periodInCents = 1200;
   bool lockKeyPattern = false;
   bool fixedYCentsScale = false;
   bool dynamicTemperament = true;
   bool EDOs = false;
   double shiftPercent = 0;
   vector<Ratio> fitRatios;
   double saturation = 1;
   double lightness = .501;
   double a = 0; // a must be between 0 and 2.
   double c = 0; // c must be between 0 and 1. .25 is probobly around where it should be.
   double shift = 0; // c must be between 0 and 1. .25 is probobly around where it should be.
   bool updateColor = false;
   double colorCents = 0;
   ColorMode colorMode = _12EDO_Rainbow;

   int colorsBlackAndWhite[24][3] =
   {
     { 255,255,255 }

     ,{ 127,127,127 }

     ,{ 255,255,255 }

     ,{ 127,127,127 }

     ,{ 255 ,255,255 }

     ,{ 127,127,127 }

     ,{ 255, 255, 255 }//A

     ,{ 127,127,127 }

     ,{ 255,255,255 }  //E

     ,{ 127,127,127 }

     ,{ 255, 255, 255 } //B 

     ,{ 127,127,127 }

     ,{ 0,0,0 }

     ,{ 127,127,127 }

     ,{ 0,0,0 }

     ,{ 127,127,127 }

     ,{ 0,0, 0 }

     ,{ 127,127,127 }

     ,{ 0,0,0 }

     ,{ 127,127,127 }

     ,{ 0,0,0 }

     ,{ 127,127,127 }

     ,{ 255,255,255 }

     ,{ 127,127,127 }
   };

   // Ranbow
   int colorsRanbow[12][3] =
   {
     { 252,255,1 }

     ,{ 132,255,0 }

     ,{ 0 ,255,127 }

     ,{ 0, 255, 213 }//A

     ,{ 0,177,255 }  //E

     ,{ 1, 113, 255 } //B 

     ,{ 80,0,255 }

     ,{ 167,0,255 }

     ,{ 255,0, 176 }

     ,{ 255,0,0 }

     ,{ 255,85,0 }

     ,{ 255,170,0 }
   };


/*
   // Ranbow
   int colors[24][3] =
   {
     { 252,255,1 }
     ,{ 153,99,157 }

     ,{ 132,255,0 }
     ,{ 111,78,124 }

     ,{ 0 ,255,127 }
     ,{ 169,139,170 }

     ,{ 0, 255, 213 }//A
     ,{ 176,127,123 }

     ,{ 0,177,255 }  //E
     ,{ 203,155,128 }

     ,{ 1, 113, 255 } //B 
     ,{ 181,174,114 }

     ,{ 80,0,255 }
     ,{ 124,171,107 }

     ,{ 167,0,255 }
     ,{ 135,190,146 }

     ,{ 255,0, 176 }
     ,{ 121,150,130 }

     ,{ 255,0,0 }
     ,{ 114,163,163 }

     ,{ 255,85,0 }
     ,{ 75,150,198 }

     ,{ 255,170,0 }
     ,{ 129,132,221 }
   };


 //black and white
   int colors[12][3] =
   {
     { 255,255,255 }

     ,{ 255,255,255 }

     ,{ 255 ,255,255 }

     ,{ 255, 255, 255 }//A

     ,{ 255,255,255 }  //E

     ,{ 255, 255, 255 } //B 

     ,{ 0,0,0 }

     ,{ 0,0,0 }

     ,{ 0,0, 0 }

     ,{ 0,0,0 }

     ,{ 0,0,0 }

     ,{255,255,255 }
   };


*/












/*
   // made with color chooser version 1
   int colors[12][3] =
   {
     {253,46,46},
     {239,141,12},
     {239,236,0},
     {121,254,0},
     {1,252,117},
     {1,252,223},
     {0,190,249},
     {9,116,255},
     {84,43,247},
     {127,18,255},
     {204,25,252},
     {255,6,156}
   };
*/

/*
   // made with color chooser version 2 messed up
   int colors[12][3] =
   {
     {250,49,39},
     {254,178,46},
     {238,235,0},
     {100,230,57}, 
     {0,252,117},
     {1,252,223},
     {74,188,26},
     {7,114,225},
     {43,39,246},
     {136,37,252},
     {241,40,16},
     {253,15,131},
   };
*/
////////////////////////

//   int colors[36][3] =
//   {
//      {255,0,0 }    
//   
//     ,{255,85,0 }    
// 
//     ,{255,170,0 }     // F
//  
////     ,{255,255,0 }     // C
//     ,{ 252,255,1 }     // C
//
//     ,{ 132,255,0}  
////     ,{ 88,255,0 }
//
//     ,{ 0 ,255,127 }  
////     ,{ 0,255, 85 }
//
//     ,{0, 255, 213}//A
// //    ,{0,255,255 }  //A  
//                   
//      
////     ,{ 0,219,255 } //E
//     ,{0,177,255 }  //E
//           
//  //   ,{0,134,255 } //B
//  //   ,{ 0, 113, 255 } //B 
//     ,{ 1, 113, 255 } //B 
//  //   ,{ 0, 104, 255 } //B 
//  //   ,{0, 92, 255 } //B 
//    // ,{ 0, 49, 255 }  //B
//
////     ,{ 0,0,255 }
//     ,{80,0,255 }  
//
////     ,{123,0,255}
//     ,{167,0,255 }  
//
//
//     ,{255,0, 176 }  
//   };


/*
int colors[36][3] =
{
 { 0,168,44 }        //green     140
,{ 106,191,0 }        //yellow-green      120
,{198,205,0}          //yellow-green     100
,{ 255, 223 , 0   }   //yellow     090
,{ 255, 157 , 0   }  //orange     070
,{ 255, 44 , 28   }  //red        040'
,{ 200, 39 , 53 }  //brick red 030
//,{ 187, 0 , 61 }  //brick red 020
//,{ 188, 0 , 96 }     //hot pint   360
//,{ 172, 25 , 131 }  //pink       340
,{ 159, 43 , 149 }  //pink       330
//,{ 125, 69 , 168 }  //purple       310
,{ 104, 76 , 186 }  //purple       300
,{ 55, 74 , 181 }    //purple-blue      290
,{ 0, 107 , 187 }    //blue      250
,{ 0, 145 , 117 }    //blue-green      180
};
*/

/*
int colors[36][3] =
{
  { 254, 252 , 1 }
  ,{ 167, 249 , 2 }
  ,{ 1  , 165 , 68 }
  ,{ 0  , 120 , 170 }
  ,{ 8  , 48  , 255 }
  ,{ 123, 45  , 216 }
  ,{ 125, 35  , 168 }
  ,{ 170, 31  , 120 }
  ,{ 255, 38  , 0 }
  ,{ 254, 125 , 1 }
  ,{ 255, 168 , 0 }
  ,{ 254, 210 , 7 }
};
*/

/*
   int colors[36][3] =
   { 
        { 247,	0,	161 },
     //   { 246,	27,	136 },

//   { 248,	29,	112 },
//   { 252,	51,	88 },
   { 252,	30,	60 },

//   { 255,	16,	13 },  //Red
//   { 250,	87,	0 },
//   { 243,	123,	0 },

   { 255,	163,	0 },
//   { 249,	187,	21 },
   { 255,	224,	32 },  //yellow

 //  { 245,	247,	24 },
   { 215,	255,	13 },
//   { 166,	249,	29 },

   { 106,	255,	34 },
//   { 25,	245,	83 },
//   { 0,	244,	131 },
   { 0,	243,	163 },
//   { 48,	255,	205 },
//   { 46,	255,	224 },
   { 64,	253,	242 },
//   { 47,	252,	255 },
//   { 67,	234,	253 },

//   { 53,	217,	251 },
//   { 44,	201,	246 },
   { 0,	187,	249 },


//   { 52,	183,	255 },
//   { 44,	166,	255 },
//   { 0,	136,	245 },

   { 14,	117,	248 }
//   { 0,	81,	245 }
//   ,{ 0,	21,	250 }

   ,{ 155,	33,	255 }
   ,{ 203,	0,	244 }
//   ,{ 255,	18,	239 }

//   ,{ 255,	42,	202 }
//   ,{ 247,	0,	161 },
//   ,{ 246,	27,	136 }
   };
*/
};

