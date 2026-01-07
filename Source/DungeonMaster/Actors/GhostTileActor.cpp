// Fill out your copyright notice in the Description page of Project Settings.
#include "GhostTileActor.h"


AGhostTileActor::AGhostTileActor()
{
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision); // Raycast'i engellemesin
}

void AGhostTileActor::SetTileMesh(UStaticMesh* Mesh)
{
	if(MeshComp) MeshComp->SetStaticMesh(Mesh);
}

void AGhostTileActor::SetValidity(bool bIsValid)
{
	if(bIsValid && ValidMaterial) MeshComp->SetMaterial(0, ValidMaterial);
	else if(!bIsValid && InvalidMaterial) MeshComp->SetMaterial(0, InvalidMaterial);
}

