// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#pragma once

#include "GameFramework/Character.h"

#include "RailheadCharacter.generated.h"

class UCameraComponent;
class URailheadCrewReadiness;
class UStaticMeshComponent;
struct FInputActionValue;

/** What the crew member is looking at that they could use. */
UENUM(BlueprintType)
enum class ERailheadInteraction : uint8
{
	None,
	/** The tender's coal: take a shovelful. */
	TakeCoal,
	/** The cab's firebox, with coal in hand: stoke it. */
	Stoke
};

// @doc
/**
 * A crew member: walks the train in first person, carries coal from the tender to the firebox.
 *
 * Interaction is decided twice. The owning machine traces every frame to know what to offer (the
 * HUD's prompt); pressing Interact asks the server, which traces again from the same view and acts.
 * The server's trace is the one that counts, so nobody stokes the boiler from the far wagon.
 */
UCLASS()
class RAILHEAD_API ARailheadCharacter : public ACharacter
{
	GENERATED_BODY()

public:

	ARailheadCharacter();

	/* AActor interface. */
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	/* AActor interface. */

	/* APawn interface. */
	virtual void OnRep_PlayerState() override;
	virtual void PossessedBy(AController* NewController) override;
	/* APawn interface. */

	/**
	 * Binds the crew's controls to this crew member: called by its readiness component once the
	 * experience's input config has loaded (the keys themselves come from the experience's Add
	 * Input Mappings stage).
	 */
	void BindCrewInput(const class URailheadInputConfig& Config);

	/** @return What pressing Interact would do right now, on the owning machine. */
	UFUNCTION(BlueprintPure, Category = "Railhead|Crew")
	ERailheadInteraction GetOffer() const { return Offer; }

	/** @return True while a shovelful of coal is in hand. */
	UFUNCTION(BlueprintPure, Category = "Railhead|Crew")
	bool IsCarryingCoal() const { return bCarryingCoal; }

	/** How far a crew member can reach, in centimetres. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Railhead|Crew", meta = (ClampMin = "50.0", Units = "cm"))
	float Reach = 260.0f;

	/** Below this height (centimetres) a crew member has fallen off, and is put back on the train. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Railhead|Crew", meta = (Units = "cm"))
	float FallenHeight = -500.0f;

protected:

	/** What a trace from this crew member's eyes would let them do. */
	ERailheadInteraction TraceForInteraction() const;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Interact();

	/** Asks the server to do what the crew member is looking at. */
	UFUNCTION(Server, Reliable)
	void ServerInteract();

	UFUNCTION()
	void OnRep_CarryingCoal();

	/** Paints the body in the player state's colour once there is a player state. */
	void ApplyCrewColor();

	/**
	 * A number from the player's settings (Gantry settings, the Crew page), or Fallback when the
	 * player has no such setting.
	 */
	double CrewSetting(FName Id, double Fallback) const;

	/** Gets this crew member ready step by step, and holds the loading screen until it is. */
	UPROPERTY(VisibleAnywhere, Category = "Railhead|Crew")
	TObjectPtr<URailheadCrewReadiness> Readiness;

	UPROPERTY(VisibleAnywhere, Category = "Railhead|Crew")
	TObjectPtr<UCameraComponent> Eyes;

	/** Body and head, seen by everyone but their owner. */
	UPROPERTY(VisibleAnywhere, Category = "Railhead|Crew")
	TObjectPtr<UStaticMeshComponent> Body;

	UPROPERTY(VisibleAnywhere, Category = "Railhead|Crew")
	TObjectPtr<UStaticMeshComponent> Head;

	/** The coal in hand: one seen from the owner's eyes, one held in front of the body for everyone else. */
	UPROPERTY(VisibleAnywhere, Category = "Railhead|Crew")
	TObjectPtr<UStaticMeshComponent> CoalInView;

	UPROPERTY(VisibleAnywhere, Category = "Railhead|Crew")
	TObjectPtr<UStaticMeshComponent> CoalHeld;

private:

	UPROPERTY(ReplicatedUsing = OnRep_CarryingCoal)
	bool bCarryingCoal = false;

	ERailheadInteraction Offer = ERailheadInteraction::None;

	bool bColorApplied = false;
};
