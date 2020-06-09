#include "pch.h"
#include "Lattice.h"

#define cos(v1,v2) ((v1).dot(v2)/(v1).norm()/(v2).norm())

#define norm2(x,y) ((x)*(x) + (y)*(y)) 
//double norm2(double x, double y) { return (x*x + y*y) };


Lattice::Lattice()
{
	this->keyMode = VORONOI;
	//this->keyMode = RECTANGLE;

	textCustomLoc.clear();
	textCustomLoc.push_back(Vector2d(0, 0));
//	textCustomLoc.push_back(Vector2d(.5, .5));
//	textCustomLoc.push_back(Vector2d(-.5, .5));

	/////// minimum two verticies which form a square
		//cellVerticies1.push_back(Vector2d(.5, 1));
		//cellVerticies2.push_back(Vector2d(1, 0));
	////


	//cellVerticies1.push_back(Vector2d(.4, 0));
	//cellVerticies1.push_back(Vector2d(-.2, .2));
	//cellVerticies1.push_back(Vector2d(-.2, .6));
	//cellVerticies2.push_back(Vector2d(.1, .5));
	//cellVerticies2.push_back(Vector2d(-.2, 1.1));


	//cellVerticies1.push_back(Vector2d(0, .25));
	//cellVerticies1.push_back(Vector2d(0, .5));
	//cellVerticies1.push_back(Vector2d(0, .75));
	//cellVerticies1.push_back(Vector2d(0, 1));
	//cellVerticies2.push_back(Vector2d(0, .25));
	//cellVerticies2.push_back(Vector2d(0, .5));
	//cellVerticies2.push_back(Vector2d(0, .75));
	//cellVerticies2.push_back(Vector2d(0, 1));
}

void Lattice::setBasis(const Vector2d& g1, const Vector2d& g2, KeyMode keyMode)
{
	this->keyMode = keyMode;
	setBasis(g1, g2);
}

void Lattice::setBasis(const Vector2d& g1, const Vector2d& g2)
{
  this->largeGen1 = g1;
  this->largeGen2 = g2;
  this->gen1 = g1;
  this->gen2 = g2;
}

void Lattice::calculateCellShape()
{
	if (keyMode == VORONOI)
		calulateVoronoiCell();
	else if (keyMode == CUSTOM)
		calulateCustomCell();
	else if(keyMode == RECTANGLE)
		calulateRectangleCell();
	//else if (keyMode == PARALLELOGRAM)
	//	calulateParallelogramCell();

}


void Lattice::calulateRectangleCell()
{
	reduceBasis();
	textPos.clear();
	textPos.push_back(Vector2d(0, 0));

	this->cellVerticiesDraw.clear();
	this->cellVerticies.clear();
	this->cellVerticies.resize(4);

  {if(largeGen1.norm() == 0) Throw(L"Fatal error: largeGen1.norm() == 0 in calulateRectangleCell");}
	Vector2d hat1 = largeGen1 / largeGen1.norm();
	this->cellVerticies[0] = Vector2d(0, 0);
	this->cellVerticies[1] = largeGen2 - largeGen2.dot(hat1) * hat1;
	this->cellVerticies[2] = this->cellVerticies[1] + largeGen1;
	this->cellVerticies[3] = largeGen1;
	

	double maxX = 0;
	double maxY = 0;

	Vector2d shift = 0.5 * (this->cellVerticies[1] + this->cellVerticies[3]);
	for (unsigned int i = 0; i < this->cellVerticies.size(); i++)
	{
		this->cellVerticies[i] = this->cellVerticies[i] - shift;
		maxX = this->cellVerticies[i](0) > maxX ? this->cellVerticies[i](0) : maxX;
		maxY = this->cellVerticies[i](1) > maxY ? this->cellVerticies[i](1) : maxY;

    {if (this->cellVerticies[i].norm() == 0) Throw(L"Fatal error: this->cellVerticies[i].norm() == 0 in calulateRectangleCell");}
		this->cellVerticiesDraw.push_back(this->cellVerticies[i] - 2*this->cellVerticies[i]/this->cellVerticies[i].norm());
	}

	this->cellWidth = 2 * maxX;
	this->cellHeight = 2 * maxY;
	this->cellMinX = -maxX;
	this->cellMaxX = maxX;
	this->cellMinY = -maxY;
	this->cellMaxY = maxY;
}




