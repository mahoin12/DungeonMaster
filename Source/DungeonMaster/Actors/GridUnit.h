#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DungeonMaster/Info/Types.h"
#include "GridUnit.generated.h"

UCLASS()
class DUNGEONMASTER_API AGridUnit : public ACharacter
{
	GENERATED_BODY()

public:
	AGridUnit();

	virtual void Tick(float DeltaTime) override;

	// Üniteyi belirli bir grid koordinatına ışınlar/başlatır
	void InitializeAt(FGridCoordinate StartCoord);

	// Bir yol listesi vererek hareketi başlatır
	void FollowPath(const TArray<FGridCoordinate>& Path);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override; // <--- Bunu ekle

	// Şu anki mantıksal grid konumu
	UPROPERTY(VisibleInstanceOnly, Category = "Grid Logic")
	FGridCoordinate CurrentGridCoord;

	// Gitmekte olduğu hedef grid konumu (Animasyon için)
	UPROPERTY(VisibleInstanceOnly, Category = "Grid Logic")
	FGridCoordinate TargetGridCoord;

	// Hareket hızı (Hücre / Saniye)
	UPROPERTY(EditAnywhere, Category = "Grid Logic")
	float MoveSpeed = 2.0f; 

	// Grid hücre boyutu (Visualizer ile aynı olmalı)
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	float GridCellSize = 100.0f;

private:
	// Hareket Mantığı Değişkenleri
	bool bIsMoving = false;
	TArray<FGridCoordinate> CurrentPath; // Gidilecek kalan yol
	FVector WorldStartPos; // Lerp başlangıcı
	FVector WorldEndPos;   // Lerp bitişi
	float MoveAlpha = 0.0f; // 0 ile 1 arası ilerleme durumu

	// Sıradaki kareye geçişi başlatır
	void MoveToNextTile();
	
	// Grid koordinatını dünya pozisyonuna çevirir (Zemine oturtarak)
	FVector GridToWorld(const FGridCoordinate& Coord) const;
};