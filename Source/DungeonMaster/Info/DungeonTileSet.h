#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DungeonTileSet.generated.h"

USTRUCT(BlueprintType)
struct FTileVisualDefinition
{
	GENERATED_BODY()

	// Bu tile için kullanılacak mesh
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UStaticMesh* Mesh = nullptr;

	// Opsiyonel: Grid üzerinde dönme açısı, scale vb. buraya eklenebilir.
};

UCLASS()
class DUNGEONMASTER_API UDungeonTileSet : public UDataAsset
{
	GENERATED_BODY()

public:
	// TileID (FName) -> Visual Data eşleşmesi
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	TMap<FName, FTileVisualDefinition> TileDefinitions;
};