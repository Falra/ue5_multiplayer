// Blaster Game. All Rights Reserved

#include "BlasterAnimInstance.h"

#include "BlasterCharacter.h"
#include "Blaster/Weapon/Weapon.h"
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
    EquippedWeapon = BlasterCharacter->GetEquippedWeapon();
    bIsCrouched = BlasterCharacter->bIsCrouched;
    bIsAiming = BlasterCharacter->IsAiming();
    TurningInPlace = BlasterCharacter->GetTurningInPlace();
    bRotateRootBone = BlasterCharacter->ShouldRotateRootBone();
    bEliminated = BlasterCharacter->IsEliminated();
    bUseFABRIK = BlasterCharacter->GetCombatState() != ECombatState::ECS_Reloading;
    bUseAimOffsets = BlasterCharacter->GetCombatState() != ECombatState::ECS_Reloading;
    bTransformRightHand = BlasterCharacter->GetCombatState() != ECombatState::ECS_Reloading;
        
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
    AO_Pitch = BlasterCharacter->GetAO_Pitch();

    // Left hand position for FABRIK
    if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && BlasterCharacter->GetMesh())
    {
        LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
        FVector OutPosition;
        FRotator OutRotation;
        BlasterCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(),
            FRotator::ZeroRotator, OutPosition, OutRotation);
        LeftHandTransform.SetLocation(OutPosition);
        LeftHandTransform.SetRotation(FQuat(OutRotation));

        if (BlasterCharacter->IsLocallyControlled())
        {
            bLocallyControlled = true;
            const FTransform RightHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("Hand_R"), ERelativeTransformSpace::RTS_World);
            const FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - BlasterCharacter->GetHitTarget()));
            RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaSeconds, 30.0f);
        }

        /*
        const FTransform MuzzleTipTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("MuzzleFlash"), RTS_World);
        const FVector MuzzleX(FRotationMatrix(MuzzleTipTransform.GetRotation().Rotator()).GetUnitAxis(EAxis::X));
        DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), MuzzleTipTransform.GetLocation() + MuzzleX * 1000.0f, FColor::Red);
        DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), BlasterCharacter->GetHitTarget(), FColor::Orange);
        */
    }
}
