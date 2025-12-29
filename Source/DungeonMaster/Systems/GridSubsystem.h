#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "DungeonMaster/Info/Types.h" // FGridCoordinate ve structlar burada varsayıyorum
#include "GridSubsystem.generated.h"

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

	// Grid'i başlatır
	void InitializeGrid(int32 Width, int32 Height);

	// Bir tile yerleştirmeyi dener. Yol tıkanıyorsa false döner.
	bool TryPlaceTile(const FGridCoordinate& Coord, FName TileID);

	// A* Yol Bulma Algoritması
	bool FindPath(FGridCoordinate Start, FGridCoordinate End, TArray<FGridCoordinate>& OutPath);

	// Yardımcılar
	bool IsBlocked(const FGridCoordinate& Coord) const;
	bool IsValidCoordinate(const FGridCoordinate& Coord) const;

private:
	TMap<FGridCoordinate, FCellData> GridMap;
	int32 GridWidth;
	int32 GridHeight;

	// Düşmanların doğduğu ve gitmeye çalıştığı yer (Sabit varsayıyoruz şimdilik)
	FGridCoordinate SpawnPoint = {0, 0};
	FGridCoordinate CorePoint = {9, 9};

	// Yol bulma için komşuları getirir
	TArray<FGridCoordinate> GetNeighbors(const FGridCoordinate& Center) const;
};