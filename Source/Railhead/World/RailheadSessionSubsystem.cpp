// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#include "World/RailheadSessionSubsystem.h"

#include "RailheadLogChannels.h"

#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RailheadSessionSubsystem)

#define LOCTEXT_NAMESPACE "RailheadSession"

namespace RailheadSession
{
	bool JoinOnlyByCommandLine()
	{
		return FParse::Param(FCommandLine::Get(), TEXT("RailheadJoinOnly"));
	}

	bool IsHostChar(TCHAR C)
	{
		return FChar::IsAlnum(C) || C == TEXT('.') || C == TEXT('-') || C == TEXT('_');
	}
}

URailheadSessionSubsystem* URailheadSessionSubsystem::Get(const UObject* WorldContextObject)
{
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	const UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	return GameInstance ? GameInstance->GetSubsystem<URailheadSessionSubsystem>() : nullptr;
}

bool URailheadSessionSubsystem::CanPlaySolo()
{
#if WITH_SERVER_CODE
	return !RailheadSession::JoinOnlyByCommandLine();
#else
	return false;
#endif
}

FText URailheadSessionSubsystem::DescribeBuild()
{
#if !WITH_SERVER_CODE
	return LOCTEXT("BuildClient", "Client build: join only");
#else
	if (RailheadSession::JoinOnlyByCommandLine())
	{
		return LOCTEXT("BuildJoinOnly", "Join only (-RailheadJoinOnly)");
	}
	return LOCTEXT("BuildFull", "Solo, host or join");
#endif
}

bool URailheadSessionSubsystem::ParseAddress(const FString& In, FString& OutUrlHost, int32& OutPort, FText& OutError)
{
	constexpr int32 DefaultPort = 7777;
	FString S = In.TrimStartAndEnd();
	S.RemoveFromStart(TEXT("unreal://"), ESearchCase::IgnoreCase);
	S.RemoveFromEnd(TEXT("/"));
	OutPort = DefaultPort;
	OutUrlHost.Reset();
	if (S.IsEmpty())
	{
		OutError = LOCTEXT("AddrEmpty", "Enter an address: ip:port or host:port.");
		return false;
	}

	FString Host;
	FString PortText;
	bool bIPv6 = false;
	if (S.StartsWith(TEXT("[")))
	{
		int32 Close = INDEX_NONE;
		if (!S.FindChar(TEXT(']'), Close))
		{
			OutError = LOCTEXT("AddrBracket", "An IPv6 address needs its closing ']'.");
			return false;
		}
		Host = S.Mid(1, Close - 1);
		const FString Rest = S.Mid(Close + 1);
		if (!Rest.IsEmpty())
		{
			if (!Rest.StartsWith(TEXT(":")))
			{
				OutError = LOCTEXT("AddrAfterBracket", "Expected ':port' after ']'.");
				return false;
			}
			PortText = Rest.Mid(1);
		}
		bIPv6 = true;
	}
	else
	{
		int32 Colons = 0;
		for (const TCHAR C : S)
		{
			Colons += C == TEXT(':') ? 1 : 0;
		}
		if (Colons == 1)
		{
			S.Split(TEXT(":"), &Host, &PortText);
		}
		else
		{
			Host = S;
			bIPv6 = Colons > 1; // a bare IPv6 address: no port can follow without brackets
		}
	}

	if (Host.IsEmpty())
	{
		OutError = LOCTEXT("AddrNoHost", "The address has no host before the port.");
		return false;
	}
	for (const TCHAR C : Host)
	{
		if (!(RailheadSession::IsHostChar(C) || (bIPv6 && (C == TEXT(':') || C == TEXT('%')))))
		{
			OutError = FText::Format(LOCTEXT("AddrBadChar", "'{0}' cannot appear in a host name."), FText::FromString(FString(1, &C)));
			return false;
		}
	}
	if (!PortText.IsEmpty())
	{
		if (!PortText.IsNumeric() || PortText.Contains(TEXT(".")) || PortText.Contains(TEXT("-")))
		{
			OutError = LOCTEXT("AddrPortNumber", "The port must be a whole number.");
			return false;
		}
		OutPort = FCString::Atoi(*PortText);
		if (OutPort < 1 || OutPort > 65535)
		{
			OutError = LOCTEXT("AddrPortRange", "The port must be between 1 and 65535.");
			return false;
		}
	}
	OutUrlHost = bIPv6 ? FString::Printf(TEXT("[%s]"), *Host) : Host;
	OutError = FText::GetEmpty();
	return true;
}

