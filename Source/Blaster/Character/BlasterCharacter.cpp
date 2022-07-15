// Blaster Game. All Rights Reserved

#include "BlasterCharacter.h"

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
    GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
}

void ABlasterCharacter::BeginPlay()
{
    Super::BeginPlay();
}

void ABlasterCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    AimOffset(DeltaTime);
}

void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
    PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &ABlasterCharacter::EquipButtonPressed);
    PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ABlasterCharacter::CrouchButtonPressed);
    PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ABlasterCharacter::AimButtonPressed);
    PlayerInputComponent->BindAction("Aim", IE_Released, this, &ABlasterCharacter::AimButtonReleased);

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
        const auto CurrentAimRotation = FRotator(0.0f, GetBaseAimRotation().Yaw, 0.0f);
        const auto DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
        AO_Yaw = DeltaAimRotation.Yaw;
        bUseControllerRotationYaw = false;
    }
    if (bIsMoving || bIsInAir)
    {
        StartingAimRotation = FRotator(0.0f, GetBaseAimRotation().Yaw, 0.0f);
        AO_Yaw = 0.0f;
        bUseControllerRotationYaw = true;
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

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
}