void Lattice::calulateCustomCell()
{
	if (cellVerticies1.size() == 0 || cellVerticies2.size() == 0)
	{
		cellVerticies1.clear();
		cellVerticies2.clear();
		cellVerticies3.clear();

    {if (largeGen1.norm() == 0) Throw(L"Fatal error: this->cellVerticies[i].norm() == 0 in calulateCustomCell");}
		Vector2d hat1 = largeGen1 / largeGen1.norm();
		right = largeGen2 - largeGen2.dot(hat1) * hat1;

		down = largeGen1;
		downShift = largeGen2.dot(hat1) * hat1;

		while (downShift.dot(largeGen1) < 0)
			downShift = downShift + down;

		while (downShift.norm() > down.norm())
			downShift = downShift - down;

		largeGen2 = right + downShift;

		Vector2d p = right/2 - down/2 + downShift;
		Lattice::cartesianToLattice(&p(0), &p(1), &largeGen2, &largeGen1);
		cellVerticies1.push_back(p);

		p = right/2 + down/2;
		Lattice::cartesianToLattice(&p(0), &p(1), &largeGen2, &largeGen1);
		cellVerticies2.push_back(p);
	}

	reduceBasis();
	textPos = textCustomLoc;

	this->cellVerticiesDraw.clear();
	this->cellVerticies.clear();
	this->cellBounderyPoints.clear();

	Vector2dList& p1 = cellVerticies1;
	Vector2dList& p2 = cellVerticies2;
	Vector2dList& p3 = cellVerticies3;

	Vector2d v1 = largeGen1;
	Vector2d v2 = largeGen2;

	this->cellVerticies.push_back(-v1 + p2.back()(0)*v2 + p2.back()(1) * v1);
	this->cellBounderyPoints.push_back(&p2.back());

	for (int i = 0; i < p1.size(); i++)
	{
		this->cellVerticies.push_back(p1[i](0)*v2 + p1[i](1)*v1);
		this->cellBounderyPoints.push_back(&p1[i]);
	}

	for (int i = 0; i < p2.size(); i++)
	{
		this->cellVerticies.push_back(p2[i](0)*v2 + p2[i](1)*v1);
		this->cellBounderyPoints.push_back(&p2[i]);
	}

	for (int i = 0; i < p3.size(); i++)
	{
		this->cellVerticies.push_back(p3[i](0)*v2 + p3[i](1)*v1);
		this->cellBounderyPoints.push_back(&p3[i]);
	}

	for (int i = p1.size() - 1; i >= 0; i--)
	{
		this->cellVerticies.push_back(v1-v2 +p1[i](0)*v2 + p1[i](1)*v1);
		this->cellBounderyPoints.push_back(&p1[i]);
	}

	for (int i = p2.size() - 1; i >= 0; i--)
	{
		this->cellVerticies.push_back(-v2 + p2[i](0)*v2 + p2[i](1)*v1);
		this->cellBounderyPoints.push_back(&p2[i]);
	}

	this->cellVerticies.push_back(-v2 + p1.back()(0)*v2 + p1.back()(1)*v1);
	this->cellBounderyPoints.push_back(&p1.back());

	for (int i = p3.size() - 1; i >= 0; i--)
	{
		this->cellVerticies.push_back(-v1+p3[i](0)*v2 + p3[i](1)*v1);
		this->cellBounderyPoints.push_back(&p3[i]);
	}

	double maxX = this->cellVerticies[0](0);
	double maxY = this->cellVerticies[0](1);
	double minX = this->cellVerticies[0](0);
	double minY = this->cellVerticies[0](1);

	Vector2d* prev = &this->cellVerticies[0];
	for (unsigned int i = 1; i < this->cellVerticies.size();)
	{
		//double dx = this->cellVerticies[i](0) - prev(0);
		//double dy = this->cellVerticies[i](1) - prev(1);
		//if (dx*dx + dy*dy < 0.0001)
		//	this->cellVerticies.erase(this->cellVerticies.begin() + i);
		//else
		{
			maxX = this->cellVerticies[i](0) > maxX ? this->cellVerticies[i](0) : maxX;
			maxY = this->cellVerticies[i](1) > maxY ? this->cellVerticies[i](1) : maxY;
			minX = this->cellVerticies[i](0) < minX ? this->cellVerticies[i](0) : minX;
			minY = this->cellVerticies[i](1) < minY ? this->cellVerticies[i](1) : minY;

			prev = &this->cellVerticies[i];
			i++;
		}
	}

	//{
	//	double dx = this->cellVerticies[0](0) - prev(0);
	//	double dy = this->cellVerticies[0](1) - prev(1);
	//	if (dx*dx + dy*dy < 0.0001)
	//		this->cellVerticies.pop_back();
	//}

	this->cellWidth = maxX - minX;
	this->cellHeight = maxY - minY;

	this->cellMinX = minX;
	this->cellMaxX = maxX;
	this->cellMinY = minY;
	this->cellMaxY = maxY;

	for (unsigned int i = 0; i < this->cellVerticies.size(); i++)
	{
		this->cellVerticies[i] = this->cellVerticies[i];
		this->cellVerticiesDraw.push_back(this->cellVerticies[i]);
	}
}


