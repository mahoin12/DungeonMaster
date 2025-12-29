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
	if (!TileSetData) return;

	// 1. Koordinatı Dünya Pozisyonuna Çevir
	FVector Location(Coord.X * GridCellSize, Coord.Y * GridCellSize, 0.0f);
	FTransform InstanceTransform(FRotator::ZeroRotator, Location);

	// 2. İlgili TileID için ISMC'yi bul
	if (UInstancedStaticMeshComponent* ISMC = GetOrCreateISMC(NewData.TileID))
	{
		// 3. Instance Ekle
		// Not: Eğer burada daha önce bir şey varsa onu silmek gerekir (GridLogic'te dolu kontrolü olduğu için şimdilik es geçiyorum)
		ISMC->AddInstance(InstanceTransform, true);
	}
}

UInstancedStaticMeshComponent* AGridVisualizer::GetOrCreateISMC(FName TileID)
{
	// 1. Zaten varsa döndür
	if (MeshComponents.Contains(TileID))
	{
		return MeshComponents[TileID];
	}

	// 2. Yoksa DataAsset'ten Mesh'i bul
	if (TileSetData && TileSetData->TileDefinitions.Contains(TileID))
	{
		UStaticMesh* MeshToUse = TileSetData->TileDefinitions[TileID].Mesh;
		if (MeshToUse)
		{
			// 3. Yeni ISMC Oluştur
			FName CompName = FName(*FString::Printf(TEXT("ISMC_%s"), *TileID.ToString()));
			UInstancedStaticMeshComponent* NewComp = NewObject<UInstancedStaticMeshComponent>(this, CompName);
			
			NewComp->SetStaticMesh(MeshToUse);
			NewComp->SetupAttachment(RootComponent);
			NewComp->RegisterComponent();
			
			// Map'e kaydet
			MeshComponents.Add(TileID, NewComp);
			return NewComp;
		}
	}

	return nullptr;
}