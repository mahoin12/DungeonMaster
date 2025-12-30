#include "DungeonMasterPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "DungeonMaster/Systems/GridSubsystem.h"
#include "DungeonMaster/Info/Types.h"

ADungeonMasterPlayerController::ADungeonMasterPlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}

void ADungeonMasterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Enhanced Input Ayarı
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (DefaultMappingContext)
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void ADungeonMasterPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (ClickAction)
		{
			EnhancedInputComponent->BindAction(ClickAction, ETriggerEvent::Started, this, &ADungeonMasterPlayerController::OnLeftClick);
		}
	}
}

void ADungeonMasterPlayerController::OnLeftClick()
{
	// Mouse'un altındaki dünyayı tara
	FHitResult Hit;
	if (GetHitResultUnderCursor(ECC_Visibility, false, Hit))
	{
		// 1. Tıklanan Lokasyonu Al
		FVector ClickLocation = Hit.Location;

		// 2. Grid Koordinatına Çevir (Snap to Grid)
		// Grid'in (0,0,0) merkezli başladığını varsayıyoruz.
		int32 X = FMath::RoundToInt(ClickLocation.X / GridSize);
		int32 Y = FMath::RoundToInt(ClickLocation.Y / GridSize);
		
		FGridCoordinate TargetCoord;
		TargetCoord.X = X;
		TargetCoord.Y = Y;

		// 3. Subsystem'e "İnşa Et" emri ver
		if (UGridSubsystem* GridSys = GetWorld()->GetSubsystem<UGridSubsystem>())
		{
			// Şimdilik test amaçlı her tıklamada "Duvar" koymayı deniyoruz.
			// İleride buraya seçili olan TileID gelecek.
			FName SelectedTileID = TEXT("Wall"); 
			
			// Eğer Shift'e basılıysa silme (Floor yapma) mantığı eklenebilir.
			bool bSuccess = GridSys->TryPlaceTile(TargetCoord, SelectedTileID);
			
			if (bSuccess)
			{
				// Ses veya efekt oynatılabilir
				// ClientPlaySoundAtLocation(...);
			}
		}
	}
}