//void Lattice::insetPolygon(Vector2dList& p, double insetDist)
//{
//	if (p.size() < 3)
//		return;
//
//	if (cross(p[0], (p[1] - p[0])) < 0)
//	  insetDist *= -1;
//
//	double  startX = p[0](0), startY = p[0](1), a, b, c, d, e, f;
//	int     i;
//
//	//  Inset the polygon.
//	c = p[p.size() - 1](0); 
//	d = p[p.size() - 1](1);
//	e = p[0](0);
//	f = p[0](1);
//
//	for (i = 0; i<p.size() - 1; i++)
//	{
//		a = c; b = d; c = e; d = f; e = p[i + 1](0); f = p[i + 1](1);
//		insetCorner(a, b, c, d, e, f, &p[i](0), &p[i](1), insetDist);
//	}
//	insetCorner(c, d, e, f, startX, startY, &p[i](0), &p[i](1), insetDist);
//}
//
//void Lattice::insetCorner(
//	double  a, double  b,   //  previous point
//	double  c, double  d,   //  current point that needs to be inset
//	double  e, double  f,   //  next point
//	double *C, double *D,   //  storage location for new, inset point
//	double insetDist) {     //  amount of inset (perpendicular to each line segment)
//
//	double  c1 = c, d1 = d, c2 = c, d2 = d, dx1, dy1, dist1, dx2, dy2, dist2, insetX, insetY;
//
//	//  Calculate length of line segments.
//	dx1 = c - a; dy1 = d - b; dist1 = sqrt(dx1*dx1 + dy1*dy1);
//	dx2 = e - c; dy2 = f - d; dist2 = sqrt(dx2*dx2 + dy2*dy2);
//
//	//  Exit if either segment is zero-length.
//	if (dist1 == 0. || dist2 == 0.) return;
//
//	//  Inset each of the two line segments.
//	insetX = dy1 / dist1*insetDist; a += insetX; c1 += insetX;
//	insetY = -dx1 / dist1*insetDist; b += insetY; d1 += insetY;
//	insetX = dy2 / dist2*insetDist; e += insetX; c2 += insetX;
//	insetY = -dx2 / dist2*insetDist; f += insetY; d2 += insetY;
//
//	//  If inset segments connect perfectly, return the connection point.
//	if (c1 == c2 && d1 == d2) {
//		*C = c1; *D = d1; return;
//	}
//
//	//  Return the intersection point of the two inset segments (if any).
//	if (lineIntersection(a, b, c1, d1, c2, d2, e, f, &insetX, &insetY)) {
//		*C = insetX; *D = insetY;
//	}
//}
//
////  Determines the intersection point of the line defined by points A and B with the
////  line defined by points C and D.
////
////  Returns YES if the intersection point was found, and stores that point in X,Y.
////  Returns NO if there is no determinable intersection point, in which case X,Y will
////  be unmodified.
//
//bool Lattice::lineIntersection(
//	double Ax, double Ay,
//	double Bx, double By,
//	double Cx, double Cy,
//	double Dx, double Dy,
//	double *X, double *Y) {
//
//	double  distAB, theCos, theSin, newX, ABpos;
//
//	//  Fail if either line is undefined.
//	if (Ax == Bx && Ay == By || Cx == Dx && Cy == Dy) return false;
//
//	//  (1) Translate the system so that point A is on the origin.
//	Bx -= Ax; By -= Ay;
//	Cx -= Ax; Cy -= Ay;
//	Dx -= Ax; Dy -= Ay;
//
//	//  Discover the length of segment A-B.
//	distAB = sqrt(Bx*Bx + By*By);
//
//	//  (2) Rotate the system so that point B is on the positive X axis.
//	theCos = Bx / distAB;
//	theSin = By / distAB;
//	newX = Cx*theCos + Cy*theSin;
//	Cy = Cy*theCos - Cx*theSin; Cx = newX;
//	newX = Dx*theCos + Dy*theSin;
//	Dy = Dy*theCos - Dx*theSin; Dx = newX;
//
//	//  Fail if the lines are parallel.
//	if (Cy == Dy) return false;
//
//	//  (3) Discover the position of the intersection point along line A-B.
//	ABpos = Dx + (Cx - Dx)*Dy / (Dy - Cy);
//
//	//  (4) Apply the discovered position to line A-B in the original coordinate system.
//	*X = Ax + ABpos*theCos;
//	*Y = Ay + ABpos*theSin;
//
//	//  Success.
//	return true;
//}

void Lattice::calulateVoronoiCell()
{
	reduceBasis();
	textPos.clear();
	textPos.push_back(Vector2d(0,0));

	Vector2dList closestPoints;
	getClosestPoints(closestPoints);

	Vector2dList closestPointsDraw;
	for (unsigned int i = 0; i < closestPoints.size(); i++)
	{
    {if (closestPoints[i].norm() == 0) Throw(L"Fatal error: closestPoints[i].norm() == 0 in calulateVoronoiCell");}
		closestPointsDraw.push_back(closestPoints[i] - 2*closestPoints[i]/closestPoints[i].norm());
	}

	calulateVoronoiCell(closestPointsDraw, this->cellVerticiesDraw);
	calulateVoronoiCell(closestPoints, this->cellVerticies);
}

