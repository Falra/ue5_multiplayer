// Blaster Game. All Rights Reserved

#include "BlasterCharacter.h"

#include "Blaster/Blaster.h"
#include "Blaster/BlasterComponents/BuffComponent.h"
#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Blaster/BlasterComponents/LagCompensationComponent.h"
#include "Blaster/GameMode/BlasterGameMode.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Blaster/Weapon/Weapon.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

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

    BuffComponent = CreateDefaultSubobject<UBuffComponent>("BuffComponent");
    BuffComponent->SetIsReplicated(true);

    LagCompensationComponent = CreateDefaultSubobject<ULagCompensationComponent>("LagCompensationComponent");
    
    // Ignore other players when trace from Camera
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
    GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
    GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
    GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

    TurningInPlace = ETurningInPlace::ETIP_NotTurning;

    NetUpdateFrequency = 66.0f;
    MinNetUpdateFrequency = 33.0f;

    GetCharacterMovement()->RotationRate = FRotator(0.0f, 0.0f, 720.0f);

    DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>("DissolveTimeline");

    AttachedGrenade = CreateDefaultSubobject<UStaticMeshComponent>("AttachedGrenade");
    AttachedGrenade->SetupAttachment(GetMesh(), FName("GrenadeSocket"));
    AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    AddHitBox(head, FName("head"));
    AddHitBox(pelvis, FName("pelvis"));
    AddHitBox(spine_02, FName("spine_02"));
    AddHitBox(spine_03, FName("spine_03"));
    AddHitBox(upperarm_l, FName("upperarm_l"));
    AddHitBox(upperarm_r, FName("upperarm_r"));
    AddHitBox(lowerarm_l, FName("lowerarm_l"));
    AddHitBox(lowerarm_r, FName("lowerarm_r"));
    AddHitBox(hand_l, FName("hand_l"));
    AddHitBox(hand_r, FName("hand_r"));
    AddHitBox(backpack, FName("backpack"));
    AddHitBox(thigh_l, FName("thigh_l"));
    AddHitBox(thigh_r, FName("thigh_r"));
    AddHitBox(calf_l, FName("calf_l"));
    AddHitBox(calf_r, FName("calf_r"));
    AddHitBox(foot_l, FName("foot_l"));
    AddHitBox(foot_r, FName("foot_r"));
    // blanket attached to backpack bone
    AddHitBox(foot_r, FName("blanket"), FName("backpack"));
}

void ABlasterCharacter::AddHitBox(UBoxComponent*& HitBox, const FName& BoxName)
{
    AddHitBox(HitBox, BoxName, BoxName);
}

void ABlasterCharacter::AddHitBox(UBoxComponent*& HitBox, const FName& BoxName, const FName& BoneName)
{
    HitBox = CreateDefaultSubobject<UBoxComponent>(BoxName);
    HitBox->SetupAttachment(GetMesh(), BoneName);
    HitBox->SetCollisionObjectType(ECC_HitBox);
    HitBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    HitBox->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
    HitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    HitCollisionBoxes.Add(BoxName, HitBox);
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
    DOREPLIFETIME(ABlasterCharacter, Health);
    DOREPLIFETIME(ABlasterCharacter, Shield);
    DOREPLIFETIME(ABlasterCharacter, bDisableGameplay);
}

void ABlasterCharacter::BeginPlay()
{
    Super::BeginPlay();
    
    BlasterPlayerController = Cast<ABlasterPlayerController>(Controller);

    if (BlasterPlayerController)
    {
        UpdateHUDHealth();
        UpdateHUDShield();
        UpdateHUDGrenades();
        SpawnDefaultWeapon();
        bHasInitialized = true;
    }
    
    if (HasAuthority())
    {
        OnTakeAnyDamage.AddDynamic(this, &ABlasterCharacter::OnReceiveDamage);
    }
    if (AttachedGrenade)
    {
        AttachedGrenade->SetVisibility(false, true);
    }
}

void ABlasterCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    RotateInPlace(DeltaTime);
    HideCameraIfCharacterClose();
    PollInit();
}

void ABlasterCharacter::SpawnDefaultWeapon()
{
    UWorld* World = GetWorld();
    if (!World || bEliminated || !DefaultWeaponClass) return;
    const ABlasterGameMode* BlasterGameMode = Cast<ABlasterGameMode>(World->GetAuthGameMode());
    if (!BlasterGameMode) return;
    if (AWeapon* StartingWeapon = World->SpawnActor<AWeapon>(DefaultWeaponClass))
    {
        CombatComponent->EquipWeapon(StartingWeapon);
        StartingWeapon->bDestroyWeapon = true;
    }
}

void ABlasterCharacter::RotateInPlace(float DeltaTime)
{
    if (bDisableGameplay)
    {
        bUseControllerRotationYaw = false;
        TurningInPlace = ETurningInPlace::ETIP_NotTurning;
        return;
    }
    if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
    {
        AimOffset(DeltaTime);
    }
    else
    {
        TimeSinceLastMovementReplication += DeltaTime;
        if (TimeSinceLastMovementReplication > 0.25f)
        {
            OnRep_ReplicatedMovement();
        }
        CalculateAO_Pitch();
    }
}

void ABlasterCharacter::Destroyed()
{
    Super::Destroyed();
    if (EliminationBotComponent)
    {
        EliminationBotComponent->DestroyComponent();
    }
    if (bDestroyWeaponOnDeath && CombatComponent && CombatComponent->EquippedWeapon)
    {
        CombatComponent->EquippedWeapon->Destroy();
    }
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
    PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ABlasterCharacter::ReloadButtonPressed);
    PlayerInputComponent->BindAction("ThrowGrenade", IE_Pressed, this, &ABlasterCharacter::GrenadeButtonPressed);

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
    if (BuffComponent)
    {
        BuffComponent->Character = this;
        BuffComponent->SetInitialSpeed(GetCharacterMovement()->MaxWalkSpeed, GetCharacterMovement()->MaxWalkSpeedCrouched);
        BuffComponent->SetInitialJumpSpeed(GetCharacterMovement()->JumpZVelocity);
    }
    if (LagCompensationComponent)
    {
        LagCompensationComponent->Character = this;
        if (Controller)
        {
            LagCompensationComponent->Controller = Cast<ABlasterPlayerController>(Controller);
        } 
    }
}

void ABlasterCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    if (!bHasInitialized)
    {
        BlasterPlayerController = Cast<ABlasterPlayerController>(NewController);
        UpdateHUDHealth();
        UpdateHUDShield();
        UpdateHUDGrenades();
        SpawnDefaultWeapon();
        bHasInitialized = true;
    }
}

void ABlasterCharacter::MoveForward(float Value)
{
    if (bDisableGameplay) return;
    if (!Controller || Value == 0.0f) return;
    const FRotator YawRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
    const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
    AddMovementInput(Direction, Value);
}

void ABlasterCharacter::MoveRight(float Value)
{
    if (bDisableGameplay) return;
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
    if (bDisableGameplay) return;

    if (CombatComponent)
    {
        ServerEquipButtonPressed();
        if (CombatComponent->ShouldSwapWeapons() && !OverlappingWeapon && !HasAuthority())
        {
            PlaySwapWeaponMontage();
            CombatComponent->CombatState = ECombatState::ECS_SwappingWeapons;
            bFinishSwapping = false;
        }
    }
}

void ABlasterCharacter::ServerEquipButtonPressed_Implementation()
{
    if (CombatComponent)
    {
        if (OverlappingWeapon)
        {
            CombatComponent->EquipWeapon(OverlappingWeapon);
        }
        else if (CombatComponent->ShouldSwapWeapons())
        {
            CombatComponent->SwapWeapons();
        }
    }
}

