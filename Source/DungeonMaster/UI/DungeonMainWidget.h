#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/WrapBox.h"
#include "DungeonMaster/Info/Types.h"
#include "DungeonMainWidget.generated.h"

// Forward Declarations
class AGhostTileActor;
class AGridVisualizer;

UCLASS()
class DUNGEONMASTER_API UDungeonMainWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	// Oyun fazı değişince UI'ı günceller (Hazırlık, Savaş vb.)
	void UpdatePhaseVisuals(EGamePhase CurrentPhase);

	// Verilen taş listesine göre sol paneli doldurur
	UFUNCTION(BlueprintCallable, Category = "Deck")
	void BuildDeck(const TArray<FName>& AvailableTiles);

protected:
	// --- Drag & Drop Olayları ---
	virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	
	// --- UI Elemanları ---
	UPROPERTY(meta = (BindWidget))
	UButton* NextPhaseBtn;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CurrentPhaseText;

	UPROPERTY(meta = (BindWidget))
	UWrapBox* TileDeckContainer;

	// --- Config ---
	// Sürüklenebilir kart widget sınıfı
	UPROPERTY(EditDefaultsOnly, Category = "UI Config")
	TSubclassOf<class UDraggableTileWidget> DraggableTileClass;

	// Mouse ucunda belirecek hayalet aktör sınıfı
	UPROPERTY(EditDefaultsOnly, Category = "UI Config")
	TSubclassOf<AGhostTileActor> GhostActorClass;

	UFUNCTION()
	void OnNextPhaseClicked();

private:
	// Sürükleme sırasında oluşturulan geçici aktör
	UPROPERTY()
	AGhostTileActor* CurrentGhostActor;

	// Sahnedeki Visualizer'a hızlı erişim için cache
	UPROPERTY()
	AGridVisualizer* CachedGridVisualizer;

	// Helper: Visualizer'ı bulur veya cache'den getirir
	AGridVisualizer* GetGridVisualizer();

	/**
	 * Mouse pozisyonuna göre Grid üzerindeki fiziksel kutuyu bulur.
	 * @return Eğer geçerli bir Grid hücresine (ISMC Instance) çarptıysa true döner.
	 */
	bool GetGridPositionFromMouse(const FGeometry& Geometry, const FPointerEvent& MouseEvent, FGridCoordinate& OutCoord, FVector& OutWorldPos);
};