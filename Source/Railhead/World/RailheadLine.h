// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#pragma once

#include "Experience/RailheadStretchDefinition.h"
#include "World/RailheadSceneryBuilder.h"

#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "Tasks/Task.h"

#include "World/RailheadWeather.h"

#include "RailheadLine.generated.h"

class ADirectionalLight;
class AExponentialHeightFog;
class UGantryExperienceDefinition;
class UInstancedStaticMeshComponent;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class UStaticMesh;
class UStaticMeshComponent;

/** Where the line is between stretches. */
UENUM(BlueprintType)
enum class ERailheadLinePhase : uint8
{
	/** Nothing to run on yet: the first stretch is still activating. */
	Waiting,
	/** Out in a stretch, heading for its tunnel. */
	Running,
	/** In the tunnel; the next stretch is loading. */
	Tunnel,
	/** The next stretch is ready and the train is heading for the tunnel's exit. */
	Leaving
};

/**
 * Which stretches the track runs through, and where. Distances are metres along the track since
 * the session started; they only ever grow.
 */
USTRUCT(BlueprintType)
struct RAILHEAD_API FRailheadLineState
{
	GENERATED_BODY()

	/** Where the line is between stretches. */
	UPROPERTY(BlueprintReadOnly, Category = "Line")
	ERailheadLinePhase Phase = ERailheadLinePhase::Waiting;

	/** The stretch the track is in, or the one the train just left the tunnel into. */
	UPROPERTY(BlueprintReadOnly, Category = "Line")
	FGameplayTag Stretch;

	/** Where Stretch begins: the exit of the tunnel before it. */
	UPROPERTY(BlueprintReadOnly, Category = "Line")
	double StretchStart = 0.0;

	/** Where Stretch ends: the entrance of the tunnel after it. */
	UPROPERTY(BlueprintReadOnly, Category = "Line")
	double StretchEnd = 0.0;

	/** The stretch before Stretch, still visible behind the train for a while. */
	UPROPERTY(BlueprintReadOnly, Category = "Line")
	FGameplayTag PreviousStretch;

	/** Where PreviousStretch ended: the entrance of the tunnel between the two. */
	UPROPERTY(BlueprintReadOnly, Category = "Line")
	double PreviousEnd = 0.0;

	/** In the tunnel: the stretch being loaded. */
	UPROPERTY(BlueprintReadOnly, Category = "Line")
	FGameplayTag NextStretch;
};

/** Where the train is and how fast it is going: the part of the line that changes every frame. */
USTRUCT(BlueprintType)
struct RAILHEAD_API FRailheadLineMotion
{
	GENERATED_BODY()

	/** Metres travelled since the session started; the train's centre. */
	UPROPERTY(BlueprintReadOnly, Category = "Line")
	double Distance = 0.0;

	/** Metres per second. */
	UPROPERTY(BlueprintReadOnly, Category = "Line")
	float Speed = 0.0f;
};

// @doc
/**
 * The world the train crosses, and the route it takes through Gantry's experiences.
 *
 * Railhead's train never moves. The line moves everything else past it: sleepers, ground,
 * scenery and tunnels scroll backwards at the train's speed, so the crew walk on an actor that
 * stands still, the track never runs out, and coordinates stay small. Each stretch of track is a
 * Gantry experience (URailheadStretchDefinition); the line asks for the next one when the train is
 * inside a tunnel, and lets the train out once Gantry says it is ready.
 *
 * - **Server:** integrates the distance, decides when to enter and leave tunnels, and asks the
 *   experience subsystem for the next stretch. In a tunnel, the train's speed follows Gantry's
 *   streaming progress: a slow load is a slow tunnel.
 * - **Everyone else:** follows the replicated distance smoothly and draws the stretches it has
 *   scenery for. Scenery placement is a pure function of the stretch and the distance, so every
 *   machine draws the same props in the same places without replicating any of them.
 * - **Scenery off the game thread:** every few metres, or when the track changes, everything in
 *   view is placed on a worker (RailheadScenery::Place). The game thread writes the result into
 *   instanced components it keeps for the whole session, in place: one per mesh and paint, never
 *   created, cleared or rebuilt as the train moves, with spare instances parked out of sight.
 * - **Dedicated server:** draws nothing.
 * - **Weather:** AddWeather lays sun, haze and snow over the stretches' light, on this machine.
 *   The Weather stage (URailheadStage_Weather) calls it. The line is also a Game Framework
 *   Component Manager receiver (listed in Gantry's Automatic Receiver Classes), so a game
 *   feature's Add Components action can give it a component that does the same.
 * - **Speed:** every tick the server runs the Railhead.Hook.Line.Speed hook (FRailheadLineSpeed)
 *   with the speed the boiler and the tunnel ask for, and heads for whatever its listeners leave.
 *
 * The game mode spawns it at the origin, alongside the train.
 */
