// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#pragma once

#include "HAL/IConsoleManager.h"

/** Development aids (cheats: not in Shipping). Each defaults to no effect and says what it is for. */
namespace RailheadCVars
{
	/** railhead.SpeedScale: multiplies the train's speed. For watching a whole route in a minute. */
	RAILHEAD_API extern TAutoConsoleVariable<float> SpeedScale;

	/** railhead.StretchLengthScale: multiplies every stretch's length, read when the stretch starts. */
	RAILHEAD_API extern TAutoConsoleVariable<float> StretchLengthScale;

	/** railhead.AutoStoke: the server keeps the boiler fed, so an empty or headless session still runs. */
	RAILHEAD_API extern TAutoConsoleVariable<bool> AutoStoke;
}