void ABlasterCharacter::CrouchButtonPressed()
{
    if (bDisableGameplay) return;
    bIsCrouched ? UnCrouch() : Crouch();
}

void ABlasterCharacter::ReloadButtonPressed()
{
    if (bDisableGameplay) return;
    if (CombatComponent)
    {
        CombatComponent->Reload();
    }
}

void ABlasterCharacter::AimButtonPressed()
{
    if (bDisableGameplay) return;
    if (CombatComponent)
    {
        CombatComponent->SetAiming(true);
    }
}

void ABlasterCharacter::AimButtonReleased()
{
    if (bDisableGameplay) return;
    if (CombatComponent)
    {
        CombatComponent->SetAiming(false);
    }
}

void ABlasterCharacter::CalculateAO_Pitch()
{
    AO_Pitch = GetBaseAimRotation().Pitch;
    if (AO_Pitch > 90.0f && !IsLocallyControlled())
    {
        // map Pitch form [270, 360) to [-90, 0)
        const FVector2D InRange(270.0f, 360.0f);
        const FVector2D OutRange(-90.0f, 0.0f);
        AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
    }
}

void ABlasterCharacter::AimOffset(float DeltaTime)
{
    if (!CombatComponent || !CombatComponent->EquippedWeapon) return;

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

    CalculateAO_Pitch();
}

void ABlasterCharacter::SimProxiesTurn()
{
    if (!CombatComponent || !CombatComponent->EquippedWeapon) return;
    bRotateRootBone = false;

    FVector Velocity = GetVelocity();
    Velocity.Z = 0.0f;
    if (!Velocity.IsZero())
    {
        TurningInPlace = ETurningInPlace::ETIP_NotTurning;
        return;
    }

    CalculateAO_Pitch();

    ProxyRotationLastFrame = ProxyRotation;
    ProxyRotation = GetActorRotation();
    ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;
    if (FMath::Abs(ProxyYaw) > TurnThreshold)
    {
        TurningInPlace = ProxyYaw > TurnThreshold ? ETurningInPlace::ETIP_Right : ETurningInPlace::ETIP_Left;
    }
    else
    {
        TurningInPlace = ETurningInPlace::ETIP_NotTurning;
    }
}

void ABlasterCharacter::Jump()
{
    if (bDisableGameplay) return;
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
    if (bDisableGameplay) return;
    if (CombatComponent)
    {
        CombatComponent->FireButtonPressed(true);
    }
}

void ABlasterCharacter::FireButtonReleased()
{
    if (bDisableGameplay) return;
    if (CombatComponent)
    {
        CombatComponent->FireButtonPressed(false);
    }
}

