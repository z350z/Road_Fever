// Copyright Square Crank Games. All Rights Reserved.

#include "Road_Fever.h"
#include "Public/Items/Weapons/Weapon.h"
#include "Public/AI/Enemy/RoadFeverEnemy.h"
#include "Public/Characters/RoadFeverCharacterNed.h"
#include "Public/Inventory/Inventory.h"



AWeapon::AWeapon()
{
	// Default weapon properties [20/11/2015 Matthew Woolley]
	WeaponProperties.bIsCoolingDown = false;
	WeaponProperties.CoolDownTime = 1;


	WeaponProperties.EffectiveRangeMaxDamage = 25;
	WeaponProperties.EffectiveRangeMinDamage = 20;

	WeaponProperties.MaximumRangeMaxDamage = 10;
	WeaponProperties.MaximumRangeMinDamage = 5;


	WeaponProperties.EffectiveRange = 50.f;
	WeaponProperties.MaximumRange = 100.f;

	WeaponProperties.NumberOfTraces = 1;
	WeaponProperties.MultiTraceSpread = 12.0f;

	// Make sure all weapons are treated as weapons. [26/7/2016 Matthew Woolley]
	ItemInfo.bIsWeapon = true;

	// Make all weapons default to a box trace, rather than a line trace. [3/2/2017 Matthew Woolley]
	WeaponProperties.bIsBoxTrace = true;

	// Allow Actor ticking [20/11/2015 Matthew Woolley]
	PrimaryActorTick.bCanEverTick = true;
}

// Called when this object is destroyed. [27/7/2016 Matthew Woolley]
void AWeapon::EndPlay( const EEndPlayReason::Type InEndPlayReason )
{
	// Stop any weapon timers that may be going. [27/7/2016 Matthew Woolley]
	GetWorld()->GetTimerManager().ClearTimer( WeaponCooldownHandle );
	GetWorld()->GetTimerManager().ClearTimer( WeaponReloadHandle );
}

