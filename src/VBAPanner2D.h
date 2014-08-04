/*
 * VBAPanner2D.h
 *
 *  Created on: Jun 28, 2014
 *      Author: chrisp
 */

#ifndef VBAPANNER2D_H_
#define VBAPANNER2D_H_

#include <vector>

#include "3DPosition.h"
#include "VBAPanner.h"

BBC_AUDIOTOOLBOX_START

class VBAPanner2D : public VBAPanner
{
public:
  VBAPanner2D();
  virtual ~VBAPanner2D();

  enum
  {
    // to avoid hardcoded 2s everywhere!
    Dimensions = 2,
    MaxSpeakersPerSet = 2,
  };

  /*--------------------------------------------------------------------------------*/
  /** Read back speaker positions (and possibly groups) from text file
   */
  /*--------------------------------------------------------------------------------*/
  void Read(const char *filename);

  /*--------------------------------------------------------------------------------*/
  /** Add a speaker at specified position with specified additional gain
   *
   * @param channel channel that the speaker appears on (< 0 == dummy / no output)
   * @param pos physical position of speaker
   * @param gain modification of gain (1 == no modification)
   */
  /*--------------------------------------------------------------------------------*/
  void AddSpeaker(sint_t channel, const Position& pos, double gain = 1.0);

  /*--------------------------------------------------------------------------------*/
  /** Add a group of speakers to group list
   *
   * @param spn a list of speaker indices
   *
   */
  /*--------------------------------------------------------------------------------*/
  void AddSpeakerGroup(const uint_t spn[Dimensions]) { UNUSED_PARAMETER(spn); ERROR("Can't manually specify speaker groups for VBAP2D"); };

  /*--------------------------------------------------------------------------------*/
  /** Find speaker with lowest azimuth.
   */
  /*--------------------------------------------------------------------------------*/
  bool operator() (Speaker_t i,Speaker_t j)
  {
    return (i.vec.Polar().pos.az < j.vec.Polar().pos.az);
  }

  /*--------------------------------------------------------------------------------*/
  /** Find groups of speakers from existing speakers
   *
   *  @note This MUST be done before using the panner object. It's done automatically if Read() is used.
   */
  /*--------------------------------------------------------------------------------*/
  virtual void FindSpeakerGroups();

protected:

  /*--------------------------------------------------------------------------------*/
  /** Sort speakers by azimuth to generate panning pairs.
   */
  /*--------------------------------------------------------------------------------*/
  void SortSpeakers();

  bool Invert(SpeakerGroup_t& group);

  /*--------------------------------------------------------------------------------*/
  /** Test that the specified speaker group is valid for the specified position and if it is, calculate the gain factors
   *
   * @param pos sound source position
   * @param group speaker group (triangle)
   * @param gains array to receive gains in or NULL
   *
   * @return error of this group (== 0 for exact match, > 0 for inexact match)
   *
   */
  /*--------------------------------------------------------------------------------*/
  virtual double TestSpeakers(const Position& pos, const SpeakerGroup_t& group, double *gains) const;

protected:
  std::vector<SpeakerGroup_t> groups;
};

BBC_AUDIOTOOLBOX_END

#endif /* VBAPANNER2D_H_ */
