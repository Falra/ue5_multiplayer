// Blaster Game. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Blaster/BlasterTypes/TurningInPlace.h"
#include "BlasterAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UBlasterAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

private:
    UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = true))
    class ABlasterCharacter* BlasterCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = true))
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = true))
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = true))
    bool bIsAccelerating;
    
    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = true))
    bool bWeaponEquipped;

    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = true))
    bool bIsCrouched;

    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = true))
    bool bIsAiming;

    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = true))
    float YawOffset;

    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = true))
    float Lean;

    UPROPERTY(BlueprintReadOnly, Category = "AimOffset", meta = (AllowPrivateAccess = true))
    float AO_Yaw;

    UPROPERTY(BlueprintReadOnly, Category = "AimOffset", meta = (AllowPrivateAccess = true))
    float AO_Pitch;

    UPROPERTY(BlueprintReadOnly, Category = "IK", meta = (AllowPrivateAccess = true))
    FTransform LeftHandTransform;

    UPROPERTY(BlueprintReadOnly, Category = "IK", meta = (AllowPrivateAccess = true))
    class AWeapon* EquippedWeapon;

    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = true))
    ETurningInPlace TurningInPlace;

    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = true))
    FRotator RightHandRotation;

    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = true))
    bool bLocallyControlled;

    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = true))
    bool bRotateRootBone;

    UPROPERTY(BlueprintReadOnly, Category = "Player stats", meta = (AllowPrivateAccess = true))
    bool bEliminated;

    UPROPERTY(BlueprintReadOnly, Category = "IK", meta = (AllowPrivateAccess = true))
    bool bUseFABRIK;

    UPROPERTY(BlueprintReadOnly, Category = "AimOffset", meta = (AllowPrivateAccess = true))
    bool bUseAimOffsets;

    UPROPERTY(BlueprintReadOnly, Category = "IK", meta = (AllowPrivateAccess = true))
    bool bTransformRightHand;

    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = true))
    bool bHoldingTheFlag;

    FRotator CharacterRotationLastFrame;
    FRotator CharacterRotation;
    FRotator DeltaRotation;
};
