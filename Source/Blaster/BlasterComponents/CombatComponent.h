// Blaster Game. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blaster/HUD/BlasterHUD.h"
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
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
protected:
    virtual void BeginPlay() override;
    void SetAiming(bool IsAiming);

    UFUNCTION(Server, Reliable)
    void ServerSetAiming(bool IsAiming);

    UFUNCTION()
    void OnRep_EquippedWeapon();
    void Fire();

    void FireButtonPressed(bool bPressed);

    UFUNCTION(Server, Reliable)
    void ServerFire(const FVector_NetQuantize& TraceHitTarget);

    UFUNCTION(NetMulticast, Reliable)
    void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

    void TraceUnderCrosshairs(FHitResult& TraceHitResult);

    void SetHUDCrosshairs(float DeltaTime);
    
private:
    UPROPERTY()
    class ABlasterCharacter* Character;

    UPROPERTY()
    class ABlasterPlayerController* Controller;

    UPROPERTY()
    class ABlasterHUD* HUD; 

    UPROPERTY(ReplicatedUsing = "OnRep_EquippedWeapon")
    AWeapon* EquippedWeapon;

    UPROPERTY(Replicated)
    bool bIsAiming;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float BaseWalkSpeed = 600.0f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float AimWalkSpeed = 450.0f;

    bool bFireButtonPressed;

    UPROPERTY(EditAnywhere, Category = "Weapon")
    float CrosshairTraceLenght = 80000.0f;

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

    UPROPERTY(VisibleAnywhere, Category = "Weapon")
    float FireDelay = 0.15f;
    UPROPERTY(VisibleAnywhere, Category = "Weapon")
    bool bAutomatic = false;

    bool bCanFire = true;
    
    void StartFireTimer();
    void FireTimerFinished();
#pragma endregion

};
