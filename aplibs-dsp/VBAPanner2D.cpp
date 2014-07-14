/*
 * VBAPanner2D2D.cpp
 *
 *  Created on: Jun 28, 2014
 *      Author: chrisp
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>

#define DEBUG_LEVEL 1

#include "VBAPanner2D.h"

BBC_AUDIOTOOLBOX_START

VBAPanner2D::VBAPanner2D()
{
  DEBUG2(("Using VBAPanner2D"));
}

VBAPanner2D::~VBAPanner2D()
{
}

/*--------------------------------------------------------------------------------*/
/** Store cartesian version speaker position
 */
/*--------------------------------------------------------------------------------*/
void VBAPanner2D::AddSpeaker(sint_t channel, const Position& pos, double gain)
{
  Speaker_t sp = {
    .channel  = channel,
    .vec      = pos.Cart().Unit(),
    .dist     = pos.Mod(),
    .usergain = gain,
    .gain     = 1.0,
    .delay    = 0.0,
    .delay_compensation = 0.0,
  };

  SetGainAndDelay(sp);

  DEBUG2(("Adding speaker index %u, channel %u", (uint_t)speakers.size(), channel));

  speakers.push_back(sp);
}

/*--------------------------------------------------------------------------------*/
/** Read back speaker positions (and possibly groups) from text file
 */
/*--------------------------------------------------------------------------------*/
void VBAPanner2D::Read(const char *filename)
{
  FILE *fp;

  if ((fp = fopen(filename, "r")) != NULL) {
    static char line[256];
    bool readingspeakers = true;
    int l;

    while ((l = ReadLine(fp, line, sizeof(line))) != EOF) {
      Position pos;
      uint_t   channel, spn[3];
      double   gain = 0.0;

      if ((l == 0) || (line[0] == '#')) continue;

      // attempt to interpret data as speaker positions or speaker indices
      if (readingspeakers && (sscanf(line, "%u %lf,%lf,%lf %lf", &channel, &pos.pos.x, &pos.pos.y, &pos.pos.z, &gain) >= 4)) {
        pos.polar   = false;

        DEBUG3(("Adding %s as speaker (channel %u, gain %0.2lfdB)", pos.ToString().c_str(), channel, gain));

        AddSpeaker(channel, pos, pow(10.0, .05 * gain));
      }
      else if (!readingspeakers && (sscanf(line, "%u,%u,%u", spn, spn + 1, spn + 2) == 3)) {
        DEBUG2(("2D panner doesn't use speaker groups"));
      }
      // else detect section
      else if (strncmp(line, "speakers", 8) == 0) {
        DEBUG3(("Reading speaker locations..."));
        readingspeakers = true;
      }
      else if (strncmp(line, "groups", 6) == 0) {
        DEBUG2(("2D panner doesn't use speaker groups"));
      }
    }

    fclose(fp);

    FindSpeakerGroups();

  }
  else ERROR("Failed to open file '%s' for reading", filename);
}

/*--------------------------------------------------------------------------------*/
/** Find gain factors for a sound source using a set of speakers from a list of speaker sets
 *
 * @param pos sound source position
 * @param speakerset structure populated by this function
 *
 * @return true if speaker set found
 */
