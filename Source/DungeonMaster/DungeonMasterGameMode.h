#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DungeonMaster/Info/Types.h"
#include "DungeonMasterGameMode.generated.h"

class UDungeonMainWidget;

UCLASS(minimalapi)
class ADungeonMasterGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ADungeonMasterGameMode();
	
	virtual void BeginPlay() override;

protected:
	// Kullanılacak Widget Sınıfı (BP'den seçilecek)
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UDungeonMainWidget> MainWidgetClass;

	// Oluşturulan Widget referansı
	UPROPERTY()
	UDungeonMainWidget* MainWidgetInstance;

	// Subsystem eventlerini dinlemek için
	UFUNCTION()
	void HandlePhaseChanged(EGamePhase NewPhase);
};