void Lattice::calulateVoronoiCell(Vector2dList& closestPoints, Vector2dList& verticies)
{
  // y = m * x + b
  // where m = - v(0)/v(1)
  // setting y = v(1) and x = v(0) gives
  // b = v(1) - m * v(0)
  // thus
  // y = m * x + v(1) - m
  // is the equation for the line perpendiclar to a vector
  // which goes though that vector.

  // Using Wolfrum alpha (I'm lazy) we find that the solution to
  //   y = m1 x + b1 and y = m2 x + b2
  // is
  //   m1-m2!=0,  x = (b2-b1)/(m1-m2),   y = (b2 m1-b1 m2)/(m1-m2)
  // This gives us our vertices for our Voronio cell.
  // Just need to loop through all pairs of closest vectors halved
 
  int length = closestPoints.size();
  verticies.clear();

  double m1, m2, b1, b2;
  Vector2d* v = &closestPoints[length - 1];
  
  if((*v)(1) != 0)
  {
    m1 = -(*v)(0)/(*v)(1);
    b1 = ((*v)(1) - m1 * (*v)(0))/2;
  }
  else
  {
    m1 = INFINITY;
    b1 = (*v)(0)/2;
  }
  
  double maxX = 0;
  double maxY = 0;
  
  for(int i = 0; i < length; i++)
  {
    v = &closestPoints[i];
    
    if((*v)(1) != 0)
    {
      m2 = -(*v)(0)/(*v)(1);
      b2 = ((*v)(1) - m2 * (*v)(0))/2;
    }
    else
    {
      m2 = INFINITY;
      b2 = (*v)(0)/2;
    }
    
    double x;
    double y;
    
    if(m1 == INFINITY)
    {
      x = b1;
      y = m2 * b1 + b2;
    }
    else if(m2 == INFINITY)
    {
      x = b2;
      y = m1 * b2 + b1;
    }
    else
    {
      x = (b2-b1)/(m1-m2);
      y = (b2*m1 - b1*m2)/(m1-m2);
    }
    
	verticies.push_back(Vector2d(x,y));
    
    if(x > maxX)
      maxX = x;
    
    if(y > maxY)
      maxY = y;
    
    m1 = m2;
    b1 = b2;
  }
  
  this->cellWidth = 2*maxX;
  this->cellHeight = 2*maxY;
  this->cellMinX = -maxX;
  this->cellMaxX = maxX;
  this->cellMinY = -maxY;
  this->cellMaxY = maxY;
}

void Lattice::reduceBasis()
{
  Vector2d u;
  Vector2d v;
  
  if(this->gen1.norm() <= this->gen2.norm())
  {
    u = this->gen1;
    v = this->gen2;
  }
  else
  {
    u = this->gen2;
    v = this->gen1;
  }
  
  while (2 * fabs(u.dot(v)) > u.norm()*u.norm())
  {
    Vector2d temp = v - u;
    
    if(temp.norm() < v.norm())
    {
      while(temp.norm() < v.norm())
      {
        v = temp;
        temp = v - u;
      }
    }
    else
    {
      temp = u + v;
      
      while(temp.norm() < v.norm())
      {
        v = temp;
        temp = u + v;
      }
    }
    
    if(u.norm() <= v.norm())
    {
      break;
    }
    else
    {
      temp = u;
      u = v;
      v = temp;
    }
  }
  
  this->gen1 = u;
  this->gen2 = v;
}

void Lattice::getClosestPoints(Vector2dList& closestPoints)
{    
	Vector2d& v1 = this->gen1;
	Vector2d& v2 = this->gen2;

    if(! cross(v1,v2))
    {
		return;
    }
    
    if(cos(v1,v2) == 0)
    {        
		closestPoints.push_back(v1);
		closestPoints.push_back(-v2);
		closestPoints.push_back(-v1);
		closestPoints.push_back(v2);
    }
    else if(cos(v1,v2) > 0)
    {
		closestPoints.push_back(v1);
		closestPoints.push_back(v1 - v2);
		closestPoints.push_back(-v2);
		closestPoints.push_back(-v1);
		closestPoints.push_back(v2 - v1);
		closestPoints.push_back(v2);
    }
    else
    {
		closestPoints.push_back(v1);
		closestPoints.push_back(-v2);
		closestPoints.push_back(-v1-v2);
		closestPoints.push_back(-v1);
		closestPoints.push_back(v2);
		closestPoints.push_back(v1+v2);
    }
}


void Lattice::latticeToCartesian(double* x, double* y)
{
  double x2 = this->gen1(0) * *x + this->gen2(0) * *y;
  double y2 = this->gen1(1) * *x + this->gen2(1) * *y;
  
  *x = x2;
  *y = y2;
}

void Lattice::latticeToCartesian(Vector2d& v, Vector2d* b1, Vector2d* b2)
{
  double x2 = (*b1)(0) * v(0) + (*b2)(0) * v(1);
  double y2 = (*b1)(1) * v(0) + (*b2)(1) * v(1);

  v(0) = x2;
  v(1) = y2;
}

void Lattice::latticeToCartesian(Eigen::Matrix<double, 2, 1, Eigen::DontAlign>& v, Vector2d* b1, Vector2d* b2)
{
  double x2 = (*b1)(0) * v(0) + (*b2)(0) * v(1);
  double y2 = (*b1)(1) * v(0) + (*b2)(1) * v(1);

  v(0) = x2;
  v(1) = y2;
}

void Lattice::latticeToCartesian(double* x, double* y, Vector2d* b1, Vector2d* b2)
{
  double x2 = (*b1)(0) * *x + (*b2)(0) * *y;
  double y2 = (*b1)(1) * *x + (*b2)(1) * *y;
  
  *x = x2;
  *y = y2;
}