UCLASS(meta = (GantryAPI, ShortTooltip = "Scrolls the world past the train and runs the route through Gantry's experiences."))
class RAILHEAD_API ARailheadLine : public AActor
{
	GENERATED_BODY()

public:

	ARailheadLine();

	/**
	 * The world's line.
	 * @param World Any world; may be null.
	 * @return The line placed in it, or null.
	 */
	static ARailheadLine* Find(const UWorld* World);

	/* AActor interface. */
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	/* AActor interface. */

	/** @return The replicated route state. */
	UFUNCTION(BlueprintPure, Category = "Railhead|Line")
	const FRailheadLineState& GetState() const { return State; }

	/** @return This machine's view of the distance travelled, in metres. */
	UFUNCTION(BlueprintPure, Category = "Railhead|Line", meta = (GantryAPI))
	double GetDistance() const { return LocalDistance; }

	/** @return The train's speed in metres per second. */
	UFUNCTION(BlueprintPure, Category = "Railhead|Line", meta = (GantryAPI))
	float GetSpeed() const { return LocalSpeed; }

	/** @return Metres from the front of the train to the next tunnel; 0 once inside. */
	UFUNCTION(BlueprintPure, Category = "Railhead|Line")
	float GetMetersToTunnel() const;

	/** @return True while any part of the train is inside a tunnel, on this machine's view. */
	UFUNCTION(BlueprintPure, Category = "Railhead|Line", meta = (GantryAPI))
	bool IsTrainInTunnel() const;

	/**
	 * Lays weather over the stretches' light on this machine, until RemoveWeather (or the source
	 * goes). What a game feature's component calls to bring rain, snow or haze, the line eases in.
	 * @param Source Who asks; asking again replaces its weather.
	 * @param Weather The weather.
	 */
	UFUNCTION(BlueprintCallable, Category = "Railhead|Line", meta = (GantryAPI))
	void AddWeather(UObject* Source, const FRailheadWeather& Weather);

	/**
	 * Takes a source's weather away again; the line eases back.
	 * @param Source Who asked.
	 */
	UFUNCTION(BlueprintCallable, Category = "Railhead|Line", meta = (GantryAPI))
	void RemoveWeather(UObject* Source);

	/** How far the train reaches from its centre when there is no train to ask, in metres. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Railhead|Line", meta = (ClampMin = "1.0", Units = "m"))
	float TrainHalfLengthMeters = 40.0f;

	/**
	 * Gain or loss of speed in metres per second, per second, unless the active experiences bring
	 * line rules (URailheadLineRules).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Railhead|Line", meta = (ClampMin = "0.1"))
	float Acceleration = 1.5f;

	/**
	 * In a tunnel, the fraction of its speed the train keeps while the next stretch has loaded
	 * nothing, unless line rules say otherwise.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Railhead|Line", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TunnelCrawlFraction = 0.2f;

	/**
	 * The nearest any scenery comes to the track's centre line, in metres. Props whose layer would
	 * put them closer (big, scaled up, or turned across the track) are moved out to it.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Railhead|Line", meta = (ClampMin = "0.0", Units = "m"))
	float TrackClearanceMeters = 4.0f;

	/** Half the train's width, for the development check that nothing beside the track crosses it. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Railhead|Line", meta = (ClampMin = "0.1", Units = "m"))
	float TrainHalfWidthMeters = 1.7f;

	/** The train's height above the rails, for the same check. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Railhead|Line", meta = (ClampMin = "0.1", Units = "m"))
	float TrainHeightMeters = 4.8f;

	/** Colour of the tunnels and the rock above them. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Railhead|Line")
	FLinearColor RockColor = FLinearColor(0.09f, 0.08f, 0.075f);

protected:

	/** Ready on the server: the first stretch starts the line, the next one opens the tunnel's exit. */
	void HandleExperienceReady(const UGantryExperienceDefinition* Experience);

