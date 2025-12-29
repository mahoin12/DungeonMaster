#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Info/Types.h"
#include "GridSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGridChanged, const FGridCoordinate&, ChangedCoord);

UCLASS()
class DUNGEONMASTER_API UGridSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// Grid oluşturma
	UFUNCTION(BlueprintCallable)
	void InitializeGrid(int32 Width, int32 Height);

	// Tile yerleştirme fonksiyonu
	UFUNCTION(BlueprintCallable)
	bool TryPlaceTile(const FGridCoordinate& Coord, FName TileID);

	// Bir koordinatın geçerli olup olmadığını ve yürünebilirliğini kontrol et
	bool IsWalkable(const FGridCoordinate& Coord) const;
    
	// Grid verisine erişim
	const FCellData* GetCellData(const FGridCoordinate& Coord) const;

	// UI veya Görsel Manager dinlesin diye Event
	UPROPERTY(BlueprintAssignable)
	FOnGridChanged OnGridChanged;

private:
	// Grid verisini TMap içinde tutuyoruz. Array'den daha güvenli (Sparse grid için)
	TMap<FGridCoordinate, FCellData> GridMap;

	int32 GridWidth;
	int32 GridHeight;
};