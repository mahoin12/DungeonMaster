#include "GridVisualizer.h"
#include "DungeonMaster/Systems/GridSubsystem.h"
#include "Components/InstancedStaticMeshComponent.h"

AGridVisualizer::AGridVisualizer()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void AGridVisualizer::BeginPlay()
{
	Super::BeginPlay();

	// Subsystem'e abone ol
	if (UWorld* World = GetWorld())
	{
		if (UGridSubsystem* GridSubsystem = World->GetSubsystem<UGridSubsystem>())
		{
			// Delegate'i dinlemeye başla
			GridSubsystem->OnGridStateChanged.AddDynamic(this, &AGridVisualizer::HandleGridChanged);
			
			// TODO: Eğer oyun başladığında kayıtlı bir grid varsa burada bir "RebuildAll" fonksiyonu çağırıp hepsini çizebilirsin.
		}
	}
}

void AGridVisualizer::HandleGridChanged(FGridCoordinate Coord, FCellData NewData)
{
	// LOG 6: Sinyal ulaştı mı?
	UE_LOG(LogTemp, Warning, TEXT("VISUALIZER: Grid Degisikligi Algilandi! Coord: (%d, %d), TileID: %s"), Coord.X, Coord.Y, *NewData.TileID.ToString());

	if (!TileSetData)
	{
		UE_LOG(LogTemp, Error, TEXT("VISUALIZER: TileSet DataAsset BOS! Lütfen Blueprint üzerinden atama yapin."));
		return;
	}

	// 1. Koordinatı Dünya Pozisyonuna Çevir
	FVector Location(Coord.X * GridCellSize, Coord.Y * GridCellSize, 0.0f);
	FTransform InstanceTransform(FRotator::ZeroRotator, Location);

	// 2. İlgili TileID için ISMC'yi bul
	if (UInstancedStaticMeshComponent* ISMC = GetOrCreateISMC(NewData.TileID))
	{
		// 3. Instance Ekle
		int32 Index = ISMC->AddInstance(InstanceTransform, true);
		UE_LOG(LogTemp, Display, TEXT("VISUALIZER: Mesh eklendi. Instance Index: %d"), Index);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("VISUALIZER: ISMC olusturulamadi! TileID DataAsset icinde tanimli mi?"));
	}
}

UInstancedStaticMeshComponent* AGridVisualizer::GetOrCreateISMC(FName TileID)
{
	if (MeshComponents.Contains(TileID))
	{
		return MeshComponents[TileID];
	}

	if (TileSetData && TileSetData->TileDefinitions.Contains(TileID))
	{
		UStaticMesh* MeshToUse = TileSetData->TileDefinitions[TileID].Mesh;
		if (MeshToUse)
		{
			FName CompName = FName(*FString::Printf(TEXT("ISMC_%s"), *TileID.ToString()));
			UInstancedStaticMeshComponent* NewComp = NewObject<UInstancedStaticMeshComponent>(this, CompName);
            
			NewComp->SetStaticMesh(MeshToUse);
			NewComp->SetupAttachment(RootComponent);
			NewComp->RegisterComponent();
            
			MeshComponents.Add(TileID, NewComp);
            
			UE_LOG(LogTemp, Log, TEXT("VISUALIZER: Yeni ISMC bileseni olusturuldu: %s"), *TileID.ToString());
			return NewComp;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("VISUALIZER: ID bulundu ama Mesh BOS! ID: %s"), *TileID.ToString());
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("VISUALIZER: TileID DataAsset icinde BULUNAMADI! ID: %s"), *TileID.ToString());
	}

	return nullptr;
}