	/** Failed on the server: the train comes out where it went in, into the same stretch again. */
	UFUNCTION()
	void HandleExperienceFailed(FGameplayTag ExperienceTag, const FString& Reason);

	/** Server: speed, distance, tunnels. */
	void TickServer(float DeltaSeconds);

	/** Clients: follow the server's distance without visible jumps. */
	void TickFollow(float DeltaSeconds);

	/** Everyone who draws: scroll, keep the scenery placed, ease the light, flicker the lamps. */
	void TickPresentation(float DeltaSeconds);

	/** Applies a finished placement, and starts the next when the train has moved on or the track changed. */
	void UpdateScenery();

	/** Writes a placement into the instancers, in place. Game thread. */
	void ApplyPlacement(RailheadScenery::FPlacement&& Placement);

	/** The session-long instancer for a batch key, made on first use. */
	UInstancedStaticMeshComponent* GetInstancer(const FString& Key, const RailheadScenery::FBatch& Batch);

	/** The track as ranges, from the replicated state and the scenery this machine has. */
	TArray<RailheadScenery::FRange> BuildRanges();

	/** A stretch's scenery as the builder reads it, made once per stretch per registry version. */
	TSharedPtr<const RailheadScenery::FStretchSpec> GetSpec(const FGameplayTag& Tag);

	/** The shared paint for a batch's colour and glow. */
	UMaterialInstanceDynamic* GetPaint(const RailheadScenery::FBatch& Batch);

	/** A stretch's length with the development scale applied. */
	static double StretchLength(const URailheadStretchDefinition* Stretch);

	/** Clients: the route state arrived. Nothing to do; every frame reads it. */
	UFUNCTION()
	void OnRep_State();

	/** Clients: the server's distance and speed arrived; remembered, with when, for TickFollow. */
	UFUNCTION()
	void OnRep_Motion();

private:

	/** The stretch a tag names, if this machine has its scenery (URailheadSceneryRegistry). */
	const URailheadStretchDefinition* FindStretch(const FGameplayTag& Tag) const;

	/**
	 * Tells anyone listening (Railhead.Message.Line) when the train goes into a tunnel and when
	 * it comes out into a new stretch, as this machine sees it.
	 */
	void Announce();

	/** The stretch last announced as reached, and whether a tunnel was. */
	FGameplayTag AnnouncedStretch;
	bool bAnnouncedTunnel = false;

	/** The registry's version when the scenery was last placed. */
	int32 SceneryVersion = -1;

	/** Which stretches the track runs through, and where; the server decides it. */
	UPROPERTY(ReplicatedUsing = OnRep_State)
	FRailheadLineState State;

	/** The server's distance and speed, sent to clients up to ten times a second. */
	UPROPERTY(ReplicatedUsing = OnRep_Motion)
	FRailheadLineMotion Motion;

	/** This machine's distance: the server's own, or a client's smoothed copy. */
	double LocalDistance = 0.0;

	/** This machine's speed. */
	float LocalSpeed = 0.0f;

	/** Client: the server's motion, and when it arrived. */
	FRailheadLineMotion ReceivedMotion;
	double ReceivedAtSeconds = 0.0;

	/** Server: in a tunnel, whether the next stretch is ready, and since when the train has been inside. */
	bool bNextReady = false;
	double TunnelEnteredSeconds = 0.0;

	/** Server: the stretch just left asks for at least this long in its tunnel. */
	double TunnelMinimumSeconds = 0.0;

	/** Where the applied placement starts, in metres: the scenery's transforms are relative to it. */
	double ScrollAnchor = 0.0;

