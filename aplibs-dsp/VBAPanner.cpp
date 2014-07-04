
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <map>
#include <string>
#include <algorithm>

#define DEBUG_LEVEL 1
#include "VBAPanner.h"

#if DEBUG_LEVEL >= 5
#define OUTPUT_GROUPS 1
#else
#define OUTPUT_GROUPS 0
#endif

using namespace std;

BBC_AUDIOTOOLBOX_START

VBAPanner::VBAPanner() : decay_power(1.4),
                         speed_of_sound(330.0),
                         max_dist(0.0),
                         max_delay(0.0)
{
}

VBAPanner::~VBAPanner()
{
}

/*--------------------------------------------------------------------------------*/
/** Set decay power due to distance (==2 for inverse square law)
 */
/*--------------------------------------------------------------------------------*/
void VBAPanner::SetDecayPower(double n)
{
  uint_t i;

  decay_power = n;

  for (i = 0; i < speakers.size(); i++) {
    SetGainAndDelay(speakers[i]);
  }
}

/*--------------------------------------------------------------------------------*/
/** Set speed of sound in m/s
 */
/*--------------------------------------------------------------------------------*/
void VBAPanner::SetSpeedOfSound(double speed)
{
  uint_t i;

  speed_of_sound = speed;
  max_delay      = 0.0;

  for (i = 0; i < speakers.size(); i++) {
    SetGainAndDelay(speakers[i]);
  }
}

/*--------------------------------------------------------------------------------*/
/** Calculate gain and delay due to distance of speaker from origin
 */
/*--------------------------------------------------------------------------------*/
void VBAPanner::SetGainAndDelay(Speaker_t& speaker)
{
  if (speaker.dist > max_dist) {
    max_dist = speaker.dist;

    uint_t i;
    for (i = 0; i < speakers.size(); i++) {
      speakers[i].gain  = speakers[i].usergain * pow(decay_power, speakers[i].dist - max_dist);
    }
  }

  // set compensating gain relative to that on the unit sphere
  speaker.gain  = speaker.usergain * pow(decay_power, speaker.dist - max_dist);

  // set delay as absolute delay from origin
  speaker.delay = speaker.dist / speed_of_sound;

  if (speaker.delay > max_delay) {
    // speaker delay is more than previous maximum, recalculate all delay compensations
    max_delay = speaker.delay;

    uint_t i;
    for (i = 0; i < speakers.size(); i++) {
      speakers[i].delay_compensation = max_delay - speakers[i].delay;
    }
  }

  speaker.delay_compensation = max_delay - speaker.delay;
}

/*--------------------------------------------------------------------------------*/
/** Store cartesian version speaker position
 */
/*--------------------------------------------------------------------------------*/
void VBAPanner::AddSpeaker(uint_t channel, const Position& pos, double gain)
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

  DEBUG5(("Adding speaker index %u, channel %u", (uint_t)speakers.size(), channel));

  speakers.push_back(sp);
}

/*--------------------------------------------------------------------------------*/
/** Read back speaker positions and groups from text file
 */
/*--------------------------------------------------------------------------------*/
void VBAPanner::Read(const char *filename)
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
        AddSpeakerGroup(spn);
      }
      // else detect section
      else if (strncmp(line, "speakers", 8) == 0) {
        DEBUG3(("Reading speaker locations..."));
        readingspeakers = true;
      }
      else if (strncmp(line, "groups", 6) == 0) {
        DEBUG3(("Reading speaker groups..."));
        readingspeakers = false;
      }
    }

    fclose(fp);

#if DEBUG_LEVEL >= 2
    {
      uint_t i;

      for (i = 0; i < speakers.size(); i++) {
        DEBUG("Speaker %u/%u: gain %0.2lfdB delay %0.3lfs (full delay %0.3lfs)", i + 1, (uint_t)speakers.size(), 20.0 * log10(speakers[i].gain), speakers[i].delay_compensation, speakers[i].delay);
      }
    }