// Called when the player wishes to attack with this weapon [20/11/2015 Matthew Woolley]
void AWeapon::OnAttack_Implementation()
{
	ARoadFeverCharacterNed* PlayerCharacter = ( ARoadFeverCharacterNed* ) GetWorld()->GetFirstPlayerController()->GetCharacter();

	// Make sure the weapon isn't currently cooling down [20/11/2015 Matthew Woolley]
	if ( WeaponProperties.bIsReloading || WeaponProperties.bIsCoolingDown || ( ItemInfo.MaxAmmo != 0 && ItemInfo.CurrentAmmo <= 0 ) )
	{
		if ( ItemInfo.CurrentAmmo <= 0 )
		{
			PlayerCharacter->GetMesh()->PlayAnimation( WeaponProperties.AttackAnimation_NoAmmo, false );
		}

		// If this weapon is reloading, but can be interrupted. [27/7/2016 Matthew Woolley]
		if ( WeaponProperties.bIsReloading )
		{
			bShouldInterrupt = true;
		}

		return;
	}

	PlayerCharacter->GetMesh()->PlayAnimation( WeaponProperties.AttackAnimation, false );

	// For each trace to complete. [21/7/2016 Matthew Woolley]
	for ( int iTraceIterator = 0; iTraceIterator < WeaponProperties.NumberOfTraces; iTraceIterator++ )
	{
		// The object found during the trace [20/11/2015 Matthew Woolley]
		FHitResult OutHit;

		// The current location of this weapon [20/11/2015 Matthew Woolley]
		FVector Start = PlayerCharacter->ShootFromPoint->GetComponentLocation();

		// The . [21/7/2016 Matthew Woolley]
		FRotator SpreadRotation = FRotator( FMath::FRandRange( WeaponProperties.MultiTraceSpread * -1, WeaponProperties.MultiTraceSpread ), FMath::FRandRange( WeaponProperties.MultiTraceSpread * -1, WeaponProperties.MultiTraceSpread ), 0 );

		// Get the furthest this weapon can attack [20/11/2015 Matthew Woolley]
		FVector End = Start + ( ( iTraceIterator == 0 ? PlayerCharacter->ShootFromPoint->GetComponentRotation() : PlayerCharacter->ShootFromPoint->GetComponentRotation() + SpreadRotation ).Vector() * WeaponProperties.MaximumRange );

		// The rotation [20/11/2015 Matthew Woolley]
		FQuat Rot = GetActorRotation().Quaternion();

		// The trace's shape [20/11/2015 Matthew Woolley]
		FCollisionShape Shape;
		Shape.ShapeType = ECollisionShape::Box;
		Shape.MakeBox( FVector( 1, 20, 20 ) );
		Shape.Box.HalfExtentX = 1;
		Shape.Box.HalfExtentY = 20;
		Shape.Box.HalfExtentZ = 20;

		// The parameters for the collision [20/11/2015 Matthew Woolley]
		FCollisionQueryParams Params;
		Params.AddIgnoredActor( this );
		Params.AddIgnoredActor( PlayerCharacter );
		Params.TraceTag = FName( "WeaponTrace" );

		// The current UWorld object [20/11/2015 Matthew Woolley]
		UWorld* const World = GetWorld();

		if ( World )
		{
			World->DebugDrawTraceTag = "WeaponTrace";
			// Whether this weapon hit something or not. [11/2/2017 Matthew Woolley]
			bool bHadBlockingHit = NULL;

			// If we are using the box trace. [3/2/2017 Matthew Woolley]
			if ( WeaponProperties.bIsBoxTrace )
			{
				// Use the box shape in the trace params. [3/2/2017 Matthew Woolley]
				bHadBlockingHit = World->SweepSingleByChannel( OutHit, Start, End, Rot, WEAPON_TRACE, Shape, Params );
			} else
			{
				// Do the trace without the box shape. [3/2/2017 Matthew Woolley]
				bHadBlockingHit = World->LineTraceSingleByChannel( OutHit, Start, End, WEAPON_TRACE, Params );
			}

			// If there was one [20/11/2015 Matthew Woolley]
			if ( bHadBlockingHit )
			{
				// Make sure it's still valid and not being destroyed [20/11/2015 Matthew Woolley]
				AActor* HitActor = OutHit.GetActor();
				if ( HitActor && !HitActor->IsPendingKill() && HitActor->IsA( ARoadFeverEnemy::StaticClass() ) )
				{
					// Cast the enemy from the hit Actor. [15/7/2016 Matthew Woolley]
					ARoadFeverEnemy* HitEnemy = ( ARoadFeverEnemy* ) HitActor;

					// If the enemy is further away than this weapon can attack with full damage. [17/7/2016 Matthew Woolley]
					if ( HitEnemy->GetDistanceTo( this ) > WeaponProperties.EffectiveRange )
					{
						// Get the distance to the enemy being attacked. [17/7/2016 Matthew Woolley]
						float DistanceToEnemy = HitEnemy->GetDistanceTo( this );

						// Get the damage we should deal (the closer to the maximum range distance, the more damage). [17/7/2016 Matthew Woolley]
						int DamageToDeal = FMath::FRandRange( WeaponProperties.EffectiveRangeMinDamage, WeaponProperties.EffectiveRangeMaxDamage ) - ( WeaponProperties.MaximumRangeMaxDamage * ( ( DistanceToEnemy - WeaponProperties.EffectiveRange ) / WeaponProperties.MaximumRange ) );

						// Deal damage to the enemy. [17/7/2016 Matthew Woolley]
						HitEnemy->TakeDamage( DamageToDeal );

					} else
					{
						// Deal damage to the enemy. [17/7/2016 Matthew Woolley]
						HitEnemy->TakeDamage( FMath::FRandRange( WeaponProperties.EffectiveRangeMinDamage, WeaponProperties.EffectiveRangeMaxDamage ) );
					}
				}
			}
		}

	}

	// Make sure the weapon cools down before shooting again [20/11/2015 Matthew Woolley]
	WeaponProperties.bIsCoolingDown = true;
	GetWorld()->GetTimerManager().SetTimer( WeaponCooldownHandle, this, &AWeapon::Cooldown, WeaponProperties.CoolDownTime );

	// If this weapon relies on ammo. [26/7/2016 Matthew Woolley]
	if ( ItemInfo.MaxAmmo != 0 )
	{
		// Remove ammo from the gun. [24/7/2016 Matthew Woolley]
		ItemInfo.CurrentAmmo--;
		PlayerCharacter->CharactersInventory->ItemSlots[ PlayerCharacter->CharactersInventory->EquippedItemsSlot ].CurrentAmmo--;
	}
}

