// Blaster Game. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FlagZone.generated.h"

UCLASS()
class BLASTER_API AFlagZone : public AActor
{
    GENERATED_BODY()

public:
    AFlagZone();

protected:
    virtual void BeginPlay() override;
};