#endif
        
    if (groups.size() == 0) {
      FindSpeakerGroups();

      sort(groups.begin(), groups.end(), SortGroups);

#if DEBUG_LEVEL >= 2
      {
        uint_t i;

        for (i = 0; i < groups.size(); i++) {
          DEBUG("Speaker group %u/%u: %u, %u, %u", i + 1, (uint_t)groups.size(), groups[i].speakers[0], groups[i].speakers[1], groups[i].speakers[2]);
        }
      }
#endif

#if OUTPUT_GROUPS
      // output data in GNUPlot format...
      FILE *fp1, *fp2, *fp3;
      if ((fp1 = fopen("plot.gnp", "w")) != NULL) {
        uint_t i, j;

        if ((fp2 = fopen("speakers.dat", "w")) != NULL) {
          for (i = 0; i < speakers.size(); i++) {
            DebugSpeaker(fp2, i);
          }
                
          fclose(fp2);
        }

        if ((fp2 = fopen("speaker-groups.dat", "w")) != NULL) {
          for (i = 0; i < groups.size(); i++) {
            DebugGroup(fp2, groups[i]);
          }
                
          fclose(fp2);
        }

        for (i = 0; i < groups.size(); i++) {
          string filename;

          Printf(filename, "speaker-group-%u.dat", i + 1);

          if ((fp3 = fopen(filename.c_str(), "w")) != NULL) {
            fprintf(fp1, "splot ");
            fprintf(fp1, "'speaker-groups.dat' using 1:2:3 with points lt 0 pt 1, ");
            fprintf(fp1, "'%s' using 1:2:3 with lines lt 0,", filename.c_str());
            fprintf(fp1, "'%s' using 5:6:7 with point lt 1", filename.c_str());
            fprintf(fp1, "\npause -1\n");

            for (j = 0; j <= i; j++) {
              DebugGroup(fp2, groups[j]);
            }
                        
            fclose(fp3);
          }
        }

        fclose(fp1);
      }
#endif
    }
  }
  else ERROR("Failed to open file '%s' for reading", filename);
}

void VBAPanner::DebugSpeaker(FILE *fp, uint_t sp, const char *str)
{
  const Speaker_t& speaker = speakers[sp];

  fprintf(fp, "%0.16le %0.16le %0.16le %u %s\n", speaker.vec.pos.x, speaker.vec.pos.y, speaker.vec.pos.z, sp, str ? str : "");
}

void VBAPanner::DebugGroup(FILE *fp, const SpeakerGroup_t& group, const char *str)
{
  Position pos;
  string str2;
  uint_t i;

  for (i = 0; i < NUMBEROF(group.speakers); i++) {
    pos += speakers[group.speakers[i]].vec;
  }

  pos /= (double)NUMBEROF(group.speakers);

  Printf(str2, "%0.16le %0.16le %0.16le %s", pos.pos.x, pos.pos.y, pos.pos.z, str ? str : "");

  for (i = 0; i <= NUMBEROF(group.speakers); i++) {
    DebugSpeaker(fp, group.speakers[i % NUMBEROF(group.speakers)], str2.c_str());
  }
    
  fprintf(fp, "\n\n");
}

/*--------------------------------------------------------------------------------*/
/** Add a group of speakers to group list
 *
 * @param spn a list of speaker indices
 *
 */
