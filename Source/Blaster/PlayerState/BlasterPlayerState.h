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
    virtual void OnRep_Score() override;
    void AddToScore(float ScoreAmount);
private:

    UPROPERTY(VisibleAnywhere)
    class ABlasterCharacter* Character;

    UPROPERTY(VisibleAnywhere)
    class ABlasterPlayerController* Controller;

    void SetHUDScore();
    
};
