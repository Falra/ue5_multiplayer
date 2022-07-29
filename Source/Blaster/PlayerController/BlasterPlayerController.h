// Blaster Game. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    void SetHUDHealth(float Health, float MaxHealth);
    void SetHUDScore(float Score);
    void SetHUDDefeats(int32 Defeats);
protected:
    virtual void OnPossess(APawn* aPawn) override;
    virtual void BeginPlay() override;
private:
    UPROPERTY(VisibleAnywhere)
    class ABlasterHUD* BlasterHUD;
};
