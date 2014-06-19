
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "3DPosition.h"

using namespace std;

BBC_AUDIOTOOLBOX_START

PositionTransform::PositionTransform() : xrotation(0.0),
										 yrotation(0.0),
										 zrotation(0.0)
{
}

PositionTransform::PositionTransform(const PositionTransform& obj) : xrotation(0.0),
																	 yrotation(0.0),
																	 zrotation(0.0)
{
	operator = (obj);
}

/*--------------------------------------------------------------------------------*/
/** Assignment operator
 */
/*--------------------------------------------------------------------------------*/
PositionTransform& PositionTransform::operator = (const PositionTransform& obj)
{
	pretranslation  = obj.pretranslation;
	xrotation       = obj.xrotation;
	yrotation       = obj.yrotation;
	zrotation       = obj.zrotation;
	while (xrotation < -180.0) xrotation += 360.0;
	while (xrotation >= 180.0) xrotation -= 360.0;
	while (yrotation < -180.0) yrotation += 360.0;
	while (yrotation >= 180.0) yrotation -= 360.0;
	while (zrotation < -180.0) zrotation += 360.0;
	while (zrotation >= 180.0) zrotation -= 360.0;
	posttranslation = obj.posttranslation;

	return *this;
}

/*--------------------------------------------------------------------------------*/
/** Add transform to this transform
 */
/*--------------------------------------------------------------------------------*/
PositionTransform& PositionTransform::operator += (const PositionTransform& obj)
{
	pretranslation  += obj.pretranslation;
	xrotation       += obj.xrotation;
	yrotation       += obj.yrotation;
	zrotation       += obj.zrotation;
	while (xrotation < -180.0) xrotation += 360.0;
	while (xrotation >= 180.0) xrotation -= 360.0;
	while (yrotation < -180.0) yrotation += 360.0;
	while (yrotation >= 180.0) yrotation -= 360.0;
	while (zrotation < -180.0) zrotation += 360.0;
	while (zrotation >= 180.0) zrotation -= 360.0;
	posttranslation += obj.posttranslation;

	return *this;
}

/*--------------------------------------------------------------------------------*/
/** Subtract transform to this transform
 */
/*--------------------------------------------------------------------------------*/
PositionTransform& PositionTransform::operator -= (const PositionTransform& obj)
{
	pretranslation  -= obj.pretranslation;
	xrotation       -= obj.xrotation;
	yrotation       -= obj.yrotation;
	zrotation       -= obj.zrotation;
	while (xrotation < -180.0) xrotation += 360.0;
	while (xrotation >= 180.0) xrotation -= 360.0;
	while (yrotation < -180.0) yrotation += 360.0;
	while (yrotation >= 180.0) yrotation -= 360.0;
	while (zrotation < -180.0) zrotation += 360.0;
	while (zrotation >= 180.0) zrotation -= 360.0;
	posttranslation -= obj.posttranslation;

	return *this;
}

void PositionTransform::Transform(Position& pos) const
{
	if (pos.polar) {
		// convert to cartesian, transform and then convert back
		pos = pos.Cart();
		Transform(pos);
		pos = pos.Polar();
	}
	else {
		pos += pretranslation;
		Rotate(pos.pos.y, pos.pos.z, xrotation); 
		Rotate(pos.pos.x, pos.pos.z, yrotation); 
		Rotate(pos.pos.x, pos.pos.y, zrotation); 
		pos += posttranslation;
	}
}

void PositionTransform::Rotate(double& x, double& y, double angle) const
{
	if (angle != 0.0) {
		angle *= M_PI / 180.0;
		double x1 = x * cos(angle) - y * sin(angle);
		double y1 = x * sin(angle) + y * cos(angle);
		x = x1; y = y1;
	}
}

/*----------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------*/
/** Return the same position but as polar co-ordinates
 */
/*--------------------------------------------------------------------------------*/
Position Position::Polar() const
{
	Position newpos = *this;
		
	if (!polar) {
		newpos.polar = true;
		newpos.pos.d = sqrt(pos.x * pos.x + pos.y * pos.y + pos.z * pos.z);
		
		if (newpos.pos.d > 0.0) {
			// el = asin(z)
			// az = atan(-x / y)

			double x = pos.x / newpos.pos.d;
            double y = pos.y / newpos.pos.d;
            double z = pos.z / newpos.pos.d;

            // since z = sin(el), el = asin(z)
            newpos.pos.el = asin(z) * 180.0 / M_PI;
            // since x = -sin(az) * cos(el)
            //   and y =  cos(az) * cos(el)
            //     x/y = -sin(az) / cos(az) = -tan(az)
            // =>   az = atan(-x / y) = atan2(-x, y)
            if ((x != 0.0) || (y != 0.0)) {
                // can do atan2
				newpos.pos.az = atan2(-x, y) * 180.0 / M_PI;
			}
		}
	}
		
	return newpos;
}

