#pragma once

#include "CoreMinimal.h"
#include "WeaponTypes.h"
#include "GameFramework/Actor.h"
#include "PacketHandlers/StatelessConnectHandlerComponent.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
    EWS_Initial UMETA(DisplayName = "Initial State"),
    EWS_Equipped UMETA(DisplayName = "Equipped"),
    EWS_EquippedSecondary UMETA(DisplayName = "Equipped Secondary"),
    EWS_Dropped UMETA(DisplayName = "Dropped"),

    EWS_MAX UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EFireType : uint8
{
    EFT_HitScanWeapon UMETA(DisplayName = "Hit Scan Weapon"),
    EFT_ProjectileWeapon UMETA(DisplayName = "Projectile Weapon"),
    EFT_Shotgun UMETA(DisplayName = "Shotgun Weapon"),
    
    EFT_MAX UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class BLASTER_API AWeapon : public AActor
{
    GENERATED_BODY()

public:
    AWeapon();
    void ShowPickupWidget(const bool bShowWidget) const;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void OnRep_Owner() override;
    virtual void Fire(const FVector& HitTarget);
    virtual void DropWeapon();
    virtual void AddAmmo(int32 AmmoToAdd);
    void EnableCustomDepth(bool bEnable);
    bool bDestroyWeapon = false;

    UPROPERTY(EditAnywhere)
    EFireType FireType;

    UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
    bool bUseScatter = false;

    FVector TraceEndWithScatter(const FVector& HitTarget) const;
protected:
    UFUNCTION()
    virtual void BeginPlay() override;

    virtual void OnSetWeaponState();
    virtual void OnEquipped();
    virtual void OnDropped();
    virtual void OnEquippedSecondary();
    
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

    virtual void SetWeaponMeshState(bool bIsEnabled, bool bIsSecondary = false);

    UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
    float DistanceToSphere = 800.0f;
    
    UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
    float SphereRadius = 75.0f;
    
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

    UPROPERTY(EditAnywhere, ReplicatedUsing= "OnRep_Ammo", Category = "Weapon Properties")
    int32 Ammo;

    UFUNCTION()
    void OnRep_Ammo();
    void CheckUpdateController();

    void SpendRound();

    UPROPERTY(EditAnywhere, Category = "Weapon Properties")
    int32 MagCapacity;

    UPROPERTY(EditAnywhere, Category = "Weapon Properties")
    bool bHasStrap = false;
    
    UPROPERTY(VisibleAnywhere)
    class ABlasterCharacter* BlasterOwnerCharacter;

    UPROPERTY(VisibleAnywhere)
    class ABlasterPlayerController* BlasterOwnerController;

    UPROPERTY(EditAnywhere)
    EWeaponType WeaponType;
    
public:

#pragma region Crosshair
    
    UPROPERTY(EditAnywhere, Category = "Crosshairs")
    UTexture2D* CrosshairsCenter;

    UPROPERTY(EditAnywhere, Category = "Crosshairs")
    UTexture2D* CrosshairsLeft;

    UPROPERTY(EditAnywhere, Category = "Crosshairs")
    UTexture2D* CrosshairsRight;

    UPROPERTY(EditAnywhere, Category = "Crosshairs")
    UTexture2D* CrosshairsTop;

    UPROPERTY(EditAnywhere, Category = "Crosshairs")
    UTexture2D* CrosshairsBottom;

    UPROPERTY(EditAnywhere, Category = "Weapon Properties")
    float FireDelay = 0.15f;
    UPROPERTY(EditAnywhere, Category = "Weapon Properties")
    bool bAutomatic = true;

#pragma endregion

#pragma region FOV

    UPROPERTY(EditAnywhere, Category = "Zoom FOV")
    float ZoomedFOV = 30.0f;

    UPROPERTY(EditAnywhere, Category = "Zoom FOV")
    float ZoomedInterpSpeed = 20.0f;

#pragma endregion

    UPROPERTY(EditAnywhere)
    class USoundCue* EquipSound;
    
    void SetWeaponState(EWeaponState State);
    void ShowWeaponAmmo();
    FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
    FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
    FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
    FORCEINLINE float GetZoomedInterpSpeed() const { return ZoomedInterpSpeed; }
    bool IsEmpty() const;
    bool IsFull() const;
    FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
    FORCEINLINE int32 GetAmmo() const { return Ammo; }
    FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; }
};
