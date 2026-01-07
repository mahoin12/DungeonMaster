#include "GridSubsystem.h"

#include "WaveSubsystem.h"
#include "DungeonMaster/Actors/GridUnit.h"

void UGridSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	GridMap.Empty();
	// Kalbi yerleştir (Bir sonraki frame'de çalışması için timer koyabiliriz ama şimdilik direkt çağıralım)
	// Not: Oyun dünyası tamamen yüklenmeden çağrılırsa Visualizer yakalayamaz. 
	// Bu yüzden GameMode BeginPlay'den çağırmak daha güvenlidir ama burada logic kuralım.
}

void UGridSubsystem::SpawnCoreUnit()
{
	// Elle (0,0) yazmak yerine sınıf değişkenini kullan
	FGridCoordinate CoreCoord = CorePoint; 
    
	FCellData CoreData;
	CoreData.CellType = ECellType::Core; 
	CoreData.TileID = FName("Core");     

	GridMap.Add(CoreCoord, CoreData);

	if (OnGridStateChanged.IsBound())
	{
		OnGridStateChanged.Broadcast(CoreCoord, CoreData);
	}
}

void UGridSubsystem::InitializeGrid(int32 Width, int32 Height)
{
	GridWidth = Width;
	GridHeight = Height;
	GridMap.Empty();

	// 1. Varsayılan noktaları ayarla (Sol alt ve Sağ üst köşe)
	CorePoint = FGridCoordinate(0, 0);
	SpawnPoint = FGridCoordinate(Width - 1, Height - 1); 

	UE_LOG(LogTemp, Warning, TEXT("GRID INIT: Core: (0,0), Spawn: (%d, %d)"), SpawnPoint.X, SpawnPoint.Y);

	// Grid'i boş hücrelerle doldur
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
	// 1. Koordinat Geçerli mi?
	if (!IsValidCoordinate(Coord))
	{
		UE_LOG(LogTemp, Error, TEXT("INSAAT REDDEDILDI: Koordinat Grid disinda! (%d, %d)"), Coord.X, Coord.Y);
		return false;
	}

	// 2. Hücre zaten dolu mu? (Core veya başka duvar var mı?)
	if (IsBlocked(Coord)) 
	{
		// Detaylı bilgi ver: Orada ne var?
		FName ExistingID = TEXT("Unknown");
		if(GridMap.Contains(Coord)) ExistingID = GridMap[Coord].TileID;
		
		UE_LOG(LogTemp, Warning, TEXT("INSAAT REDDEDILDI: Hucre Zaten Dolu! (%d, %d) - Icindeki: %s"), Coord.X, Coord.Y, *ExistingID.ToString());
		return false;
	}

	// 3. Spawn veya Core noktasına inşaat yapmaya çalışıyor mu?
	if (Coord == SpawnPoint || Coord == CorePoint)
	{
		UE_LOG(LogTemp, Warning, TEXT("INSAAT REDDEDILDI: Spawn veya Core noktasina duvar koyamazsin!"));
		return false;
	}

	// --- SIMULASYON BASLIYOR ---
	
	// Eski tipi sakla
	ECellType OriginalType = ECellType::Empty;
	if (GridMap.Contains(Coord)) OriginalType = GridMap[Coord].CellType;

	// Geçici olarak duvar koy
	FCellData TempData;
	TempData.CellType = ECellType::Wall; 
	GridMap.Add(Coord, TempData);

	// 4. Yol Kontrolü: Spawn'dan Core'a hala gidilebiliyor mu?
	TArray<FGridCoordinate> TestPath;
	bool bPathExists = FindPath(SpawnPoint, CorePoint, TestPath);

	if (bPathExists)
	{
		// ONAYLANDI: Kalıcı veriyi işle
		FCellData FinalData;
		FinalData.CellType = ECellType::Room; // Veya TileID'ye göre duvar
		FinalData.TileID = TileID;
		
		GridMap.Add(Coord, FinalData);
		
		if (OnGridStateChanged.IsBound())
		{
			OnGridStateChanged.Broadcast(Coord, FinalData);
		}
		
		UE_LOG(LogTemp, Display, TEXT("INSAAT ONAYLANDI: (%d, %d) konumuna %s yerlestirildi."), Coord.X, Coord.Y, *TileID.ToString());
		return true;
	}
	else
	{
		// REDDEDILDI: Değişikliği geri al!
		FCellData RevertData;
		RevertData.CellType = OriginalType;
		GridMap.Add(Coord, RevertData);
		
		UE_LOG(LogTemp, Error, TEXT("INSAAT REDDEDILDI: Yol Tikaniyor! (Pathfinding Failed)"));
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

AGridUnit* UGridSubsystem::SpawnUnit(TSubclassOf<AGridUnit> UnitClass, FGridCoordinate StartCoord)
{
	if (!UnitClass || !GetWorld()) return nullptr;

	// Grid sınırlarında mı?
	if (!IsValidCoordinate(StartCoord)) return nullptr;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Aktörü spawn et (Konumu Unit kendi ayarlayacak)
	AGridUnit* NewUnit = GetWorld()->SpawnActor<AGridUnit>(UnitClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	
	if (NewUnit)
	{
		NewUnit->InitializeAt(StartCoord);
		ActiveUnits.Add(NewUnit);
	}

	return NewUnit;
}

void UGridSubsystem::MoveAllUnitsToCore()
{
	for (AGridUnit* Unit : ActiveUnits)
	{
		if (!Unit) continue;

		// Unit'in olduğu yerden Core'a yol bul
		// Not: Unit'in current coord değişkenini okumak için getter eklemen gerekebilir veya public yapabilirsin şimdilik.
		// Şimdilik test için (0,0)'dan (9,9)'a yol buluyoruz varsayalım.
		// Gerçek implementasyonda Unit->GetCurrentCoord() kullanılmalı.
		
		TArray<FGridCoordinate> Path;
		// Unitin başlangıç noktası SpawnPoint varsayıyoruz şimdilik
		if (FindPath(SpawnPoint, CorePoint, Path))
		{
			Unit->FollowPath(Path);
		}
	}
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

bool UGridSubsystem::HasConnectedNeighbor(const FGridCoordinate& Coord) const
{
    // Eğer Core'un tam yanına koyuyorsak true
    TArray<FGridCoordinate> Neighbors = GetNeighbors(Coord);
    for (const FGridCoordinate& N : Neighbors)
    {
        if (GridMap.Contains(N))
        {
            // Komşu dolu mu? (Empty değilse doludur)
            if (GridMap[N].CellType != ECellType::Empty)
            {
                return true;
            }
        }
    }
    return false; // Hiçbir komşu dolu değil
}

TSet<FGridCoordinate> UGridSubsystem::GetConnectedTiles() const
{
    TSet<FGridCoordinate> Visited;
    TArray<FGridCoordinate> Queue;

    // Başlangıç: Core
    if (GridMap.Contains(CorePoint))
    {
        Queue.Add(CorePoint);
        Visited.Add(CorePoint);
    }

    while (Queue.Num() > 0)
    {
        FGridCoordinate Current = Queue[0];
        Queue.RemoveAt(0);

        TArray<FGridCoordinate> Neighbors = GetNeighbors(Current);
        for (const FGridCoordinate& Next : Neighbors)
        {
            // Eğer komşu doluysa ve daha önce gezilmediyse
            if (GridMap.Contains(Next) && 
                GridMap[Next].CellType != ECellType::Empty && 
                !Visited.Contains(Next))
            {
                Visited.Add(Next);
                Queue.Add(Next);
            }
        }
    }
    return Visited;
}

void UGridSubsystem::PruneDisconnectedTiles()
{
    // 1. Core'a bağlı olanları bul
    TSet<FGridCoordinate> ConnectedSet = GetConnectedTiles();

    // 2. Grid'deki tüm dolu hücreleri gez
    TArray<FGridCoordinate> ToRemove;
    for (auto& Elem : GridMap)
    {
        // Boş değilse ve Connected setinde yoksa silinecek
        if (Elem.Value.CellType != ECellType::Empty && !ConnectedSet.Contains(Elem.Key))
        {
            ToRemove.Add(Elem.Key);
        }
    }

    // 3. Silme işlemini uygula ve desteye iade et
    for (const FGridCoordinate& Coord : ToRemove)
    {
        UE_LOG(LogTemp, Warning, TEXT("PRUNE: (%d, %d) baglantisi koptugu icin silindi."), Coord.X, Coord.Y);
        
        // TODO: Burada UI'a veya GameMode'a "Kartı desteye ekle" sinyali göndermelisin.
        // ReturnTileToDeck(GridMap[Coord].TileID); 
        
        RemoveTile(Coord);
    }
}

void UGridSubsystem::RemoveTile(FGridCoordinate Coord)
{
    if (!GridMap.Contains(Coord)) return;
    
    // Core silinemez
    if (Coord == CorePoint) return;

    // Haritadan sil (Empty yap)
    FCellData EmptyData;
    EmptyData.CellType = ECellType::Empty;
    GridMap.Add(Coord, EmptyData);

    // Görsel güncelleme için event fırlat
    if (OnGridStateChanged.IsBound())
    {
        OnGridStateChanged.Broadcast(Coord, EmptyData);
    }

    // Bir taş silindikten sonra diğerlerinin bağlantısı koptu mu?
    PruneDisconnectedTiles();
}

bool UGridSubsystem::IsTileOccupied(const FGridCoordinate& Coord) const
{
    if (!IsValidCoordinate(Coord)) return false;
    if (!GridMap.Contains(Coord)) return false;
    return GridMap[Coord].CellType != ECellType::Empty;
}

bool UGridSubsystem::ForceReplaceTile(const FGridCoordinate& Coord, FName NewTileID)
{
    // Önce eskisi varsa iade et (RemoveTile içindeki mantık tetiklenmeden manuel iade)
    if (IsTileOccupied(Coord))
    {
        FName OldID = GridMap[Coord].TileID;
        // ReturnToDeck(OldID); // Kartı geri ver
    }
    
    // Şimdi direkt yerleştir (TryPlaceTile'daki kontrollerin bazılarını atlayarak)
    // Ancak Pathfinding (Spawn -> Core) hala kapanmamalı!
    
    // ... Buraya TryPlaceTile mantığının "Force" hali gelir ...
    // Şimdilik TryPlaceTile'ı kullanabiliriz, çünkü o zaten doluysa hata veriyor.
    // Biz dolu olduğunu bildiğimiz halde "Dolu Değilmiş" gibi davranıp logic kuracağız.
    
    return TryPlaceTile(Coord, NewTileID); // Not: TryPlaceTile'ı "Replace" parametresi alacak şekilde güncellemek daha temizdir.
}