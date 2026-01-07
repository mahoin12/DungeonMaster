#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "DungeonMaster/Info/Types.h" // FGridCoordinate ve structlar burada varsayıyorum
#include "GridSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGridStateChanged, FGridCoordinate, Coord, FCellData, NewData);

// Yol bulma sırasında kullanılacak geçici düğüm yapısı
struct FPathNode
{
	FGridCoordinate Coord;
	FGridCoordinate ParentCoord; // Geldiğimiz yön (Geri iz sürmek için)
	float G; // Başlangıçtan buraya maliyet
	float H; // Buradan hedefe tahmini maliyet (Heuristic)
	float F() const { return G + H; } // Toplam skor

	bool operator<(const FPathNode& Other) const
	{
		return F() < Other.F(); // Priority Queue için sıralama
	}
};


class AGridUnit;
/**
 * UGridSubsystem
 * * Amaç: Tüm Grid verisini ve oyun mantığını yöneten singleton benzeri sınıf.
 * Harita burada saklanır, yol bulma (Pathfinding) burada yapılır.
 * Subsystem olduğu için levele actor koymaya gerek kalmadan her yerden erişilir.
 */
UCLASS()
class DUNGEONMASTER_API UGridSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "Grid Logic")
	void SpawnCoreUnit();

	// Grid'i başlatır
	void InitializeGrid(int32 Width, int32 Height);

	// Bir tile yerleştirmeyi dener. Yol tıkanıyorsa false döner.
	bool TryPlaceTile(const FGridCoordinate& Coord, FName TileID);

	// Belirtilen koordinat, dolu olan herhangi bir hücreye komşu mu? (Core'a bağlı zincirin parçası mı?)
	bool HasConnectedNeighbor(const FGridCoordinate& Coord) const;

	UFUNCTION(BlueprintCallable, Category = "Grid Logic")
	void RemoveTile(FGridCoordinate Coord);

	// Bağlantısı kopan (Core'a ulaşamayan) tüm taşları bulur ve siler.
	void PruneDisconnectedTiles();

	// Replacement için: Koordinat dolu mu kontrolü
	bool IsTileOccupied(const FGridCoordinate& Coord) const;

	// Replacement onayı gelirse zorla yerleştirme
	bool ForceReplaceTile(const FGridCoordinate& Coord, FName NewTileID);

	// A* Yol Bulma Algoritması
	bool FindPath(FGridCoordinate Start, FGridCoordinate End, TArray<FGridCoordinate>& OutPath);

	// Yardımcılar
	bool IsBlocked(const FGridCoordinate& Coord) const;
	bool IsValidCoordinate(const FGridCoordinate& Coord) const;
	
	UFUNCTION(BlueprintCallable, Category = "Grid Logic")
	AGridUnit* SpawnUnit(TSubclassOf<AGridUnit> UnitClass, FGridCoordinate StartCoord);

	// Tüm unitleri Kalbe doğru yürüt (Test için)
	UFUNCTION(BlueprintCallable, Category = "Grid Logic")
	void MoveAllUnitsToCore();

	FGridCoordinate GetSpawnPoint() const { return SpawnPoint; }
	FGridCoordinate GetCorePoint() const { return CorePoint; }
	
	UFUNCTION(BlueprintPure, Category = "Grid Info")
	int32 GetGridWidth() const { return GridWidth; }
	UFUNCTION(BlueprintPure, Category = "Grid Info")
	int32 GetGridHeight() const { return GridHeight; }

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnGridStateChanged OnGridStateChanged;

private:
	TMap<FGridCoordinate, FCellData> GridMap;
	int32 GridWidth;
	int32 GridHeight;

	// Düşmanların doğduğu ve gitmeye çalıştığı yer (Sabit varsayıyoruz şimdilik)
	FGridCoordinate SpawnPoint = {0, 0};
	FGridCoordinate CorePoint = {5, 5};

	UPROPERTY()
	TArray<AGridUnit*> ActiveUnits;

	// Yol bulma için komşuları getirir
	TArray<FGridCoordinate> GetNeighbors(const FGridCoordinate& Center) const;

	// BFS ile Core'dan ulaşılabilen tüm koordinatları döndürür.
	TSet<FGridCoordinate> GetConnectedTiles() const;
};