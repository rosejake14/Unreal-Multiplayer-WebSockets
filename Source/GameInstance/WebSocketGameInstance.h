// No Rights Reserved @ Team Expedition 

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "IWebSocket.h"
#include "RemotePlayer.h"
#include "WebSocketGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class EXPPRODEV_API UWebSocketGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;
	virtual void Shutdown() override;

	UFUNCTION(BlueprintCallable)
	void SendMovement(const FVector& Position, const FRotator& Rotation);

	UFUNCTION(BlueprintCallable)
	void SendHitcast(const FHitResult& HitResult);

	TSharedPtr<IWebSocket> Socket;

private:
	FString CurrentPlayerId;
	TMap<FString, ARemotePlayer*> RemotePlayers;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ARemotePlayer> RemotePlayerClass;

	UPROPERTY(EditAnywhere)
	FVector TargetPosition;
	UPROPERTY(EditAnywhere)
	FRotator TargetRotation;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float LerpAlpha = 15.0f; 
};
