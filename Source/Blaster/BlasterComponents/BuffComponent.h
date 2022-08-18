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
    void BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime);
    void SetInitialSpeed(float BaseSpeed, float CrouchSpeed);
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
protected:
    virtual void BeginPlay() override;
    void HealRampUp(float DeltaTime);
private:
    UPROPERTY()
    class ABlasterCharacter* Character;

#pragma region HealBuff
    bool bHealing = false;
    float HealingRate = 0.0f;
    float AmountToHeal = 0.0f;
#pragma endregion

#pragma region SpeedBuff
    FTimerHandle SpeedBuffTimer;
    void ResetSpeeds();
    float InitialBaseSpeed;
    float InitialCrouchSpeed;
#pragma endregion

};