/*--------------------------------------------------------------------------------*/
void VBAPanner::AddSpeakerGroup(const uint_t spn[Dimensions])
{
  SpeakerGroup_t group;
  uint_t i;

  memset(&group, 0, sizeof(group));

  for (i = 0; i < NUMBEROF(group.speakers); i++) {
    if (spn[i] < speakers.size()) {
      group.speakers[i] = spn[i];
    }
    else {
      ERROR("Invalid speaker index specified (speaker %u, number of speakers %u)", spn[i], (uint_t)speakers.size());
      break;
    }
  }
    
  if (i == NUMBEROF(group.speakers)) {
    if (Invert(group)) {
      groups.push_back(group);
    }
    else ERROR("Cannot invert matrix formed by speaker group (%u, %u, %u)", spn[0], spn[1], spn[2]);
  }
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
double VBAPanner::TestSpeakers(const Position& pos, const SpeakerGroup_t& group, double gains[MaxSpeakersPerSet]) const
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

/*--------------------------------------------------------------------------------*/
/** Find gain factors for a sound source using a set of speakers from a list of speaker sets
 *
 * @param pos sound source position
 * @param speakerset structure populated by this function
 *
 * @return true if speaker set found
 */
/*--------------------------------------------------------------------------------*/
bool VBAPanner::FindSpeakers(const Position& pos, SpeakerSet_t& speakerset) const
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
      for (i = 0; (i < NUMBEROF(speakerset.speakers)) && (i < NUMBEROF(group.speakers)); i++) {
        const Speaker_t& speaker = speakers[group.speakers[i]];

        DEBUG3(("Speaker %u: index %u channel %u gain (%0.3le * %0.3le * %0.3le (=power(%0.3le, (1.0 - %0.3le))) = %0.3le) delay %0.3lfs", i, group.speakers[i], speaker.channel, bestgains[i], speaker.gain, src_gain, decay_power, src_dist, bestgains[i] * speaker.gain * src_gain, speaker.delay_compensation + src_delay));

        speakerset.speakers[i].index   = group.speakers[i];
        speakerset.speakers[i].channel = speaker.channel;
        speakerset.speakers[i].gain    = bestgains[i] * speaker.gain * src_gain;    // apply gain due to source position
        speakerset.speakers[i].delay   = speaker.delay_compensation + src_delay;    // add delay due to source position
      }
    }
    else {
      // no solution found -> collapse to first speakers
      memset(speakerset.speakers, 0, sizeof(speakerset.speakers));

      for (i = 0; (i < NUMBEROF(speakerset.speakers)) && (i < speakers.size()); i++) {
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
/** Calculate the matrix required to solve the speaker position problem
 */
/*--------------------------------------------------------------------------------*/
bool VBAPanner::Invert(SpeakerGroup_t& group)
{
  double mat[Dimensions][Dimensions], trans[Dimensions][Dimensions], adjoint[Dimensions][Dimensions];
  uint_t i, j;

  memset(group.inv, 0, sizeof(group.inv));

  // create matrix from positions of speakers
  for (i = 0; i < NUMBEROF(group.speakers); i++) {
    const Position& pos = speakers[group.speakers[i]].vec;  // use unit vector of position
    mat[0][i] = pos.pos.x;
    mat[1][i] = pos.pos.y;
    mat[2][i] = pos.pos.z;
  }

  /* Calculate the determinant of mat[][] */
  double a   = mat[0][0] * (mat[1][1] * mat[2][2] - mat[2][1] * mat[1][2]);
  double b   = mat[0][1] * (mat[1][0] * mat[2][2] - mat[2][0] * mat[1][2]);
  double c   = mat[0][2] * (mat[1][0] * mat[2][1] - mat[2][0] * mat[1][1]);
  double det = a - b + c;

  // can't invert
  if (det == 0.0) {
    ERROR("Cannot invert matrix because determinant is zero (speakers %u, %u, %u)", group.speakers[0], group.speakers[1], group.speakers[2]);
    return false;
  }

  /* Find the transpose matrix of mat[][] */
  for (i = 0; i < Dimensions; i++) {
    for (j = 0; j < Dimensions; j++) {
      trans[i][j] = mat[j][i];
    }
  }
    
  /* Calculate the adjoint matrix of mat[][] */
  adjoint[0][0] =   trans[1][1] * trans[2][2] - trans[2][1] * trans[1][2];
  adjoint[0][1] = -(trans[1][0] * trans[2][2] - trans[2][0] * trans[1][2]);
  adjoint[0][2] =   trans[1][0] * trans[2][1] - trans[2][0] * trans[1][1];
  adjoint[1][0] = -(trans[0][1] * trans[2][2] - trans[2][1] * trans[0][2]);
  adjoint[1][1] =   trans[0][0] * trans[2][2] - trans[2][0] * trans[0][2];
  adjoint[1][2] = -(trans[0][0] * trans[2][1] - trans[2][0] * trans[0][1]);
  adjoint[2][0] =   trans[0][1] * trans[1][2] - trans[1][1] * trans[0][2];
  adjoint[2][1] = -(trans[0][0] * trans[1][2] - trans[1][0] * trans[0][2]);
  adjoint[2][2] =   trans[0][0] * trans[1][1] - trans[1][0] * trans[0][1];
    
  /* Calculate the inverse matrix of mat[][] */
  for (i = 0; i < Dimensions; i++) {
    for (j = 0; j < Dimensions; j++) {
      group.inv[i][j] = adjoint[i][j] / det;
    }
  }

#if DEBUG_LEVEL >= 4
  // test
  for (i = 0; i < NUMBEROF(group.speakers); i++) {
    const Position& pos = speakers[group.speakers[i]].vec;  // use unit vector of position
    double gains[MaxSpeakersPerSet];
    double error;

    if ((error = TestSpeakers(pos, group, set)) == 0.0) {
      DEBUG5(("Group (%u, %u, %u) speaker %u : %0.4lf, %0.4lf, %0.4lf",
              group.speakers[0], group.speakers[1], group.speakers[2],
              group.speakers[i],
              gains[0], gains[1], gains[2]));
    }
    else {
      ERROR("Group (%u, %u, %u) speaker %u : error %0.4le : %0.4lf, %0.4lf, %0.4lf",
            group.speakers[0], group.speakers[1], group.speakers[2],
            group.speakers[i],
            error,
            gains[0], gains[1], gains[2]));
  }
}
#endif

return true;
}

/*----------------------------------------------------------------------------------------------------*/

#if DEBUG_LEVEL >= 4
#define DEBUG_INTERSECTIONS 1
#define DEBUG_REJECTED      1
#else
#define DEBUG_INTERSECTIONS 0
#define DEBUG_REJECTED      0
#endif

VBAPannerPulkki::VBAPannerPulkki() : VBAPanner()
{
}

VBAPannerPulkki::~VBAPannerPulkki()
{
}

/*--------------------------------------------------------------------------------*/
/** Given two points on an unit sphere and an third point, return true if the third point lines on the line formed between the two points
 *
 * @param p1 point 1 of line
 * @param p2 point 2 of line
 * @param p3 point to test
 *
 * @return true if p3 lines on line formed by p1 and p2
 */
/*--------------------------------------------------------------------------------*/
bool VBAPannerPulkki::TestIntersection(const Position& p1, const Position& p2, const Position& p3)
{
  double lim  = .01;
  double a1   = AbsAngle(p1, p3);
  double a2   = AbsAngle(p3, p2);
  double a3   = AbsAngle(p1, p2);
  bool within = (/*(a1 >= lim) && (a2 >= lim) &&*/ ((a1 + a2 - a3) < lim));

  DEBUG5(("P1(%0.3lf,%0.3lf,%0.3lf) - %0.1lf deg - Intersection(%0.3lf,%0.3lf,%0.3lf) - %0.1lf deg - P2(%0.3lf,%0.3lf,%0.3lf) (P1 - P2: %0.1lf deg): ((%0.3lf >= %0.3lf) && (%0.3lf >= %0.3lf) && (%0.3lf < %0.3lf)): %u",
          p1.pos.x, p1.pos.y, p1.pos.z,
          a1,
          p3.pos.x, p3.pos.y, p3.pos.z,
          a2,
          p2.pos.x, p2.pos.y, p2.pos.z,
          a3,
          a1, lim,
          a2, lim,
          a1 + a2 - a3, lim,
          (uint_t)within));

  return within;
}

/*--------------------------------------------------------------------------------*/
/** Return whether two lines on a unit sphere intersect
 *
 * @param l1p1 point 1 of line 1
 * @param l1p2 point 2 of line 1
 * @param l2p1 point 1 of line 2
 * @param l2p2 point 2 of line 2
 *
 * @return true if lines intersect
 */
/*--------------------------------------------------------------------------------*/
bool VBAPannerPulkki::LinesIntersect(const Position& l1p1, const Position& l1p2, const Position& l2p1, const Position& l2p2)
{
  // determine whether lines intersect by finding the point of intersection of the two great circles formed by the lines
  // and then checking that the point lies within each line

  // TODO: optimize after confirming that it works!
    
  // the point of intersection = ((l1p1 x l1p2) x (l2p1 x l2p2)) and its antipodean version
  Position int1 = CrossProduct(CrossProduct(l1p1, l1p2), CrossProduct(l2p1, l2p2)).Unit();
  Position int2 = -int1;

  // check if int1 within line 1:
  // | l1p1 /_ l2p2 | ~= | l1p1 /_ int1 | + | int1 /_ l1p2 | (within .1 degree)
  bool int1inl1 = TestIntersection(l1p1, l1p2, int1);

  // check if int1 within line 2:
  // | l2p1 /_ l2p2 | ~= | l2p1 /_ int1 | + | int1 /_ l2p2 | (within .1 degree)
  bool int1inl2 = TestIntersection(l2p1, l2p2, int1);

  // if int1 in line 1 AND line 2, return true
  if (int1inl1 && int1inl2) return true;

  // check if int2 within line 1:
  // | l1p1 /_ l2p2 | ~= | l1p1 /_ int2 | + | int2 /_ l1p2 | (within .1 degree)
  bool int2inl1 = TestIntersection(l1p1, l1p2, int2);

  // check if int2 within line 2:
  // | l2p1 /_ l2p2 | ~= | l2p1 /_ int2 | + | int2 /_ l2p2 | (within .1 degree)
  bool int2inl2 = TestIntersection(l2p1, l2p2, int2);

  // if int2 in line 1 AND line 2, return true
  return (int2inl1 && int2inl2);
}

#if DEBUG_INTERSECTIONS
static bool SpeakerOfIntereset(uint_t sp)
{
  static const uint_t speakersofinterest[] = {0,1,2,16,24};
  uint_t i;
    
  for (i = 0; (i < NUMBEROF(speakersofinterest)) && (sp != speakersofinterest[i]); i++) ;

  return (i < NUMBEROF(speakersofinterest));
}
#endif

/*--------------------------------------------------------------------------------*/
/** Find groups of speakers from existing speakers
 *
 * Method:
 * 1. From the list of speaker positions, create a list of unique speaker pairs, i.e.
 *    every speaker paired with every subsequent speaker
 * 2. Sort this list by the distance* between them, nearest first
 * 3. Remove pairs that cross* any nearer pair
 * 4. Form all possible triangles from pairs of pairs (with a common speaker), ensuring
 *    the 'other' pair (formed by the two non-common speakers) is also a valid pair
 * 5. Discard any triangle for which any OTHER speaker lies within it
 * 
 * The resultant list of triangles is the best set of speaker groups
 *
 * * all distance measurements and tests are done on a unit sphere
 */
/*--------------------------------------------------------------------------------*/
void VBAPannerPulkki::FindSpeakerGroups()
{
  vector<Pair_t> pairs;
#if DEBUG_INTERSECTIONS
  FILE *fp;
#endif
  uint_t i, j;

  // create a list of [angular] distances between pairs of speakers
  for (i = 0; i < speakers.size(); i++) {
    for (j = i + 1; j < speakers.size(); j++) {
      Pair_t pair = {
        .sp1  = i,
        .sp2  = j,
        .dist = AbsAngle(speakers[i].vec, speakers[j].vec),
      };

      DEBUG5(("Pair %u speaker %u->%u dist %0.1lf deg", (uint_t)pairs.size(), i, j, pair.dist));

      pairs.push_back(pair);
    }
  }

  DEBUG3(("Sorting %u pairs...", (uint_t)pairs.size()));

  // sort list, sortest distances first
  sort(pairs.begin(), pairs.end(), SortPairsByDistance);

#if DEBUG_INTERSECTIONS
  uint_t index = 0;
  FILE *scriptfp = fopen("plot-intersections.gnp", "w");
#endif

  // remove further apart pairs that cross nearer pairs
  vector<Pair_t>::iterator it1, it2, it3;
  for (it1 = pairs.begin(); it1 != pairs.end(); ++it1) {
    const Pair_t& pair1 = *it1;

    for (it2 = it1 + 1; it2 != pairs.end();) {
      const Pair_t& pair2 = *it2;

      // don't check pairs with common speaker(s)
      if ((pair2.sp1 != pair1.sp1) &&
          (pair2.sp1 != pair1.sp2) &&
          (pair2.sp2 != pair1.sp1) &&
          (pair2.sp2 != pair1.sp2)) {
        DEBUG5(("Testing pairs %u->%u/%u->%u",
                pair1.sp1, pair1.sp2,
                pair2.sp1, pair2.sp2));

        bool   intersects = LinesIntersect(speakers[pair1.sp1].vec, speakers[pair1.sp2].vec,
                                           speakers[pair2.sp1].vec, speakers[pair2.sp2].vec);

#if DEBUG_INTERSECTIONS
        if (SpeakerOfIntereset(pair1.sp1) &&
            SpeakerOfIntereset(pair1.sp2) &&
            SpeakerOfIntereset(pair2.sp1) &&
            SpeakerOfIntereset(pair2.sp2)) {
          string filename;

          Printf(filename, "intersections_%u_%u-%u_%u-%u.dat", index++, pair1.sp1, pair1.sp2, pair2.sp1, pair2.sp2);

          if ((fp = fopen(filename.c_str(), "w")) != NULL) {
#if SHOW_INTERSECTION_LINES_TO_ORIGIN
            fprintf(fp, "0 0 0 0 0 0\n");
#endif

            DebugSpeaker(fp, pair1.sp1);
            DebugSpeaker(fp, pair1.sp2);

#if SHOW_INTERSECTION_LINES_TO_ORIGIN
            fprintf(fp, "0 0 0 0 0 0\n");
#else
            fprintf(fp, "\n\n");
#endif

            DebugSpeaker(fp, pair2.sp1);
            DebugSpeaker(fp, pair2.sp2);

#if SHOW_INTERSECTION_LINES_TO_ORIGIN
            fprintf(fp, "0 0 0 0 0 0\n");
#endif

            fclose(fp);

            if (scriptfp) {
              fprintf(scriptfp, "splot ");
              fprintf(scriptfp, "'speakers.dat' using 1:2:3 with points, ");
              //fprintf(scriptfp, "'speakers.dat' using 1:2:3 with points, ");
              fprintf(scriptfp, "'%s' using 1:2:3 with lines lt %u\n", filename.c_str(), intersects ? 3 : 2);
              fprintf(scriptfp, "pause -1\n\n");
            }
          }
        }
#endif

        if (intersects) {
          // speaker lines cross over, delete longer one
          DEBUG4(("Pair (%u->%u) and pair (%u->%u) intersect!",
                  pair1.sp1, pair1.sp2,
                  pair2.sp1, pair2.sp2));

          it2 = pairs.erase(it2);
        }
        else {
          DEBUG5(("Pair (%u->%u) and pair (%u->%u) don't intersect",
                  pair1.sp1, pair1.sp2,
                  pair2.sp1, pair2.sp2));

          ++it2;
        }
      }
      else ++it2;
    }
  }

  DEBUG3(("After deletions, %u pairs...", (uint_t)pairs.size()));

  // sort list, lower speaker numbers first
  sort(pairs.begin(), pairs.end(), SortPairsBySpeakers);

#if DEBUG_REJECTED
  uint_t rejindex = 0;
  FILE   *rejfp = NULL;
#endif

  // form triplets from pairs of pairs (with common speaker) and then
  // check that no OTHER speakers lie within the triangle, if any do,
  // discard the triangle
  for (it1 = pairs.begin(); it1 != pairs.end(); ++it1) {
    const Pair_t& pair1 = *it1;

    for (it2 = it1 + 1; it2 != pairs.end(); ++it2) {
      const Pair_t& pair2 = *it2;

      if (pair2.sp1 == pair1.sp2) {
        SpeakerGroup_t group;

#if DEBUG_REJECTED
        if (!rejfp) {
          string filename;

          Printf(filename, "rejected-group-%u.dat", rejindex++);
          rejfp = fopen(filename.c_str(), "w");
        }
#endif

        memset(&group, 0, sizeof(group));
        group.speakers[0] = pair1.sp1;
        group.speakers[1] = pair1.sp2;
        group.speakers[2] = pair2.sp2;

        // test third pair is valid
        for (it3 = pairs.begin(); it3 != pairs.end(); ++it3) {
          const Pair_t& pair3 = *it3;

          if ((pair3.sp1 == pair1.sp1) && (pair3.sp2 == pair2.sp2)) break;
        }

        if (it3 != pairs.end()) {
          // if both pairs share a common speaker, test group made up from them

          DEBUG4(("Testing group made up from pair (%u->%u) and pair (%u->%u)",
                  pair1.sp1, pair1.sp2,
                  pair2.sp1, pair2.sp2));

          // invert matrix formed by the points of the group, this can be used to test if any other speaker is within the polygon formed by the group
          if (Invert(group)) {
            double gains[MaxSpeakersPerSet];

            for (i = 0; i < speakers.size(); i++) {
              // don't test the speakers which form the group!
              if ((i != group.speakers[0]) &&
                  (i != group.speakers[1]) &&
                  (i != group.speakers[2]) &&
                  (TestSpeakers(speakers[i].vec, group, gains) == 0.0)) {
                // speaker is within polygon formed by group, reject the group
                DEBUG4(("Cannot use because of speaker %u (%0.6le, %0.6le, %0.6le)", i, gain[0], gain[1], gain[2]));
                break;
              }
            }

            if (i == speakers.size()) {
              uint_t sp[] = {pair1.sp1, pair1.sp2, pair2.sp2};

              AddSpeakerGroup(sp);
            }
#if DEBUG_REJECTED
            else if (rejfp) {
              const Position& pos = speakers[i].vec;
              Position vector;
              string str;
              uint_t j;

              for (j = 0; j < NUMBEROF(set.speakers); j++) {
                vector += speakers[set.speakers[j].index].vec * gains[j];
              }

              vector = vector.Unit();

              Printf(str, "%0.16le %0.16le %0.16le %0.16le %0.16le %0.16le %0.16le %0.16le %0.16le Rejected because of speaker %u",
                     pos.pos.x, pos.pos.y, pos.pos.z,
                     vector.pos.x, vector.pos.y, vector.pos.z,
                     gains[0], gains[1], gains[2],
                     i);
              DebugGroup(rejfp, group, str.c_str());
            }
#endif
          }
#if DEBUG_REJECTED
          else if (rejfp) {
            DebugGroup(rejfp, group, "Cannot invert matrix");
          }
#endif
        }
#if DEBUG_REJECTED
        else if (rejfp) {
          string str;
                    
          Printf(str, "3rd pair (%u->%u invalid)", pair1.sp1, pair2.sp2);

          DebugGroup(rejfp, group, str.c_str());
        }

        if (rejfp && (ftell(rejfp) > 0)) {
          fclose(rejfp);
          rejfp = NULL;
        }
#endif
      }
    }
  }

#if DEBUG_INTERSECTIONS
  if (scriptfp) fclose(scriptfp);
#endif
}

BBC_AUDIOTOOLBOX_END
