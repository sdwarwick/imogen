#pragma once

#include "Parameters.h"
#include "Meters.h"
#include "Internals.h"


namespace Imogen
{
struct CustomStateData : SerializableData
{
private:

	void serialize (TreeReflector& ref) final;
};


struct State : plugin::CustomState<Parameters, CustomStateData>
{
	State();

	Internals internals;
	Meters	  meters;
};

}  // namespace Imogen