// Called when the user wishes to reload; bShouldUseFullClip will be true if they don't hold the reload key. [25/7/2016 Matthew Woolley]
void AWeapon::Reload( bool bUseFullClip )
{
	if ( WeaponProperties.bIsReloading || WeaponProperties.bIsCoolingDown || ItemInfo.CurrentAmmo == ItemInfo.MaxAmmo )
		return;

	// Get the UWorld object to spawn the ammo class. [27/7/2016 Matthew Woolley]
	UWorld* const World = GetWorld();

	// If the UWorld instance exists. [27/7/2016 Matthew Woolley]
	if ( World )
	{
		// Spawn the ammo class. [27/7/2016 Matthew Woolley]
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = Instigator;
		TemporaryItemInfoHolder = World->SpawnActor<AItem>( WeaponProperties.AmmoType, FVector( 0, 0, 0 ), GetActorRotation(), SpawnParams );

		// If the ammo was created successfully. [27/7/2016 Matthew Woolley]
		if ( TemporaryItemInfoHolder )
		{
			GEngine->AddOnScreenDebugMessage( -1, 5.f, FColor::Red, TEXT( "1" ) );
			// Hide the ammo class. [28/7/2016 Matthew Woolley]
			TemporaryItemInfoHolder->SetActorHiddenInGame( true );

			// Tell the code whether the user requested a full clip or not. [27/7/2016 Matthew Woolley]
			bShouldUseFullClip = bUseFullClip;

			// Tell the code we are reloading. [27/7/2016 Matthew Woolley]
			WeaponProperties.bIsReloading = true;

			// If this weapon uses a clip. [27/7/2016 Matthew Woolley]
			if ( TemporaryItemInfoHolder->ItemInfo.bIsClip )
			{
				GetWorld()->GetTimerManager().SetTimer( WeaponReloadHandle, this, &AWeapon::FullReload, WeaponProperties.ReloadTime, false );
			} else
			{
				GetWorld()->GetTimerManager().SetTimer( WeaponReloadHandle, this, &AWeapon::SingleRoundReload, WeaponProperties.ReloadTime, true );
			}
		}
	}
}

// Called when the weapon has cooled down. [27/7/2016 Matthew Woolley]
void AWeapon::Cooldown()
{
	WeaponProperties.bIsCoolingDown = false;
}

