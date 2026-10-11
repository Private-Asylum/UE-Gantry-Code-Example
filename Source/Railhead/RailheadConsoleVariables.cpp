// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#include "RailheadConsoleVariables.h"

namespace RailheadCVars
{
	TAutoConsoleVariable<float> SpeedScale(
		TEXT("railhead.SpeedScale"), 1.0f,
		TEXT("Multiplies the train's speed. For watching a whole route in a minute."),
		ECVF_Cheat);

	TAutoConsoleVariable<float> StretchLengthScale(
		TEXT("railhead.StretchLengthScale"), 1.0f,
		TEXT("Multiplies every stretch's length, read as each stretch starts."),
		ECVF_Cheat);

	TAutoConsoleVariable<bool> AutoStoke(
		TEXT("railhead.AutoStoke"), false,
		TEXT("The server keeps the boiler fed, so an empty or headless session still runs."),
		ECVF_Cheat);
}
