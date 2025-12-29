#pragma once

#include "CoreMinimal.h"
#include "Info/Types.h"
#include "GridVisualizer.generated.h"

UCLASS()
class DUNGEONMASTER_API AGridVisualizer : public AActor
{
	GENERATED_BODY()
    
public:
	virtual void BeginPlay() override;

protected:
	// Subsystem'den gelen sinyali yakalar
	UFUNCTION()
	void HandleGridChanged(const FGridCoordinate& Coord);

	// Grid'i oluşturmak için mesh kütüphanesi (Data Asset kullanacağız ileride)
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	UDataAsset* TileSetData; 

	// Basit zeminler için
	UPROPERTY(VisibleAnywhere)
	UInstancedStaticMeshComponent* FloorISMC;
};