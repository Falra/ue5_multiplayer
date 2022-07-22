// Blaster Game. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blaster/BlasterTypes/TurningInPlace.h"
#include "Blaster/Interfaces/InteractWithCrosshairsInterface.h"
#include "GameFramework/Character.h"
#include "BlasterCharacter.generated.h"

UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter, public IInteractWithCrosshairsInterface
{
    GENERATED_BODY()

public:
    ABlasterCharacter();
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    virtual void PostInitializeComponents() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    void PlayFireMontage(bool bAiming);
    
protected:
    virtual void BeginPlay() override;

    void MoveForward(float Value);
    void MoveRight(float Value);
    void Turn(float Value);
    void LookUp(float Value);
    void EquipButtonPressed();
    void CrouchButtonPressed();
    void AimButtonPressed();
    void AimButtonReleased();
    void AimOffset(float DeltaTime);
    virtual void Jump() override;
    void FireButtonPressed();
    void FireButtonReleased();

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

    UPROPERTY(VisibleAnywhere)
    class UCombatComponent* CombatComponent;

    UFUNCTION(Server, Reliable)
    void ServerEquipButtonPressed();

    float AO_Yaw;
    float InterpAO_Yaw;
    float AO_Pitch;
    FRotator StartingAimRotation;

    ETurningInPlace TurningInPlace;
    void TurnInPlace(float DeltaTime);

    UPROPERTY(EditAnywhere, Category = "Montage")
    class UAnimMontage* FireWeaponMontage;

    void HideCameraIfCharacterClose();

    UPROPERTY(EditAnywhere, Category = "Camera")
    float CameraThreshold = 200.0f;
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
};
