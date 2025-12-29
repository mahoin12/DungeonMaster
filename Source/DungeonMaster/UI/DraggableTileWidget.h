#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DraggableTileWidget.generated.h"

/**
 * UDraggableTileWidget
 * * Amaç: Oyuncunun envanterinde/marketinde gördüğü tekil eşya kartıdır.
 * Üzerine tıklandığında sürükleme işlemini (DragDetect) başlatır.
 */
UCLASS()
class DUNGEONMASTER_API UDraggableTileWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Bu widget hangi tile'ı temsil ediyor? (Tasarım zamanında veya spawn edilirken atanmalı)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FName TileID;

protected:
	// Mouse tuşuna basıldığında Unreal'a "Sürükleme ihtimali var, takip et" deriz.
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	// Sürükleme (Drag) başladığı an tetiklenir. Burada DragDropOperation oluşturulur.
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
};