#ifndef __VBAPANNER__
#define __VBAPANNER__

#include <vector>

#include "3DPosition.h"

BBC_AUDIOTOOLBOX_START

/*--------------------------------------------------------------------------------*/
/** VBAP based panner
 *
 * Uses a list of speaker triplets to find the best triplet for any position and then
 * calculates the gain for each speaker within the triplet to create the impression
 * that the sound is coming from the desired position
 *
 */
/*--------------------------------------------------------------------------------*/
class VBAPanner {
public:
	VBAPanner();
	virtual ~VBAPanner();

	enum {
		// to avoid hardcoded 3's everywhere!
		Dimensions = 3,
		MaxSpeakersPerSet = 4,
	};

	/*--------------------------------------------------------------------------------*/
	/** Set decay power due to distance (==2 for inverse square law)
	 */
	/*--------------------------------------------------------------------------------*/
	void SetDecayPower(double n);

	/*--------------------------------------------------------------------------------*/
	/** Set speed of sound in m/s
	 */
	/*--------------------------------------------------------------------------------*/
	void SetSpeedOfSound(double speed);

	/*--------------------------------------------------------------------------------*/
	/** Read back speaker positions (and possibly groups) from text file
	 */
	/*--------------------------------------------------------------------------------*/
	void Read(const char *filename);

	/*--------------------------------------------------------------------------------*/
	/** Add a speaker at specified position with specified additional gain
	 *
	 * @param channel channel that the speaker appears on 
	 * @param pos physical position of speaker
	 * @param gain modification of gain (1 == no modification)
	 */
	/*--------------------------------------------------------------------------------*/
	void AddSpeaker(uint_t channel, const Position& pos, double gain = 1.0);

	/*--------------------------------------------------------------------------------*/
	/** Add a group of speakers to group list
	 *
	 * @param spn a list of speaker indices
	 *
	 */
	/*--------------------------------------------------------------------------------*/
	void AddSpeakerGroup(const uint_t spn[Dimensions]);

	/*--------------------------------------------------------------------------------*/
	/** Find groups of speakers from existing speakers
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void FindSpeakerGroups() {}

	// structure describing a set of speakers with individual gains to combine to create a virtual source
	typedef struct {
		bool   valid;			// true if group found
		uint_t group;			// speaker group
		double error;			// speaker group error
		double x, y, z;			// last test position
		struct {
			uint_t index;		// speaker index
			uint_t channel;		// output channel
			double gain;		// gain of this speaker
			double delay;		// delay in s to delay this speaker by
		} speakers[MaxSpeakersPerSet];
	} SpeakerSet_t;

	typedef struct {
		uint_t channel;		// output channel
		double gain;		// gain of this speaker
		double delay;		// delay in s to delay this speaker by
	} SpeakerOutput_t;

	/*--------------------------------------------------------------------------------*/
	/** Find gain factors for a sound source using a set of speakers from a list of speaker sets
	 *
	 * @param pos sound source position
	 * @param speakerset structure populated by this function
	 *
	 * @return true if speaker set found
	 */
	/*--------------------------------------------------------------------------------*/
	bool FindSpeakers(const Position& pos, SpeakerSet_t& speakerset) const;

protected:
	typedef struct {
		uint_t   channel;						// channel on which this speaker appears
		Position vec;							// unit vector of speaker position
		double   dist;							// distance of speaker
		double   usergain;						// user supplied gain of speaker
		double   gain;							// compensating gain of speaker (due to distance from origin) INCLUDE usergain
		double   delay;							// delay in s of speaker (due to distance from origin)
		double   delay_compensation;			// delay compensation (= maximum delay of all speakers - delay of this speaker)
	} Speaker_t;
	typedef struct {
		uint_t speakers[Dimensions];			// index of speaker in this group
		double inv[Dimensions][Dimensions];		// inverse matrix of speaker positions
	} SpeakerGroup_t;

	static bool SortGroups(const SpeakerGroup_t& a, const SpeakerGroup_t& b) {
		uint_t na = ((1000 * a.speakers[0]) + a.speakers[1]) * 1000 + a.speakers[2];
		uint_t nb = ((1000 * b.speakers[0]) + b.speakers[1]) * 1000 + b.speakers[2];
		return (na < nb);
	}

	/*--------------------------------------------------------------------------------*/
	/** Calculate gain and delay due to distance of speaker from origin
	 */
	/*--------------------------------------------------------------------------------*/
	void SetGainAndDelay(Speaker_t& speaker);

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
	double TestSpeakers(const Position& pos, const SpeakerGroup_t& group, double gains[MaxSpeakersPerSet]) const;

	void DebugSpeaker(FILE *fp, uint_t sp, const char *str = NULL);
	void DebugGroup(FILE *fp, const SpeakerGroup_t& group, const char *str = NULL);

protected:
	std::vector<Speaker_t>      speakers;
	std::vector<SpeakerGroup_t> groups;
	double decay_power;
	double speed_of_sound;
	double max_dist;
	double max_delay;
};

/*----------------------------------------------------------------------------------------------------*/

class VBAPannerPulkki : public VBAPanner {
public:
	VBAPannerPulkki();
	virtual ~VBAPannerPulkki();

	/*--------------------------------------------------------------------------------*/
	/** Find groups of speakers from existing speakers
	 *
	 * @note currently does NOT work!
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void FindSpeakerGroups();

protected:
	typedef struct {
		uint_t sp1, sp2;
		double dist;
	} Pair_t;
	typedef struct {
		uint_t sp1, sp2, sp3;
		double dist;
	} Triplet_t;
	static bool SortPairsByDistance(const Pair_t& a, const Pair_t& b)	   	   {return (a.dist < b.dist);}
	static bool SortPairsBySpeakers(const Pair_t& a, const Pair_t& b)	   	   {return ((a.sp1 * 1000 + a.sp2) < (b.sp1 * 1000 + b.sp2));}
	static bool SortTripletsByDistance(const Triplet_t& a, const Triplet_t& b) {return (a.dist < b.dist);}

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
	static bool TestIntersection(const Position& p1, const Position& p2, const Position& p3);

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
	static bool LinesIntersect(const Position& l1p1, const Position& l1p2, const Position& l2p1, const Position& l2p2);
};

BBC_AUDIOTOOLBOX_END

#endif