bool URailheadSessionSubsystem::PlaySolo(FText& OutError)
{
	if (!CanPlaySolo())
	{
		OutError = LOCTEXT("NoSolo", "This build can only join a crew.");
		return false;
	}
	UE_LOG(LogRailhead, Log, TEXT("Session: solo on %s."), LineMap());
	UGameplayStatics::OpenLevel(GetGameInstance(), FName(LineMap()), true);
	return true;
}

bool URailheadSessionSubsystem::Host(int32 Port, FText& OutError)
{
	if (!CanHost())
	{
		OutError = LOCTEXT("NoHost", "This build can only join a crew.");
		return false;
	}
	if (Port < 1 || Port > 65535)
	{
		OutError = LOCTEXT("HostPortRange", "The port must be between 1 and 65535.");
		return false;
	}
	// The listen URL takes its port from the default when it travels; set it and leave it set.
	FURL::UrlConfig.DefaultPort = Port;
	UE_LOG(LogRailhead, Log, TEXT("Session: hosting %s on port %d."), LineMap(), Port);
	UGameplayStatics::OpenLevel(GetGameInstance(), FName(LineMap()), true, TEXT("listen"));
	return true;
}

bool URailheadSessionSubsystem::Join(const FString& Address, FText& OutError)
{
	FString UrlHost;
	int32 Port = 0;
	if (!ParseAddress(Address, UrlHost, Port, OutError))
	{
		return false;
	}
	const FString Url = FString::Printf(TEXT("%s:%d"), *UrlHost, Port);
	UE_LOG(LogRailhead, Log, TEXT("Session: joining %s."), *Url);
	UWorld* World = GetGameInstance()->GetWorld();
	if (APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr)
	{
		PC->ClientTravel(Url, TRAVEL_Absolute);
	}
	else if (GEngine && World)
	{
		GEngine->SetClientTravel(World, *Url, TRAVEL_Absolute);
	}
	return true;
}

void URailheadSessionSubsystem::LeaveToMenu()
{
	UE_LOG(LogRailhead, Log, TEXT("Session: back to the menu."));
	UGameplayStatics::OpenLevel(GetGameInstance(), FName(MenuMap()), true);
}

void URailheadSessionSubsystem::Quit()
{
	UE_LOG(LogRailhead, Log, TEXT("Session: quitting."));
	FPlatformMisc::RequestExit(false, TEXT("RailheadSession.Quit"));
}

FText URailheadSessionSubsystem::DescribeWorld(const UWorld* World)
{
	if (!World)
	{
		return FText::GetEmpty();
	}
	const AGameStateBase* GameState = World->GetGameState();
	const int32 Crew = GameState ? GameState->PlayerArray.Num() : 0;
	const FNumberFormattingOptions Plain = FNumberFormattingOptions::DefaultNoGrouping();
	switch (World->GetNetMode())
	{
	case NM_Standalone:
		return LOCTEXT("ModeSolo", "Solo");
	case NM_ListenServer:
		return FText::Format(LOCTEXT("ModeHost", "Hosting on port {0}  ·  {1} crew"), FText::AsNumber(World->URL.Port, &Plain), Crew);
	case NM_Client:
		return FText::Format(LOCTEXT("ModeClient", "Aboard {0}:{1}  ·  {2} crew"), FText::FromString(World->URL.Host), FText::AsNumber(World->URL.Port, &Plain), Crew);
	default:
		return FText::GetEmpty();
	}
}

#undef LOCTEXT_NAMESPACE
