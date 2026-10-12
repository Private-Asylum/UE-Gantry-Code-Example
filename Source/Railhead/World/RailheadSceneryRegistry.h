// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "Subsystems/WorldSubsystem.h"

#include "RailheadSceneryRegistry.generated.h"

class URailheadStretchDefinition;

// @doc
/**
 * The stretches this machine has scenery for, as the scenery stage loads them.
 *
 * A world subsystem rather than a field on the line: on a client the scenery stage can finish
 * before the line has replicated, and the line draws whatever is here whenever it arrives.
 */
UCLASS()
class RAILHEAD_API URailheadSceneryRegistry : public UWorldSubsystem
{
	GENERATED_BODY()

public:

	/**
	 * @param World Any world; may be null.
	 * @return The world's registry, or null.
	 */
	static URailheadSceneryRegistry* Get(const UWorld* World);

	/**
	 * Adds a stretch whose scenery meshes are in memory.
	 * @param Stretch The stretch; null is ignored.
	 */
	void AddStretch(const URailheadStretchDefinition* Stretch);

	/**
	 * @param Tag A stretch's experience tag.
	 * @return The stretch a tag names, if its scenery is here.
	 */
	const URailheadStretchDefinition* FindStretch(const FGameplayTag& Tag) const;

	/**
	 * Forgets every stretch not in Keep, so their meshes can go.
	 * @param Keep The stretches the track can still show, matched exactly.
	 */
	void Prune(const FGameplayTagContainer& Keep);

	/** @return A number that changes whenever a stretch is added, for readers that cache. */
	int32 GetVersion() const { return Version; }

protected:

	/** Game and play-in-editor worlds only. */
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:

	/** The stretches with scenery in memory, by experience tag. */
	UPROPERTY(Transient)
	TMap<FGameplayTag, TObjectPtr<const URailheadStretchDefinition>> Stretches;

	/** Counts additions; see GetVersion. */
	int32 Version = 0;
};
