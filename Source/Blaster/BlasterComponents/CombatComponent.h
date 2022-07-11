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
private:
    UPROPERTY()
    class ABlasterCharacter* Character;
    
    UPROPERTY(ReplicatedUsing = "OnRep_EquippedWeapon")
    AWeapon* EquippedWeapon;

    UPROPERTY(Replicated)
    bool bIsAiming;
    
protected:
    virtual void BeginPlay() override;
    void SetAiming(bool IsAiming);

    UFUNCTION(Server, Reliable)
    void ServerSetAiming(bool IsAiming);

    UFUNCTION()
    void OnRep_EquippedWeapon();

};
