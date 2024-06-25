// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PavlovWeapon.generated.h"

UCLASS()
class PAVLOV_API APavlovWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APavlovWeapon();

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void PostNetInit() override;

	// Cannot override PostNetInit directly because we cannot call super() in blueprints
	UFUNCTION(BlueprintImplementableEvent)
	void OnPostNetInit();
};