// Called when the weapon used is using a clip and requires a "full-reload" each time. [27/7/2016 Matthew Woolley]
void AWeapon::FullReload()
{
	// Get Ned so we can use his inventory later. [25/7/2016 Matthew Woolley]
	ARoadFeverCharacterNed* PlayerCharacter = Cast<ARoadFeverCharacterNed>( GetWorld()->GetFirstPlayerController()->GetPawn() );

	// Setup its inventory properties. [27/7/2016 Matthew Woolley]
	FInventoryItem ClipToAdd;
	ClipToAdd.bIsClip = true;
	ClipToAdd.bIsEquipable = false;
	ClipToAdd.CurrentAmmo = ItemInfo.CurrentAmmo;
	ClipToAdd.DisplayIcon = TemporaryItemInfoHolder->ItemInfo.DisplayIcon;
	ClipToAdd.DisplayName = TemporaryItemInfoHolder->ItemInfo.DisplayName;
	ClipToAdd.ItemClass = ItemInfo.ItemClass;
	ClipToAdd.ItemToolTip = TemporaryItemInfoHolder->ItemInfo.ItemToolTip;
	ClipToAdd.MaxItemStack = TemporaryItemInfoHolder->ItemInfo.MaxItemStack;

	// Add it to the inventory. [27/7/2016 Matthew Woolley]
	bool bAddedItem =  ItemInfo.CurrentAmmo != 0 ?  PlayerCharacter->AddItemToInventory( ClipToAdd ) : false;

	// Remove the current ammo from the gun. [27/7/2016 Matthew Woolley]
	ItemInfo.CurrentAmmo = 0;

	// Store the largest ammo count found. [27/7/2016 Matthew Woolley]
	FInventoryItem LargestAmmoCount;
	int32 LargestAmmoCountNumber = 0;
	int32 LargestAmmoCountSlotNumber = 0;

	// In case there was an issue with the partial reload, revert to this (if there is one). [27/7/2016 Matthew Woolley]
	FInventoryItem FullClip;
	int32 FullClipSlotNumber = 0;
	bool bFoundClip = false;

	// For each slot in the inventory, search for the ammo this weapon uses. [27/7/2016 Matthew Woolley]
	for ( int32 iSlotIterator = 0; iSlotIterator < PlayerCharacter->CharactersInventory->ItemSlots.Num(); iSlotIterator++ )
	{
		// If the slot we have found has more ammo than the last one we found AND is the ammo this weapon uses. [27/7/2016 Matthew Woolley]
		if ( LargestAmmoCountNumber <= PlayerCharacter->CharactersInventory->ItemSlots[ iSlotIterator ].CurrentAmmo && TemporaryItemInfoHolder->ItemInfo.DisplayName == PlayerCharacter->CharactersInventory->ItemSlots[ iSlotIterator ].DisplayName )
		{
			// If the user has requested a partial reload AND this is a full clip. [27/7/2016 Matthew Woolley]
			if ( !bShouldUseFullClip && PlayerCharacter->CharactersInventory->ItemSlots[ iSlotIterator ].CurrentAmmo == ItemInfo.MaxAmmo )
			{
				// Store the full clip in case there is an issue with the partial clip. [27/7/2016 Matthew Woolley]
				FullClip = PlayerCharacter->CharactersInventory->ItemSlots[ iSlotIterator ];
				FullClipSlotNumber = iSlotIterator;
				bFoundClip = true;
				continue;
			} else
			{
				// Store the clip for a reload. [27/7/2016 Matthew Woolley]
				LargestAmmoCount = PlayerCharacter->CharactersInventory->ItemSlots[ iSlotIterator ];
				LargestAmmoCountNumber = PlayerCharacter->CharactersInventory->ItemSlots[ iSlotIterator ].CurrentAmmo;
				LargestAmmoCountSlotNumber = iSlotIterator;
				bFoundClip = true;
			}
		}
	}

	// If we found a clip (partial or full) that is of the type requested (partial or full). [27/7/2016 Matthew Woolley]
	if ( LargestAmmoCount.CurrentAmmo > 0 && bFoundClip )
	{
		// Reload this weapon with the amount of ammo in the clip. [27/7/2016 Matthew Woolley]
		ItemInfo.CurrentAmmo = LargestAmmoCount.CurrentAmmo;
		PlayerCharacter->CharactersInventory->ItemSlots[ LargestAmmoCountSlotNumber ].CurrentItemStack--;
	} else if ( FullClip.CurrentAmmo > 0 && bFoundClip ) // If we only found a full clip, but a partial was requested. [27/7/2016 Matthew Woolley]
	{
		// Reload the full clip. [27/7/2016 Matthew Woolley]
		ItemInfo.CurrentAmmo = FullClip.CurrentAmmo;
		PlayerCharacter->CharactersInventory->ItemSlots[ FullClipSlotNumber ].CurrentItemStack--;
	}

	// If the old clip wasn't added to the inventory. [25/7/2016 Matthew Woolley]
	if ( !bAddedItem && ClipToAdd.CurrentAmmo != 0 )
	{
		// Throw the item onto the ground in front of Ned. [25/7/2016 Matthew Woolley]
		FVector SpawnLocation = PlayerCharacter->GetActorRotation().Vector() * 20;
		TemporaryItemInfoHolder->SetActorLocation( SpawnLocation );
		TemporaryItemInfoHolder->SetActorHiddenInGame( false );

	} else // If the item was added. [25/7/2016 Matthew Woolley]
	{
		// Destroy it from the level. [25/7/2016 Matthew Woolley]
		TemporaryItemInfoHolder->Destroy();
	}

	// Tell the code the weapon is no longer reloading. [27/7/2016 Matthew Woolley]
	PlayerCharacter->CharactersInventory->ItemSlots[ PlayerCharacter->CharactersInventory->EquippedItemsSlot ].CurrentAmmo = ItemInfo.CurrentAmmo;
	WeaponProperties.bIsReloading = false;
}

