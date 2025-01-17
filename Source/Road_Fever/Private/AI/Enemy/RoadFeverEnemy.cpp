// Copyright Square Crank Games. All Rights Reserved.

#include "Road_Fever.h"
#include "RoadFeverEnemy.h"



// Setup default values for this enemy. [15/7/2016 Matthew Woolley]
ARoadFeverEnemy::ARoadFeverEnemy()
{
	// Set-up the enemy's stats. [8/2/2017 Matthew Woolley]
	EnemyHealth = 100;
	MinMoveSpeed = 1;
	MaxMoveSpeed = 1;

	// Set-up the mesh so it isn't orientated strangely. [8/2/2017 Matthew Woolley]
	GetMesh()->SetRelativeLocation( FVector( 0, 0, -90 ) );
	GetMesh()->SetRelativeRotation( FRotator( 0, -90, 0 ) );
}

void ARoadFeverEnemy::BeginPlay()
{
	MoveSpeed = FMath::FRandRange( MinMoveSpeed, MaxMoveSpeed );

	// If there is an enemy in the random enemy pool. [3/2/2017 Matthew Woolley]
	if ( EnemyMeshes.Num() != 0 )
	{
		// Pick a random enemy mesh. [3/2/2017 Matthew Woolley]
		int EnemyMeshToUse = FMath::FRandRange( 0, EnemyMeshes.Num() );

		// Set it as the mesh for this instance. [3/2/2017 Matthew Woolley]
		GetMesh()->SetSkeletalMesh( EnemyMeshes[EnemyMeshToUse] );
	}

	Super::BeginPlay();
}

// Called when this enemy no longer has any health. [15/7/2016 Matthew Woolley]
void ARoadFeverEnemy::Die_Implementation()
{
	// No default implementation of this method. [8/2/2017 Matthew Woolley]
}

// Takes away health from this enemy. [15/7/2016 Matthew Woolley]
void ARoadFeverEnemy::TakeDamage_Implementation( int32 InDamage )
{
	// Deal damage to the enemy. [8/2/2017 Matthew Woolley]
	EnemyHealth -= InDamage;

	// Check if the enemy has died. [8/2/2017 Matthew Woolley]
	if ( EnemyHealth <= 0 )
	{
		Die();
	}
}