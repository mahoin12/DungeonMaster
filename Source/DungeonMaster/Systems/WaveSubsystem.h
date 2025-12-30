#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "DungeonMaster/Info/Types.h"
#include "DungeonMaster/Info/WaveDefinition.h"
#include "WaveSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGamePhaseChanged, EGamePhase, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWaveProgress, int32, EnemiesRemaining, int32, TotalEnemies);

UCLASS()
class DUNGEONMASTER_API UWaveSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// Faz Yönetimi
	UFUNCTION(BlueprintCallable, Category = "Game Flow")
	void StartCombatPhase(UWaveDefinition* WaveData);

	UFUNCTION(BlueprintCallable, Category = "Game Flow")
	void ReturnToPreparation();

	// Aktif Fazı Sorgula
	UFUNCTION(BlueprintPure, Category = "Game Flow")
	EGamePhase GetCurrentPhase() const { return CurrentPhase; }

	// Düşman öldüğünde/kalbe ulaştığında çağrılacak
	void RegisterEnemyDeath(AActor* DeadUnit);

	UFUNCTION(BlueprintCallable, Category = "Game Flow")
	void AdvanceToNextPhase();

	// Savaş bittiğinde (Tüm düşmanlar ölünce) çağrılır
	void OnCombatEnded(bool bPlayerWon);

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnGamePhaseChanged OnGamePhaseChanged;
	
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnWaveProgress OnWaveProgress;

private:
	EGamePhase CurrentPhase = EGamePhase::Preparation;
	
	// Dalga Mantığı Değişkenleri
	UPROPERTY()
	UWaveDefinition* CurrentWaveData;
	
	FTimerHandle SpawnTimerHandle;
	int32 CurrentGroupIndex = 0;
	int32 UnitsSpawnedInGroup = 0;
	int32 TotalEnemiesAlive = 0;

	// Timer fonksiyonu
	void ProcessSpawnLoop();
	void FinishWave();
};