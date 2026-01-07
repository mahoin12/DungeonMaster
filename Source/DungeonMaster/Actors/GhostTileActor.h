// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GhostTileActor.generated.h"

UCLASS()
class DUNGEONMASTER_API AGhostTileActor : public AActor
{
	GENERATED_BODY()
public:
	AGhostTileActor();
    
	void SetTileMesh(UStaticMesh* Mesh);
	void SetValidity(bool bIsValid); // Yeşil veya Kırmızı materyal

protected:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComp;

	UPROPERTY(EditDefaultsOnly)
	UMaterialInterface* ValidMaterial; // Yarı saydam Yeşil

	UPROPERTY(EditDefaultsOnly)
	UMaterialInterface* InvalidMaterial; // Yarı saydam Kırmızı
};
