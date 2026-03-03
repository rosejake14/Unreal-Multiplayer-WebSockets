// No Rights Reserved @ Team Expedition 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "RemotePlayer.generated.h"

UCLASS()
class EXPPRODEV_API ARemotePlayer : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ARemotePlayer();

	// RoseJ: WEBSOCKETS - START
	UPROPERTY(Category=WS, EditAnywhere, BlueprintReadOnly)
	FVector WSTargetPos;
	UPROPERTY(Category=WS, EditAnywhere, BlueprintReadOnly)
	FRotator WSTargetRot;

	UPROPERTY(Category=WS, EditAnywhere, BlueprintReadOnly)
	float LerpSpeed = 15.0f;
	// RoseJ: WEBSOCKETS - END

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable, Category = "Remote")
	void SetRemoteLocation(const FVector& NewLocation);
};
