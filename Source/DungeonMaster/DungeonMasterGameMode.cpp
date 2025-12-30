#include "DungeonMasterGameMode.h"
#include "DungeonMaster/UI/DungeonMainWidget.h"
#include "DungeonMaster/Systems/WaveSubsystem.h"
#include "DungeonMasterPlayerController.h" 
#include "Kismet/GameplayStatics.h"
#include "Systems/GridSubsystem.h"

ADungeonMasterGameMode::ADungeonMasterGameMode()
{
	// Varsayılan PlayerController sınıfını ata
	PlayerControllerClass = ADungeonMasterPlayerController::StaticClass();
}

void ADungeonMasterGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (UGridSubsystem* GridSys = GetWorld()->GetSubsystem<UGridSubsystem>())
	{
		// Önce temizle, sonra Core koy
		GridSys->InitializeGrid(10, 10); // Boyut şimdilik önemsiz, dinamik büyüyebilir
		GridSys->SpawnCoreUnit();
	}

	// 1. Widget'ı Oluştur ve Ekrana Bas
	if (MainWidgetClass)
	{
		MainWidgetInstance = CreateWidget<UDungeonMainWidget>(GetWorld(), MainWidgetClass);
		if (MainWidgetInstance)
		{
			MainWidgetInstance->AddToViewport();

			// 2. Başlangıç Destesini Oluştur (Hardcoded Test Verisi)
			// İleride burası PlayerState veya bir Envanter Componentinden gelecek.
			TArray<FName> StarterDeck;
			StarterDeck.Add(FName("Room"));
			StarterDeck.Add(FName("Room"));
			StarterDeck.Add(FName("Room"));
			StarterDeck.Add(FName("Wall"));
			StarterDeck.Add(FName("Trap_Spike")); // Eğer varsa

			// 3. Desteyi UI'a gönder
			MainWidgetInstance->BuildDeck(StarterDeck);
		}
	}

	// 2. Subsystem'deki değişiklikleri UI'a yansıtmak için dinle
	if (UWaveSubsystem* WaveSys = GetWorld()->GetSubsystem<UWaveSubsystem>())
	{
		WaveSys->OnGamePhaseChanged.AddDynamic(this, &ADungeonMasterGameMode::HandlePhaseChanged);
	}
}

void ADungeonMasterGameMode::HandlePhaseChanged(EGamePhase NewPhase)
{
	if (MainWidgetInstance)
	{
		MainWidgetInstance->UpdatePhaseVisuals(NewPhase);
	}
}