#include "GridSubsystem.h"

void UGridSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	// Varsayılan bir başlangıç
	InitializeGrid(10, 10);
}

void UGridSubsystem::InitializeGrid(int32 Width, int32 Height)
{
	GridWidth = Width;
	GridHeight = Height;
	GridMap.Empty();

	// Grid'i boş hücrelerle doldur (İsteğe bağlı, TMap olduğu için doldurmasak da olur ama yer tutsun)
	for (int32 x = 0; x < Width; x++)
	{
		for (int32 y = 0; y < Height; y++)
		{
			FGridCoordinate Coord = {x, y};
			FCellData NewCell;
			NewCell.CellType = ECellType::Empty;
			GridMap.Add(Coord, NewCell);
		}
	}
}

bool UGridSubsystem::TryPlaceTile(const FGridCoordinate& Coord, FName TileID)
{
	if (!IsValidCoordinate(Coord)) return false;

	// Zaten dolu mu?
	if (IsBlocked(Coord)) 
	{
		UE_LOG(LogTemp, Warning, TEXT("Hücre zaten dolu!"));
		return false;
	}

	// 1. Geçici olarak haritaya "Duvar" koyduğumuzu simüle et
	ECellType OriginalType = ECellType::Empty;
	if (GridMap.Contains(Coord)) OriginalType = GridMap[Coord].CellType;

	FCellData TempData;
	TempData.CellType = ECellType::Wall; // Şimdilik her şeyi duvar sayalım
	GridMap.Add(Coord, TempData);

	// 2. Yol hala açık mı kontrol et (Girişten Kalbe yol var mı?)
	TArray<FGridCoordinate> TestPath;
	bool bPathExists = FindPath(SpawnPoint, CorePoint, TestPath);

	if (bPathExists)
	{
		// YOL AÇIK: İnşaatı onayla
		FCellData FinalData;
		FinalData.CellType = ECellType::Wall; // Veya TileID'ye göre tip
		FinalData.TileID = TileID;
		GridMap.Add(Coord, FinalData);
		
		// Event yayınla (Görselleştirici bunu dinleyip mesh koyacak)
		// OnGridChanged.Broadcast(Coord); 
		return true;
	}
	else
	{
		// YOL TIKALI: Değişikliği geri al
		FCellData RevertData;
		RevertData.CellType = OriginalType;
		GridMap.Add(Coord, RevertData);
		
		UE_LOG(LogTemp, Error, TEXT("İnşaat Yolu Tıkıyor! İzin verilmedi."));
		return false;
	}
}

bool UGridSubsystem::IsBlocked(const FGridCoordinate& Coord) const
{
	if (!GridMap.Contains(Coord)) return true; // Harita dışı veya tanımsız
	const FCellData& Data = GridMap[Coord];
	// Duvar veya başka engelleyici bir şey var mı?
	return Data.CellType == ECellType::Wall; 
}

bool UGridSubsystem::IsValidCoordinate(const FGridCoordinate& Coord) const
{
	return Coord.X >= 0 && Coord.X < GridWidth && Coord.Y >= 0 && Coord.Y < GridHeight;
}

TArray<FGridCoordinate> UGridSubsystem::GetNeighbors(const FGridCoordinate& Center) const
{
	TArray<FGridCoordinate> Neighbors;
	// 4 Yön (Kuzey, Güney, Doğu, Batı)
	FGridCoordinate Directions[] = { {0, 1}, {0, -1}, {1, 0}, {-1, 0} };

	for (const auto& Dir : Directions)
	{
		FGridCoordinate Next = { Center.X + Dir.X, Center.Y + Dir.Y };
		if (IsValidCoordinate(Next))
		{
			Neighbors.Add(Next);
		}
	}
	return Neighbors;
}

bool UGridSubsystem::FindPath(FGridCoordinate Start, FGridCoordinate End, TArray<FGridCoordinate>& OutPath)
{
	OutPath.Empty();

	if (!IsValidCoordinate(Start) || !IsValidCoordinate(End) || IsBlocked(End))
	{
		return false; // Başlangıç/Bitiş geçersizse veya hedef duvarsa
	}

	// A* (A-Star) Algoritması
	TArray<FPathNode> OpenSet;
	TMap<FGridCoordinate, float> GScore; // En ucuz maliyet tablosu
	TMap<FGridCoordinate, FGridCoordinate> CameFrom; // Yol haritası

	// Başlangıç düğümü
	FPathNode StartNode;
	StartNode.Coord = Start;
	StartNode.G = 0;
	StartNode.H = FMath::Abs(End.X - Start.X) + FMath::Abs(End.Y - Start.Y); // Manhattan
	
	OpenSet.Add(StartNode);
	GScore.Add(Start, 0);

	while (OpenSet.Num() > 0)
	{
		// 1. En düşük F skoruna sahip düğümü bul
		OpenSet.Sort([](const FPathNode& A, const FPathNode& B) {
			return A.F() < B.F(); 
		});
		
		FPathNode Current = OpenSet[0];
		OpenSet.RemoveAt(0);

		// 2. Hedefe ulaştık mı?
		if (Current.Coord == End)
		{
			// Yolu geriye doğru oluştur
			FGridCoordinate Temp = End;
			while (!(Temp == Start))
			{
				OutPath.Add(Temp);
				if(CameFrom.Contains(Temp))
				{
					Temp = CameFrom[Temp];
				}
				else
				{
					return false; // Bir hata oldu, yol kopuk
				}
			}
			OutPath.Add(Start);
			Algo::Reverse(OutPath); // Başlangıçtan sona sırala
			return true;
		}

		// 3. Komşuları gez
		for (FGridCoordinate Neighbor : GetNeighbors(Current.Coord))
		{
			if (IsBlocked(Neighbor)) continue; // Duvarlardan geçemeyiz

			float TentativeG = Current.G + 1; // Her kare 1 birim uzaklıkta

			// Eğer bu komşuya daha önce hiç gelmediysek veya yeni yol daha kısaysa
			if (!GScore.Contains(Neighbor) || TentativeG < GScore[Neighbor])
			{
				CameFrom.Add(Neighbor, Current.Coord);
				GScore.Add(Neighbor, TentativeG);

				FPathNode NeighborNode;
				NeighborNode.Coord = Neighbor;
				NeighborNode.G = TentativeG;
				NeighborNode.H = FMath::Abs(End.X - Neighbor.X) + FMath::Abs(End.Y - Neighbor.Y);
				
				// OpenSet'te zaten var mı diye bakmak lazım (Basitlik için direkt ekliyoruz, optimize edilebilir)
				bool bFoundInOpen = false;
				for(auto& Node : OpenSet)
				{
					if(Node.Coord == Neighbor)
					{
						Node.G = TentativeG; // Güncelle
						Node.H = NeighborNode.H;
						bFoundInOpen = true;
						break;
					}
				}
				if (!bFoundInOpen)
				{
					OpenSet.Add(NeighborNode);
				}
			}
		}
	}

	return false; // Yol bulunamadı
}