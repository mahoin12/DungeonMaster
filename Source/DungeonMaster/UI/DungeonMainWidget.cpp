#include "DungeonMainWidget.h"
#include "DungeonMaster/Systems/WaveSubsystem.h"
#include "DraggableTileWidget.h"
#include "DungeonTileDrag.h"
#include "DungeonMaster/Actors/GhostTileActor.h"
#include "DungeonMaster/Actors/GridVisualizer.h"
#include "DungeonMaster/Systems/GridSubsystem.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

void UDungeonMainWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (NextPhaseBtn)
	{
		NextPhaseBtn->OnClicked.AddDynamic(this, &UDungeonMainWidget::OnNextPhaseClicked);
	}
	
	UpdatePhaseVisuals(EGamePhase::Preparation);
}

void UDungeonMainWidget::OnNextPhaseClicked()
{
	if (UWorld* World = GetWorld())
	{
		if (UWaveSubsystem* WaveSys = World->GetSubsystem<UWaveSubsystem>())
		{
			WaveSys->AdvanceToNextPhase();
		}
	}
}

void UDungeonMainWidget::UpdatePhaseVisuals(EGamePhase CurrentPhase)
{
	FText PhaseName;
	FLinearColor TextColor = FLinearColor::White;

	switch (CurrentPhase)
	{
	case EGamePhase::Preparation:
		PhaseName = FText::FromString("PREPARATION PHASE");
		TextColor = FLinearColor::Green;
		if(NextPhaseBtn) NextPhaseBtn->SetIsEnabled(true);
		break;
	case EGamePhase::Combat:
		PhaseName = FText::FromString("COMBAT PHASE");
		TextColor = FLinearColor::Red;
		if(NextPhaseBtn) NextPhaseBtn->SetIsEnabled(false);
		break;
	case EGamePhase::Victory:
		PhaseName = FText::FromString("VICTORY!");
		TextColor = FLinearColor::Yellow;
		if(NextPhaseBtn) NextPhaseBtn->SetIsEnabled(true);
		break;
	case EGamePhase::Defeat:
		PhaseName = FText::FromString("DEFEAT");
		TextColor = FLinearColor::Gray;
		if(NextPhaseBtn) NextPhaseBtn->SetIsEnabled(true);
		break;
	}

	if (CurrentPhaseText)
	{
		CurrentPhaseText->SetText(PhaseName);
		CurrentPhaseText->SetColorAndOpacity(FSlateColor(TextColor));
	}
}

void UDungeonMainWidget::BuildDeck(const TArray<FName>& AvailableTiles)
{
	if (!TileDeckContainer || !DraggableTileClass) return;

	TileDeckContainer->ClearChildren();

	for (const FName& TileID : AvailableTiles)
	{
		if (UDraggableTileWidget* NewTileWidget = CreateWidget<UDraggableTileWidget>(GetOwningPlayer(), DraggableTileClass))
		{
			NewTileWidget->InitializeTile(TileID);
			TileDeckContainer->AddChild(NewTileWidget);
		}
	}
}

AGridVisualizer* UDungeonMainWidget::GetGridVisualizer()
{
	if (!CachedGridVisualizer)
	{
		// Sahnedeki GridVisualizer aktörünü bul
		CachedGridVisualizer = Cast<AGridVisualizer>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridVisualizer::StaticClass()));
	}
	return CachedGridVisualizer;
}

// ==============================================================================
// DRAG & DROP SİSTEMİ
// ==============================================================================

void UDungeonMainWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);
}

bool UDungeonMainWidget::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UDungeonTileDrag* TileDragOp = Cast<UDungeonTileDrag>(InOperation);
	if (!TileDragOp) return false;

	// 1. Grid Visualizer'ı Aktif Et (Collision kutularını aç)
	if (AGridVisualizer* Vis = GetGridVisualizer())
	{
		Vis->SetPlacementGridActive(true);
	}

	FGridCoordinate HoverCoord;
	FVector WorldPos;
	
	// 2. Mouse'un altındaki Grid kutusunu bul (Raycast ile)
	bool bHit = GetGridPositionFromMouse(InGeometry, InDragDropEvent, HoverCoord, WorldPos);

	// 3. Ghost Actor Yönetimi
	if (!CurrentGhostActor && GhostActorClass && bHit)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		CurrentGhostActor = GetWorld()->SpawnActor<AGhostTileActor>(GhostActorClass, WorldPos, FRotator::ZeroRotator, SpawnParams);
		// İstersen burada TileDragOp->TileID'ye göre Ghost Actor meshini değiştirebilirsin.
	}

	if (CurrentGhostActor)
	{
		if (bHit)
		{
			// Snap olmuş konuma taşı
			CurrentGhostActor->SetActorLocation(WorldPos);
			CurrentGhostActor->SetActorHiddenInGame(false);

			// Geçerlilik Kontrolü (Renk değişimi için)
			if (UGridSubsystem* GridSys = GetWorld()->GetSubsystem<UGridSubsystem>())
			{
				bool bOccupied = GridSys->IsTileOccupied(HoverCoord);
				bool bHasNeighbor = GridSys->HasConnectedNeighbor(HoverCoord);
				
				// Normalde: Dolu değilse VE komşusu varsa (bitişikse) geçerlidir.
				bool bIsValid = !bOccupied && bHasNeighbor;

				// Replacement (Değiştirme) İstisnası:
				// Eğer doluysa ama Core değilse ve yine de bağlıysa -> Geçerli say (Mavi/Sarı yanabilir ama şimdilik yeşil geçiyoruz)
				if (bOccupied && bHasNeighbor && HoverCoord != GridSys->GetCorePoint())
				{
					bIsValid = true; 
				}

				CurrentGhostActor->SetValidity(bIsValid);
			}
		}
		else
		{
			// Grid üzerinde değilsek gizle
			CurrentGhostActor->SetActorHiddenInGame(true);
		}
	}

	return true;
}

void UDungeonMainWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	// Sürükleme iptal oldu, temizlik yap
	if (CurrentGhostActor)
	{
		CurrentGhostActor->Destroy();
		CurrentGhostActor = nullptr;
	}

	// Grid Collision'ı kapat
	if (AGridVisualizer* Vis = GetGridVisualizer())
	{
		Vis->SetPlacementGridActive(false);
	}
}

bool UDungeonMainWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UDungeonTileDrag* TileDragOp = Cast<UDungeonTileDrag>(InOperation);
	if (!TileDragOp) return false;

	// --- Temizlik ---
	if (CurrentGhostActor)
	{
		CurrentGhostActor->Destroy();
		CurrentGhostActor = nullptr;
	}
	if (AGridVisualizer* Vis = GetGridVisualizer())
	{
		Vis->SetPlacementGridActive(false);
	}
	// ----------------

	FGridCoordinate TargetCoord;
	FVector WorldPos;

	// Bırakılan yeri tespit et
	if (GetGridPositionFromMouse(InGeometry, InDragDropEvent, TargetCoord, WorldPos))
	{
		if (UGridSubsystem* GridSys = GetWorld()->GetSubsystem<UGridSubsystem>())
		{
			// 1. Replacement (Değiştirme) Kontrolü
			if (GridSys->IsTileOccupied(TargetCoord))
			{
				// Core değiştirilemez
				if (TargetCoord == GridSys->GetCorePoint()) 
				{
					UE_LOG(LogTemp, Warning, TEXT("DROP: Core degistirilemez!"));
					return false;
				}

				// Zorla değiştir
				UE_LOG(LogTemp, Log, TEXT("DROP: Tas degistiriliyor..."));
				return GridSys->ForceReplaceTile(TargetCoord, TileDragOp->TileID);
			}

			// 2. Bitişiklik (Adjacency) Kontrolü
			if (!GridSys->HasConnectedNeighbor(TargetCoord))
			{
				UE_LOG(LogTemp, Warning, TEXT("DROP: Baglantisiz (Havada) tas koyamazsiniz!"));
				// TODO: Kartı desteye geri ekle
				return false;
			}

			// 3. Normal Yerleştirme
			bool bSuccess = GridSys->TryPlaceTile(TargetCoord, TileDragOp->TileID);
			if (!bSuccess)
			{
				UE_LOG(LogTemp, Warning, TEXT("DROP: Yerlestirme basarisiz (Yol kapaniyor vs.)"));
				// TODO: Kartı desteye geri ekle
			}
			return bSuccess;
		}
	}

	return false;
}

bool UDungeonMainWidget::GetGridPositionFromMouse(const FGeometry& Geometry, const FPointerEvent& MouseEvent, FGridCoordinate& OutCoord, FVector& OutWorldPos)
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC) return false;

	// 1. Viewport Ölçeklemesi (DPI Scaling Fix)
	FVector2D WidgetSize = Geometry.GetLocalSize();
	FVector2D LocalMousePos = Geometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
	FVector2D ViewportPosition = FVector2D::ZeroVector;

	if (UWorld* World = GetWorld())
	{
		if (UGameViewportClient* ViewportClient = World->GetGameViewport())
		{
			FVector2D ViewportSize;
			ViewportClient->GetViewportSize(ViewportSize);
			if (WidgetSize.X > 0 && WidgetSize.Y > 0)
			{
				ViewportPosition = FVector2D(
					(LocalMousePos.X / WidgetSize.X) * ViewportSize.X,
					(LocalMousePos.Y / WidgetSize.Y) * ViewportSize.Y
				);
			}
		}
	}

	// 2. Deproject ve Raycast
	FVector WorldLoc, WorldDir;
	if (PC->DeprojectScreenPositionToWorld(ViewportPosition.X, ViewportPosition.Y, WorldLoc, WorldDir))
	{
		FVector Start = WorldLoc;
		FVector End = WorldLoc + (WorldDir * 50000.0f);
		FHitResult Hit;
		
		FCollisionQueryParams Params;
		Params.bTraceComplex = true; // Instance'lara tam oturması için

		// Sadece Visibility kanalına çarpıyoruz (Grid Collision bu kanalda blokluyor olmalı)
		if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
		{
			// Vurduğumuz şey bir Instanced Static Mesh mi?
			if (UInstancedStaticMeshComponent* HitISMC = Cast<UInstancedStaticMeshComponent>(Hit.Component))
			{
				// Item Index geçerli mi? (-1 değilse bir instance'a vurduk demektir)
				if (Hit.Item != INDEX_NONE)
				{
					if (AGridVisualizer* Vis = GetGridVisualizer())
					{
						// Visualizer'dan bu indexin koordinatını istiyoruz
						OutCoord = Vis->GetCoordinateFromIndex(Hit.Item);
						
						// Kutunun tam merkezini alalım (Snap işlemi)
						FTransform InstTransform;
						HitISMC->GetInstanceTransform(Hit.Item, InstTransform, true);
						OutWorldPos = InstTransform.GetLocation();
						
						return true;
					}
				}
			}
		}
	}

	return false;
}