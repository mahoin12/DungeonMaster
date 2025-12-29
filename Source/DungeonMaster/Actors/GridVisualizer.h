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

protected:
	virtual void BeginPlay() override;

	// Subsystem'den gelen sinyali işler
	UFUNCTION()
	void HandleGridChanged(FGridCoordinate Coord, FCellData NewData);

	// Grid Tile boyutumuz (Örn: 100x100)
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	float GridCellSize = 100.0f;

	// Hangi ID'nin hangi Mesh olduğunu tutan DataAsset
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	UDungeonTileSet* TileSetData;

private:
	// Hangi Mesh için hangi ISMC'yi oluşturduğumuzu tutan harita.
	// Key: TileID (veya Mesh Pointer'ı), Value: O mesh'i çizen ISMC
	UPROPERTY()
	TMap<FName, UInstancedStaticMeshComponent*> MeshComponents;

	// Koordinat -> Instance Index eşleşmesi. 
	// Bir hücreyi silmek veya güncellemek istersek hangi index'te olduğunu bilmeliyiz.
	// TMap<FGridCoordinate, FInstanceHandle> gibi bir yapı gerekebilir ama şimdilik basit tutalım:
	// Şimdilik sadece "Ekleme" mantığına odaklanıyoruz. Silme/Değiştirme için ISMC index yönetimi gerekir.
	
	// Helper: İlgili TileID için ISMC getir veya yoksa oluştur
	UInstancedStaticMeshComponent* GetOrCreateISMC(FName TileID);
};