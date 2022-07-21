#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PacketHandlers/StatelessConnectHandlerComponent.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
    EWS_Initial UMETA(DisplayName = "Initial State"),
    EWS_Equipped UMETA(DisplayName = "Equipped"),
    EWS_Dropped UMETA(DisplayName = "Dropped"),

    EWS_MAX UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class BLASTER_API AWeapon : public AActor
{
    GENERATED_BODY()

public:
    AWeapon();
    void ShowPickupWidget(const bool bShowWidget) const;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void Fire(const FVector& HitTarget);
    
protected:
    UFUNCTION()
    virtual void BeginPlay() override;

    UFUNCTION()
    virtual void OnSphereOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult
        );

    UFUNCTION()
    void OnSphereEndOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex
        );
    
private:
    UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
    USkeletalMeshComponent* WeaponMesh;

    UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
    class USphereComponent* AreaSphere;

    UPROPERTY(ReplicatedUsing = "OnRep_WeaponState", VisibleAnywhere, Category = "Weapon Properties")
    EWeaponState WeaponState;

    UFUNCTION()
    void OnRep_WeaponState();

    UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
    class UWidgetComponent* PickupWidget;

    UPROPERTY(EditAnywhere, Category = "Weapon Properties")
    class UAnimationAsset* FireAnimation;

    UPROPERTY(EditAnywhere, Category = "Weapon Properties")
    TSubclassOf<class ACasing> CasingClass;

    UPROPERTY(EditAnywhere, Category = "Weapon Properties")
    FVector2D ShellOffset {-15.0f, 15.0f};
public:
    void SetWeaponState(EWeaponState State);
    FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
    FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
    
};