	/** One instancer per mesh and paint, kept for the session. */
	UPROPERTY(Transient)
	TMap<FString, TObjectPtr<UInstancedStaticMeshComponent>> Instancers;

	/** How many of each instancer's instances are showing; the rest are parked out of sight. */
	TMap<FString, int32> Showing;

	/** The placement being worked out on a worker, if any. */
	TOptional<UE::Tasks::TTask<RailheadScenery::FPlacement>> PendingPlacement;

	/** Where the last placement was asked for, and what the track looked like then. */
	double RequestedAt = -1.0e12;
	uint32 RequestedTrackKey = 0;

	/** One paint per colour and glow, shared by every chunk. */
	UPROPERTY(Transient)
	TMap<FString, TObjectPtr<UMaterialInstanceDynamic>> Paints;

	/** Stretch scenery as the builder reads it, for the registry version they were made at. */
	TMap<FGameplayTag, TSharedPtr<const RailheadScenery::FStretchSpec>> Specs;
	int32 SpecsVersion = -1;

	/** Batches already reported as crossing the track, so each is reported once. */
	TSet<FString> ReportedOverlaps;

	/** Tunnel lamp paint, which flickers while this machine is loading, and its last glow. */
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> LampPaint;
	float LampGlow = -1.0f;

	/** Metres from the train's centre to its front, and to the back of its last car. */
	double GetFrontReach() const;
	double GetRearReach() const;

	/** Weather laid over the light, by source. */
	TMap<TWeakObjectPtr<UObject>, FRailheadWeather> Weathers;

	/** The snow cover being shown, eased toward the weathers' own. */
	float CurrentSnow = 0.0f;

	/** Paints snow whitens, with their own colour. */
	TArray<TPair<TWeakObjectPtr<UMaterialInstanceDynamic>, FLinearColor>> SnowPaints;

	/** Gives a paint snow can whiten its current share of snow. */
	void AddSnowPaint(UMaterialInstanceDynamic* Paint, const FLinearColor& Color);

	/** The sun and haze the line eases toward each stretch's light; settled once it gets there. */
	TWeakObjectPtr<ADirectionalLight> Sun;
	TWeakObjectPtr<AExponentialHeightFog> Fog;
	FRailheadStretchLight CurrentLight;
	FGameplayTag LightStretch;
	bool bHasLight = false;
	bool bLightSettled = false;

	/** Server: the line's listener on Gantry's high ready tier. */
	FDelegateHandle ReadyHandle;

	/** Stands still at the origin; the track pieces and the scroll root hang from it. */
	UPROPERTY(VisibleAnywhere, Category = "Railhead|Line")
	TObjectPtr<USceneComponent> Root;

	/** Everything that scrolls with the scenery. */
	UPROPERTY(VisibleAnywhere, Category = "Railhead|Line")
	TObjectPtr<USceneComponent> ScrollRoot;

	/** Sleepers scroll on their own, by less than one sleeper's pitch. */
	UPROPERTY(VisibleAnywhere, Category = "Railhead|Line")
	TObjectPtr<UInstancedStaticMeshComponent> Sleepers;

	/** The left rail: one long bar through the drawn window, which never needs to move. */
	UPROPERTY(VisibleAnywhere, Category = "Railhead|Line")
	TObjectPtr<UStaticMeshComponent> RailLeft;

	/** The right rail, the same. */
	UPROPERTY(VisibleAnywhere, Category = "Railhead|Line")
	TObjectPtr<UStaticMeshComponent> RailRight;

	/** The stone bed under the sleepers, the same. */
	UPROPERTY(VisibleAnywhere, Category = "Railhead|Line")
	TObjectPtr<UStaticMeshComponent> Ballast;

	/** The engine's basic cube, which the track and the tunnels are made of. */
	UPROPERTY()
	TObjectPtr<UStaticMesh> CubeMesh;

	/** The flat paint every piece is drawn with, tinted per colour (M_Railhead_Paint). */
	UPROPERTY()
	TObjectPtr<UMaterialInterface> PaintMaterial;
};
