// Copyright Square Crank Games. All Rights Reserved.

#pragma once

#include "GameFramework/Character.h"
#include "RoadFeverCharacterNed.generated.h"

UCLASS()
class ROAD_FEVER_API ARoadFeverCharacterNed : public ACharacter
{
	GENERATED_BODY()

public:
	// Called when this Character enters memory. [10/12/2015 Matthew Woolley]
	ARoadFeverCharacterNed();

	// Called at the beginning of game-play. [10/12/2015 Matthew Woolley]
	virtual void BeginPlay() override;

	// Called every frame. [10/12/2015 Matthew Woolley]
	virtual void Tick( float InDeltaSeconds ) override;

	// Called to bind player input. [10/12/2015 Matthew Woolley]
	virtual void SetupPlayerInputComponent( class UInputComponent* InInputComponent ) override;

	// The current input from the player for forwards-movement. [10/12/2015 Matthew Woolley]
	UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = "Movement" )
	float MoveForwardAxis;

	// Whether or not the Character is sprinting. [10/12/2015 Matthew Woolley]
	UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = "Movement" )
	bool bIsSprinting;

	// The maximum distance the player can be from an enemy and still auto-aim. [10/12/2015 Matthew Woolley]
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Attack" )
	float AutoAimMaxDistance;

	// Whether or not the player is aiming. [10/12/2015 Matthew Woolley]
	UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = "Attack" )
	bool bIsAiming;

	// The speed of which the Character sprints at. [10/12/2015 Matthew Woolley]
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Movement" )
	float CharacterSprintSpeed;

	// The speed that the Character quick-turns at. [10/12/2015 Matthew Woolley]
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Movement" )
	float QuickTurnSpeed;

	// The time it takes for the user to regain control from the quick-turn. [10/12/2015 Matthew Woolley]
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Movement" )
	float QuickTurnWaitTime;

	// The sensitivity of the turning input. [10/12/2015 Matthew Woolley]
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Movement" )
	float TurnSensitivity;

	// The speed the Character walks at. [11/12/2015 Matthew Woolley]
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Movement" )
	float BaseMovementSpeed;

	// The camera that will render the scene. [11/12/2015 Matthew Woolley]
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Rendering" )
	class UCameraComponent* CharactersCamera;

	// The inventory that the player has. [12/12/2015 Matthew Woolley]
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Stats" )
	class UInventory* CharactersInventory;

	// Gets the camera dummy. [11/12/2015 Matthew Woolley]
	class ARoadFeverCameraDummy* GetCameraDummy() { return CameraDummy; }

	// The current level of health that the player has. [25/12/2015 Matthew Woolley]
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Stats" )
	int32 Health;

	// Returns the text that the inventory screen should display for the health value. [25/12/2015 Matthew Woolley]
	UFUNCTION( BlueprintCallable, Category = "Stats" )
	FString UpdateHealthMessage();

	// The current level of the blood loss for the player. [25/12/2015 Matthew Woolley]
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Stats" )
	int32 BloodLoss;

	// Returns the text that the inventory screen should display for the blood loss value. [25/12/2015 Matthew Woolley]
	UFUNCTION( BlueprintCallable, Category = "Stats" )
	FString UpdateBloodMessage();

protected:
	// Moves the Character in the X axis. [10/12/2015 Matthew Woolley]
	UFUNCTION( BlueprintCallable, Category = "Movement" )
	void MoveForward( float InInputVal );
	
	// Called when the player wishes to turn. [10/12/2015 Matthew Woolley]
	UFUNCTION( BlueprintCallable, Category = "Attack" )
	void Turn( float InInputVal );

	// Used to detect items when interacting. [10/12/2015 Matthew Woolley]
	UPROPERTY( BlueprintReadWrite, Category = "Interact" )
	class UBoxComponent* CollectionArea;

	// Called when the player attempts to interact. [10/12/2015 Matthew Woolley]
	UFUNCTION( BlueprintNativeEvent, BlueprintCallable, Category = "Interact" )
	void OnCharacterInteract();
	virtual void OnCharacterInteract_Implementation();

	// Called when the player wishes to sprint. [10/12/2015 Matthew Woolley]
	UFUNCTION( BlueprintCallable, Category = "Movement" )
	void OnBeginSprint();

	// Called when the player stops sprinting. [10/12/2015 Matthew Woolley]
	UFUNCTION( BlueprintCallable, Category = "Movement" )
	void OnEndSprint();

	// Called when the player wishes to do a quick-turn. [10/12/2015 Matthew Woolley]
	UFUNCTION( BlueprintCallable, Category = "Movement" )
	void OnBeginQuickTurn();

	// Whether or not the player is performing a quick-turn. [10/12/2015 Matthew Woolley]
	UPROPERTY( BlueprintReadWrite, Category = "Movement" )
	bool bIsDoingQuickTurn;

	// Called when the player begins aiming. [10/12/2015 Matthew Woolley]
	UFUNCTION( BlueprintCallable, Category = "Attack" )
	void OnBeginAim();

	// Called when the player stops aiming. [10/12/2015 Matthew Woolley]
	UFUNCTION( BlueprintCallable, Category = "Attack" )
	void OnEndAim();

	// The sphere used for enemy detection and aiming. [11/12/2015 Matthew Woolley]
	UPROPERTY( BlueprintReadWrite, Category = "Attack" )
	class USphereComponent* AutoAimSphere;

private:
	// Called to preform generic movement function. [10/12/2015 Matthew Woolley]
	UFUNCTION( Category = "Movement" )
	void _move( float InInputVal, EAxis::Type InMoveAxis );

	// Used by the camera-system when detecting the Character. [11/12/2015 Matthew Woolley]
	UPROPERTY()
	class ARoadFeverCameraDummy* CameraDummy;
};
