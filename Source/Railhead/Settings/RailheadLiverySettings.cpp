// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#include "Settings/RailheadLiverySettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RailheadLiverySettings)

const URailheadLiverySettings* URailheadLiverySettings::GetCurrent()
{
	URailheadLiverySettings* Settings = GetMutableDefault<URailheadLiverySettings>();
	Settings->LoadConfig();
	return Settings;
}
