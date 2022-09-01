// Blaster Game. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"

USTRUCT(BlueprintType)
struct FBoxInformation
{
    GENERATED_BODY()

    UPROPERTY()
    FVector Location;

    UPROPERTY()
    FRotator Rotation;

    UPROPERTY()
    FVector BoxExtent;
};

USTRUCT(BlueprintType)
struct FFramePackage
{
    GENERATED_BODY()

    UPROPERTY()
    float Time = 0.0f;

    UPROPERTY()
    TMap<FName, FBoxInformation> HitBoxInfo;

    UPROPERTY()
    class ABlasterCharacter* Character = nullptr;
};

USTRUCT(BlueprintType)
struct FShotgunServerSideRewindResult
{
    GENERATED_BODY()

    UPROPERTY()
    TMap<ABlasterCharacter*, uint32> HeadShots;

    UPROPERTY()
    TMap<ABlasterCharacter*, uint32> BodyShots;
};

USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
    GENERATED_BODY()

    UPROPERTY()
    bool bHitConfirmed;

    UPROPERTY()
    bool bHeadShot;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BLASTER_API ULagCompensationComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    ULagCompensationComponent();
    friend class ABlasterCharacter;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    void ShowFramePackage(const FFramePackage& Package, const FColor& Color) const;
    FServerSideRewindResult ServerSideRewind(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart,
        const FVector_NetQuantize& HitLocation, float HitTime) const;
    FServerSideRewindResult ProjectileServerSideRewind(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart,
        const FVector_NetQuantize100& InitialVelocity, float HitTime) const;
    FShotgunServerSideRewindResult ShotgunServerSideRewind(const TArray<ABlasterCharacter*>& HitCharacters,
        const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime) const;
    UFUNCTION(Server, Reliable)
    void ServerScoreRequest(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation,
        float HitTime, class AWeapon* DamageCauser);
    UFUNCTION(Server, Reliable)
    void ProjectileServerScoreRequest(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart,
        const FVector_NetQuantize100& InitialVelocity, float HitTime, class AWeapon* DamageCauser);
    UFUNCTION(Server, Reliable)
    void ShotgunServerScoreRequest(const TArray<ABlasterCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart,
        const TArray<FVector_NetQuantize>& HitLocations, float HitTime, class AWeapon* DamageCauser);
protected:
    virtual void BeginPlay() override;
    void SaveFramePackage(FFramePackage& Package);
    static FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime);
    FServerSideRewindResult ConfirmHit(const FFramePackage& Package, ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart,
        const FVector_NetQuantize& HitLocation) const;
    FServerSideRewindResult ProjectileConfirmHit(const FFramePackage& Package, ABlasterCharacter* HitCharacter,
        const FVector_NetQuantize& TraceStart,
        const FVector_NetQuantize100& InitialVelocity, float HitTime) const;
    FShotgunServerSideRewindResult ShotgunConfirmHit(const TArray<FFramePackage>& Packages, const FVector_NetQuantize& TraceStart,
        const TArray<FVector_NetQuantize>& HitLocations) const;
    static void CacheBoxPosition(ABlasterCharacter* HitCharacter, FFramePackage& OutFramePackage);
    static void MoveBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package);
    static void ResetHitBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package);
    static FFramePackage GetFrameToCheck(ABlasterCharacter* HitCharacter, float HitTime);
private:
    UPROPERTY()
    ABlasterCharacter* Character;

    UPROPERTY()
    class ABlasterPlayerController* Controller;

    TDoubleLinkedList<FFramePackage> FrameHistory;

    UPROPERTY(EditAnywhere)
    float MaxRecordTime = 4.0f;

    void SaveFramePackageOnTick();
    void DrawDebugHitBox(TWeakObjectPtr<UPrimitiveComponent> HitComponent, const FColor& DrawColor) const;
};
