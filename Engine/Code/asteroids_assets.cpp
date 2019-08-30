
internal
PLATFORM_WORK_QUEUE_CALLBACK(LoadAssetWork)
{
    load_asset_work *Work = (load_asset_work *)Data;
    
    loaded_bitmap Bitmap = DEBUGLoadBMP(DEBUGReadEntireFile, Work->FileName);
    
    if(Bitmap.Pixels)
    {
        asset_slot *Slot = Work->Assets->Bitmaps + (Work->ID - 3);
        Slot->Bitmap = Bitmap;
        Slot->State = AssetState_Loaded;
        PlatformPrintString("Thread loaded an asset", LogicalThreadIndex);
    }
}

internal void
LoadAsset(game_assets *Assets, game_asset_id ID, asset_slot *Slot)
{
    if(AtomicCompareExchangeUInt32((u32 *)&Slot->State, AssetState_Unloaded, AssetState_Queued) ==
       AssetState_Unloaded)
    {
        load_asset_work *Work = PushStruct(&Assets->Arena, load_asset_work);
        Work->Assets = Assets;
        Work->ID = ID;
        Work->Slot = Slot;
        Work->FinalState = AssetState_Loaded;
        
        switch(ID)
        {
            case GAI_SpriteRandom:
            {
                Work->FileName = "head.bmp";
            } break;
        }
        
        PlatformAddWorkQueueEntry(Assets->TranState->Queue, LoadAssetWork, Work);
    }
}

inline line_mesh *GetLineMesh(game_assets *Assets, game_asset_id ID)
{
    line_mesh *Result = Assets->LineMeshes + ID;
    
    return Result;
}

inline loaded_bitmap *GetBitmap(game_assets *Assets, game_asset_id ID)
{
    asset_slot *Slot = Assets->Bitmaps + (ID - 3);
    if(Slot->State == AssetState_Unloaded)
    {
        LoadAsset(Assets, ID, Slot);
    }
    
    return &Slot->Bitmap;
}

internal void
AllocateAssets(transient_state *TranState , memory_index Size)
{
    memory_arena *Arena = &TranState->TranArena;
    game_assets *Assets = PushStruct(Arena, game_assets);
    SubArena(&Assets->Arena, Arena, Size);
    
    Assets->TranState = TranState;
    
    Assets->BitmapCount = GAI_Count;
    Assets->Bitmaps = PushArray(Arena, Assets->BitmapCount, asset_slot);
    
    Assets->TagCount = 0;
    Assets->Tags = 0;
    
    Assets->AssetCount = Assets->BitmapCount;
    
}


internal u32
PickBest(u32 InfoCount, asset_bitmap_info *Infos, asset_tag *Tags,
         f32 *MatchVector, f32 *WeightVector)
{
    f32 BestDiff = Real32Maximum;
    u32 BestIndex = 0;
    for(u32 InfoIndex = 0;
        InfoIndex < InfoCount;
        ++InfoIndex)
    {
        asset_bitmap_info *Info = Infos + InfoIndex;
        
        f32 TotalWeightedDiff = 0.0f;
        for(u32 TagIndex = Info->FirstTagIndex;
            TagIndex < Info->OnePastLastTagIndex;
            ++TagIndex)
        {
            asset_tag *Tag = Tags + TagIndex;
            f32 Difference = MatchVector[Tag->ID] - Tag->Value;
            f32 Weighted = WeightVector[Tag->ID]*Absolute(Difference);
            TotalWeightedDiff += Weighted;
        }
        
        if(BestDiff > TotalWeightedDiff)
        {
            BestDiff = TotalWeightedDiff;
            BestIndex = InfoIndex;
        }
    }
    
    return BestIndex;
}