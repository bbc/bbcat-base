#ifndef __DISTANCE_MODEL__
#define __DISTANCE_MODEL__

#include "3DPosition.h"

BBC_AUDIOTOOLBOX_START

/*--------------------------------------------------------------------------------*/
/** A simple singleton to handle time and level calculation based on distance
 *
 */
/*--------------------------------------------------------------------------------*/
class DistanceModel
{
public:
  /*--------------------------------------------------------------------------------*/
  /** Return singleton instance of this object 
   */
  /*--------------------------------------------------------------------------------*/
  static DistanceModel& Get();

  /*--------------------------------------------------------------------------------*/
  /** Set decay power due to distance (==2 for inverse square law, set to 0 for no decay)
   */
  /*--------------------------------------------------------------------------------*/
  void SetDecayPower(double power)   {decaypower   = power;}

  /*--------------------------------------------------------------------------------*/
  /** Set speed of sound in m/s (set to 0 for no delay)
   */
  /*--------------------------------------------------------------------------------*/
  void SetSpeedOfSound(double speed) {speedofsound = speed;}

  /*--------------------------------------------------------------------------------*/
  /** Get level due to distance
   */
  /*--------------------------------------------------------------------------------*/
  double GetLevel(double d) const;

  /*--------------------------------------------------------------------------------*/
  /** Get delay (in s or samples if delayscale = samplerate) due to distance
   */
  /*--------------------------------------------------------------------------------*/
  double GetDelay(double d, double delayscale = 1.0) const;

  /*--------------------------------------------------------------------------------*/
  /** Get level and delay due to distance
   */
  /*--------------------------------------------------------------------------------*/
  void   GetLevelAndDelay(double d, double& level, double& delay, double delayscale = 1.0) const;

  /*--------------------------------------------------------------------------------*/
  /** Get level due to distance
   */
  /*--------------------------------------------------------------------------------*/
  double GetLevel(const Position& pos) const;

  /*--------------------------------------------------------------------------------*/
  /** Get delay (in s or samples if delayscale = samplerate) due to distance
   */
  /*--------------------------------------------------------------------------------*/
  double GetDelay(const Position& pos, double delayscale = 1.0) const;

  /*--------------------------------------------------------------------------------*/
  /** Get level and delay due to distance
   */
  /*--------------------------------------------------------------------------------*/
  void   GetLevelAndDelay(const Position& pos, double& level, double& delay, double delayscale = 1.0) const;

protected:
  DistanceModel();
  ~DistanceModel() {}

protected:
  double decaypower;
  double speedofsound;
};

BBC_AUDIOTOOLBOX_END

#endif

