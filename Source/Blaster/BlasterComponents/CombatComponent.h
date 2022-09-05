// Blaster Game. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blaster/BlasterTypes/CombatState.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BLASTER_API UCombatComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombatComponent();
    friend class ABlasterCharacter;
    void EquipWeapon(class AWeapon* WeaponToEquip);
    void SwapWeapons();
    void Reload();

    UFUNCTION(BlueprintCallable)
    void FinishReloading();

    UFUNCTION(BlueprintCallable)
    void FinishSwapWeapons();

    UFUNCTION(BlueprintCallable)
    void FinishSwapAttachWeapons();
    
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    void FireButtonPressed(bool bPressed);
    UFUNCTION(BlueprintCallable)
    void ShotgunShellReload();
    bool IsEquippedSniperRifle() const { return EquippedWeapon && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle; };
    bool IsEquippedShotgun() const { return EquippedWeapon && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun; };
    void JumpToShotgunEnd() const;
    UFUNCTION(BlueprintCallable)
    void ThrowGrenadeFinished();
    UFUNCTION(BlueprintCallable)
    void LaunchGrenade();
    UFUNCTION(Server, Reliable)
    void ServerLaunchGrenade(const FVector_NetQuantize& Target);
    void PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount);
    bool bLocallyReloading = false;
protected:
    virtual void BeginPlay() override;
    void SetAiming(bool IsAiming);

    UFUNCTION(Server, Reliable)
    void ServerSetAiming(bool IsAiming);

    UFUNCTION()
    void OnRep_EquippedWeapon();

    UFUNCTION()
    void OnRep_SecondaryWeapon();
    
    void Fire();

    void FireProjectileWeapon();
    void FireHitScanWeapon();
    void FireShotgun();
    
    UFUNCTION(Server, Reliable)
    void ServerFire(const FVector_NetQuantize& TraceHitTarget, float FireDelay = 0.0f);

    UFUNCTION(NetMulticast, Reliable)
    void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

    void LocalFire(const FVector_NetQuantize& TraceHitTarget);

    UFUNCTION(Server, Reliable)
    void ServerShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets, float FireDelay = 0.0f);

    UFUNCTION(NetMulticast, Reliable)
    void MulticastShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

    void LocalShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);
    
    void TraceUnderCrosshairs(FHitResult& TraceHitResult);
    void ReloadEmptyWeapon();
    void UpdateCarriedAmmo();
    void DropEquippedWeapon();
    void AttachActorToRightHand(AActor* ActorToAttach);
    void AttachActorToLeftHand(AActor* ActorToAttach);
    void AttachActorToBackpack(AActor* ActorToAttach);
    void PlayEquipEffects(AWeapon* WeaponToEquip) const;
    void SetWeaponStateAndAttach();

    void SetHUDCrosshairs(float DeltaTime);

    UFUNCTION(Server, Reliable)
    void ServerReload();

    void HandleReload();
    int32 AmountToReload();

    void ThrowGrenade();

    UFUNCTION(Server, Reliable)
    void ServerThrowGrenade();
    void ShowAttachedGrenade(bool bShow);

    UPROPERTY(EditAnywhere)
    TSubclassOf<class AProjectile> GrenadeClass;

    void EquipPrimaryWeapon(AWeapon* WeaponToEquip);
    void EquipSecondaryWeapon(AWeapon* WeaponToEquip);
private:
    UPROPERTY()
    class ABlasterCharacter* Character;

    UPROPERTY()
    class ABlasterPlayerController* Controller;

    UPROPERTY()
    class ABlasterHUD* HUD; 

    UPROPERTY(ReplicatedUsing = "OnRep_EquippedWeapon")
    AWeapon* EquippedWeapon;

    UPROPERTY(ReplicatedUsing = "OnRep_SecondaryWeapon")
    AWeapon* SecondaryWeapon;

    UPROPERTY(ReplicatedUsing = "OnRep_Aiming")
    bool bIsAiming = false;

    bool bAimButtonPressed = false;
    
    UFUNCTION()
    void OnRep_Aiming();
    
    UPROPERTY(EditAnywhere, Category = "Movement")
    float BaseWalkSpeed = 600.0f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float AimWalkSpeed = 450.0f;

    bool bFireButtonPressed;

    FHUDPackage HUDPackage;

    bool bIsAimingPlayer;

#pragma region Crosshair
    float CrosshairVelocityFactor;
    float CrosshairInAirFactor;
    float CrosshairAimFactor;
    float CrosshairShootingFactor;
    float CrosshairAimPlayerFactor;

    FVector HitTarget;
#pragma endregion

#pragma region FOV
    float DefaultFOV;

    float CurrentFOV;

    UPROPERTY(EditAnywhere, Category = "Zoom FOV")
    float ZoomedFOV = 30.0f;

    UPROPERTY(EditAnywhere, Category = "Zoom FOV")
    float ZoomedInterpSpeed = 20.0f;

    void InterpFOV(float DeltaTime);
#pragma endregion

#pragma region AutoFire
    FTimerHandle FireTimer;
    
    bool bCanFire = true;
    
    void StartFireTimer();
    void FireTimerFinished();

    bool CanFire() const;

    UPROPERTY(ReplicatedUsing = "OnRep_CarriedAmmo")
    int32 CarriedAmmo;

    UFUNCTION()
    void OnRep_CarriedAmmo();

    TMap<EWeaponType, int32> CarriedAmmoMap;

    UPROPERTY(EditAnywhere, Category = "Weapon")
    int32 MaxCarriedAmmo = 500;

#pragma endregion

#pragma region InitialAmmo

    UPROPERTY(EditAnywhere, Category = "Weapon")
    int32 StartingARAmmo = 30;

    UPROPERTY(EditAnywhere, Category = "Weapon")
    int32 StartingRocketAmmo = 8;

    UPROPERTY(EditAnywhere, Category = "Weapon")
    int32 StartingPistolAmmo = 20;

    UPROPERTY(EditAnywhere, Category = "Weapon")
    int32 StartingSMGAmmo = 60;

    UPROPERTY(EditAnywhere, Category = "Weapon")
    int32 StartingShotgunAmmo = 10;

    UPROPERTY(EditAnywhere, Category = "Weapon")
    int32 StartingSniperAmmo = 4;

    UPROPERTY(EditAnywhere, Category = "Weapon")
    int32 StartingGrenadeAmmo = 4;
    
    void InitializeCarriedAmmo();

#pragma endregion

    UPROPERTY(ReplicatedUsing = "OnRep_CombatState")
    ECombatState CombatState = ECombatState::ECS_Unoccupied;

    UFUNCTION()
    void OnRep_CombatState();

    void UpdateAmmoValues();
    void UpdateShotgunAmmoValues();

    UPROPERTY(ReplicatedUsing = "OnRep_Grenades")
    int32 Grenades = 4;

    UFUNCTION()
    void OnRep_Grenades();

    void UpdateHUDGrenades();
    
    UPROPERTY(EditAnywhere)
    int32 MaxGrenades = 4;

public:
    FORCEINLINE int32 GetGrenades() const { return Grenades; }
    FORCEINLINE AWeapon* GetEquippedWeapon() const { return EquippedWeapon; }
    bool ShouldSwapWeapons() const;
};
