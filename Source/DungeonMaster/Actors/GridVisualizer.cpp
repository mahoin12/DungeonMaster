#include "GridVisualizer.h"
#include "DungeonMaster/Systems/GridSubsystem.h"
#include "Components/InstancedStaticMeshComponent.h"

AGridVisualizer::AGridVisualizer()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// ISMC Oluştur
	GridCollisionISMC = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("GridCollisionISMC"));
	GridCollisionISMC->SetupAttachment(RootComponent);
	
	// Collision Ayarları: Visibility kanalından bloklamalı ki Mouse Raycast'i çarpsın.
	GridCollisionISMC->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	GridCollisionISMC->SetCollisionResponseToAllChannels(ECR_Ignore);
	GridCollisionISMC->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	
	// Başlangıçta gizli (HiddenInGame=true olunca Raycast çarpar ama gözükmez. 
	// Tamamen disable etmek için Collision'ı kapatacağız.)
	GridCollisionISMC->SetHiddenInGame(true);
}

void AGridVisualizer::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld())
	{
		if (UGridSubsystem* GridSubsystem = World->GetSubsystem<UGridSubsystem>())
		{
			GridSubsystem->OnGridStateChanged.AddDynamic(this, &AGridVisualizer::HandleGridChanged);
			
			// Grid Collision Alanını Oluştur
			GenerateGridCollision(GridSubsystem->GetGridWidth(), GridSubsystem->GetGridHeight());
		}
	}
}

void AGridVisualizer::GenerateGridCollision(int32 Width, int32 Height)
{
	if (!GridCollisionISMC) return;

	GridCollisionISMC->ClearInstances();
	CachedGridWidth = Width;

	// Grid'in (0,0) noktası merkez mi köşe mi? Kodlarınızda (0,0) köşe gibi duruyor.
	// Pivotu ortada olan bir Plane Mesh (SM_Plane) kullanırsanız 50 birim offset gerekebilir.
	// Varsayım: Pivot sol alt köşe veya merkez, ona göre Offset ekleyebilirsin.
	FVector Offset(GridCellSize / 2.0f, GridCellSize / 2.0f, 0.5f); // Z hafif yukarıda olsun z-fighting olmasın

	for (int32 y = 0; y < Height; y++)
	{
		for (int32 x = 0; x < Width; x++)
		{
			FVector Location(x * GridCellSize, y * GridCellSize, 0.0f);
			// Eğer mesh pivotu merkezdeyse offset ekle:
			// Location += Offset; 

			FTransform Transform(FRotator::ZeroRotator, Location, FVector(1.0f)); // Scale gerekirse ayarla (Mesh 100x100 ise 1.0)
			GridCollisionISMC->AddInstance(Transform);
		}
	}

	// Varsayılan olarak kapalı
	SetPlacementGridActive(false);
}

void AGridVisualizer::SetPlacementGridActive(bool bActive)
{
	if (!GridCollisionISMC) return;

	// Görünürlük
	GridCollisionISMC->SetHiddenInGame(!bActive);

	// Collision'ı sadece aktifken açalım ki normal oyunda tıklamaları engellemesin
	GridCollisionISMC->SetCollisionEnabled(bActive ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
}

FGridCoordinate AGridVisualizer::GetCoordinateFromIndex(int32 InstanceIndex) const
{
	if (CachedGridWidth <= 0) return { -1, -1 };

	// Matematiksel sihir: Index = Y * Width + X
	int32 Y = InstanceIndex / CachedGridWidth;
	int32 X = InstanceIndex % CachedGridWidth;

	return { X, Y };
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