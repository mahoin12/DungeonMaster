#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DungeonMaster/Info/Types.h" // FGridCoordinate struct'ı burada
#include "DungeonMainWidget.generated.h"

/**
 * UDungeonMainWidget
 * * Amaç: Oyunun ana arayüzüdür (HUD). Tüm ekranı kaplar.
 * Sol panelden sürüklenen bir tile, 3D dünyaya bırakılmak istendiğinde
 * "OnDrop" fonksiyonu burada çalışır ve koordinat hesaplamasını yapar.
 */
UCLASS()
class DUNGEONMASTER_API UDungeonMainWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	// Bir sürükleme işlemi bu widget'ın üzerine bırakıldığında çalışır (Native C++ Override)
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

private:
	// Ekran koordinatını (2D) Dünya koordinatına (3D Grid) çeviren yardımcı fonksiyon
	bool GetGridCoordinateFromScreen(const FVector2D& ScreenPosition, struct FGridCoordinate& OutCoord);
};