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

    const auto AimRotation = BlasterCharacter->GetBaseAimRotation();
    const auto MovementRotation = UKismetMathLibrary::MakeRotFromX(BlasterCharacter->GetVelocity());
    YawOffset = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;
}
