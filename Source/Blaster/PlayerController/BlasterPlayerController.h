// Blaster Game. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blaster/Weapon/WeaponTypes.h"
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
    void SetHUDWeaponAmmo(int32 Ammo);
    void SetHUDCarriedAmmo(int32 Ammo);
    void SetHUDWeaponType(EWeaponType WeaponType);
    void SetHUDMatchCountdown(float CountdownTime);
    UFUNCTION(NetMulticast, Unreliable)
    void MulticastShowDefeatedAnimation();
    virtual void Tick(float DeltaSeconds) override;
protected:
    virtual void OnPossess(APawn* aPawn) override;
    virtual void BeginPlay() override;
    void SetHUDTime();
private:
    UPROPERTY(VisibleAnywhere)
    class ABlasterHUD* BlasterHUD;
    bool IsHUDValid();

    float MatchTime = 120.0f;
    uint32 CountdownInt = 0;
};
