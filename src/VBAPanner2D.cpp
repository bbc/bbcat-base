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
#include "EnhancedFile.h"
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
  Speaker_t sp =
  {
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
  EnhancedFile fp;

  if (fp.fopen(filename, "r"))
  {
    static char line[256];
    bool readingspeakers = true;
    int l;

    while ((l = fp.readline(line, sizeof(line))) != EOF)
    {
      Position pos;
      uint_t   channel, spn[3];
      double   gain = 0.0;

      if ((l == 0) || (line[0] == '#')) continue;

      // attempt to interpret data as speaker positions or speaker indices
      if (readingspeakers && (sscanf(line, "%u %lf,%lf,%lf %lf", &channel, &pos.pos.x, &pos.pos.y, &pos.pos.z, &gain) >= 4))
      {
        pos.polar   = false;

        DEBUG3(("Adding %s as speaker (channel %u, gain %0.2lfdB)", pos.ToString().c_str(), channel, gain));

        AddSpeaker(channel, pos, pow(10.0, .05 * gain));
      }
      else if (!readingspeakers && (sscanf(line, "%u,%u,%u", spn, spn + 1, spn + 2) == 3))
      {
        DEBUG2(("2D panner doesn't use speaker groups"));
      }
      // else detect section
      else if (strncmp(line, "speakers", 8) == 0)
      {
        DEBUG3(("Reading speaker locations..."));
        readingspeakers = true;
      }
      else if (strncmp(line, "groups", 6) == 0)
      {
        DEBUG2(("2D panner doesn't use speaker groups"));
      }
    }

    fp.fclose();

    FindSpeakerGroups();
  }
  else ERROR("Failed to open file '%s' for reading", filename);
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

    for (i = 0; i < speakers.size(); i++)
    {
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
  for (i = 0; i < Dimensions; i++)
  {
    const Position& pos = speakers[group.speakers[i]].vec;  // use unit vector of position
    mat[0][i] = pos.pos.x;
    mat[1][i] = pos.pos.y;
  }

  /* Calculate the determinant of mat[][] */
  double det = mat[0][0] * mat[1][1] - mat[0][1] * mat[1][0];

  // can't invert
  if (det == 0.0)
  {
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
  for (i = 0; i < Dimensions; i++)
  {
    const Position& pos = speakers[group.speakers[i]].vec;  // use unit vector of position
    double gains[MaxSpeakersPerSet];
    double error;

    if ((error = TestSpeakers(pos, group, set)) == 0.0)
    {
      DEBUG5(("Group (%u, %u) speaker %u : %0.4lf, %0.4lf",
              group.speakers[0], group.speakers[1],
              group.speakers[i],
              gains[0], gains[1]));
    }
    else
    {
      ERROR("Group (%u, %u) speaker %u : error %0.4le : %0.4lf, %0.4lf",
            group.speakers[0], group.speakers[1],
            group.speakers[i],
            error,
            gains[0], gains[1]);
    }
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

  for (i = 0; (i < MaxSpeakersPerSet) && (i < NUMBEROF(group.inv)); i++)
  {
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
