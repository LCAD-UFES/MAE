/*
This file is part of MAE.

History:

[2006.03.29 - Helio Perroni Filho] Created.
*/

#include "garbage_collector.hpp"
using mae::GarbageCollector;
using mae::Object;

#include "logging.hpp"
using mae::logging::trace;

/*
Constructor Section
*/

GarbageCollector::GarbageCollector()
{
	trace("GarbageCollector::GarbageCollector");
}

GarbageCollector::~GarbageCollector()
{
	trace("GarbageCollector::~GarbageCollector");

	while (objects.size() > 0)
	{
		Object* object = objects.back();
		objects.pop_back();
		delete object;
	}
}

GarbageCollector& GarbageCollector::getDefault()
{
	trace("GarbageCollector::getDefault");

	static GarbageCollector collector;

	return collector;
}

/*
Method Section
*/

void GarbageCollector::manage(Object& object)
{
	trace("GarbageCollector::manage", object.toString());
	objects.push_back(&object);
}
