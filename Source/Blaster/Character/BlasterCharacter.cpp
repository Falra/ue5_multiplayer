// Blaster Game. All Rights Reserved

#include "BlasterCharacter.h"

#include "Blaster/Blaster.h"
#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Blaster/Weapon/Weapon.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"

ABlasterCharacter::ABlasterCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>("CameraBoom");
    CameraBoom->SetupAttachment(GetMesh());
    CameraBoom->TargetArmLength = 600.0f;
    CameraBoom->bUsePawnControlRotation = true;

    FollowCamera = CreateDefaultSubobject<UCameraComponent>("FollowCamera");
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    bUseControllerRotationYaw = false;
    GetCharacterMovement()->bOrientRotationToMovement = true;

    OverheadWidget = CreateDefaultSubobject<UWidgetComponent>("OverheadWidget");
    OverheadWidget->SetupAttachment(GetRootComponent());

    CombatComponent = CreateDefaultSubobject<UCombatComponent>("CombatComponent");
    CombatComponent->SetIsReplicated(true);

    // Ignore other players when trace from Camera
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
    GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
    GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
    GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

    TurningInPlace = ETurningInPlace::ETIP_NotTurning;

    NetUpdateFrequency = 66.0f;
    MinNetUpdateFrequency = 33.0f;

    GetCharacterMovement()->RotationRate = FRotator(0.0f, 0.0f, 720.0f);
}

void ABlasterCharacter::BeginPlay()
{
    Super::BeginPlay();
}

void ABlasterCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy)
    {
        AimOffset(DeltaTime);
    }
    else
    {
        SimProxiesTurn();
    }

    HideCameraIfCharacterClose();
}

void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ABlasterCharacter::Jump);
    PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &ABlasterCharacter::EquipButtonPressed);
    PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ABlasterCharacter::CrouchButtonPressed);
    PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ABlasterCharacter::AimButtonPressed);
    PlayerInputComponent->BindAction("Aim", IE_Released, this, &ABlasterCharacter::AimButtonReleased);
    PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ABlasterCharacter::FireButtonPressed);
    PlayerInputComponent->BindAction("Fire", IE_Released, this, &ABlasterCharacter::FireButtonReleased);

    PlayerInputComponent->BindAxis("MoveForward", this, &ABlasterCharacter::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &ABlasterCharacter::MoveRight);
    PlayerInputComponent->BindAxis("Turn", this, &ABlasterCharacter::Turn);
    PlayerInputComponent->BindAxis("LookUp", this, &ABlasterCharacter::LookUp);

    GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
}

void ABlasterCharacter::PostInitializeComponents()
{
    Super::PostInitializeComponents();
    if (CombatComponent)
    {
        CombatComponent->Character = this;
    }
}

void ABlasterCharacter::MoveForward(float Value)
{
    if (!Controller || Value == 0.0f) return;
    const FRotator YawRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
    const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
    AddMovementInput(Direction, Value);
}

void ABlasterCharacter::MoveRight(float Value)
{
    if (!Controller || Value == 0.0f) return;
    const FRotator YawRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
    const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
    AddMovementInput(Direction, Value);
}

void ABlasterCharacter::Turn(float Value)
{
    AddControllerYawInput(Value);
}

void ABlasterCharacter::LookUp(float Value)
{
    AddControllerPitchInput(Value);
}

void ABlasterCharacter::EquipButtonPressed()
{
    if (CombatComponent)
    {
        if (HasAuthority())
        {
            CombatComponent->EquipWeapon(OverlappingWeapon);
        }
        else
        {
            ServerEquipButtonPressed();
        }
    }
}

void ABlasterCharacter::ServerEquipButtonPressed_Implementation()
{
    if (CombatComponent)
    {
        CombatComponent->EquipWeapon(OverlappingWeapon);
    }
}

void ABlasterCharacter::CrouchButtonPressed()
{
    bIsCrouched ? UnCrouch() : Crouch();
}

void ABlasterCharacter::AimButtonPressed()
{
    if (CombatComponent)
    {
        CombatComponent->SetAiming(true);
    }
}

void ABlasterCharacter::AimButtonReleased()
{
    if (CombatComponent)
    {
        CombatComponent->SetAiming(false);
    }
}

