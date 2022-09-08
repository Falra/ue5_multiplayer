// Blaster Game. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blaster/BlasterTypes/Team.h"
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
    void ShowDefeatedAnimation();
private:

    UPROPERTY(VisibleAnywhere)
    class ABlasterCharacter* Character;

    UPROPERTY(VisibleAnywhere)
    class ABlasterPlayerController* Controller;

    void SetHUDScore();
    void SetHUDDefeats();
    
    void CheckSetController();

    UPROPERTY(VisibleAnywhere, ReplicatedUsing = "OnRep_Defeats")
    int32 Defeats = 0;

    UPROPERTY(ReplicatedUsing = "OnRep_Team")
    ETeam Team = ETeam::ET_NoTeam;
    
    UFUNCTION()
    void OnRep_Team();

public:
    FORCEINLINE ETeam GetTeam() const { return Team; }
    void SetTeam(ETeam TeamToSet);
};
