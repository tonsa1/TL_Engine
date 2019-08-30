#if !defined(ASTEROIDS_ASSETS_H)

enum game_asset_id
{
    GAI_LineMeshPlayer,
    GAI_LineMeshAsteroid,
    GAI_LineMeshBullet,
    GAI_SpriteRandom,
    
    GAI_Count,
};

enum asset_state
{
    AssetState_Unloaded,
    AssetState_Queued, 
    AssetState_Loaded,
    AssetState_Locked,
};

struct asset_slot
{
    asset_state State;
    loaded_bitmap Bitmap;
};

enum asset_tag_id
{
    Tag_Random,
};

struct asset_tag
{
    game_asset_id ID;
    u32 Value;
};

struct asset_type
{
    u32 Count;
    u32 FirstAssetIndex;
    u32 OnePastLastAssetIndex;
};

struct asset_bitmap_info
{
    v2 AlignPercentage;
    f32 WidthOverWidth;
    u32 Width;
    u32 Height;
    
    u32 FirstTagIndex;
    u32 OnePastLastTagIndex;
};

struct asset_group
{
    u32 FirstTagIndex;
    u32 OnePastLastTagIndex;
};

struct asset
{
    u32 FirstTagIndex;
    u32 OnePAstLastTagIndex;
    u32 SlotID;
};

struct game_assets
{
    memory_arena Arena;
    struct transient_state *TranState;
    
    line_mesh LineMeshes[GAI_Count];
    
    u32 BitmapCount;
    
    asset_slot *Bitmaps;
    
    u32 AssetCount;
    asset *Assets;
    
    u32 TagCount;
    asset_tag *Tags;
};

struct load_asset_work
{
    game_assets *Assets;
    char* FileName;
    game_asset_id ID;
    asset_slot *Slot;
    
    b32 HasAlignment;
    u32 AlignX;
    u32 TopDownAlignY;
    
    asset_state FinalState;
};

#define ASTEROIDS_ASSETS_H
#endif