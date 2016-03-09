/*
This file is part of MAE.

History:

[2006.03.29 - Helio Perroni Filho] Created.
*/

#ifndef __MAE_GARBAGE_COLLECTOR
#define __MAE_GARBAGE_COLLECTOR

#include <string>
using std::string;

#include <vector>
using std::vector;

#include "primitives.hpp"
using mae::util::Primitives;

namespace mae
{
	class GarbageCollector;
	
	class Object;
};

/**
An object under the responsibility of a garbage collector. Classes that want to be "collectable" must either inherit from this class or wrap themselves around an instance of a subclass.
*/
class mae::Object
{
	/*
	Constructor Section
	*/

	public: virtual ~Object()
	{
	};
	
	/*
	Method Section
	*/
	
	public: virtual string toString()
	{
		return Primitives::toString((int) this);
	}
};

class mae::GarbageCollector
{
	/*
	Attribute Section
	*/

	/** Vector of managed objects. */
	private: vector<Object*> objects;

	/*
	Constructor Section
	*/
	
	/**
	Default constructor.
	*/
	public: GarbageCollector();
	
	/**
	Default destructor.
	*/
	public: ~GarbageCollector();
	
	/**
	Returns a reference to the default garbage collector.
	*/
	public: static GarbageCollector& getDefault();

	/*
	Method Section
	*/
	
	/**
	Adds an object to this garbage collector.
	
	@param object Object to be managed.
	
	@throws runtime_error If this object is already managed by this garbage collector.
	*/
	public: void manage(Object& object);
};

namespace mae
{
	/**
	Template wrapper to manage objects that can't or won't inherit from <code>mae::Object</code>.
	*/
	template<class Wrapped> class WrapperObject: public Object
	{
		private: Wrapped* wrapped;
		
		public: WrapperObject(Wrapped* wrapped)
		{
			this->wrapped = wrapped;
		};
		
		public: virtual ~WrapperObject()
		{
			delete wrapped;
		};
		
		public: virtual string toString()
		{
			return (*wrapped).toString();
		};
	};


	template<class Wrapped> void manage(Wrapped* wrapped)
	{
		WrapperObject<Wrapped>* wrapper = new WrapperObject<Wrapped>(wrapped);
		GarbageCollector& collector = GarbageCollector::getDefault();
		collector.manage(*wrapper);
	};
};

#endif
