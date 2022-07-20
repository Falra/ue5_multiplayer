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
private:
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* CasingMesh;

    UPROPERTY(EditAnywhere)
    float ShellEjectionImpulse = 10.0f;
    
protected:
    virtual void BeginPlay() override;

};
