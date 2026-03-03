// No Rights Reserved @ Team Expedition 


#include "WebSockets/WebSocketGameInstance.h"
#include "WebSocketsModule.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "RemotePlayer.h"
#include "Variant_Shooter/ShooterCharacter.h"
#include "GameFramework/Character.h"

void UWebSocketGameInstance::Init()
{
	Super::Init();

	if (!FModuleManager::Get().IsModuleLoaded("WebSockets"))
	{
		FModuleManager::Get().LoadModule("WebSockets");
	}

	Socket = FWebSocketsModule::Get().CreateWebSocket("ws://localhost:8080");

	Socket->OnConnected().AddLambda([]()
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, "Connected");
		});

	Socket->OnConnectionError().AddLambda([](const FString& ErrorString)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, ErrorString);
	});
	
	Socket->OnClosed().AddLambda([](int32 StatusCode, const FString& Reason, bool bWasClean)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, bWasClean ? FColor::Green : FColor::Red, "Connection Closed: " + Reason);
	});

	Socket->OnMessageSent().AddLambda([](const FString& MessageString)
	{
		//UE_LOG(LogTemp, Log, TEXT("Message Recieved: %s"), *MessageString);
	});

	Socket->OnMessage().AddLambda([this](const FString& MessageString)
	{
		//UE_LOG(LogTemp, Log, TEXT("Message Recieved: %s"), *MessageString);

		TSharedPtr<FJsonObject> Json;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(MessageString);
        if (!FJsonSerializer::Deserialize(Reader, Json)) return;

        FString Type = Json->GetStringField("type");

		// I've implemented my own RPC system (kinda) 
        if (Type == "welcome")
        {
            CurrentPlayerId = Json->GetStringField("assignedId");
        	
            UE_LOG(LogTemp, Log, TEXT("My ID: %s"), *CurrentPlayerId);
        }
        else if (Type == "playerJoined")
        {
            FString Id = Json->GetStringField("id");
            if (Id == CurrentPlayerId) return;

            FVector Pos(
                Json->GetObjectField("position")->GetNumberField("x"),
                Json->GetObjectField("position")->GetNumberField("y"),
                Json->GetObjectField("position")->GetNumberField("z")
            );

            if (RemotePlayerClass && GetWorld())
            {
                ARemotePlayer* NewPlayer = GetWorld()->SpawnActor<ARemotePlayer>(RemotePlayerClass, Pos, FRotator::ZeroRotator);
                if (NewPlayer)
                {
                	// Immediately set current location (no initial pop)
					NewPlayer->SetActorLocation(Pos);
					NewPlayer->SetActorRotation(FRotator::ZeroRotator);

					// set targets so interpolation starts from correct place
					NewPlayer->WSTargetPos = Pos;
					NewPlayer->WSTargetRot = FRotator::ZeroRotator;
                					
                    RemotePlayers.Add(Id, NewPlayer);
                	
                    UE_LOG(LogTemp, Log, TEXT("Spawned remote player %s"), *Id);
                }
            }
        }
        else if (Type == "playerMoved")
        {
            FString Id = Json->GetStringField("id");
            if (Id == CurrentPlayerId) return;

            ARemotePlayer** Found = RemotePlayers.Find(Id);
            if (Found && *Found)
            {
            	ARemotePlayer* RemoteChar = Cast<ARemotePlayer>(*Found);
				if (RemoteChar)  // Cast succeeds only if it's really the custom class
				{
					FVector Pos(
						Json->GetObjectField("position")->GetNumberField("x"),
						Json->GetObjectField("position")->GetNumberField("y"),
						Json->GetObjectField("position")->GetNumberField("z")
					);
					//(*Found)->SetActorLocation(Pos);
					FRotator Rot(
						Json->GetObjectField("rotation")->GetNumberField("pitch"),
						Json->GetObjectField("rotation")->GetNumberField("yaw"),
						Json->GetObjectField("rotation")->GetNumberField("roll")
					);
					//(*Found)->SetActorRotation(Rot);
					RemoteChar->WSTargetPos = Pos;
					RemoteChar->WSTargetRot = Rot;
				}
            	else
				{
					GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, TEXT("Failed to Cast to Remote Character"));
				}
            }
        }
		else if (Type == "playerFired")
		{
			FString Id = Json->GetStringField("id");
			if (Id == CurrentPlayerId) return;

			ARemotePlayer** Found = RemotePlayers.Find(Id);
			if (Found && *Found)
			{
				ARemotePlayer* RemoteChar = Cast<ARemotePlayer>(*Found);
				if (RemoteChar)  // Cast succeeds only if it's really the custom class
				{
					FVector HitLocation(
					Json->GetObjectField("hitLocation")->GetNumberField("x"),
					Json->GetObjectField("hitLocation")->GetNumberField("y"),
					Json->GetObjectField("hitLocation")->GetNumberField("z")
					);

					// Visual feedback for remote fire
					DrawDebugSphere(
						GetWorld(),
						HitLocation,
						20.0f,
						16,
						FColor::Purple,
						false,
						4.0f, // Last 4 seconds
						0,
						2.0f
					);

					UE_LOG(LogTemp, Log, TEXT("Remote player %s fired → impact seen"), *Id);
				}
			}
		}
        else if (Type == "playerLeft")
        {
            FString Id = Json->GetStringField("id");
            ARemotePlayer** Found = RemotePlayers.Find(Id);
            if (Found && *Found)
            {
                (*Found)->Destroy();
                RemotePlayers.Remove(Id);
            }
        }
	});
	
	Socket->Connect();
}

