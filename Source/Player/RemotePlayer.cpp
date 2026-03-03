// No Rights Reserved @ Team Expedition 


#include "WebSockets/RemotePlayer.h"

// Sets default values
ARemotePlayer::ARemotePlayer()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = false;
}

// Called when the game starts or when spawned
void ARemotePlayer::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ARemotePlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// interpolate position
	FVector CurrentLocation = GetActorLocation();
	FVector NewLocation = FMath::VInterpTo(CurrentLocation, WSTargetPos, DeltaTime, LerpSpeed); // 15 = speed (higher = faster)
	SetActorLocation(NewLocation);

	// interpolate rotation
	FRotator CurrentRotation = GetActorRotation();
	FRotator NewRotation = FMath::RInterpTo(CurrentRotation, WSTargetRot, DeltaTime, LerpSpeed);
	SetActorRotation(NewRotation);
}

// Called to bind functionality to input
void ARemotePlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ARemotePlayer::SetRemoteLocation(const FVector& NewLocation)
{
}


