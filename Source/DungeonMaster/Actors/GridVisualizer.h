#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DungeonMaster/Info/Types.h"
#include "DungeonMaster/Info/DungeonTileSet.h" 
#include "GridVisualizer.generated.h"

UCLASS()
class DUNGEONMASTER_API AGridVisualizer : public AActor
{
	GENERATED_BODY()
	
public:	
	AGridVisualizer();

	// Grid'i oluşturur (Collision kutularını dizer)
	void GenerateGridCollision(int32 Width, int32 Height);

	// Grid'i Görünür/Görünmez yapar (Sürükleme başlayınca çağıracağız)
	void SetPlacementGridActive(bool bActive);

	// Instance Index'ten Koordinat bulur (Matematiği burada encapsulate ediyoruz)
	FGridCoordinate GetCoordinateFromIndex(int32 InstanceIndex) const;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleGridChanged(FGridCoordinate Coord, FCellData NewData);

	UPROPERTY(EditDefaultsOnly, Category = "Config")
	float GridCellSize = 100.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Config")
	UDungeonTileSet* TileSetData;

	// YENİ: Hem görsel hem collision için kullanılacak Instance Mesh
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UInstancedStaticMeshComponent* GridCollisionISMC;

private:
	UPROPERTY()
	TMap<FName, UInstancedStaticMeshComponent*> MeshComponents;

	UInstancedStaticMeshComponent* GetOrCreateISMC(FName TileID);

	// Genişliği saklamamız lazım ki Index -> (X,Y) dönüşümü yapabilelim
	int32 CachedGridWidth = 0;
};