void Lattice::cartesianToLattice(double* x, double* y)
{
  double m,n;

  m = (this->gen2(1) * *x - this->gen2(0) * *y)/
           (this->gen1(0) * this->gen2(1)  - this->gen1(1) *this->gen2(0));
  n = (this->gen1(1) * *x - this->gen1(0) * *y)/
           (this->gen1(1) * this->gen2(0) - this->gen1(0) * this->gen2(1));

  *x = m;
  *y = n;
}

void Lattice::changeToBasis(Vector2d* v, Vector2d* b1, Vector2d* b2)
{
	double m, n;
	m = ((*b2)(1) * (*v)(0)- (*b2)(0) * (*v)(1)) / ((*b1)(0) * (*b2)(1) - (*b1)(1) *(*b2)(0));
	n = ((*b1)(1) * (*v)(0) - (*b1)(0) * (*v)(1)) / ((*b1)(1) * (*b2)(0) - (*b1)(0) * (*b2)(1));

	(*v)(0) = m;
	(*v)(1) = n;
}

void Lattice::cartesianToLattice(double* x, double* y, Vector2d* b1, Vector2d* b2)
{
  double m,n;
  
  m = ((*b2)(1) * *x - (*b2)(0) * *y)/((*b1)(0) * (*b2)(1)  - (*b1)(1) *(*b2)(0));
  n = ((*b1)(1) * *x - (*b1)(0) * *y)/((*b1)(1) * (*b2)(0) - (*b1)(0) * (*b2)(1));
  
  *x = m;
  *y = n;
}


/**
*  Takes cartisian coordinates, and changes them latice coordinates of the closest latice point.
*/
void Lattice::getClosestLatticePoint(double* x, double* y)
{
  double X0 = *x;
  double Y0 = *y;
  
  cartesianToLattice(x, y);
  
  double x1 = floor(*x);
  double x2 = ceil(*x);
  double y1 = floor(*y);
  double y2 = ceil(*y);
  
  double X1 = x1;
  double Y1 = y1;
  latticeToCartesian(&X1, &Y1);
  double minDis2 = norm2(X1-X0, Y1-Y0);
  *x = x1;
  *y = y1;

  
  X1 = x2;
  Y1 = y1;
  latticeToCartesian(&X1, &Y1);
  double dis2 = norm2(X1-X0,Y1-Y0);
  if(dis2 < minDis2)
  {
    minDis2 = dis2;
    *x = x2;
  }
  
  X1 = x1;
  Y1 = y2;
  latticeToCartesian(&X1, &Y1);
  dis2 = norm2(X1-X0,Y1-Y0);
  if(dis2 < minDis2)
  {
    minDis2 = dis2;
    *x = x1;
    *y = y2;
  }
  
  X1 = x2;
  Y1 = y2;
  latticeToCartesian(&X1, &Y1);
  dis2 = norm2(X1-X0, Y1-Y0);
  if(dis2 < minDis2)
  {
    *x = x2;
    *y = y2;
  }
}

