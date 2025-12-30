#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "DungeonMasterPlayerController.generated.h"

UCLASS()
class DUNGEONMASTER_API ADungeonMasterPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ADungeonMasterPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	// Input Mapping
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputMappingContext* DefaultMappingContext;

	// Sol Tık Aksiyonu
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* ClickAction;

	// Tıklama fonksiyonu
	void OnLeftClick();

private:
	// Grid Hücre boyutu (Raycast hesaplaması için)
	const float GridSize = 100.0f;
};