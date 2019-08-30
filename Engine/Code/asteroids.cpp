#include "asteroids.h"
#include "asteroids_assets.cpp"



extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
    PlatformAddWorkQueueEntry = Memory->PlatformAddWorkQueueEntry;
    PlatformDoNextWorkQueueEntry = Memory->PlatformDoNextWorkQueueEntry;
    PlatformCompleteAllWorkQueueWork = Memory->PlatformCompleteAllWorkQueueWork;
    
    u32 GameStateSize = sizeof(game_state);
    Assert( GameStateSize <= Memory->PermanentStorageSize);
    game_state *GameState = (game_state *)Memory->PermanentStorage;
    
    game_buffer *GameBuffer = &GameState->GameBuffer;
    world *World = GameState->GameWorld;
    
    f32 ScreenXInMeters = 20.0f;
    v2 ScreenDimensions = V2i(Memory->ScreenDimensionX, Memory->ScreenDimensionY);
    f32 ScreenYInMeters = ScreenXInMeters * (ScreenDimensions.y / ScreenDimensions.x);
    f32 MetersToPixels = ScreenDimensions.x / ScreenXInMeters;
    GameBuffer->MetersToPixels = MetersToPixels;
    GameBuffer->PixelsToMeters = 1.0f/MetersToPixels;
    v2 ScreenCenter  = V2(ScreenDimensions.x/2, ScreenDimensions.y/2);
    v2 ScreenCenterInMeters = ScreenCenter/MetersToPixels;
    GameState->CameraOffsetForFrame = V2(0,0);
    
    
    
    entity *PlayerEntity;
    
    if(!Memory->IsInitialized)
    {
        InitializeArena(&GameState->Arena, (Memory->PermanentStorageSize - sizeof(game_state)),
                        (u8 *)Memory->PermanentStorage + sizeof(game_state));
        
        
        GameState->GameWorld = PushStruct(&GameState->Arena, world);
        World = GameState->GameWorld;
        InitializeWorld(World, ScreenXInMeters, ScreenYInMeters);
        
        
        
        v2_i StartChunk = V2_i(0,0);
        world_chunk *Chunk = GetWorldChunk(World, StartChunk, &GameState->Arena);
        World->WorldCenter = ScreenCenterInMeters;
        GameState->NextID = 1;
        DEBUGReadEntireFile = Memory->DEBUGPlatformReadEntireFile;
        
        InitializeCamera(GameState,
                         V2(0,0), //-ScreenCenterInMeters,
                         V2(0,0), EntityType_Camera,
                         StartChunk);
        
        
        
        
        Memory->IsInitialized = true;
    }
    
    Assert(sizeof(transient_state) <= Memory->TransientStorageSize);
    transient_state *TranState = (transient_state *)Memory->TransientStorage;
    if(!TranState->IsInitialized)
    {
        InitializeArena(&TranState->TranArena, Memory->TransientStorageSize - sizeof(transient_state),
                        (u8 *)Memory->TransientStorage + sizeof(transient_state));
        
        
        AllocateAssets(TranState, Megabytes(32));
        
        v2_i StartChunk = V2_i(0,0);
        world_chunk *Chunk = GetWorldChunk(World, StartChunk, &GameState->Arena);
        
        PlayerEntity = Chunk->Entities + 
            AddEntity(World, GameState, 
                      V2(2.8f, 2.65f),
                      V2(0.0f,0.0f),
                      EntityType_Player,
                      (collision_flags)(Flag_Asteroid),
                      GAI_LineMeshPlayer,
                      HitboxType_LineMesh,
                      StartChunk,
                      true, true);
        
        entity *ImageEntity = Chunk->Entities + AddEntity(World, GameState, 
                                                          V2(5.0f, 5.0f),
                                                          V2(1.0f,1.0f),
                                                          EntityType_Sprite,
                                                          (collision_flags)(Flag_None),
                                                          GAI_SpriteRandom,
                                                          HitboxType_None,
                                                          StartChunk,
                                                          false, false);
        
        
        
        line_mesh PlayerLineMesh = {};
        
        line Line1 = {};
        line Line2 = {};
        line Line3 = {};
        line Line4 = {};
        line Line5 = {};
        line Line6 = {};
        PlayerEntity->LineMesh = GetLineMesh(TranState->Assets, GAI_LineMeshPlayer);
        PlayerEntity->LineMesh->LineCount = 3;
        PlayerLineMesh.LineCount = 3;
        PlayerEntity->Color = V4(1.0f, 1.0f, 1.0f, 1.0f);
        Line1.Start = V2(-0.15f, -0.25f);
        Line1.End = V2(0, 0.25f);
        Line1.Color = V4(1.0f,1,1,1.0f);
        Line1.Vector = Line1.End - Line1.Start;
        Line1.Normal = Perp(Line1.Vector);
        Line1.Normal = Line1.Normal*(1 / Length(Line1.Normal));
        
        Line2.Start = V2(0.0f, 0.25f);
        Line2.End = V2(0.15f, -0.25f);
        Line2.Color = V4(1.0f,1,1,1.0f),
        Line2.Vector = Line2.End - Line2.Start;
        Line2.Normal = Perp(Line2.Vector);
        Line2.Normal = Line2.Normal*(1 / Length(Line2.Normal));
        
        Line3.Start = V2(0.15f, -0.25f);
        Line3.End = V2(-0.15f, -0.25f);
        Line3.Color = V4(1.0f,1,1,1.0f),
        Line3.Vector = Line3.End - Line3.Start;
        Line3.Normal = Perp(Line3.Vector);
        Line3.Normal = Line3.Normal*(1 / Length(Line3.Normal));
        
        //PlayerEntity->LineMesh.Lines[0] = Line1;
        //PlayerEntity->LineMesh.Lines[1] = Line2;
        //PlayerEntity->LineMesh.Lines[2] = Line3;
        
        
        PlayerLineMesh.Lines[0] = Line1;
        PlayerLineMesh.Lines[1] = Line2;
        PlayerLineMesh.Lines[2] = Line3;
        
        *GetLineMesh(TranState->Assets, GAI_LineMeshPlayer) = PlayerLineMesh;
        
        GameState->AsteroidSpawnCount = 16;
        v2 *Spawns = PushArray(&GameState->Arena, GameState->AsteroidSpawnCount, v2);
        GameState->AsteroidSpawns = Spawns;
        
        f32 OverY = GameState->GameWorld->ChunkDim.y + 2.0f;
        f32 OverX = GameState->GameWorld->ChunkDim.x + 2.0f;
        
        Spawns[0] = V2(-2.0f,1.0f);
        Spawns[1] = V2(-2.0f,3.0f);
        Spawns[2] = V2(-2.0f,9.0f);
        Spawns[3] = V2(2.0f,-2.0f);
        Spawns[4] = V2(17.0f,-2.0f);
        Spawns[5] = V2(8.0f,-2.0f);
        Spawns[6] = V2(4.0f,-2.0f);
        Spawns[7] = V2(OverX,2.0f);
        Spawns[8] = V2(OverX,7.0f);
        Spawns[9] = V2(OverX,3.0f);
        Spawns[10] = V2(OverX,9.0f);
        Spawns[11] = V2(OverX,6.0f);
        Spawns[12] = V2(2.0f,OverY);
        Spawns[13] = V2(10.0f,OverY);
        Spawns[14] = V2(18.0f,OverY);
        Spawns[15] = V2(13.0f,OverY);
        Spawns[16] = V2(7.0f,OverY);
        
        CreateAsteroid(GameState, TranState->Assets, World, Chunk, AsteroidType_Big, (rand() % 1 + 0.5f));
        CreateAsteroid(GameState, TranState->Assets, World, Chunk, AsteroidType_Big, (rand() % 1 + 0.5f));
        CreateAsteroid(GameState, TranState->Assets, World, Chunk, AsteroidType_Big, (rand() % 1 + 0.5f));
        CreateAsteroid(GameState, TranState->Assets, World, Chunk, AsteroidType_Big, (rand() % 1 + 0.5f));
        CreateAsteroid(GameState, TranState->Assets, World, Chunk, AsteroidType_Big, (rand() % 1 + 0.5f));
        
        GameState->CameraOffsetForFrame = -ScreenCenterInMeters;
        GameState->GameBuffer.Width = Buffer->Width;
        GameState->GameBuffer.Height = Buffer->Height;
        GameState->GameBuffer.BytesPerPixel = 4;
        GameState->GameBuffer.Pitch = Buffer->Width * 4;
        GameState->GameBuffer.Memory = (u8 *)PushArray(&GameState->Arena,
                                                       Buffer->Width * Buffer->Height, u32);
        
        TranState->IsInitialized = true;
    }
    
    temporary_memory TempMem = BeginTemporaryMemory(&TranState->TranArena);
    
    EndTemporaryMemory(TempMem);
    
    ResetBufferBlack(GameBuffer);
    
    f32 dt = Input->dtForFrame;
    World->BulletCooldown += dt;
    
    if(World->BulletCooldown > 0.0f)
    {
        World->BulletCooldown = 0.0f;
    }
    world_chunk *CameraChunk = GetWorldChunk(World, GameState->CameraEntity.Chunk);
    
    // UPDATE VELOCITY
    for(u32 TestEntityIndex = 0;
        TestEntityIndex < CameraChunk->EntityCount;
        ++TestEntityIndex)
    {
        entity *TestEntity = CameraChunk->Entities + TestEntityIndex;
        if(TestEntity->Movable)
        {
            switch(TestEntity->EntityType)
            {
                case EntityType_Player:
                {
                    v2 PlayerddP = {};
                    f32 PlayerSpeed = 0.0f; // m/s^2
                    f32 Multiplier = 3.0f;
                    f32 RotationSpeed = 3.0f;
                    
                    if(Input->KeyboardInput->Space.EndedDown)
                    {
                        CreateBullet(GameState,TranState->Assets, World, CameraChunk, TestEntity);
                    }
                    
                    if(Input->KeyboardInput->Left.EndedDown)
                    {
                        TestEntity->ddR = -RotationSpeed;
                    }
                    
                    if(Input->KeyboardInput->Right.EndedDown)
                    {
                        TestEntity->ddR = RotationSpeed;
                    }
                    
                    if(Input->KeyboardInput->Up.EndedDown)
                    {
                        TestEntity->PlayerSpeed = Multiplier;
                    }
                    
                    if(Input->KeyboardInput->Down.EndedDown)
                    {
                        TestEntity->PlayerSpeed = -Multiplier;
                    }
                    
                    if(PlayerddP.x != 0.0f && PlayerddP.y != 0.0f)
                    {
                        f32 playerddPLength = SquareRoot(Inner(PlayerddP, PlayerddP));
                        TestEntity->ddP *= 1.0f / playerddPLength;
                    }
                    
                    f32 Boost = 14.0f;
                    
                    if(TestEntity->P.x < 2.0f)
                    {
                        TestEntity->ddP += Boost*V2(2.0f - Absolute(TestEntity->P.x), 0.0f);
                    }
                    
                    if(TestEntity->P.x > GameState->GameWorld->ChunkDim.x - 1.5f)
                    {
                        
                        TestEntity->ddP -=
                            Boost*V2(TestEntity->P.x - (GameState->GameWorld->ChunkDim.x - 1.5f), 0.0f);
                    }
                    
                    if(TestEntity->P.y < 2.0f)
                    {
                        TestEntity->ddP += Boost*V2(0.0f, 2.0f - Absolute(TestEntity->P.y));
                    }
                    
                    if(TestEntity->P.y > GameState->GameWorld->ChunkDim.y - 1.5f)
                    {
                        TestEntity->ddP -= 
                            Boost*V2(0.0f, TestEntity->P.y - (GameState->GameWorld->ChunkDim.y - 1.5f));
                    }
                } break;
            }
        }
    }
    
    if(CameraChunk->EntityCount == 1)
    {
        u32 asd = 5;
    }
    
    u32 CurrentEntityCount = 0;
    
    // MOVE ENTITIES
    for(u32 TestEntityIndex = 0;
        TestEntityIndex < CameraChunk->EntityCount;
        ++TestEntityIndex)
    {
        entity *TestEntity = CameraChunk->Entities + TestEntityIndex;
        CurrentEntityCount = CameraChunk->EntityCount;
        if(TestEntity->Movable)
        {
            switch(TestEntity->EntityType)
            {
                case EntityType_Player:
                {
                    MovePlayer(&GameState->Arena,TranState->Assets, World, GameState, CameraChunk, TestEntity, TestEntityIndex, dt);
                    CameraChunk = GetWorldChunk(World, GameState->CameraEntity.Chunk);
                }break;
                
                case EntityType_Asteroid:
                {
                    MoveAsteroid(GameState,TranState->Assets, World, CameraChunk, TestEntity->EntityID, dt);
                }break;
                
                case EntityType_Bullet:
                {
                    MoveBullet(GameState,TranState->Assets, World, CameraChunk, TestEntity->EntityID, dt);
                }break;
            }
        }
        
        if(CurrentEntityCount < CameraChunk->EntityCount)
        {
            --TestEntityIndex;
        }
    }
    
    
    // RENDER
    
    for(u32 TestEntityIndex = 0;
        TestEntityIndex < CameraChunk->EntityCount;
        ++TestEntityIndex)
    {
        entity *TestEntity = CameraChunk->Entities + TestEntityIndex;
        line_mesh *TestEntityLineMesh = 0;
        switch(TestEntity->EntityType)
        {
            case EntityType_Bullet:
            case EntityType_Player:
            case EntityType_Asteroid:
            {
                TestEntityLineMesh = GetLineMesh(TranState->Assets, TestEntity->GameAssetID);
                if(TestEntityLineMesh)
                {
                    if(TestEntity->EntityType == EntityType_Bullet)
                    {
                        if(TestEntity->CollisionEntityType == EntityType_Asteroid)
                        {
                            TestEntity->Color = V4(0,1,0,1);
                        }
                        else
                        {
                            TestEntity->Color = V4(1,1,1,1);
                        }
                    }
                    
                    
                    for(u32 LineIndex = 0;
                        LineIndex < TestEntityLineMesh->LineCount;
                        ++LineIndex)
                    {
                        line Line = TestEntityLineMesh->Lines[LineIndex];
                        v2 TempStart = (TestEntity->P + Line.Start.x*TestEntity->Basis.X +
                                        Line.Start.y*TestEntity->Basis.Y);
                        
                        v2 TempEnd = (TestEntity->P + Line.End.x*TestEntity->Basis.X + Line.End.y*TestEntity->Basis.Y);
                        
                        
                        DrawLine(GameBuffer, Memory,
                                 TempStart,
                                 TempEnd,
                                 TestEntity->Color,
                                 .75f);
                    }
                    //GameState->CameraOffsetForFrame += TestEntity->P - OldP;
                }
            } break;
            case EntityType_Sprite:
            {
                loaded_bitmap *Bitmap = GetBitmap(TranState->Assets, TestEntity->GameAssetID);
                if(Bitmap)
                {
                    RenderBitmap(GameBuffer, Bitmap, TestEntity->P, V2(0,0));
                }
                
            } break;
            
            case EntityType_Object:
            {
                for(u32 LineIndex = 0;
                    LineIndex < TestEntity->LineMesh->LineCount;
                    ++LineIndex)
                {
                    line Line = TestEntity->LineMesh->Lines[LineIndex];
                    DrawLine(GameBuffer, Memory,
                             (TestEntity->P + Line.Start.x*TestEntity->Basis.X + Line.Start.y*TestEntity->Basis.Y),
                             (TestEntity->P + Line.End.x*TestEntity->Basis.X + Line.End.y*TestEntity->Basis.Y),
                             Line.Color,
                             .75f);
                }
            } break;
        }
        
        if(TestEntity->EntityType != EntityType_Camera)
        {
            //TestEntity->P -= GameState->CameraOffsetForFrame;
            //RenderRect(GameBuffer, TestEntity->P,
            //           TestEntity->Dim, V4(1,1,1,1));
        }
    }
    
    RenderBufferToScreen(Buffer, GameBuffer);
    
    CheckMemory(&GameState->Arena);
    CheckMemory(&TranState->TranArena);
} 