/*--------------------------------------------------------------------------------*/
bool VBAPanner2D::FindSpeakers(const Position& pos, SpeakerSet_t& speakerset) const
{
  Position cpos  = pos.Cart();                                // cartesian version of position
  Position cupos = cpos.Unit();                               // use unit vector version of cartesian position
  uint_t   i;

  // only update if no previous solution found or position has changed
  if (!speakerset.valid ||
      (cpos.pos.x != speakerset.x) ||
      (cpos.pos.y != speakerset.y) ||
      (cpos.pos.z != speakerset.z)) {
    double bestgains[MaxSpeakersPerSet];
    double besterror = 1.0e30;
    double src_dist  = cpos.Mod();                          // source distance away from origin
    double src_delay = src_dist / speed_of_sound;           // source delay due to distance
    double src_gain  = pow(decay_power, 1.0 - src_dist);    // source gain compensation due to distance, note that gain gets larger as source moves closer to origin (assuming 1m == 1.0)
    uint_t bestgroup = ~0;

    // try to use old speaker group and see if it's still valid
    if (speakerset.valid && (speakerset.group < groups.size())) {
      bestgroup = speakerset.group;
      besterror = TestSpeakers(cupos, groups[bestgroup], bestgains);
    }

    // cycle through speaker sets and look for minimal error (and stop if error = 0.0)
    for (i = 0; (i < groups.size()) && (besterror > 0.0); i++) {
      double gains[MaxSpeakersPerSet];
      double error;

      if ((error = TestSpeakers(cupos, groups[i], gains)) < besterror) {
        // this speaker set is better than current best set
        bestgroup = i;
        besterror = error;
        memcpy(bestgains, gains, sizeof(gains));
      }
    }

    // a solution (possibly inexact) found
    if (bestgroup < groups.size()) {
      const SpeakerGroup_t& group = groups[bestgroup];

      // found a solution
      speakerset.valid = true;
      speakerset.group = bestgroup;
      speakerset.error = besterror;

      // grab speaker data
      for (i = 0; (i < MaxSpeakersPerSet) && (i < Dimensions); i++) {//NUMBEROF(speakerset.speakers)) && (i < NUMBEROF(group.speakers)); i++) {
        const Speaker_t& speaker = speakers[group.speakers[i]];

        DEBUG3(("VB2D Speaker %u: index %u channel %u gain (%0.3le * %0.3le * %0.3le (=power(%0.3le, (1.0 - %0.3le))) = %0.3le) delay %0.3lfs", i, group.speakers[i], speaker.channel, bestgains[i], speaker.gain, src_gain, decay_power, src_dist, bestgains[i] * speaker.gain * src_gain, speaker.delay_compensation + src_delay));

        speakerset.speakers[i].index   = group.speakers[i];
        speakerset.speakers[i].channel = speaker.channel;
        speakerset.speakers[i].gain    = bestgains[i] * speaker.gain * src_gain;    // apply gain due to source position
        speakerset.speakers[i].delay   = speaker.delay_compensation + src_delay;    // add delay due to source position
      }
    }
    else {
      // no solution found -> collapse to first speakers
      DEBUG3(("No panning solution found?!"));
      memset(speakerset.speakers, 0, sizeof(speakerset.speakers));

      for (i = 0; (i < MaxSpeakersPerSet) && (i < Dimensions); i++) {//for (i = 0; (i < NUMBEROF(speakerset.speakers)) && (i < speakers.size()); i++) {
        const Speaker_t& speaker = speakers[i];

        speakerset.speakers[i].index   = i;
        speakerset.speakers[i].channel = speaker.channel;
        speakerset.speakers[i].gain    = speaker.gain * src_gain;                   // apply gain due to source position
        speakerset.speakers[i].delay   = speaker.delay_compensation + src_delay;    // add delay due to source position
      }

      speakerset.valid = true;
      speakerset.group = 0;
      speakerset.error = 0.0;
    }

    if (speakerset.valid) {
      speakerset.x = cpos.pos.x;
      speakerset.y = cpos.pos.y;
      speakerset.z = cpos.pos.z;
    }
  }

  return speakerset.valid;
}

/*--------------------------------------------------------------------------------*/
/** Find groups of speakers from existing speakers
 */
/*--------------------------------------------------------------------------------*/
void VBAPanner2D::FindSpeakerGroups()
{
  uint_t i,j;
  SpeakerGroup_t g;

  SortSpeakers();

  // populate groups vector with adjacent pairs of sorted speakers
  groups.empty();
  for (i=0; i<speakers.size(); i++)
  {
    for (j=0; j<Dimensions; j++)
    {
      g.speakers[j] = (i+j < speakers.size()) ? i+j : 0;
    }
    Invert(g);
    groups.push_back(g);
  }
}