/*--------------------------------------------------------------------------------*/
/** Return the same position but as cartesian co-ordinates
 */
/*--------------------------------------------------------------------------------*/
Position Position::Cart() const
{
	Position newpos = *this;
		
	if (polar) {
		// x = -sin(az) * cos(el)
		// y =  cos(az) * cos(el)
		// z =  sin(el)

		newpos.polar = false;
		newpos.pos.x = -sin(pos.az * M_PI / 180.0) * cos(pos.el * M_PI / 180.0);
		newpos.pos.y =  cos(pos.az * M_PI / 180.0) * cos(pos.el * M_PI / 180.0);
		newpos.pos.z =  sin(pos.el * M_PI / 180.0);
	}
		
	return newpos;
}

/*--------------------------------------------------------------------------------*/
/** Limit azimuth and elevation
 */
/*--------------------------------------------------------------------------------*/
void Position::LimitAngles()
{
	// ensure azimuth is within limits
	pos.az = fmod(pos.az, 360.0);
    if (pos.az <    0.0) pos.az += 360.0;
	if (pos.az >= 360.0) pos.az -= 360.0;

	// ensure elevation is within limits
	pos.el = fmod(pos.el, 180.0);
    if (pos.el <  -90.0) pos.el += 180.0;
	if (pos.el >=  90.0) pos.el -= 180.0;
}

/*--------------------------------------------------------------------------------*/
/** Translate the current position by the supplied position in cartesian space
 *
 * @note the object remains in the same co-ordinate system as it was
 */
/*--------------------------------------------------------------------------------*/
Position& Position::operator += (const Position& obj)
{
	if (polar || obj.polar) {
		// translating polar co-ordinates requires changing both to cartesian, translating
		// and then converting this object back to polar
		Position pos = Cart() + obj.Cart();
		*this = polar ? pos.Polar() : pos;
	}
	else {
		pos.x += obj.pos.x;
		pos.y += obj.pos.y;
		pos.z += obj.pos.z;
	}

	return *this;
}

/*--------------------------------------------------------------------------------*/
/** Translate the current position by the supplied position in cartesian space
 *
 * @note the object remains in the same co-ordinate system as it was
 */
/*--------------------------------------------------------------------------------*/
Position& Position::operator -= (const Position& obj)
{
	if (polar || obj.polar) {
		// translating polar co-ordinates requires changing both to cartesian, translating
		// and then converting this object back to polar
		Position pos = Cart() - obj.Cart();
		*this = polar ? pos.Polar() : pos;
	}
	else {
		pos.x -= obj.pos.x;
		pos.y -= obj.pos.y;
		pos.z -= obj.pos.z;
	}

	return *this;
}

/*--------------------------------------------------------------------------------*/
/** Scale the current position
 */
/*--------------------------------------------------------------------------------*/
Position& Position::operator *= (double val)
{
	if (polar) pos.d *= val;
	else {
		pos.x *= val;
		pos.y *= val;
		pos.z *= val;
	}
	return *this;
}

Position& Position::operator *= (const double vals[3])
{
	if (polar) {
		// translating polar co-ordinates requires changing both to cartesian, translating
		// and then converting this object back to polar
		*this = Cart();

		// perform translation and convert back
		*this *= vals;

		*this = Polar();
	}
	else {
		pos.x *= vals[0];
		pos.y *= vals[1];
		pos.z *= vals[2];
	}

	return *this;
}

Position& Position::operator *= (const double vals[3][3])
{
	if (polar) {
		// translating polar co-ordinates requires changing both to cartesian, translating
		// and then converting this object back to polar
		*this = Cart();

		// perform translation and convert back
		*this *= vals;

		*this = Polar();
	}
	else {
		double x = pos.x * vals[0][0] + pos.y * vals[0][1] + pos.z * vals[0][2];
		double y = pos.x * vals[1][0] + pos.y * vals[1][1] + pos.z * vals[1][2];
		double z = pos.x * vals[2][0] + pos.y * vals[2][1] + pos.z * vals[2][2];

		pos.x = x;
		pos.y = y;
		pos.z = z;
	}

	return *this;
}

/*--------------------------------------------------------------------------------*/
/** Comparison operators
 */
