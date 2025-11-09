#include "LevelD.h"

LevelD::LevelD() : Scene { {0.0f}, nullptr } {}
LevelD::LevelD(Vector2 origin, const char *bgHexCode) : Scene { origin, bgHexCode } {}
LevelD::~LevelD() { shutdown(); }

void LevelD::initialise()
{

    mGameState.nextSceneID = -1;

    mGameState.spawnPoint = {mOrigin.x-200, mOrigin.y -     650.0f};

    mGameState.bgm = LoadMusicStream("assets/game/song.mp3");
    SetMusicVolume(mGameState.bgm, 0.33f);
    PlayMusicStream(mGameState.bgm);
    mGameState.jumpSound = LoadSound("assets/game/step.mp3");
    mGameState.damage = LoadSound("assets/game/damage.mp3");
    mGameState.win = LoadSound("assets/game/win.mp3");

    mPlayerWon = false;
    mWinTimer  = 5.0f;
    mWinDelay  = 5.0f; 

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

    mGameState.player->setJumpingPower(600.0f);
    mGameState.player->setFrameSpeed(20);
    mGameState.player->setAcceleration({0.0f, ACCELERATION_OF_GRAVITY});

    mGameState.player->setLives(MAX_HEALTH);

    /*
        ----------- Enemy -----------
    */

    Vector2 ENEMY_ATLAS_DIMENSIONS = { 6, 6 };

    std::map<Direction, std::vector<int>> TrashAnimationAtlas = {
        {LEFT, { 0, 1, 2, 3, 4, 5, 6 }},
        {L_ATTACK, {25,26,27,28,29,30}}
    };

    mGameState.enemies = new std::vector<Entity>();
    mGameState.enemies->reserve(1);

    Vector2 BossSpawnPoint = {mOrigin.x+400, mOrigin.y-400};

    mGameState.enemies->emplace_back(
        BossSpawnPoint,
        Vector2{200.0f * sizeRatio, 200.0f},
        "assets/trash/Sprite2.png",
        ATLAS,
        ENEMY_ATLAS_DIMENSIONS,
        TrashAnimationAtlas,
        NPC
    );
    mGameState.enemies->at(0).setSpawn(BossSpawnPoint);
    mGameState.enemies->at(0).setAIType(BOSS);
    mGameState.enemies->at(0).setAIState(IDLE);
    mGameState.enemies->at(0).setSpeed(110);
    mGameState.enemies->at(0).setAcceleration({0.0f, ACCELERATION_OF_GRAVITY});
    mGameState.enemies->at(0).setOrigin({static_cast<float>(mGameState.enemies->at(0).getScale().x) / 2.0f, static_cast<float>(mGameState.enemies->at(0).getScale().y) / 2.5f});
    mGameState.enemies->at(0).setColliderDimensions({
        mGameState.enemies->at(0).getScale().x * 0.8f,
        mGameState.enemies->at(0).getScale().y * 0.45f
    });
    mGameState.enemies->at(0).setCoolDown(1.5f);
    mGameState.enemies->at(0).setFrameSpeed(10);
    mGameState.enemies->at(0).activate();
    
    /*
        ----------- CAMERA -----------
    */
    mGameState.camera = { 0 };
    mGameState.camera.target = mGameState.player->getPosition();
    mGameState.camera.offset = mOrigin;
    mGameState.camera.rotation = 0.0f;
    mGameState.camera.zoom = 1.0f;
}

void LevelD::update(float deltaTime)
{
    int livesBefore = mGameState.player->getLives();

    UpdateMusicStream(mGameState.bgm);

   
    mGameState.player->update(
        deltaTime,
        nullptr,
        mGameState.map,
        mGameState.enemies,
        1
    );

    for (int i = 0; i < (int)mGameState.enemies->size(); ++i)
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
        PlaySound(mGameState.damage);
        mGameState.player->setPosition(mGameState.spawnPoint);
    }

    Vector2 currentPlayerPosition = mGameState.player->getPosition();
    if (!mPlayerWon){
        panCamera(&mGameState.camera, &currentPlayerPosition);
    }


    if (!mPlayerWon && mGameState.player->getPosition().y > END_GAME_THRESHOLD) {
        PlaySound(mGameState.win);
        mPlayerWon = true;
        mWinTimer  = mWinDelay;
    }

    if (mPlayerWon) {
        mWinTimer -= deltaTime;
        if (mWinTimer <= 0.0f) {
            mGameState.nextSceneID = 0; // main menu
        }
    }
}


void LevelD::render()
{
    if (mPlayerWon) {
        DrawText("You Win!", 300, 280, 30, WHITE);
        mGameState.camera.target = mOrigin;       
        mGameState.camera.offset = mOrigin;        
        mGameState.camera.zoom   = 1.0f;
        mGameState.camera.rotation = 0.0f;
    }
    ClearBackground(ColorFromHex(mBGColourHexCode));

    mGameState.player->render();
    
    for (int i = 0; i < 1; i++)
    {
        mGameState.enemies->at(i).render();
    }
    
    mGameState.map->render();


}

void LevelD::shutdown()
{
    delete mGameState.player;
    delete mGameState.enemies; 
    delete mGameState.map;

    UnloadTexture(mGameState.heartTexture);
    UnloadMusicStream(mGameState.bgm);
    UnloadSound(mGameState.jumpSound);
}