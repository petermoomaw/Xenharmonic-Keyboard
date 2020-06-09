#pragma once
#include <vector>

#include "Common.h"
#include <eigen/Eigen/Dense>
#include <eigen/Eigen/StdVector>
//using namespace Eigen;
using Eigen::Vector2d;

using namespace std;
#define cross(v1,v2) ((v1)(0) * (v2)(1) - (v1)(1) * (v2)(0))
typedef vector<Vector2d, Eigen::aligned_allocator<Eigen::Vector2d>> Vector2dList;

typedef enum KeyMode
{
	VORONOI,
	RECTANGLE,
	CUSTOM,
} KeyMode;

class Lattice
{
public:
  Vector2d right;
  Vector2d down;
  Vector2d downShift;
  Vector2dList textPos;
  Vector2dList textCustomLoc;
  Vector2dList cellVerticies1;
  Vector2dList cellVerticies2;
  Vector2dList cellVerticies3;
  vector<Vector2d*> cellBounderyPoints;
  Vector2dList cellBounderyDown;
  Vector2d largeGen1;
  Vector2d largeGen2;
  Vector2d gen1;
  Vector2d gen2;
  Vector2dList cellVerticies;
  Vector2dList cellVerticiesDraw;
  double cellHeight;
  double cellWidth;
  double cellMinX;
  double cellMaxX;
  double cellMinY;
  double cellMaxY;
//  Vector2d cellShift;
  KeyMode keyMode;

  double overhang = 0;
  double bevel1 = 0;
  double bevel2 = 0;

  Lattice();

  void latticeToCartesian(double* x, double* y);
  static void latticeToCartesian(Vector2d& v, Vector2d* b1, Vector2d* b2);
  static void latticeToCartesian(Eigen::Matrix<double, 2, 1, Eigen::DontAlign>& v, Vector2d* b1, Vector2d* b2);
  
  static void latticeToCartesian(double* x, double* y, Vector2d* b1, Vector2d* b2);

  void cartesianToLattice(double* x, double* y);
  static void cartesianToLattice(double* x, double* y, Vector2d* b1, Vector2d* b2);
  static void changeToBasis(Vector2d* v, Vector2d* b1, Vector2d* b2);

  void getClosestLatticePoint(double* x, double* y);
  void getCell(double* x, double* y);
  void setBasis(const Vector2d& g1, const Vector2d& g2, KeyMode keyMode);
  void setBasis(const Vector2d& g1, const Vector2d& g2);
  void calculateCellShape();
  int doesCellContainPoint(double latticeX, double latticeY, double x, double y);
  double getDistanceSquaredFromLineSegment(Vector2d* v, Vector2d* w, Vector2d* p);
  Vector2d* getBounderyPoint(double x, double y);
  double projectPointOntoLineSegment(Vector2d* end1, Vector2d* end2, Vector2d* point, Vector2d* newVertex);

  bool eraseBounderyPoint(Vector2d* p);
  bool maybeRemoveBounderyPoint(Vector2d* p);    ///returns true if boundery point is removed
  void snapToPosition(double& value, double threshold);

private:
	void calulateRectangleCell();
	void calulateCustomCell();
//	void calulateParallelogramCell();
//	void calulateHexagonCell();
	void calulateVoronoiCell();
	void calulateVoronoiCell(Vector2dList& closestPoints, Vector2dList& verticies);
	void reduceBasis();
	void getClosestPoints(Vector2dList& closestPoints);

	//void insetPolygon(Vector2dList& p, double insetDist);
	//void insetCorner(
	//	double  a, double  b,   //  previous point
	//	double  c, double  d,   //  current point that needs to be inset
	//	double  e, double  f,   //  next point
	//	double *C, double *D,   //  storage location for new, inset point
	//	double insetDist);

	//bool lineIntersection(
	//	double Ax, double Ay,
	//	double Bx, double By,
	//	double Cx, double Cy,
	//	double Dx, double Dy,
	//	double *X, double *Y);
};
