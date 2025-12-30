#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DraggableTileWidget.generated.h"

class UImage;
class UTextBlock;

UCLASS()
class DUNGEONMASTER_API UDraggableTileWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Widget yaratıldığında hangi Tile olduğunu buna söyleyeceğiz
	UFUNCTION(BlueprintCallable, Category = "Tile Data")
	void InitializeTile(FName ID);

	void SetTileID(FName ID) { TileID = ID; }
	FName GetTileID() const { return TileID; }
	

protected:

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TileNameText;
	UPROPERTY(meta = (BindWidget))
	UImage* TileIconImage;
	
	
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;

	UPROPERTY(BlueprintReadOnly, Category = "Tile Data")
	FName TileID;
};