Vector2d*  Lattice::getBounderyPoint(double x, double y)
{
	Vector2d* point = 0;
	double minimumDis = -1;

	double xCell = 0;
	double yCell = 0;
	latticeToCartesian(&xCell, &xCell);

	for (int i = 0; i < cellVerticies.size(); i++)
	{
		double dist = (cellVerticies[i](0) + xCell - x)*(cellVerticies[i](0) + xCell - x)+(cellVerticies[i](1) + yCell - y)*(cellVerticies[i](1) + yCell - y);
		if (dist <= 100 && (dist < minimumDis || minimumDis == -1))
		{
			point = cellBounderyPoints[i];
			minimumDis = dist;
		}
	}

	for (int i = 0; i < textCustomLoc.size(); i++)
	{
		double textPos_x = textCustomLoc[i](0)*largeGen2(0) + textCustomLoc[i](1)* largeGen1(0);
		double textPos_y = textCustomLoc[i](0)*largeGen2(1) + textCustomLoc[i](1)* largeGen1(1);
		double dist = (textPos_x + xCell - x)*(textPos_x + xCell - x) + (textPos_y + yCell - y)*(textPos_y + yCell - y);
		if (dist <= 100 && (dist < minimumDis || minimumDis == -1))
		{
			point = &textCustomLoc[i];
			minimumDis = dist;
		}
	}

	if (minimumDis > 0)
		return point;

	if (cellVerticies.size() < 2)
		return 0;

	Vector2d p(x - xCell, y - yCell);
	// Point was not on a vertex. Now check line segments and see if we need to add a boundey point.
	Vector2d* end1 = &cellVerticies[cellVerticies.size() - 1];
	Vector2d newVertex;
	int index = -1;
	for (int i = 0; i < cellVerticies.size(); i++)
	{
		Vector2d* end2 = &cellVerticies[i];
		double dist	= projectPointOntoLineSegment(end1, end2, &p, &newVertex);
		if (dist >= 0 && dist < 10 && dist > minimumDis)
		{
			minimumDis = dist;
			index = i;
		}

		end1 = end2;
	}

	if (minimumDis > 0)
	{
		// we need to add in the new boundery point
		Vector2dList& p1 = cellVerticies1;
		Vector2dList& p2 = cellVerticies2;
		Vector2dList& p3 = cellVerticies3;


		int index2 = 0;
		//Vector2d v = newVertex + 0.5*(right + down);
		Vector2d v = newVertex;

		Vector2d v1 = largeGen1;
		Vector2d v2 = largeGen2;
		if (index == index2)
		{
			Vector2d p = v +v1;
			Lattice::cartesianToLattice(&p(0), &p(1), &largeGen2, &largeGen1);
			p3.insert(p3.begin(), p);
			return &p3[0];
		}

		index2++;

		for (int i = 0; i < p1.size(); i++)
		{
			if (index == index2)
			{
				Vector2d p = v;
				Lattice::cartesianToLattice(&p(0), &p(1), &largeGen2, &largeGen1);
				p1.insert(p1.begin() + i, p);
				return &p1[i];
			}

			index2++;
		}


		for (int i = 0; i < p2.size(); i++)
		{
			if (index == index2)
			{
				Vector2d p = v;
				Lattice::cartesianToLattice(&p(0), &p(1), &largeGen2, &largeGen1);
				p2.insert(p2.begin() + i, p);
				return &p2[i];
			}

			index2++;
		}


		for (int i = 0; i < p3.size() + 1; i++)
		{
			if (index == index2)
			{
				Vector2d p = v;
				Lattice::cartesianToLattice(&p(0), &p(1), &largeGen2, &largeGen1);
				p3.insert(p3.begin()+i, p);
				return &p3[i];
			}

			index2++;
		}


		for (int i = p1.size() - 1; i >= 0; i--)
		{
			if (index == index2)
			{
				Vector2d p = v - (v1-v2);
				Lattice::cartesianToLattice(&p(0), &p(1), &largeGen2, &largeGen1);
				p1.insert(p1.begin() + i, p);
				return &p1[i];
			}

			index2++;
		}


		for (int i = p2.size() - 1; i >= 0; i--)
		{
			if (index == index2)
			{
				Vector2d p = v + v2;
				Lattice::cartesianToLattice(&p(0), &p(1), &largeGen2, &largeGen1);

				p2.insert(p2.begin() + i, p);
				return &p2[i];
			}

			index2++;
		}

		//// v = r + p(0)*r + p(1)*d1 - d1 = (1+p(0))*r.dot(rHat)*rHat + (p(1) - 1)*d1.dot(dHat)*dHat
		//// v.dot(rHat) = (1+p(0))*r.dot(rHat)     ==> p(0) = v.dot(rHat)/r.dot(rHat) - 1
		//// v.dot(dHat) = (p(1) - 1)*d1.dot(dHat)  ==> p(1) =  v.dot(dHat)/d1.dot(dHat)  + 1
		//this->cellVerticies.push_back(Vector2d::subtract(Vector2d::add(r, Vector2d::add(r.scale(p1.back()(0)), d1.scale(p1.back()(1)))), d1));
		//this->cellBounderyPoints.push_back(&p1.back());

		for (int i = p3.size(); i >= 0; i--)
		{
			if (index == index2)
			{
				Vector2d p = v+v1;
				Lattice::cartesianToLattice(&p(0), &p(1), &largeGen2, &largeGen1);
				p3.insert(p3.begin() + i, p);
				return &p3[i];
			}

			index2++;
		}
	}

	return 0;
}

double Lattice::projectPointOntoLineSegment(Vector2d* v, Vector2d* w, Vector2d* p, Vector2d* newVertex)
{
	// Return minimum distance between line segment vw and point p
	double l2 = (*v-*w).squaredNorm();  // i.e. |w-v|^2 -  avoid a sqrt
	if (l2 == 0.0)
		return -1;   

	double t = (*p-*v).dot(*w-*v) / l2;
	if (t <= 0 || t >= 1)
		return -1;

//	*newVertex = Vector2d::add(*v, Vector2d::subtract(*w,*v).scale(t));  // Projection falls on the segment
	*newVertex = *p;
//	return Vector2d::subtract(*p, *newVertex).norm();
	return (*p - *v - t*(*w-*v)).norm();
}

//// Return minimum distance between line segment vw and point p
//const float l2 = length_squared(v, w);  // i.e. |w-v|^2 -  avoid a sqrt
//if (l2 == 0.0) return distance(p, v);   // v == w case
//										// Consider the line extending the segment, parameterized as v + t (w - v).
//										// We find projection of point p onto the line. 
//										// It falls where t = [(p-v) . (w-v)] / |w-v|^2
//										// We clamp t from [0,1] to handle points outside the segment vw.
//const float t = max(0, min(1, dot(p - v, w - v) / l2));
//const vec2 projection = v + t * (w - v);  // Projection falls on the segment
//return distance(p, projection);

