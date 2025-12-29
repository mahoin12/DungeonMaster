#include "GridUnit.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AGridUnit::AGridUnit()
{
	PrimaryActorTick.bCanEverTick = true;

	// Karakter ayarları
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	
	// Karakterin rotasyonu hareket yönüne dönsün (Görsel olarak)
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Fiziksel hareketi kapatalım, biz yöneteceğiz
	GetCharacterMovement()->GravityScale = 0.0f; 
	GetCharacterMovement()->DefaultLandMovementMode = MOVE_Flying; 
}

void AGridUnit::BeginPlay()
{
	Super::BeginPlay();
}

void AGridUnit::InitializeAt(FGridCoordinate StartCoord)
{
	CurrentGridCoord = StartCoord;
	TargetGridCoord = StartCoord;
	SetActorLocation(GridToWorld(StartCoord));
}

void AGridUnit::FollowPath(const TArray<FGridCoordinate>& Path)
{
	if (Path.Num() == 0) return;

	CurrentPath = Path;
	
	// Eğer yolun başındaysak ilk noktayı atla (zaten oradayız)
	if (CurrentPath.Num() > 0 && CurrentPath[0] == CurrentGridCoord)
	{
		CurrentPath.RemoveAt(0);
	}

	bIsMoving = true;
	MoveToNextTile();
}

void AGridUnit::MoveToNextTile()
{
	if (CurrentPath.Num() == 0)
	{
		bIsMoving = false;
		// TODO: Hedefe ulaşıldı eventi
		return;
	}

	// Sıradaki hedefi al
	TargetGridCoord = CurrentPath[0];
	CurrentPath.RemoveAt(0);

	// Lerp parametrelerini hazırla
	WorldStartPos = GetActorLocation();
	WorldEndPos = GridToWorld(TargetGridCoord);
	MoveAlpha = 0.0f;

	// Karakterin yönünü hedefe çevir (Görsel)
	FRotator LookAtRot = (WorldEndPos - WorldStartPos).Rotation();
	LookAtRot.Pitch = 0.0f;
	LookAtRot.Roll = 0.0f;
	SetActorRotation(LookAtRot);
}

void AGridUnit::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsMoving)
	{
		// Lerp ilerlemesi
		MoveAlpha += DeltaTime * MoveSpeed;

		if (MoveAlpha >= 1.0f)
		{
			// Hedefe tam vardık
			SetActorLocation(WorldEndPos);
			CurrentGridCoord = TargetGridCoord;
			MoveToNextTile(); // Bir sonrakine geç
		}
		else
		{
			// Ara pozisyonu hesapla
			FVector NewLoc = FMath::Lerp(WorldStartPos, WorldEndPos, MoveAlpha);
			SetActorLocation(NewLoc);
		}
	}
}

FVector AGridUnit::GridToWorld(const FGridCoordinate& Coord) const
{
	// GridVisualizer ile aynı matematik
	// Z = 100.0f diyerek biraz yukarı kaldırıyoruz ki yere gömülmesin (Capsule Half Height)
	return FVector(Coord.X * GridCellSize, Coord.Y * GridCellSize, 100.0f); 
}