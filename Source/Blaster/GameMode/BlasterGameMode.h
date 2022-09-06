// Blaster Game. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BlasterGameMode.generated.h"


namespace MatchState
{
    extern BLASTER_API const FName Cooldown; // Match ended, display player stats and restart timer
}

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterGameMode : public AGameMode
{
    GENERATED_BODY()
public:
    ABlasterGameMode();
    virtual void Tick(float DeltaSeconds) override;
    virtual void PlayerEliminated(class ABlasterCharacter* EliminatedCharacter, class ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController);
    virtual void RequestRespawn(ACharacter* EliminatedCharacter, AController* EliminatedController);
    void PlayerLeftGame(class ABlasterPlayerState* PlayerLeaving);
    
    UPROPERTY(EditDefaultsOnly, Category = "Game")
    float WarmupTime = 10.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Game")
    float MatchTime = 120.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Game")
    float CooldownTime = 10.0f;

    float LevelStartingTime = 0.0f;
protected:
    virtual void BeginPlay() override;
    virtual void OnMatchStateSet() override;
private:
    float CountdownTime = 0.0f;
public:
    FORCEINLINE float GetCountdownTime() const { return CountdownTime; };
};
