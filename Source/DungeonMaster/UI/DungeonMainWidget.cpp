#include "DungeonMainWidget.h"
#include "DungeonMaster/Systems/WaveSubsystem.h"
#include "DraggableTileWidget.h"
#include "DungeonTileDrag.h"
#include "Components/WrapBox.h"
#include "DungeonMaster/Systems/GridSubsystem.h"
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

	// Önce paneli temizle (Eski elden kalanları sil)
	TileDeckContainer->ClearChildren();

	// Yeni kartları oluştur
	for (const FName& TileID : AvailableTiles)
	{
		if (UDraggableTileWidget* NewTileWidget = CreateWidget<UDraggableTileWidget>(GetOwningPlayer(), DraggableTileClass))
		{
			NewTileWidget->InitializeTile(TileID);
			TileDeckContainer->AddChild(NewTileWidget);
		}
	}
}

bool UDungeonMainWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    // 1. Payload kontrolü
	UDungeonTileDrag* TileDragOp = Cast<UDungeonTileDrag>(InOperation);
	if (!TileDragOp) return false;

	APlayerController* PC = GetOwningPlayer();
	if (!PC) return false;

	// --- KOORDİNAT DÜZELTME (SENİN YÖNTEMİN) ---
	FVector2D ViewportPosition = FVector2D::ZeroVector;

	// Widget'ın o anki boyutu
	const FVector2D WidgetSize = InGeometry.GetLocalSize();
	
	// Mouse'un widget içindeki lokal pozisyonu
	const FVector2D LocalMousePos = InGeometry.AbsoluteToLocal(InDragDropEvent.GetScreenSpacePosition());

	if (UWorld* World = GetWorld())
	{
		if (UGameViewportClient* ViewportClient = World->GetGameViewport())
		{
			FVector2D ViewportSize;
			ViewportClient->GetViewportSize(ViewportSize);

			// Sıfıra bölünme hatası olmasın
			if (WidgetSize.X > 0 && WidgetSize.Y > 0)
			{
				// Oran orantı ile gerçek Viewport pikselini buluyoruz
				ViewportPosition = FVector2D(
					(LocalMousePos.X / WidgetSize.X) * ViewportSize.X,
					(LocalMousePos.Y / WidgetSize.Y) * ViewportSize.Y
				);
			}
		}
	}
	// --------------------------------------------

	FVector WorldLocation;
	FVector WorldDirection;

	// 2. Deproject (Artık hesaplanmış ViewportPosition kullanıyoruz)
	if (PC->DeprojectScreenPositionToWorld(ViewportPosition.X, ViewportPosition.Y, WorldLocation, WorldDirection))
	{
		// 3. Raycast (Line Trace)
		FVector TraceStart = WorldLocation;
		FVector TraceEnd = WorldLocation + (WorldDirection * 50000.0f); // 50k birim ileri

		FHitResult HitResult;
		FCollisionQueryParams QueryParams;
		QueryParams.bTraceComplex = false;

		bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams);

		if (bHit)
		{
			// DEBUG: Nereye vurduğumuzu görelim (Yeşil Küre)
			DrawDebugSphere(GetWorld(), HitResult.Location, 30.0f, 12, FColor::Green, false, 5.0f);
			UE_LOG(LogTemp, Warning, TEXT("DROP SUCCESS: Vurulan Yer: %s"), *HitResult.Location.ToString());

			// 4. GRID SNAP MATEMATİĞİ
			float GridSize = 100.0f;
			int32 GridX = FMath::RoundToInt(HitResult.Location.X / GridSize);
			int32 GridY = FMath::RoundToInt(HitResult.Location.Y / GridSize);

			FGridCoordinate TargetCoord;
			TargetCoord.X = GridX;
			TargetCoord.Y = GridY;

			// 5. Subsystem'e gönder
			if (UGridSubsystem* GridSys = GetWorld()->GetSubsystem<UGridSubsystem>())
			{
				bool bSuccess = GridSys->TryPlaceTile(TargetCoord, TileDragOp->TileID);
				return bSuccess;
			}
		}
		else
		{
			// Raycast boşa giderse Kırmızı çizgi çizelim, yönü görelim
			DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, 5.0f);
			UE_LOG(LogTemp, Error, TEXT("DROP: Raycast BOSA GITI!"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("DROP: Deproject Başarısız Oldu!"));
	}

	return false;
}
