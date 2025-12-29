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

UCLASS()
class DUNGEONMASTER_API UGridSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category="Grid Logic")
	void InitializeGrid(int32 Width, int32 Height);

	// İki nokta arasında yol var mı? Varsa yolu döner.
	UFUNCTION(BlueprintCallable, Category="Grid Logic")
	bool FindPath(FGridCoordinate Start, FGridCoordinate End, TArray<FGridCoordinate>& OutPath);

	// Bir kare bloklu mu? (Duvar var mı?)
	bool IsBlocked(const FGridCoordinate& Coord) const;

	// Grid sınırları içinde mi?
	bool IsValidCoordinate(const FGridCoordinate& Coord) const;

private:
	TMap<FGridCoordinate, FCellData> GridMap;
	int32 GridWidth;
	int32 GridHeight;

	// A* algoritması için komşuları getiren yardımcı fonksiyon
	TArray<FGridCoordinate> GetNeighbors(const FGridCoordinate& Center) const;
};