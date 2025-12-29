#include "DraggableTileWidget.h"
#include "DungeonTileDrag.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

FReply UDraggableTileWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// Sadece Sol Tık ile sürüklemeye izin veriyoruz
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		// "Handled" döndürürken "DetectDrag" diyerek sisteme sürüklemeyi dinlemesini söylüyoruz.
		return FReply::Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
	}

	return FReply::Unhandled();
}

void UDraggableTileWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	// 1. Drag Operasyonunu oluştur
	UDungeonTileDrag* DragOp = NewObject<UDungeonTileDrag>();
	
	// 2. Veriyi içine koy
	DragOp->TileID = this->TileID;
	
	// 3. Görsel ayarlama (Sürüklerken bu widget'ın aynısı mouse ucunda gözüksün)
	// Not: Performans için basitleştirilmiş bir widget da oluşturulabilir.
	DragOp->DefaultDragVisual = this; 
	DragOp->Pivot = EDragPivot::CenterCenter;

	// 4. Operasyonu dışarı ver (Unreal bunu alıp yönetmeye başlar)
	OutOperation = DragOp;
}