/*--------------------------------------------------------------------------------*/
/** Sort speakers by azimuth to generate panning pairs.
 */
/*--------------------------------------------------------------------------------*/
void VBAPanner2D::SortSpeakers()
{
  // sort speakers by azimuth angle
  std::sort(speakers.begin(), speakers.end(), *this); // use bool operator() (Speaker_t, Speaker_t) method to sort

#if DEBUG_LEVEL >= 2
  {
    uint_t i;

    for (i = 0; i < speakers.size(); i++) {
      DEBUG("Speaker %u/%u: gain %0.2lfdB delay %0.3lfs (full delay %0.3lfs)", i + 1, (uint_t)speakers.size(), 20.0 * log10(speakers[i].gain), speakers[i].delay_compensation, speakers[i].delay);
    }
  }
#endif
}

/*--------------------------------------------------------------------------------*/
/** Calculate the matrix required to solve the speaker position problem
 */
/*--------------------------------------------------------------------------------*/
bool VBAPanner2D::Invert(SpeakerGroup_t& group)
{
  double mat[Dimensions][Dimensions];
  uint_t i;

  memset(group.inv, 0, sizeof(group.inv));

  // create matrix from positions of speakers
  for (i = 0; i < Dimensions; i++) {//NUMBEROF(group.speakers); i++) {
    const Position& pos = speakers[group.speakers[i]].vec;  // use unit vector of position
    mat[0][i] = pos.pos.x;
    mat[1][i] = pos.pos.y;
  }

  /* Calculate the determinant of mat[][] */
  double det = mat[0][0] * mat[1][1] - mat[0][1] * mat[1][0];

  // can't invert
  if (det == 0.0) {
    ERROR("Cannot invert matrix because determinant is zero (speakers %u, %u)", group.speakers[0], group.speakers[1]);
    return false;
  }

  /* Calculate the inverse matrix of mat[][] */
  group.inv[0][0] =  mat[0][0] / det;
  group.inv[1][0] = -mat[0][1] / det;
  group.inv[0][1] = -mat[1][0] / det;
  group.inv[1][1] =  mat[1][1] / det;

#if DEBUG_LEVEL >= 4
  // test
  for (i = 0; i < Dimensions; i++) {//NUMBEROF(group.speakers); i++) {
    const Position& pos = speakers[group.speakers[i]].vec;  // use unit vector of position
    double gains[MaxSpeakersPerSet];
    double error;

    if ((error = TestSpeakers(pos, group, set)) == 0.0) {
      DEBUG5(("Group (%u, %u) speaker %u : %0.4lf, %0.4lf",
              group.speakers[0], group.speakers[1],
              group.speakers[i],
              gains[0], gains[1]));
    }
    else {
      ERROR("Group (%u, %u) speaker %u : error %0.4le : %0.4lf, %0.4lf",
            group.speakers[0], group.speakers[1],
            group.speakers[i],
            error,
            gains[0], gains[1]));
  }
}
#endif

return true;
}

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
double VBAPanner2D::TestSpeakers(const Position& pos, const SpeakerGroup_t& group, double gains[MaxSpeakersPerSet]) const
{
  static const double limit = -.001;  // limit of gain before this group becomes erroneous
  double error = 0.0;
  uint_t i;

  // clear all gains
  memset(gains, 0, MaxSpeakersPerSet * sizeof(*gains));

  for (i = 0; (i < MaxSpeakersPerSet) && (i < NUMBEROF(group.inv)); i++) {
    // calculate gain of this speaker
    double gain   = DotProduct(pos, group.inv[i]);

    // calculate error for this entry (= level below limit)
    double errori = MAX(limit - gain, 0.0);

    // add error
    error += errori * errori;

    // ensure no negative gains!
    if (gains) gains[i] = MAX(gain, 0.0);
  }

  // if error = 0, position lies within group (exact match)
  // if error > 0, position lies outside group with smaller errors being better
  return error;
}

BBC_AUDIOTOOLBOX_END
