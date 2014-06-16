#ifndef __POSITION_CURSOR__
#define __POSITION_CURSOR__

#include "3DPosition.h"

/*--------------------------------------------------------------------------------*/
/** Base class for the tracking of position as it changes over time
 *
 * Typically, an instance of a derived version of this class would be used for each track
 */
/*--------------------------------------------------------------------------------*/
class PositionCursor {
public:
	PositionCursor() {}
	virtual ~PositionCursor() {}

	/*--------------------------------------------------------------------------------*/
	/** Get position at specified time (ns)
	 */
	/*--------------------------------------------------------------------------------*/
	virtual bool Seek(uint64_t t) = 0;

	/*--------------------------------------------------------------------------------*/
	/** Return position at current time
	 */
	/*--------------------------------------------------------------------------------*/
	virtual const Position *GetPosition() const = 0;
	
	/*--------------------------------------------------------------------------------*/
	/** Return supplementary information
	 */
	/*--------------------------------------------------------------------------------*/
	virtual const ParameterSet *GetPositionSupplement() const {return NULL;}
};

#endif
