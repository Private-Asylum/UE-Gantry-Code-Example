// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#pragma once

#include "Subsystems/GameInstanceSubsystem.h"

#include "RailheadSessionSubsystem.generated.h"

// @doc
/**
 * Getting aboard: play alone, host a crew, join one, or go back to the menu.
 *
 * - **Solo:** the line map, standalone.
 * - **Host:** the line map as a listen server on a chosen port. The engine takes a listen server's
 *   port from FURL::UrlConfig.DefaultPort when it travels (there is no per-travel port option), so
 *   hosting sets it; a join always names its port.
 * - **Join:** travel to "host:port"; the server's experience follows by replication.
 *
 * What a build can do depends on its target: the Client target has no server code
 * (WITH_SERVER_CODE is 0), so it can only join. Run any other build with -RailheadJoinOnly to see
 * the Client's menu.
 */
UCLASS()
class RAILHEAD_API URailheadSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	/** @return The game instance's session subsystem, or null. */
	static URailheadSessionSubsystem* Get(const UObject* WorldContextObject);

	/** @return Whether this build can play alone (and so host). False in the Client target. */
	UFUNCTION(BlueprintPure, Category = "Railhead|Session")
	static bool CanPlaySolo();

	/** @return Whether this build can host a crew. */
	UFUNCTION(BlueprintPure, Category = "Railhead|Session")
	static bool CanHost() { return CanPlaySolo(); }

	/** @return What this build is, for the menu's footer ("Client build: join only"). */
	UFUNCTION(BlueprintPure, Category = "Railhead|Session")
	static FText DescribeBuild();

	/**
	 * Reads what a player types into the join box: "host", "host:port", "[v6]:port" or a bare IPv6
	 * address. The port defaults to 7777.
	 * @return False, with a reason a player can act on, when it cannot be used.
	 */
	static bool ParseAddress(const FString& In, FString& OutUrlHost, int32& OutPort, FText& OutError);

	/** Plays the line alone. */
	UFUNCTION(BlueprintCallable, Category = "Railhead|Session")
	bool PlaySolo(FText& OutError);

	/** Hosts the line on Port for others to join. */
	UFUNCTION(BlueprintCallable, Category = "Railhead|Session")
	bool Host(int32 Port, FText& OutError);

	/** Joins a crew at Address ("host:port"). */
	UFUNCTION(BlueprintCallable, Category = "Railhead|Session")
	bool Join(const FString& Address, FText& OutError);

	/** Back to the main menu. */
	UFUNCTION(BlueprintCallable, Category = "Railhead|Session")
	void LeaveToMenu();

	/** Closes the game. */
	UFUNCTION(BlueprintCallable, Category = "Railhead|Session")
	void Quit();

	/** @return What the world is doing, for the pause menu ("Hosting on port 7777 · 3 crew"). */
	static FText DescribeWorld(const UWorld* World);

	/** The map the crew ride on, and the menu's. */
	static const TCHAR* LineMap() { return TEXT("/Game/Railhead/Maps/L_Line"); }
	static const TCHAR* MenuMap() { return TEXT("/Game/Railhead/Maps/L_MainMenu"); }
};