void ABlasterCharacter::GrenadeButtonPressed()
{
    if (!CombatComponent) return;
    CombatComponent->ThrowGrenade();
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

void ABlasterCharacter::SetHealth(float HealthToSet)
{
    Health = FMath::Clamp(HealthToSet, 0.0f, MaxHealth);

    if (HasAuthority())
    {
        UpdateHUDHealth();
    }
}

void ABlasterCharacter::SetShield(float ShieldToSet)
{
    Shield = FMath::Clamp(ShieldToSet, 0.0f, MaxShield);

    if (HasAuthority())
    {
        UpdateHUDShield();
    }
}

ECombatState ABlasterCharacter::GetCombatState() const
{
    if (!CombatComponent) return ECombatState::ECS_MAX;
    return CombatComponent->CombatState;
}

bool ABlasterCharacter::IsLocallyReloading() const
{
    return CombatComponent && CombatComponent->bLocallyReloading;
}

void ABlasterCharacter::PlayFireMontage(bool bAiming)
{
    if (!CombatComponent || !FireWeaponMontage || !CombatComponent->EquippedWeapon) return;

    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (!AnimInstance) return;
    AnimInstance->Montage_Play(FireWeaponMontage);
    const FName SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
    AnimInstance->Montage_JumpToSection(SectionName);
}

void ABlasterCharacter::PlayReloadMontage()
{
    if (!CombatComponent || !ReloadMontage || !CombatComponent->EquippedWeapon) return;

    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (!AnimInstance) return;
    AnimInstance->Montage_Play(ReloadMontage);
    FName SectionName;

    switch (CombatComponent->EquippedWeapon->GetWeaponType())
    {
        case EWeaponType::EWT_AssaultRifle:
            SectionName = FName("Rifle");
            break;
        case EWeaponType::EWT_RocketLauncher:
            SectionName = FName("RocketLauncher");
            break;
        case EWeaponType::EWT_Pistol:
            SectionName = FName("Pistol");
            break;
        case EWeaponType::EWT_SubmachineGun:
            SectionName = FName("Pistol");
            break;
        case EWeaponType::EWT_Shotgun:
            SectionName = FName("Shotgun");
            break;
        case EWeaponType::EWT_SniperRifle:
            SectionName = FName("SniperRifle");
            break;
        case EWeaponType::EWT_GrenadeLauncher:
            SectionName = FName("GrenadeLauncher");
            break;
    }

    AnimInstance->Montage_JumpToSection(SectionName);
}

void ABlasterCharacter::OnRep_ReplicatedMovement()
{
    Super::OnRep_ReplicatedMovement();
    SimProxiesTurn();
    TimeSinceLastMovementReplication = 0.0f;
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

void ABlasterCharacter::OnReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy,
    AActor* DamageCauser)
{
    if (bEliminated) return;

    
    const float DamageToShield = FMath::Min(Damage, Shield);
    const float DamageToHealth = Damage - DamageToShield;
    Shield -= DamageToShield;
    Health = FMath::Clamp(Health - DamageToHealth, 0.0f, MaxHealth);

    PlayHitReactMontage();
    UpdateHUDHealth();
    UpdateHUDShield();

    if (Health == 0.0f)
    {
        if (auto BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>())
        {
            const auto InstigatorController = Cast<ABlasterPlayerController>(InstigatedBy);
            BlasterGameMode->PlayerEliminated(this, BlasterPlayerController, InstigatorController);
        }
    }
}

void ABlasterCharacter::OnRep_Health(float LastHealth)
{
    UpdateHUDHealth();

    if (Health < LastHealth)
    {
        PlayHitReactMontage();
    }
    
}

void ABlasterCharacter::OnRep_Shield(float LastShield)
{
    UpdateHUDShield();

    if (Health < LastShield)
    {
        PlayHitReactMontage();
    }
}

void ABlasterCharacter::UpdateHUDHealth()
{
    BlasterPlayerController = !BlasterPlayerController ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
    if (BlasterPlayerController)
    {
        BlasterPlayerController->SetHUDHealth(Health, MaxHealth);
    }
}

void ABlasterCharacter::UpdateHUDShield()
{
    BlasterPlayerController = !BlasterPlayerController ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
    if (BlasterPlayerController)
    {
        BlasterPlayerController->SetHUDShield(Shield, MaxShield);
    }
}

void ABlasterCharacter::UpdateHUDGrenades()
{
    BlasterPlayerController = !BlasterPlayerController ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
    if (BlasterPlayerController)
    {
        BlasterPlayerController->SetHUDGrenades(CombatComponent->GetGrenades());
    }
}

void ABlasterCharacter::UpdateHUDAmmo()
{
    BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
    if (BlasterPlayerController && CombatComponent && CombatComponent->EquippedWeapon)
    {
        BlasterPlayerController->SetHUDCarriedAmmo(CombatComponent->CarriedAmmo);
        BlasterPlayerController->SetHUDWeaponAmmo(CombatComponent->EquippedWeapon->GetAmmo());
    }
}
#pragma region Elimination

void ABlasterCharacter::DropOrDestroyWeapon(AWeapon* Weapon)
{
    if (!Weapon) return;
    
    if (Weapon->bDestroyWeapon)
    {
        Weapon->Destroy();
    }
    else
    {
        Weapon->DropWeapon();
    }
}

void ABlasterCharacter::Eliminate()
{
    MulticastEliminate();
    GetWorldTimerManager().SetTimer(EliminationTimer, this, &ABlasterCharacter::EliminationTimerFinished, EliminationDelay);
    if (CombatComponent)
    {
        DropOrDestroyWeapon(CombatComponent->EquippedWeapon);
        DropOrDestroyWeapon(CombatComponent->SecondaryWeapon);
    }
}

void ABlasterCharacter::MulticastEliminate_Implementation()
{
    bEliminated = true;
    PlayEliminatedMontage();
    StartDissolve();
    StopMovementAndCollision();
    SpawnEliminationBot();

    if (BlasterPlayerController)
    {
        BlasterPlayerController->SetHUDWeaponAmmo(0);
    }
    if (IsLocallyControlled() && CombatComponent && CombatComponent->bIsAiming && CombatComponent->IsEquippedSniperRifle())
    {
        ShowSniperScopeWidget(false); 
    }
}

void ABlasterCharacter::EliminationTimerFinished()
{
    if (auto BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>())
    {
        BlasterGameMode->RequestRespawn(this, Controller);
    }
}

void ABlasterCharacter::PlayEliminatedMontage()
{
    if (!EliminatedMontage) return;
    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (!AnimInstance) return;
    AnimInstance->Montage_Play(EliminatedMontage);
}

void ABlasterCharacter::UpdateDissolveMaterial(float DissolveValue)
{
    if (DynamicDissolveMaterialInstance)
    {
        DynamicDissolveMaterialInstance->SetScalarParameterValue("Dissolve", DissolveValue);
    }
}

void ABlasterCharacter::StartDissolve()
{
    if (!DissolveCurve || !DissolveTimeline || !DissolveMaterialInstance) return;

    DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
    if (!DynamicDissolveMaterialInstance) return;
    GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);
    DynamicDissolveMaterialInstance->SetScalarParameterValue("Dissolve", 0.55f);
    DynamicDissolveMaterialInstance->SetScalarParameterValue("Glow", 200.0f);

    DissolveTrack.BindDynamic(this, &ABlasterCharacter::UpdateDissolveMaterial);
    DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
    DissolveTimeline->Play();
}

