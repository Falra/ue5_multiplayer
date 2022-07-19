// Blaster Game. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
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
private:
    UPROPERTY()
    class ABlasterCharacter* Character;

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

    FVector HitTarget;
protected:
    virtual void BeginPlay() override;
    void SetAiming(bool IsAiming);

    UFUNCTION(Server, Reliable)
    void ServerSetAiming(bool IsAiming);

    UFUNCTION()
    void OnRep_EquippedWeapon();

    void FireButtonPressed(bool bPressed);

    UFUNCTION(Server, Reliable)
    void ServerFire();

    UFUNCTION(NetMulticast, Reliable)
    void MulticastFire();

    void TraceUnderCrosshairs(FHitResult& TraceHitResult);
};