// Called when the weapon used is loaded one-shot at a time. [27/7/2016 Matthew Woolley]
void AWeapon::SingleRoundReload()
{
	// If the gun has full ammo. [27/7/2016 Matthew Woolley]
	if ( ItemInfo.CurrentAmmo == ItemInfo.MaxAmmo )
	{
		// Stop the gun from reloading further. [27/7/2016 Matthew Woolley]
		WeaponProperties.bIsReloading = false;
		GetWorld()->GetTimerManager().ClearTimer( WeaponReloadHandle );
		TemporaryItemInfoHolder->Destroy();
	}

	// Get Ned so we can use his inventory later. [25/7/2016 Matthew Woolley]
	ARoadFeverCharacterNed* PlayerCharacter = ( ARoadFeverCharacterNed* ) GetWorld()->GetFirstPlayerController()->GetPawn();

	bool bFoundAmmo = false;

	// For each slot in the inventory, search for the ammo this weapon uses. [27/7/2016 Matthew Woolley]
	for ( int32 iSlotIterator = 0; iSlotIterator < PlayerCharacter->CharactersInventory->ItemSlots.Num(); iSlotIterator++ )
	{
		// If this slot contains the ammo this weapon uses AND there is actually ammo in that slot. [27/7/2016 Matthew Woolley]
		if ( TemporaryItemInfoHolder->ItemInfo.DisplayName == PlayerCharacter->CharactersInventory->ItemSlots[ iSlotIterator ].DisplayName && PlayerCharacter->CharactersInventory->ItemSlots[ iSlotIterator ].CurrentItemStack > 0 )
		{
			PlayerCharacter->CharactersInventory->ItemSlots[ iSlotIterator ].CurrentItemStack--;
			ItemInfo.CurrentAmmo++;
			PlayerCharacter->CharactersInventory->ItemSlots[ PlayerCharacter->CharactersInventory->EquippedItemsSlot ].CurrentAmmo++;
			bFoundAmmo = true;
		}
	}

	// If the gun is full, or there isn't any ammo. [27/7/2016 Matthew Woolley]
	if ( !bFoundAmmo || ItemInfo.CurrentAmmo == ItemInfo.MaxAmmo )
	{
		// Stop the gun from reloading further. [27/7/2016 Matthew Woolley]
		WeaponProperties.bIsReloading = false;
		GetWorld()->GetTimerManager().ClearTimer( WeaponReloadHandle );
		TemporaryItemInfoHolder->Destroy();
	}

	if ( bShouldInterrupt )
	{
		WeaponProperties.bIsReloading = false;
		GetWorld()->GetTimerManager().ClearTimer( WeaponReloadHandle );
		TemporaryItemInfoHolder->Destroy();
		bShouldInterrupt = false;
	}
}

