#include "DungeonMainWidget.h"
#include "DungeonTileDrag.h"
#include "DungeonMaster/Systems/GridSubsystem.h"
#include "Kismet/GameplayStatics.h"

bool UDungeonMainWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	// 1. Gelen paket bizim tile paketi mi?
	UDungeonTileDrag* TileDragOp = Cast<UDungeonTileDrag>(InOperation);
	if (!TileDragOp)
	{
		// Bizimle ilgili değilse işlemi başkasına sal (Unhandled)
		return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
	}

	// 2. Mouse'un ekran üzerindeki pozisyonunu al
	FVector2D ScreenPosition = InDragDropEvent.GetScreenSpacePosition();

	// 3. 3D Grid Koordinatını hesapla
	FGridCoordinate TargetCoord;
	if (GetGridCoordinateFromScreen(ScreenPosition, TargetCoord))
	{
		// 4. Subsystem'e emir ver: "Buraya inşa etmeye çalış"
		UGridSubsystem* GridSys = GetWorld()->GetSubsystem<UGridSubsystem>();
		if (GridSys)
		{
			// TryPlaceTile fonksiyonu GridSubsystem'de tanımlı olmalı
			bool bSuccess = GridSys->TryPlaceTile(TargetCoord, TileDragOp->TileID);
			
			if (bSuccess)
			{
				// Ses efekti veya görsel feedback burada verilebilir
				UE_LOG(LogTemp, Log, TEXT("Insaat Basarili: [%d, %d]"), TargetCoord.X, TargetCoord.Y);
			}
		}
		
		return true; // İşlemi biz hallettik (Handled)
	}

	return false;
}

bool UDungeonMainWidget::GetGridCoordinateFromScreen(const FVector2D& ScreenPosition, FGridCoordinate& OutCoord)
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC) return false;

	FVector WorldLocation, WorldDirection;
	
	// Unreal'ın yerleşik fonksiyonu ile 2D -> 3D Işın (Ray) çevrimi
	if (PC->DeprojectScreenPositionToWorld(ScreenPosition.X, ScreenPosition.Y, WorldLocation, WorldDirection))
	{
		// Oyunumuz düz bir zemin (Plane) üzerinde geçiyor (Z = 0)
		// Ray-Plane Intersection (Işın-Düzlem Kesişimi) Matematiği:
		
		FVector PlaneNormal = FVector::UpVector; // Z ekseni yukarı (0,0,1)
		float PlaneHeight = 0.0f; // Grid Z=0'da

		// Payda 0 ise ışın yere paraleldir, kesişmez.
		float Denom = FVector::DotProduct(WorldDirection, PlaneNormal);
		if (FMath::IsNearlyZero(Denom)) return false;

		// t = (PlaneOrigin - RayOrigin) . Normal / (RayDir . Normal)
		float t = FVector::DotProduct(FVector(0, 0, PlaneHeight) - WorldLocation, PlaneNormal) / Denom;
		
		// Kesişim kameranın arkasında mı?
		if (t < 0.0f) return false; 

		FVector HitPoint = WorldLocation + (WorldDirection * t);

		// Grid Boyutu (Örn: Her kare 100cm). Bunu Subsystem'den çekmek daha doğru olur.
		const float TileSize = 100.0f; 

		// Dünya koordinatını Grid indeksine çevir (Yuvarlama)
		OutCoord.X = FMath::RoundToInt(HitPoint.X / TileSize);
		OutCoord.Y = FMath::RoundToInt(HitPoint.Y / TileSize);

		return true;
	}

	return false;
}