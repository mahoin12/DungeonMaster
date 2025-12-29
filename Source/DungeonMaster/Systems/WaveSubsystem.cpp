#include "WaveSubsystem.h"
#include "GridSubsystem.h"
#include "DungeonMaster/Actors/GridUnit.h"

void UWaveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	CurrentPhase = EGamePhase::Preparation;
}

void UWaveSubsystem::StartCombatPhase(UWaveDefinition* WaveData)
{
	if (!WaveData || CurrentPhase == EGamePhase::Combat) return;

	CurrentWaveData = WaveData;
	CurrentPhase = EGamePhase::Combat;
	OnGamePhaseChanged.Broadcast(CurrentPhase);

	// Değişkenleri sıfırla
	CurrentGroupIndex = 0;
	UnitsSpawnedInGroup = 0;
	TotalEnemiesAlive = 0;

	// İlk spawn işlemini başlat
	ProcessSpawnLoop();
}

void UWaveSubsystem::ReturnToPreparation()
{
	CurrentPhase = EGamePhase::Preparation;
	GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
	OnGamePhaseChanged.Broadcast(CurrentPhase);
}

void UWaveSubsystem::ProcessSpawnLoop()
{
	if (!CurrentWaveData) return;

	// Tüm gruplar bitti mi?
	if (CurrentGroupIndex >= CurrentWaveData->WaveGroups.Num())
	{
		// Spawn bitti ama düşmanlar hala yaşıyor olabilir.
		// Bu yüzden Combat fazını hemen bitirmiyoruz, son düşman ölene kadar bekliyoruz.
		return; 
	}

	const FWaveGroup& CurrentGroup = CurrentWaveData->WaveGroups[CurrentGroupIndex];

	// Bu gruptan spawn edilecekler bitti mi?
	if (UnitsSpawnedInGroup >= CurrentGroup.Count)
	{
		// Bir sonraki gruba geç
		CurrentGroupIndex++;
		UnitsSpawnedInGroup = 0;
		
		// Grup arası bekleme süresi
		float Delay = CurrentGroup.DelayAfterGroup;
		if (Delay <= 0.0f) Delay = 0.1f; // Güvenlik

		GetWorld()->GetTimerManager().SetTimer(SpawnTimerHandle, this, &UWaveSubsystem::ProcessSpawnLoop, Delay, false);
		return;
	}

	// === SPAWN İŞLEMİ ===
	if (UGridSubsystem* GridSys = GetWorld()->GetSubsystem<UGridSubsystem>())
	{
		// GridSubsystem'e spawn komutu ver (SpawnPoint koordinatını GridSubsystem biliyor)
		// Not: GridSubsystem'e "GetSpawnPoint" getter'ı eklemen gerekebilir veya direkt (0,0) kullan.
		FGridCoordinate SpawnCoord = {0, 0}; 
		
		AGridUnit* NewUnit = GridSys->SpawnUnit(CurrentGroup.UnitClass, SpawnCoord);
		if (NewUnit)
		{
			TotalEnemiesAlive++;
			// Düşmanın öldüğünü veya yok olduğunu takip etmemiz lazım.
			// Basitçe Delegate ile bağlanabiliriz:
			// NewUnit->OnDestroyed.AddDynamic(this, ...); // Unreal Actor OnDestroyed
			// Ama biz özel bir fonksiyon yazsak daha iyi kontrol ederiz.
		}
		
		// Hedefe yürüt
		GridSys->MoveAllUnitsToCore(); // Optimize edilebilir: Sadece yeni geleni yürüt.
	}

	UnitsSpawnedInGroup++;
	OnWaveProgress.Broadcast(TotalEnemiesAlive, 0); // UI için güncelleme

	// Bir sonraki birim için timer kur
	float Interval = CurrentGroup.SpawnInterval;
	if (Interval <= 0.0f) Interval = 0.5f;
	
	GetWorld()->GetTimerManager().SetTimer(SpawnTimerHandle, this, &UWaveSubsystem::ProcessSpawnLoop, Interval, false);
}

void UWaveSubsystem::RegisterEnemyDeath(AActor* DeadUnit)
{
	if (CurrentPhase != EGamePhase::Combat) return;

	TotalEnemiesAlive--;
	if (TotalEnemiesAlive <= 0 && CurrentGroupIndex >= CurrentWaveData->WaveGroups.Num())
	{
		// Tüm düşmanlar bitti, dalga temizlendi!
		FinishWave();
	}
}

void UWaveSubsystem::FinishWave()
{
	UE_LOG(LogTemp, Log, TEXT("Wave Complete! Returning to Build Phase."));
	ReturnToPreparation();
}