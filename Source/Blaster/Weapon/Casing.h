// Blaster Game. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Casing.generated.h"

UCLASS()
class BLASTER_API ACasing : public AActor
{
    GENERATED_BODY()

public:
    ACasing();
    
protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

private:
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* CasingMesh;

    UPROPERTY(EditAnywhere)
    float ShellEjectionImpulse = 10.0f;

    UPROPERTY(EditAnywhere)
    class USoundCue* ShellSound;
};
