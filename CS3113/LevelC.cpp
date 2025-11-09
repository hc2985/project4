#include "LevelC.h"

LevelC::LevelC() : Scene { {0.0f}, nullptr } {}
LevelC::LevelC(Vector2 origin, const char *bgHexCode) : Scene { origin, bgHexCode } {}

LevelC::~LevelC() { shutdown(); }

void LevelC::initialise()
{
    mGameState.nextSceneID = 3;

    mGameState.spawnPoint = {mOrigin.x - 600.0f, mOrigin.y + 450.0f};

    mGameState.bgm = LoadMusicStream("assets/game/song.mp3");
    SetMusicVolume(mGameState.bgm, 0.33f);
    PlayMusicStream(mGameState.bgm);
    mGameState.damage = LoadSound("assets/game/damage.mp3");
    mGameState.win = LoadSound("assets/game/win.mp3");
    mGameState.jumpSound = LoadSound("assets/game/step.mp3");

    /* 
       ----------- Health -------------
    */
    mGameState.heartTexture = LoadTexture("assets/hearts.png");

    /*
        ----------- MAP -----------
    */
    mGameState.map = new Map(
        LEVEL_WIDTH, LEVEL_HEIGHT,
        (unsigned int *) mLevelData,
        "assets/moss/TileSet.png",
        TILE_DIMENSION,
        7, 7,
        mOrigin
    );

    /*
        ----------- PROTAGONIST -----------
    */
    constexpr Vector2 ATLAS_DIMENSIONS = { 11,12 };

    std::map<Direction, std::vector<int>> playerAnimationAtlas = {
        {LEFT,  {  1,  2,  3,  4,  5,  6,  7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24}},
        {FALL,  {  36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47 }},
        {STANDING,  {  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 }}  
    };

    constexpr Vector2 IDLE_DIMENSIONS = { 1,18 };
    float sizeRatio  = 52.0f / 64.0f;

    mGameState.player = new Entity(
        mGameState.spawnPoint,
        {250.0f * sizeRatio, 250.0f},
        "assets/Character/base/redhood.png",
        ATLAS,
        ATLAS_DIMENSIONS,
        playerAnimationAtlas,
        PLAYER,
        "assets/Character/idle/idlesheet.png",
        IDLE_DIMENSIONS
    );

    mGameState.player->setColliderDimensions({
        mGameState.player->getScale().x * 0.1f,
        mGameState.player->getScale().y * 0.2f
    });

    mGameState.player->setOrigin({static_cast<float>(mGameState.player->getScale().x) / 2.0f, static_cast<float>(mGameState.player->getScale().y) / 1.7f});
    mGameState.player->setFrameSpeed(20);
    mGameState.player->setJumpingPower(600.0f);
    mGameState.player->setAcceleration({0.0f, ACCELERATION_OF_GRAVITY});

    mGameState.player->setLives(MAX_HEALTH);

    /*
        ----------- Enemy -----------
    */

    Vector2 ENEMY_ATLAS_DIMENSIONS = { 1, 7 };

    std::map<Direction, std::vector<int>> BatAnimationAtlas = {
        {LEFT, { 0, 1, 2, 3, 4, 5, 6 }}
    };

    mGameState.enemies = new std::vector<Entity>();
    mGameState.enemies->reserve(3);

    mGameState.enemies->emplace_back(
        Vector2{mOrigin.x - 300.0f, mOrigin.y - 170.0f},
        Vector2{120.0f * sizeRatio, 120.0f},
        "assets/Bat/fly.png",
        ATLAS,
        ENEMY_ATLAS_DIMENSIONS,
        BatAnimationAtlas,
        NPC
    );

    mGameState.enemies->at(0).setAIType(FLYER);
    mGameState.enemies->at(0).setAIState(WALKING);
    mGameState.enemies->at(0).setSpeed(110);
    mGameState.enemies->at(0).setAcceleration({0.0f, 0.0f});
    mGameState.enemies->at(0).setOrigin({static_cast<float>(mGameState.enemies->at(0).getScale().x)/2, static_cast<float>(mGameState.enemies->at(0).getScale().y) / 2.0f});
    mGameState.enemies->at(0).setColliderDimensions({
        mGameState.enemies->at(0).getScale().x * 0.5f,
        mGameState.enemies->at(0).getScale().y * 0.3f
    });
    mGameState.enemies->at(0).setFrameSpeed(10);
    mGameState.enemies->at(0).activate();

    mGameState.enemies->emplace_back(
        Vector2{mOrigin.x, mOrigin.y - 80.0f},
        Vector2{120.0f * sizeRatio, 120.0f},
        "assets/Bat/fly.png",
        ATLAS,
        ENEMY_ATLAS_DIMENSIONS,
        BatAnimationAtlas,
        NPC
    );

    mGameState.enemies->at(1).setAIType(FLYER);
    mGameState.enemies->at(1).setAIState(WALKING);
    mGameState.enemies->at(1).setSpeed(110);
    mGameState.enemies->at(1).setAcceleration({0.0f, 0.0f});
    mGameState.enemies->at(1).setOrigin({static_cast<float>(mGameState.enemies->at(1).getScale().x)/2, static_cast<float>(mGameState.enemies->at(1).getScale().y) / 2.0f});
    mGameState.enemies->at(1).setColliderDimensions({
        mGameState.enemies->at(1).getScale().x * 0.5f,
        mGameState.enemies->at(1).getScale().y * 0.3f
    });
    mGameState.enemies->at(1).setFrameSpeed(10);
    mGameState.enemies->at(1).activate();

    ENEMY_ATLAS_DIMENSIONS = { 6, 6 };

    std::map<Direction, std::vector<int>> TrashAnimationAtlas = {
        {LEFT, { 0, 1, 2, 3, 4, 5 }}
    };

    mGameState.enemies->emplace_back(
        Vector2{mOrigin.x, mOrigin.y},
        Vector2{130.0f * sizeRatio, 130.0f},
        "assets/trash/Sprite.png",
        ATLAS,
        ENEMY_ATLAS_DIMENSIONS,
        TrashAnimationAtlas,
        NPC
    );

    mGameState.enemies->at(2).setAIType(WANDERER);
    mGameState.enemies->at(2).setAIState(WALKING);
    mGameState.enemies->at(2).setSpeed(110);
    mGameState.enemies->at(2).setAcceleration({0.0f, ACCELERATION_OF_GRAVITY});
    mGameState.enemies->at(2).setOrigin({static_cast<float>(mGameState.enemies->at(2).getScale().x) / 2.0f, static_cast<float>(mGameState.enemies->at(2).getScale().y) / 2.5f});
    mGameState.enemies->at(2).setColliderDimensions({
        mGameState.enemies->at(2).getScale().x * 0.8f,
        mGameState.enemies->at(2).getScale().y * 0.45f
    });
    mGameState.enemies->at(2).setFrameSpeed(10);
    mGameState.enemies->at(2).activate();
    /*
        ----------- CAMERA -----------
    */
    mGameState.camera = { 0 };
    mGameState.camera.target = mGameState.player->getPosition();
    mGameState.camera.offset = mOrigin;
    mGameState.camera.rotation = 0.0f;
    mGameState.camera.zoom = 1.0f;
}

