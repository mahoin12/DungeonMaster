#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "DungeonTileDrag.generated.h"

/**
 * UI'dan sürüklenen Tile bilgisini tutar.
 */
UCLASS()
class DUNGEONMASTER_API UDungeonTileDrag : public UDragDropOperation
{
	GENERATED_BODY()

public:
	// Hangi Tile tipi sürükleniyor? (Örn: "FireTrap", "Archer")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drag Drop")
	FName TileID;

	// Gerekirse görsel referansı
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drag Drop")
	UTexture2D* Icon;
};