void UWebSocketGameInstance::SendMovement(const FVector& Position, const FRotator& Rotation)
{
	if (!Socket.IsValid() || !Socket->IsConnected()) return;

	// 'Move' Type
	TSharedPtr<FJsonObject> Json = MakeShareable(new FJsonObject);
	Json->SetStringField("type", "move");

	// Position Field
	TSharedPtr<FJsonObject> PosObj = MakeShareable(new FJsonObject);
	PosObj->SetNumberField("x", FMath::RoundToFloat(Position.X * 100.0f) / 100.0f);
	PosObj->SetNumberField("y", FMath::RoundToFloat(Position.Y * 100.0f) / 100.0f);
	PosObj->SetNumberField("z", FMath::RoundToFloat(Position.Z * 100.0f) / 100.0f);
	Json->SetObjectField("position", PosObj);

	// Rotation Field
	TSharedPtr<FJsonObject> RotObj = MakeShareable(new FJsonObject);
	RotObj->SetNumberField("pitch", FMath::RoundToFloat(Rotation.Pitch * 100.0f) / 100.0f);
	RotObj->SetNumberField("yaw",   FMath::RoundToFloat(Rotation.Yaw   * 100.0f) / 100.0f);
	RotObj->SetNumberField("roll",  FMath::RoundToFloat(Rotation.Roll  * 100.0f) / 100.0f);
	Json->SetObjectField("rotation", RotObj);

	// Send JSON to server
	FString Output;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Output);
	FJsonSerializer::Serialize(Json.ToSharedRef(), Writer);

	Socket->Send(Output);
	
	// UE_LOG(LogTemp, Log, TEXT("Sent movement -> %s"), *Output);
}

void UWebSocketGameInstance::SendHitcast(const FHitResult& HitResult)
{
	if (!Socket.IsValid() || !Socket->IsConnected()) return;

	// `Fire` Type
	TSharedPtr<FJsonObject> Json = MakeShareable(new FJsonObject);
	Json->SetStringField("type", "fire");

	// Hit Location Field
	TSharedPtr<FJsonObject> HitObj = MakeShareable(new FJsonObject);
	HitObj->SetNumberField("x", FMath::RoundToFloat(HitResult.Location.X * 100.0f) / 100.0f);
	HitObj->SetNumberField("y", FMath::RoundToFloat(HitResult.Location.Y * 100.0f) / 100.0f);
	HitObj->SetNumberField("z", FMath::RoundToFloat(HitResult.Location.Z * 100.0f) / 100.0f);
	Json->SetObjectField("hitLocation", HitObj);

	// Send JSON to server
	FString Output;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Output);
	FJsonSerializer::Serialize(Json.ToSharedRef(), Writer);

	Socket->Send(Output);
	
	// UE_LOG(LogTemp, Log, TEXT("Sent Fire -> %s"), *Output);
}

/*
Create a player state when a player connects
Each player will have its own transform rotation
Add another array, once we have the client now create a player for them
Each player will have its own class - all it will need is the transform

When creates - broadcast meessage to all clients to say a player has been created
When a new connects - check is there other players connected for that player

When a client presses "forward" (ie) 
send message data to say player has changed trasnform
then apply to each client

Review CLient Side Prediction to account for latency
Server Reconcilision

*/

void UWebSocketGameInstance::Shutdown()
{
	if (Socket->IsConnected())
	{
		Socket->Close();
	}
	Super::Shutdown();
}