void ABlasterCharacter::StopMovementAndCollision()
{
    GetCharacterMovement()->DisableMovement();
    GetCharacterMovement()->StopMovementImmediately();
    bDisableGameplay = true;
    if (CombatComponent)
    {
        CombatComponent->FireButtonPressed(false);
    }
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

#pragma endregion

void ABlasterCharacter::PlayThrowGrenadeMontage()
{
    if (!ThrowGrenadeMontage) return;

    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (!AnimInstance) return;
    AnimInstance->Montage_Play(ThrowGrenadeMontage);
}

void ABlasterCharacter::PlaySwapWeaponMontage()
{
    if (!SwapWeaponMontage) return;

    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (!AnimInstance) return;
    AnimInstance->Montage_Play(SwapWeaponMontage);
}

void ABlasterCharacter::SpawnEliminationBot()
{
    if (!EliminationBotEffect) return;
    const FVector BotSpawnPoint(GetActorLocation() + EliminationBotOffset);
    EliminationBotComponent = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), EliminationBotEffect, BotSpawnPoint, GetActorRotation());

    if (!EliminationSound) return;
    UGameplayStatics::PlaySoundAtLocation(GetWorld(), EliminationSound, BotSpawnPoint);
}

void ABlasterCharacter::PollInit()
{
    if (!BlasterPlayerState)
    {
        BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
        if (BlasterPlayerState)
        {
            BlasterPlayerState->AddToScore(0.0f);
            BlasterPlayerState->AddToDefeats(0);
        }
    }
}