void LevelC::update(float deltaTime)
{
    int livesBefore = mGameState.player->getLives();

    UpdateMusicStream(mGameState.bgm);

    mGameState.player->update(
        deltaTime,
        nullptr,
        mGameState.map,
        mGameState.enemies,
        3
    );

    for (int i = 0; i < 3; i++)
    {
        mGameState.enemies->at(i).update(
            deltaTime,
            mGameState.player,
            mGameState.map,
            nullptr,
            0
        );
    }

    int livesAfter = mGameState.player->getLives();
    if (livesAfter < livesBefore) {
        mGameState.player->setPosition(mGameState.spawnPoint);
    }

    Vector2 currentPlayerPosition = mGameState.player->getPosition();
    panCamera(&mGameState.camera, &currentPlayerPosition);

    if (mGameState.player->getPosition().y > END_GAME_THRESHOLD) 
        mGameState.nextSceneID = 4;
}

void LevelC::render()
{
    ClearBackground(ColorFromHex(mBGColourHexCode));

    mGameState.player->render();
    
    for (int i = 0; i < 3; i++)
    {
        mGameState.enemies->at(i).render();
    }
    
    mGameState.map->render();
}

void LevelC::shutdown()
{
    delete mGameState.player;
    delete mGameState.enemies; 
    delete mGameState.map;

    UnloadTexture(mGameState.heartTexture);
    UnloadMusicStream(mGameState.bgm);
    UnloadSound(mGameState.jumpSound);
}