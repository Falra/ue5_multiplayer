// Blaster Game. All Rights Reserved

#include "BlasterCharacter.h"

#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Blaster/Weapon/Weapon.h"
#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
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
}

void ABlasterCharacter::BeginPlay()
{
    Super::BeginPlay();
}

void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
    PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &ABlasterCharacter::EquipButtonPressed);
    PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ABlasterCharacter::CrouchButtonPressed);

    PlayerInputComponent->BindAxis("MoveForward", this, &ABlasterCharacter::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &ABlasterCharacter::MoveRight);
    PlayerInputComponent->BindAxis("Turn", this, &ABlasterCharacter::Turn);
    PlayerInputComponent->BindAxis("LookUp", this, &ABlasterCharacter::LookUp);
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
    Crouch();
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

void ABlasterCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
}
