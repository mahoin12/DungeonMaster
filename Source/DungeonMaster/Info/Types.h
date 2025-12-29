#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Types.generated.h"

// Grid üzerindeki bir hücrenin durumu
UENUM(BlueprintType)
enum class ECellType : uint8
{
	Empty       UMETA(DisplayName = "Empty"),
	Room        UMETA(DisplayName = "Room"), // Üzerine inşa yapılabilir zemin
	Wall        UMETA(DisplayName = "Wall"), // Aşılamaz
	Entrance    UMETA(DisplayName = "Entrance"),
	Core        UMETA(DisplayName = "Core") // Korunacak hedef
};

UENUM(BlueprintType)
enum class EGamePhase : uint8
{
	Preparation UMETA(DisplayName = "Preparation Phase"), // İnşaat yapılabilir
	Combat      UMETA(DisplayName = "Combat Phase"),      // Düşmanlar gelir, inşaat kilitli
	Victory     UMETA(DisplayName = "Victory"),
	Defeat      UMETA(DisplayName = "Defeat")
};

// Grid koordinatları (FIntPoint alternatifi ama Hash fonksiyonu için özelleştirebiliriz)
USTRUCT(BlueprintType)
struct FGridCoordinate
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 X = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Y = 0;

	// TMap'te Key olarak kullanmak için gerekli operatör
	bool operator==(const FGridCoordinate& Other) const
	{
		return X == Other.X && Y == Other.Y;
	}

	friend uint32 GetTypeHash(const FGridCoordinate& Other)
	{
		return HashCombine(GetTypeHash(Other.X), GetTypeHash(Other.Y));
	}
};

// Bir hücrenin taşıdığı veri
USTRUCT(BlueprintType)
struct FCellData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	ECellType CellType = ECellType::Empty;

	// O hücredeki tile'ın ID'si veya DataAsset referansı
	UPROPERTY(BlueprintReadOnly)
	FName TileID = NAME_None;

	// Sinerjiler için tag'ler (Örn: Status.Wet, Trap.Fire)
	UPROPERTY(BlueprintReadOnly)
	FGameplayTagContainer CellTags;

	// Görsel aktör referansı (Opsiyonel, sadece logic için gerekliyse boş kalabilir)
	// TWeakObjectPtr kullanmak garbage collection için daha güvenlidir
	TWeakObjectPtr<AActor> SpawnedActor; 
};