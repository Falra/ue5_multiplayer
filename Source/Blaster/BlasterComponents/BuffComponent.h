// Blaster Game. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BLASTER_API UBuffComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UBuffComponent();
    friend class ABlasterCharacter;
    void Heal(float HealAmount, float HealingTime = 0.0f);
    void BuffShield(float ShieldAmount, float BuffTime = 0.0f);
    void BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime);
    void BuffJump(float BuffJumpSpeed, float BuffTime);
    void SetInitialSpeed(float BaseSpeed, float CrouchSpeed);
    void SetInitialJumpSpeed(float JumpSpeed);
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
protected:
    virtual void BeginPlay() override;
    void HealRampUp(float DeltaTime);
    void ShieldRampUp(float DeltaTime);
private:
    UPROPERTY()
    class ABlasterCharacter* Character;

#pragma region HealBuff
    bool bHealing = false;
    float HealingRate = 0.0f;
    float AmountToHeal = 0.0f;
#pragma endregion

#pragma region ShieldBuff
    bool bShieldBuff = false;
    float ShieldRate = 0.0f;
    float ShieldAmountToAdd = 0.0f;
#pragma endregion

#pragma region SpeedBuff
    FTimerHandle SpeedBuffTimer;
    void ResetSpeeds();
    float InitialBaseSpeed;
    float InitialCrouchSpeed;
    UFUNCTION(NetMulticast, Reliable)
    void MulticastSpeedBuff(float BaseSpeed, float CrouchSpeed);
#pragma endregion

#pragma region JumpBuff
    FTimerHandle JumpBuffTimer;
    void ResetJumpSpeed();
    float InitialJumpSpeed;
    UFUNCTION(NetMulticast, Reliable)
    void MulticastJumpBuff(float JumpSpeed);
#pragma endregion

};
