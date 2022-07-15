// Blaster Game. All Rights Reserved

#include "BlasterAnimInstance.h"

#include "BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UBlasterAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
}

void UBlasterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);
    if(!BlasterCharacter) return;

    FVector Velocity = BlasterCharacter->GetVelocity();
    Velocity.Z = 0.0f;
    Speed = Velocity.Size();

    bIsInAir = BlasterCharacter->GetCharacterMovement()->IsFalling();

    bIsAccelerating = !BlasterCharacter->GetCharacterMovement()->GetCurrentAcceleration().IsZero();
    bWeaponEquipped = BlasterCharacter->IsWeaponEquipped();
    bIsCrouched = BlasterCharacter->bIsCrouched;
    bIsAiming = BlasterCharacter->IsAiming();

    // Yaw offset for strafing
    const auto AimRotation = BlasterCharacter->GetBaseAimRotation();
    const auto MovementRotation = UKismetMathLibrary::MakeRotFromX(BlasterCharacter->GetVelocity());
    const auto DeltaYaw = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
    DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaYaw, DeltaSeconds, 6.0f);
    YawOffset = DeltaRotation.Yaw;

    // Lean
    CharacterRotationLastFrame = CharacterRotation;
    CharacterRotation = BlasterCharacter->GetActorRotation();
    const auto DeltaLean = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
    const float Target = DeltaLean.Yaw / DeltaSeconds;
    const float Interpolated = FMath::FInterpTo(Lean, Target, DeltaSeconds, 6.0f);
    Lean = FMath::Clamp(Interpolated, -90.f, 90.f);

    // Aim offset
    AO_Yaw = BlasterCharacter->GetAO_Yaw();
    
}