/*--------------------------------------------------------------------------------*/
bool operator == (const Position& obj1, const Position& obj2)
{
	// try comparing positions as cartesian first
	Position pos1 = obj1.Cart();
	Position pos2 = obj2.Cart();

	if ((pos1.pos.x == pos2.pos.x) &&
		(pos1.pos.y == pos2.pos.y) &&
		(pos1.pos.z == pos2.pos.z)) return true;

	// if that fails, try comparing them as polar
	pos1 = obj1.Polar();
	pos2 = obj2.Polar();

	if ((pos1.pos.az == pos2.pos.az) &&
		(pos1.pos.el == pos2.pos.el) &&
		(pos1.pos.d  == pos2.pos.d)) return true;

	return false;
}

/*--------------------------------------------------------------------------------*/
/** Apply position transform
 */
/*--------------------------------------------------------------------------------*/
Position& Position::operator *= (const PositionTransform& trans)
{
	trans.Transform(*this);
	return *this;
}

/*--------------------------------------------------------------------------------*/
/** Apply position transform
 */
/*--------------------------------------------------------------------------------*/
Position operator * (const Position& pos, const PositionTransform& trans)
{
	Position res = pos;
	trans.Transform(res);
	return res;
}

/*--------------------------------------------------------------------------------*/
/** Return antipodean version of position
 */
/*--------------------------------------------------------------------------------*/
Position Position::operator - () const
{
	Position res = *this;

	if (res.polar) {
		// move azimuth by 180 degrees (wrapping if necessary)
		res.pos.az += 180.0;
		if (res.pos.az >= 180.0) res.pos.az -= 360.0;

		// flip elevation
		res.pos.el  = -res.pos.el;
	}
	else {
		res.pos.x = -res.pos.x;
		res.pos.y = -res.pos.y;
		res.pos.z = -res.pos.z;
	}

	return res;
}

/*--------------------------------------------------------------------------------*/
/** Return unit vector version of this Position
 */
/*--------------------------------------------------------------------------------*/
Position Position::Unit() const
{
	Position pos1 = *this;
	double   d    = pos1.Mod();

	if (d > 0.0) pos1 *= 1.0 / d;

	return pos1;
}

/*--------------------------------------------------------------------------------*/
/** Return dot product of two positions
 */
/*--------------------------------------------------------------------------------*/
double DotProduct(const Position& obj1, const Position& obj2)
{
	Position pos1 = obj1.Cart();		// must be cartesian!
	Position pos2 = obj2.Cart();		// must be cartesian!

	return pos1.pos.x * pos2.pos.x + pos1.pos.y * pos2.pos.y + pos1.pos.z * pos2.pos.z;
}

double DotProduct(const Position& obj1, const double vals[3])
{
	Position pos1 = obj1.Cart();		// must be cartesian!

	return pos1.pos.x * vals[0] + pos1.pos.y * vals[1] + pos1.pos.z * vals[2];
}

/*--------------------------------------------------------------------------------*/
/** Return cross product of two positions
 */
/*--------------------------------------------------------------------------------*/
Position CrossProduct(const Position& obj1, const Position& obj2)
{
	return Position(+(obj1.pos.y * obj2.pos.z - obj2.pos.y * obj1.pos.z),
					-(obj1.pos.x * obj2.pos.z - obj2.pos.x * obj1.pos.z),
					+(obj1.pos.x * obj2.pos.y - obj2.pos.x * obj1.pos.y));
}

/*--------------------------------------------------------------------------------*/
/** Return angle between two vectors
 */
/*--------------------------------------------------------------------------------*/
double Angle(const Position& obj1, const Position& obj2)
{
	Position pos1 = obj1.Unit();
	Position pos2 = obj2.Unit();
	double   dot  = DotProduct(pos1, obj2.Unit());
	if ((dot < -1.01) || (dot > 1.01)) {
		ERROR("Dot product of (%0.3lf, %0.3lf, %0.3lf) and (%0.3lf, %0.3lf, %0.3lf) = %0.6lf",
			  pos1.pos.x, pos1.pos.y, pos1.pos.z,
			  pos2.pos.x, pos2.pos.y, pos2.pos.z,
			  dot);
	}
	dot = LIMIT(dot, -1.0, 1.0);
	return acos(dot) * 180.0 / M_PI;
}

string Position::ToString() const
{
	string str;

	if (polar) {
		Printf(str, "polar (%0.1lf, %0.1lf) x %0.1lfm", pos.az, pos.el, pos.d);
	}
	else {
		Printf(str, "cart (%0.1lfm, %0.1lfm, %0.1lfm)", pos.x, pos.y, pos.z);
	}
	
	return str;
}

BBC_AUDIOTOOLBOX_END