void ABlasterCharacter::AimOffset(float DeltaTime)
{
    if(!CombatComponent || !CombatComponent->EquippedWeapon) return;
    
    FVector Velocity = GetVelocity();
    Velocity.Z = 0.0f;
    const bool bIsMoving = !Velocity.IsZero();
    const bool bIsInAir = GetCharacterMovement()->IsFalling();
    
    if (!bIsMoving && !bIsInAir)
    {
        bRotateRootBone = true;
        const auto CurrentAimRotation = FRotator(0.0f, GetBaseAimRotation().Yaw, 0.0f);
        const auto DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
        AO_Yaw = DeltaAimRotation.Yaw;
        if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
        {
            InterpAO_Yaw = AO_Yaw;
        }
        bUseControllerRotationYaw = true;
        TurnInPlace(DeltaTime);
    }
    if (bIsMoving || bIsInAir)
    {
        bRotateRootBone = false;
        StartingAimRotation = FRotator(0.0f, GetBaseAimRotation().Yaw, 0.0f);
        AO_Yaw = 0.0f;
        bUseControllerRotationYaw = true;
        TurningInPlace = ETurningInPlace::ETIP_NotTurning;
    }

    AO_Pitch = GetBaseAimRotation().Pitch;
    if (AO_Pitch > 90.0f && !IsLocallyControlled())
    {
        // map Pitch form [270, 360) to [-90, 0)
        const FVector2D InRange(270.0f, 360.0f);
        const FVector2D OutRange(-90.0f, 0.0f);
        AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
    }
}

void ABlasterCharacter::SimProxiesTurn()
{
    if(!CombatComponent || !CombatComponent->EquippedWeapon) return;
    
    bRotateRootBone = false;
}

void ABlasterCharacter::Jump()
{
    if (bIsCrouched)
    {
        UnCrouch();
    }
    else
    {
        Super::Jump();
    }
    
}

void ABlasterCharacter::FireButtonPressed()
{
   if (CombatComponent)
   {
       CombatComponent->FireButtonPressed(true);
   }
}

void ABlasterCharacter::FireButtonReleased()
{
    if (CombatComponent)
    {
        CombatComponent->FireButtonPressed(false);
    }
}

void ABlasterCharacter::TurnInPlace(float DeltaTime)
{
    if (AO_Yaw > 90.0f)
    {
        TurningInPlace = ETurningInPlace::ETIP_Right;
    }
    else if (AO_Yaw < - 90.0f)
    {
        TurningInPlace = ETurningInPlace::ETIP_Left;
    }
    if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
    {
        InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.0f, DeltaTime, 4.0f);
        AO_Yaw = InterpAO_Yaw;
        if (FMath::Abs(AO_Yaw) < 15.0f)
        {
            TurningInPlace = ETurningInPlace::ETIP_NotTurning;
            StartingAimRotation = FRotator(0.0f, GetBaseAimRotation().Yaw, 0.0f);
        }
    }
}

void ABlasterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
    if (LastWeapon)
    {
        LastWeapon->ShowPickupWidget(false);
    }

    if (OverlappingWeapon)
    {
        OverlappingWeapon->ShowPickupWidget(true);
    }
}

void ABlasterCharacter::HideCameraIfCharacterClose()
{
    if (!IsLocallyControlled()) return;
    if ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)
    {
        GetMesh()->SetVisibility(false);
        if (CombatComponent && CombatComponent->EquippedWeapon)
        {
            CombatComponent->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
        }
    }
    else
    {
        GetMesh()->SetVisibility(true);
        if (CombatComponent && CombatComponent->EquippedWeapon)
        {
            CombatComponent->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
        }
    }
}

void ABlasterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
    if (OverlappingWeapon)
    {
        OverlappingWeapon->ShowPickupWidget(false);
    }

    OverlappingWeapon = Weapon;
    if (IsLocallyControlled() && OverlappingWeapon)
    {
        OverlappingWeapon->ShowPickupWidget(true);
    }
}

bool ABlasterCharacter::IsWeaponEquipped() const
{
    return (CombatComponent && CombatComponent->EquippedWeapon);
}

bool ABlasterCharacter::IsAiming() const
{
    return (CombatComponent && CombatComponent->bIsAiming);
}

AWeapon* ABlasterCharacter::GetEquippedWeapon() const
{
    if (!CombatComponent) return nullptr;
    return CombatComponent->EquippedWeapon;
}

FVector ABlasterCharacter::GetHitTarget() const
{
    if (!CombatComponent) return FVector();
    return CombatComponent->HitTarget;
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
}

void ABlasterCharacter::PlayFireMontage(bool bAiming)
{
    if (!CombatComponent || !FireWeaponMontage || !CombatComponent->EquippedWeapon) return;

    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (!AnimInstance) return;
    AnimInstance->Montage_Play(FireWeaponMontage);
    const FName SectionName = bAiming ? FName("RifleName") : FName("RifleHip");
    AnimInstance->Montage_JumpToSection(SectionName);
}

void ABlasterCharacter::PlayHitReactMontage() const
{
    if (!CombatComponent || !HitReactMontage || !CombatComponent->EquippedWeapon) return;
    
    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (!AnimInstance) return;
    AnimInstance->Montage_Play(HitReactMontage);
    const FName SectionName("FromFront");
    AnimInstance->Montage_JumpToSection(SectionName);
}

void ABlasterCharacter::MulticastHit_Implementation() const
{
    PlayHitReactMontage();
}