// Blaster Game. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blaster/BlasterTypes/CombatState.h"
#include "Blaster/BlasterTypes/TurningInPlace.h"
#include "Blaster/Interfaces/InteractWithCrosshairsInterface.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Character.h"
#include "BlasterCharacter.generated.h"

UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter, public IInteractWithCrosshairsInterface
{
    GENERATED_BODY()

public:
    ABlasterCharacter();
    virtual void Tick(float DeltaTime) override;
    virtual void Destroyed() override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    virtual void PostInitializeComponents() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    void PlayFireMontage(bool bAiming);
    void PlayReloadMontage();
    virtual void OnRep_ReplicatedMovement() override;
    void Eliminate();
    UFUNCTION(NetMulticast, Reliable)
    void MulticastEliminate();
    void PlayEliminatedMontage();

    UPROPERTY(Replicated)
    bool bDisableGameplay = false;
protected:
    virtual void BeginPlay() override;
    void RotateInPlace(float DeltaTime);

    void MoveForward(float Value);
    void MoveRight(float Value);
    void Turn(float Value);
    void LookUp(float Value);
    void EquipButtonPressed();
    void CrouchButtonPressed();
    void ReloadButtonPressed();
    void AimButtonPressed();
    void AimButtonReleased();
    void CalculateAO_Pitch();
    void AimOffset(float DeltaTime);
    void SimProxiesTurn();
    virtual void Jump() override;
    void FireButtonPressed();
    void FireButtonReleased();
    void PlayHitReactMontage() const;
    void UpdateHUDHealth();
    UFUNCTION()
    void OnReceiveDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser );
    void StopMovementAndCollision();
    void SpawnEliminationBot();

    void PollInit();
    
private:
    UPROPERTY(VisibleAnywhere, Category = "Camera")
    class USpringArmComponent* CameraBoom;

    UPROPERTY(VisibleAnywhere, Category = "Camera")
    class UCameraComponent* FollowCamera;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    class UWidgetComponent* OverheadWidget;
    
    UPROPERTY(ReplicatedUsing = "OnRep_OverlappingWeapon")
    class AWeapon* OverlappingWeapon;

    UFUNCTION()
    void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    class UCombatComponent* CombatComponent;

    UFUNCTION(Server, Reliable)
    void ServerEquipButtonPressed();

    float AO_Yaw;
    float InterpAO_Yaw;
    float AO_Pitch;
    FRotator StartingAimRotation;

    ETurningInPlace TurningInPlace;
    void TurnInPlace(float DeltaTime);

#pragma region AnimationMontages   

    UPROPERTY(EditAnywhere, Category = "Montage")
    class UAnimMontage* FireWeaponMontage;

    UPROPERTY(EditAnywhere, Category = "Montage")
    UAnimMontage* ReloadMontage;

    UPROPERTY(EditAnywhere, Category = "Montage")
    UAnimMontage* HitReactMontage;

    UPROPERTY(EditAnywhere, Category = "Montage")
    UAnimMontage* EliminatedMontage;

#pragma endregion 
    
    void HideCameraIfCharacterClose();

    UPROPERTY(EditAnywhere, Category = "Camera")
    float CameraThreshold = 200.0f;

    bool bRotateRootBone;
    float TurnThreshold = 0.5f;
    FRotator ProxyRotationLastFrame;
    FRotator ProxyRotation;
    float ProxyYaw;
    float TimeSinceLastMovementReplication;

    UPROPERTY(EditAnywhere, Category = "Player stats")
    float MaxHealth = 100.0f;

    UPROPERTY(VisibleAnywhere, ReplicatedUsing = "OnRep_Health", Category = "Player stats")
    float Health;

    UPROPERTY(VisibleAnywhere, Category = "Elimination")
    bool bEliminated = false;

    FTimerHandle EliminationTimer;

    UPROPERTY(EditDefaultsOnly, Category = "Elimination")
    float EliminationDelay = 3.0f;
    
    void EliminationTimerFinished();
    
    UFUNCTION()
    void OnRep_Health();

#pragma region DissolveEffect
    
    UPROPERTY(VisibleAnywhere, Category = "Elimination")
    UTimelineComponent* DissolveTimeline;
    
    FOnTimelineFloat DissolveTrack;

    UPROPERTY(EditAnywhere, Category = "Elimination")
    UCurveFloat* DissolveCurve;

    UPROPERTY(EditAnywhere, Category = "Elimination")
    UMaterialInstance* DissolveMaterialInstance;

    UPROPERTY(VisibleAnywhere, Category = "Elimination")
    UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;

    UFUNCTION()
    void UpdateDissolveMaterial(float DissolveValue);

    void StartDissolve();
#pragma endregion 

#pragma region EliminationBot

    UPROPERTY(EditAnywhere, Category = "Elimination")
    UParticleSystem* EliminationBotEffect;

    UPROPERTY(VisibleAnywhere, Category = "Elimination")
    UParticleSystemComponent* EliminationBotComponent;

    UPROPERTY(EditAnywhere, Category = "Elimination")
    FVector EliminationBotOffset{0.0f, 0.0f, 200.0f};

    UPROPERTY(EditAnywhere, Category = "Elimination")
    class USoundCue* EliminationSound;

#pragma endregion 
    
    UPROPERTY(VisibleAnywhere)
    class ABlasterPlayerController* BlasterPlayerController;

    UPROPERTY(VisibleAnywhere)
    class ABlasterPlayerState* BlasterPlayerState;

    UPROPERTY(EditDefaultsOnly, Category = "Elimination")
    bool bDestroyWeaponOnDeath = false;
public:
    void SetOverlappingWeapon(AWeapon* Weapon);
    bool IsWeaponEquipped() const;
    bool IsAiming() const;
    FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
    FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
    AWeapon* GetEquippedWeapon() const;
    FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
    FVector GetHitTarget() const;
    FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
    FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }
    FORCEINLINE bool IsEliminated() const { return bEliminated; }
    FORCEINLINE float GetHealth() const { return Health; }
    FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
    ECombatState GetCombatState() const;
    FORCEINLINE UCombatComponent* GetCombatComponent() const { return CombatComponent; }
};