bool Lattice::eraseBounderyPoint(Vector2d* p)
{
	for (int i = 0; i < cellVerticies1.size() - 1; i++)  //can't erase last point
	{
		if (p == &cellVerticies1[i])
		{
			cellVerticies1.erase(cellVerticies1.begin() + i);
			return true;
		}
	}


	for (int i = 0; i < cellVerticies2.size() - 1; i++)  //can't erase last point
	{
		if (p == &cellVerticies2[i])
		{
			cellVerticies2.erase(cellVerticies2.begin() + i);
			return true;
		}
	}

	for (int i = 0; i < cellVerticies3.size(); i++)  //errasing last point is fine
	{
		if (p == &cellVerticies3[i])
		{
			cellVerticies3.erase(cellVerticies3.begin() + i);
			return true;
		}
	}

	return false;
}

bool Lattice::maybeRemoveBounderyPoint(Vector2d* p)
{
	Vector2dList* list = 0;
	int index = -1;

	for (int i = 0; i < textCustomLoc.size(); i++)
	{
		if (p == &textCustomLoc[i])
		{
			list = &textCustomLoc;
			index = i;
		}
	}

	if (list != 0)
	{
		double x = (*p)(0)*largeGen2(0) + (*p)(1)*largeGen1(0);
		double y = (*p)(0)*largeGen2(1) + (*p)(1)*largeGen1(1);

		for (int i = 0; i < list->size(); i++)
		{
			if (p != &(*list)[i])
			{
				double x2 = (*list)[i](0)*largeGen2(0) + (*list)[i](1)*largeGen1(0);
				double y2 = (*list)[i](0)*largeGen2(1) + (*list)[i](1)*largeGen1(1);
				double dist = (x - x2)*(x - x2) + (y - y2)*(y - y2);
				if (dist <= 100)
				{
					list->erase(list->begin() + index);
					return true;
				}
			}
		}

		return true;
	}

	for (int i = 0; i < cellVerticies1.size(); i++)
	{
		if (p == &cellVerticies1[i])
		{
			list = &cellVerticies1;
			index = i;
		}
	}

	for (int i = 0; i < cellVerticies2.size(); i++)
	{
		if (p == &cellVerticies2[i])
		{
			list = &cellVerticies2;
			index = i;
		}
	}

	for (int i = 0; i < cellVerticies3.size(); i++)
	{
		if (p == &cellVerticies3[i])
		{
			list = &cellVerticies3;
			index = i;
		}
	}

	//// if point was not removed, see if it can be spapped to an appripriate position
	//double threshold = .05;
	//snapToPosition(p(0), threshold);
	//snapToPosition(p(1), threshold);

	if (list == 0)
		return true;

		
	for (int i = 0; i < cellVerticies.size(); i++)
	{
		if(cellBounderyPoints[i] == p)
		{
			Vector2d* vertex = &cellVerticies[i];
			Vector2d* prevVertex = 0;
			if (i > 0)
				prevVertex = &cellVerticies[i - 1];
			else
				prevVertex = &cellVerticies.back();

			double dist = ((*vertex)(0) - (*prevVertex)(0))*((*vertex)(0) - (*prevVertex)(0)) + ((*vertex)(1) - (*prevVertex)(1))*((*vertex)(1) - (*prevVertex)(1));
			if (dist <= 100)
			{
				if (p == &cellVerticies1.back() || p == &cellVerticies2.back())
				{
					// erase other point
					if (eraseBounderyPoint(cellBounderyPoints[i>0?i - 1: cellBounderyPoints.size()-1]))
						return true;
				}
				else
				{
					//list->erase(list->begin() + index);
					if (eraseBounderyPoint(p))
						return true;
				}
			}
	
			Vector2d* nextVertex = 0;
			if (i < cellVerticies.size() - 1)
				nextVertex = &cellVerticies[i + 1];
			else
				nextVertex = &cellVerticies[0];

			dist = ((*vertex)(0) - (*nextVertex)(0))*((*vertex)(0) - (*nextVertex)(0)) + ((*vertex)(1) - (*nextVertex)(1))*((*vertex)(1) - (*nextVertex)(1));
			if (dist <= 100)
			{
				if (p == &cellVerticies1.back() || p == &cellVerticies2.back())
				{
					if (eraseBounderyPoint(cellBounderyPoints[i<cellBounderyPoints.size()-1 ? i + 1 : 0]))
						return true;
				}
				else
				{
			//		list->erase(list->begin() + index);
					if (eraseBounderyPoint(p))
					  return true;
				}
			}

			/*
			if (abs(vertex(0) - prevVertex(0)) < 10)
			{
				vertex(0) = prevVertex(0);
			}
			
			if (abs(vertex(1) - prevVertex(1)) < 10)
			{
				vertex(1) = prevVertex(1);
			}

			if (abs(vertex(0) - nextVertex(0)) < 10)
			{
				vertex(0) = nextVertex(0);
			}

			if (abs(vertex(1) - nextVertex(1)) < 10)
			{
				vertex(1) = nextVertex(1);
			}*/
		}
	}

	return true;
}

