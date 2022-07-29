// Blaster Game. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "BlasterPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterPlayerState : public APlayerState
{
    GENERATED_BODY()

public:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void OnRep_Score() override;
    UFUNCTION()
    virtual void OnRep_Defeats();
    void AddToScore(float ScoreAmount);
    void AddToDefeats(int32 DefeatsAmount);
    void CheckSetController();
private:

    UPROPERTY(VisibleAnywhere)
    class ABlasterCharacter* Character;

    UPROPERTY(VisibleAnywhere)
    class ABlasterPlayerController* Controller;

    void SetHUDScore();
    void SetHUDDefeats();

    UPROPERTY(VisibleAnywhere, ReplicatedUsing = "OnRep_Defeats")
    int32 Defeats = 0;
    
};
