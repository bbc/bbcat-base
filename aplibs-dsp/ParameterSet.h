#ifndef __PARAMETER_SET__
#define __PARAMETER_SET__

#include <string>
#include <map>

#include "misc.h"

/*--------------------------------------------------------------------------------*/
/** Collection of parameters, each with name/value pair with type conversion
 *
 */
/*--------------------------------------------------------------------------------*/
class ParameterSet {
public:
	ParameterSet() {}
	ParameterSet(const ParameterSet& obj);
	~ParameterSet() {}

	/*--------------------------------------------------------------------------------*/
	/** Assignment operator
	 */
	/*--------------------------------------------------------------------------------*/
	ParameterSet& operator = (const ParameterSet& obj);

	/*--------------------------------------------------------------------------------*/
	/** Comparison operators
	 */
	/*--------------------------------------------------------------------------------*/
	bool operator == (const ParameterSet& obj) const {return map_compare(values, obj.values);}
	bool operator != (const ParameterSet& obj) const {return !operator == (obj);}

	/*--------------------------------------------------------------------------------*/
	/** Return whether parameter set is empty
	 */
	/*--------------------------------------------------------------------------------*/
	bool IsEmpty() const {return values.empty();}

	/*--------------------------------------------------------------------------------*/
	/** Clear parameter set
	 */
	/*--------------------------------------------------------------------------------*/
	void Clear() {values.clear();}

	/*--------------------------------------------------------------------------------*/
	/** Return a string with each parameter and value (as string)
	 */
	/*--------------------------------------------------------------------------------*/
	std::string ToString() const;

	/*--------------------------------------------------------------------------------*/
	/** Set parameter
	 *
	 * @note any existing value with the same name will be overwritten
	 *
	 */
	/*--------------------------------------------------------------------------------*/
	void Set(const std::string& name, const std::string& val);
	void Set(const std::string& name, bool	      	   	 val);
	void Set(const std::string& name, sint_t     	   	 val);
	void Set(const std::string& name, slong_t   	   	 val);
	void Set(const std::string& name, double     	   	 val);

	/*--------------------------------------------------------------------------------*/
	/** Return whether a parameter exists
	 */
	/*--------------------------------------------------------------------------------*/
	bool Exists(const std::string& name) const {return (values.find(name) != values.end());}

	/*--------------------------------------------------------------------------------*/
	/** Get start and iterators to allow iteration through all values
	 */
	/*--------------------------------------------------------------------------------*/
	typedef std::map<std::string,std::string>::const_iterator Iterator;
	
	Iterator GetBegin() const {return values.begin();}
	Iterator GetEnd()   const {return values.end();}

	/*--------------------------------------------------------------------------------*/
	/** Get value
	 *
	 * @param name parameter name
	 * @param val reference to value to be set
	 *
	 * @return true if parameter found and value extracted
	 */
	/*--------------------------------------------------------------------------------*/
	bool Get(const std::string& name, std::string& val) const;
	bool Get(const std::string& name, bool&        val) const;
	bool Get(const std::string& name, sint_t&      val) const;
	bool Get(const std::string& name, uint_t&      val) const;
	bool Get(const std::string& name, slong_t&     val) const;
	bool Get(const std::string& name, ulong_t&     val) const;
	bool Get(const std::string& name, double&      val) const;

protected:
	std::map<std::string,std::string> values;
};

#endif
