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
    
private:
    UPROPERTY()
    class ABlasterCharacter* Character;
    
    UPROPERTY()
    AWeapon* EquippedWeapon;
    
protected:
    virtual void BeginPlay() override;

};
