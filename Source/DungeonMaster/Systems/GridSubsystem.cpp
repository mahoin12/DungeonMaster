#include "GridSubsystem.h"

void UGridSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UGridSubsystem::InitializeGrid(int32 Width, int32 Height)
{
}

bool UGridSubsystem::TryPlaceTile(const FGridCoordinate& Coord, FName TileID)
{
	return false;
}

bool UGridSubsystem::IsWalkable(const FGridCoordinate& Coord) const
{
	return false;
}

const FCellData* UGridSubsystem::GetCellData(const FGridCoordinate& Coord) const
{
	return nullptr;
}
