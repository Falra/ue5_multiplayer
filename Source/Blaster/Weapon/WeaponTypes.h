﻿#pragma once

#define TRACE_LENGTH 80000.0f

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
    EWT_AssaultRifle UMETA(DisplayName = "Assault Rifle"),
    EWT_RocketLauncher UMETA(DisplayName = "Rocket Launcher"),
    EWT_Pistol UMETA(DisplayName = "Pistol"),
    EWT_SubmachineGun UMETA(DisplayName = "Submachine Gun"),
    EWT_Shotgun UMETA(DisplayName = "Shotgun"),
    
    EWT_MAX UMETA(DisplayName = "DefaultMAX")
};