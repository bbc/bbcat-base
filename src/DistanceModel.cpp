
#include <math.h>

#define DEBUG_LEVEL 0
#include "DistanceModel.h"

BBC_AUDIOTOOLBOX_START

/*--------------------------------------------------------------------------------*/
/** A simple singleton to handle time and level calculation based on distance
 *
 */
/*--------------------------------------------------------------------------------*/
DistanceModel::DistanceModel() : decaypower(2.0),
                                 speedofsound(340.0)
{
}

/*--------------------------------------------------------------------------------*/
/** Return singleton instance of this object 
 */
/*--------------------------------------------------------------------------------*/
DistanceModel& DistanceModel::Get()
{
  static DistanceModel model;
  return model;
}

/*--------------------------------------------------------------------------------*/
/** Get level due to distance
 */
/*--------------------------------------------------------------------------------*/
double DistanceModel::GetLevel(double d) const
{
  return pow(decaypower, -d);
}

/*--------------------------------------------------------------------------------*/
/** Get delay (in s or samples if delayscale = samplerate) due to distance
 */
/*--------------------------------------------------------------------------------*/
double DistanceModel::GetDelay(double d, double delayscale) const
{
  return (speedofsound > 0.0) ? delayscale * d / speedofsound : 0.0;
}

/*--------------------------------------------------------------------------------*/
/** Get level and delay due to distance
 */
/*--------------------------------------------------------------------------------*/
void DistanceModel::GetLevelAndDelay(double d, double& level, double& delay, double delayscale) const
{
  level = GetLevel(d);
  delay = GetDelay(d, delayscale);
}

/*--------------------------------------------------------------------------------*/
/** Get level due to distance
 */
/*--------------------------------------------------------------------------------*/
double DistanceModel::GetLevel(const Position& pos) const
{
  return GetLevel(pos.Polar().pos.d);
}

/*--------------------------------------------------------------------------------*/
/** Get delay (in s or samples if delayscale = samplerate) due to distance
 */
/*--------------------------------------------------------------------------------*/
double DistanceModel::GetDelay(const Position& pos, double delayscale) const
{
  return GetDelay(pos.Polar().pos.d, delayscale);
}

/*--------------------------------------------------------------------------------*/
/** Get level and delay due to distance
 */
/*--------------------------------------------------------------------------------*/
void DistanceModel::GetLevelAndDelay(const Position& pos, double& level, double& delay, double delayscale) const
{
  GetLevelAndDelay(pos.Polar().pos.d, level, delay, delayscale);
}

BBC_AUDIOTOOLBOX_END