void Lattice::snapToPosition(double& value, double threshold)
{
	if (abs(value) < threshold)
		value = 0;
	else if (abs(value - .5) < threshold)
		value = .5;
	else if (abs(value + .5) < threshold)
		value = -.5;
	else if (abs(value - .25) < threshold)
		value = .25;
	else if (abs(value + .25) < threshold)
		value = -.25;
	else if (abs(value - .75) < threshold)
		value = .75;
	else if (abs(value + .75) < threshold)
		value = -.75;
	else if (abs(value - 1.0/3) < threshold)
		value = 1.0 / 3;
	else if (abs(value + 1.0 / 3) < threshold)
		value = -1.0 / 3;
	else if (abs(value - 2.0/3) < threshold)
		value = 2.0 / 3;
	else if (abs(value + 2.0 / 3) < threshold)
		value = -2.0 / 3;
}

/*
* takes cartician coodinates, and changes them to the latice coordinates of the cell containing that point.
*/
void Lattice::getCell(double* x, double* y)
{
	double xC = *x;
	double yC = *y;

	double x0 = *x;
	double y0 = *y;
	getClosestLatticePoint(&x0, &y0);

	for (int dis = 0; dis < 100; dis++)
	{
		for (int i = 0; i <= dis; i++)
		{
			int j = dis - i;
			*x = x0 + i;
			*y = y0 + j;
			if (doesCellContainPoint(*x, *y, xC, yC))
				return;

			if (i != 0)
			{
				*x = x0 - i;
				*y = y0 + j;
				if (doesCellContainPoint(*x, *y, xC, yC))
					return;

				if (j != 0)
				{
					*x = x0 - i;
					*y = y0 - j;
					if (doesCellContainPoint(*x, *y, xC, yC))
						return;
				}
			}

			if (j != 0)
			{
				*x = x0 + i;
				*y = y0 - j;
				if (doesCellContainPoint(*x, *y, xC, yC))
					return;
			}
		}
	}

	*x = x0;
	*y = y0;

	DebugThrow(L"Fatal Error: Point not found in cell");
	return;
}


// the containment algorithm was pulled, and slightly modified from http://geomalgorithms.com/a03-_inclusion.html

// Copyright 2000 softSurfer, 2012 Dan Sunday
// This code may be freely used, distributed and modified for any purpose
// providing that this copyright notice is included with it.
// SoftSurfer makes no warranty for this code, and cannot be held
// liable for any real or imagined damage resulting from its use.
// Users of this code must verify correctness for their application.

/**
* Tests whether the catician point (x,y) is contained by a cell with lattice coordinates (xL, yL) 
*/
int Lattice::doesCellContainPoint(double xL, double yL, double x, double y)
{
	latticeToCartesian(&xL, &yL);

	Vector2d p(x - xL, y - yL);

	//First do a bounding box test
	if (x < xL + cellMinX || x > xL + cellMaxX || y < yL + cellMinY || y > yL + cellMaxY)
		return 0;

	int    cn = 0;    // the  crossing number counter
	int n = cellVerticies.size();
	// loop through all edges of the polygon
	for (int i = 0; i<n; i++)
	{   
		if (getDistanceSquaredFromLineSegment(&cellVerticies[i], &cellVerticies[(i + 1) % n], &p) <= 9)  // this provides a little overlap so points don't fall in the gaps between cells
		{
			PRINT(L"Point on boundery");
			return 1;
		}

		// edge from V[i]  to V[i+1]
		if (((cellVerticies[i](1) + yL <= y) && (cellVerticies[(i + 1)%n](1) + yL > y))     // an upward crossing
			|| ((cellVerticies[i](1) + yL > y) && (cellVerticies[(i + 1)%n](1) + yL <= y))) // a downward crossing
		{ 
			// compute  the actual edge-ray intersect x-coordinate
			double vt = (y - cellVerticies[i](1) - yL) / (cellVerticies[(i + 1)%n](1) - cellVerticies[i](1));
			if (x <  cellVerticies[i](0) + xL + vt * (cellVerticies[(i + 1)%n](0) - cellVerticies[i](0))) // x < intersect
				++cn;   // a valid crossing of y=P(1) right of P(0)
		}
	}

	//Debug consistancy check.
	if ((cn & 1) && (x < xL + cellMinX || x > xL + cellMaxX || y < yL + cellMinY || y > yL + cellMaxY))
		Throw(L"Fatal Error: inconsistancy in cell hit detection algorithm");

	return (cn & 1);    // 0 if even (out), and 1 if  odd (in)
}


double Lattice::getDistanceSquaredFromLineSegment(Vector2d* v, Vector2d* w, Vector2d* p)
{
	// Return minimum distance between line segment vw and point p
	double l2 = (*w - *v).squaredNorm();  // i.e. |w-v|^2 -  avoid a sqrt
	if (l2 == 0.0)
		return (*p - *v).squaredNorm();

	double t = (*p - *v).dot(*w - *v) / l2;
	if (t <= 0)
	{
	  return (*p - *v).squaredNorm();
	}
	else if(t >= 1)
	{
	  return (*p - *w).squaredNorm();
	}
	else
	{
	  return (*p - *v - t*(*w - *v)).squaredNorm();
	}
}