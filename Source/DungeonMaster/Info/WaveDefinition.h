#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DungeonMaster/Actors/GridUnit.h"
#include "WaveDefinition.generated.h"

// Tek bir düşman grubunun tanımı (Örn: 5 tane Hızlı Goblin)
USTRUCT(BlueprintType)
struct FWaveGroup
{
	GENERATED_BODY()

	// Spawn edilecek düşman sınıfı (BP_Goblin vb.)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AGridUnit> UnitClass;

	// Kaç adet çıkacak?
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin=123))
	int32 Count = 1;

	// Bu gruptaki düşmanlar arası bekleme süresi (saniye)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin=0.1f))
	float SpawnInterval = 1.0f;

	// Bir sonraki gruba geçmeden önce beklenecek süre
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin=0.0f))
	float DelayAfterGroup = 2.0f;
};

// Bir Level'ın tüm dalga yapısını tutan Data Asset
UCLASS()
class DUNGEONMASTER_API UWaveDefinition : public UDataAsset
{
	GENERATED_BODY()

public:
	// Sırayla çalışacak düşman grupları
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wave Config")
	TArray<FWaveGroup> WaveGroups;
};