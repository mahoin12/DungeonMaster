#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "DungeonMaster/Info/Types.h"
#include "DungeonMainWidget.generated.h"

class UWrapBox;

UCLASS()
class DUNGEONMASTER_API UDungeonMainWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	// Faz değişince metni günceller
	void UpdatePhaseVisuals(EGamePhase CurrentPhase);

	// Desteyi (İkonları) oluşturur
	UFUNCTION(BlueprintCallable, Category = "Deck")
	void BuildDeck(const TArray<FName>& AvailableTiles);

protected:

	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	
	UPROPERTY(meta = (BindWidget))
	UButton* NextPhaseBtn;

	// --- YENİ: Tek bir text bloğu ---
	UPROPERTY(meta = (BindWidget))
	UTextBlock* CurrentPhaseText;

	// --- YENİ: Deste Paneli (Soldaki liste) ---
	// Editörde buna bir VerticalBox veya ScrollBox bağlayacağız.
	UPROPERTY(meta = (BindWidget))
	UWrapBox* TileDeckContainer;

	// Hangi widget sınıfını spawn edeceğiz? (BP_DraggableTileWidget)
	UPROPERTY(EditDefaultsOnly, Category = "UI Config")
	TSubclassOf<class UDraggableTileWidget> DraggableTileClass;

	UFUNCTION()
	void OnNextPhaseClicked();
};