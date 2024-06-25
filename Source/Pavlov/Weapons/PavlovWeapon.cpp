// Fill out your copyright notice in the Description page of Project Settings.

#include "PavlovWeapon.h"

// Sets default values
APavlovWeapon::APavlovWeapon()
{
	PrimaryActorTick.bCanEverTick = true;
}

void APavlovWeapon::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void APavlovWeapon::PostNetInit()
{
	Super::PostNetInit();
	OnPostNetInit();
}
