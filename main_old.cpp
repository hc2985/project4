#include "CS3113/Entity.h"

struct GameState
{
    Entity *player;
    std::vector<Entity> *enemies;
    Map *map;
    Music bgm;
    Sound jumpSound;
    Camera2D camera;

    Texture2D heartTexture;
    Vector2 spawnPoint;
};

// Global Constants
constexpr int SCREEN_WIDTH  = 1000,
              SCREEN_HEIGHT = 600,
              FPS           = 120;

constexpr char    BG_COLOUR[]      = "#011627";
constexpr Vector2 ORIGIN           = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 },
                  ATLAS_DIMENSIONS = { 11,12 },
                  IDLE_DIMENSIONS = { 1,18 };


constexpr int   NUMBER_OF_TILES         = 20,
                NUMBER_OF_BLOCKS        = 3;
constexpr float TILE_DIMENSION          = 75.0f,
                // in m/ms², since delta time is in ms
                ACCELERATION_OF_GRAVITY = 981.0f,
                FIXED_TIMESTEP          = 1.0f / 60.0f,
                END_GAME_THRESHOLD      = 800.0f;

constexpr int LEVEL_WIDTH  = 24,
              LEVEL_HEIGHT = 18;

constexpr int MAX_HEALTH = 3;

// Global Variables
AppStatus gAppStatus   = RUNNING;
float gPreviousTicks   = 0.0f,
      gTimeAccumulator = 0.0f;

unsigned int gLevelData[] = {
    4 , 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4,
    11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,25, 0,11,
    11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,25, 0, 0, 0, 0, 0, 1, 3, 0, 0, 0, 0,11,
    11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,15,17, 0, 0, 0, 0,11,
    11, 0, 0, 0, 0, 0,25, 0, 0,25, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,25, 0,11,
    11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,25, 0, 0, 0, 0, 0, 0, 0, 0,11,
    11, 0, 0,25, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 3, 0, 0, 0,11,
    11, 0, 0, 0, 0,25, 0, 0, 0, 0, 0,25, 0, 0, 0, 0,15,16,16,17, 0, 0, 0,11,
    11, 0, 0, 0, 0, 0, 1, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,37,
    11, 0, 0, 0,25, 0,15,16,17, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,15,30,
    11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,25, 0, 0, 0, 0, 0, 0, 0, 0, 0,25, 0,11,
    11, 0, 0, 0, 0, 0, 0, 0, 0,25, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,11,
    11, 0, 1, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 2, 2, 2, 2,37,
    11, 0,15,17, 0,25, 0,25, 0, 0,25, 0, 0, 0, 0, 0,15,16,16,16,16,16,16,30,
    11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,11,
    36, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,37,
    8 , 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,10,
    15,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,17
};


GameState gState;

// Function Declarations
void initialise();
void processInput();
void update();
void render();
void shutdown();

void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Maps");
    InitAudioDevice();

    gState.spawnPoint = {ORIGIN.x - 620.0f, ORIGIN.y + 200.0f};

    gState.bgm = LoadMusicStream("assets/game/aura.mp3");
    SetMusicVolume(gState.bgm, 0.33f);
    PlayMusicStream(gState.bgm);

    gState.jumpSound = LoadSound("assets/game/Dirt Jump.wav");

    /* 
       ----------- Health -------------
    */

    gState.heartTexture = LoadTexture("assets/hearts.png");

    /*
        ----------- MAP -----------
    */
    gState.map = new Map(
        LEVEL_WIDTH, LEVEL_HEIGHT,   // map grid cols & rows
        (unsigned int *) gLevelData, // grid data
        "assets/moss/TileSet.png",   // texture filepath
        TILE_DIMENSION,              // tile size
        7, 7,                        // texture cols & rows
        ORIGIN                       // in-game origin
    );

    /*
        ----------- PROTAGONIST -----------
    */
    std::map<Direction, std::vector<int>> playerAnimationAtlas = {
        {LEFT,  {  1,  2,  3,  4,  5,  6,  7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24}},
        {FALL,  {  36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47 }},
        {STANDING,  {  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 }}  
    };

    float sizeRatio  = 52.0f / 64.0f;


    // Assets from @see https://sscary.itch.io/the-adventurer-female
    gState.player = new Entity(
        gState.spawnPoint, // position
        {250.0f * sizeRatio, 250.0f},           // scale
        "assets/Character/base/redhood.png",                 // texture file address
        ATLAS,                                  // single image or atlas?
        ATLAS_DIMENSIONS,                       // atlas dimensions
        playerAnimationAtlas,                  // actual atlas
        PLAYER,                                  // entity type
        "assets/Character/idle/idlesheet.png",
        IDLE_DIMENSIONS
    );

    gState.player->setColliderDimensions({
        gState.player->getScale().x * 0.1f,  // width
        gState.player->getScale().y * 0.2f    // height
    });

    gState.player->setOrigin({static_cast<float>(gState.player->getScale().x) / 2.0f, static_cast<float>(gState.player->getScale().y) / 1.7f});

    gState.player->setJumpingPower(600.0f);
    gState.player->setAcceleration({0.0f, ACCELERATION_OF_GRAVITY});

    gState.player->setLives(MAX_HEALTH);

    /*
        ----------- Enemy -----------
    */

    Vector2 ENEMY_ATLAS_DIMENSIONS = { 5, 6 };

    std::map<Direction, std::vector<int>> EnemyAnimationAtlas = {
        {LEFT, { 0, 1, 2, 3, 4, 5 }}
    };

    gState.enemies = new std::vector<Entity>();
    gState.enemies->reserve(2);  

    gState.enemies->emplace_back(
        Vector2{ORIGIN.x + 300.0f, ORIGIN.y + 400.0f},
        Vector2{130.0f * sizeRatio, 130.0f},
        "assets/trash/Sprite.png",
        ATLAS,
        ENEMY_ATLAS_DIMENSIONS,
        EnemyAnimationAtlas,
        NPC
    );

    gState.enemies->at(0).setAIType(WANDERER);
    gState.enemies->at(0).setAIState(WALKING);
    gState.enemies->at(0).setSpeed(110);
    gState.enemies->at(0).setAcceleration({0.0f, ACCELERATION_OF_GRAVITY});
    gState.enemies->at(0).setOrigin({static_cast<float>(gState.enemies->at(0).getScale().x) / 2.0f, static_cast<float>(gState.enemies->at(0).getScale().y) / 3.2f});
    gState.enemies->at(0).setColliderDimensions({
        gState.enemies->at(0).getScale().x * 0.8f,
        gState.enemies->at(0).getScale().y * 0.5f
    });
    gState.enemies->at(0).setFrameSpeed(10);

    gState.enemies->at(0).activate();

    // second enemy

    ENEMY_ATLAS_DIMENSIONS = { 1, 7 };

    EnemyAnimationAtlas = {
        {LEFT, { 0, 1, 2, 3, 4, 5, 6 }}
    };

    gState.enemies->emplace_back(
        Vector2{ORIGIN.x + 300.0f, ORIGIN.y - 410.0f},
        Vector2{120.0f * sizeRatio, 120.0f},
        "assets/Bat/fly.png",
        ATLAS,
        ENEMY_ATLAS_DIMENSIONS,
        EnemyAnimationAtlas,
        NPC
    );

    gState.enemies->at(1).setAIType(FLYER);
    gState.enemies->at(1).setAIState(WALKING);
    gState.enemies->at(1).setSpeed(120);
    gState.enemies->at(1).setAcceleration({0.0f, 0.0f});
    gState.enemies->at(1).setOrigin({static_cast<float>(gState.enemies->at(1).getScale().x)/2, static_cast<float>(gState.enemies->at(1).getScale().y) / 2.5f});
    gState.enemies->at(1).setColliderDimensions({
        gState.enemies->at(1).getScale().x * 0.5f,
        gState.enemies->at(1).getScale().y * 0.5f
    });
    gState.enemies->at(1).setFrameSpeed(10);

    gState.enemies->at(1).activate();



    /*
        ----------- CAMERA -----------
    */
    gState.camera = { 0 };                                // zero initialize
    gState.camera.target = gState.player->getPosition(); // camera follows player
    gState.camera.offset = ORIGIN;                        // camera offset to center of screen
    gState.camera.rotation = 0.0f;                        // no rotation
    gState.camera.zoom = 1.0f;                            // default zoom

    SetTargetFPS(FPS);
}

void processInput() 
{
    gState.player->resetMovement();

    if      (IsKeyDown(KEY_A)) gState.player->moveLeft();
    else if (IsKeyDown(KEY_D)) gState.player->moveRight();

    if (IsKeyPressed(KEY_W) && gState.player->isCollidingBottom())
    {
        gState.player->jump();
        PlaySound(gState.jumpSound);
    }

    if (GetLength(gState.player->getMovement()) > 1.0f) 
        gState.player->normaliseMovement();

    if (IsKeyPressed(KEY_Q) || WindowShouldClose()) gAppStatus = TERMINATED;
}

void update() 
{
    // Delta time
    float ticks = (float) GetTime();
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks  = ticks;

    // Fixed timestep
    deltaTime += gTimeAccumulator;

    if (deltaTime < FIXED_TIMESTEP)
    {
        gTimeAccumulator = deltaTime;
        return;
    }

    while (deltaTime >= FIXED_TIMESTEP)
    {

        int livesBefore = gState.player->getLives();


        UpdateMusicStream(gState.bgm);

        gState.player->update(
            FIXED_TIMESTEP,
            nullptr,
            gState.map,
            gState.enemies,
            2
        );

        gState.enemies->at(0).update(
            FIXED_TIMESTEP,
            gState.player,
            gState.map,
            nullptr,       // Enemy doesn't collide with other enemies yet
            0              // 0 count
        );

        gState.enemies->at(1).update(
            FIXED_TIMESTEP,
            gState.player,
            gState.map,
            nullptr,       // Enemy doesn't collide with other enemies yet
            0              // 0 count
        );

        int livesAfter = gState.player->getLives();
        if (livesAfter < livesBefore) { //Respawn
            gState.player->setPosition(gState.spawnPoint);
        }

        deltaTime -= FIXED_TIMESTEP;

        Vector2 currentPlayerPosition = gState.player->getPosition();

        panCamera(&gState.camera, &currentPlayerPosition);

        if (gState.player->getPosition().y > 800.0f) gAppStatus = TERMINATED;
    }
}

void renderUI()
{

    int heartSize = 50;
    int spacing = 10;
    int startX = SCREEN_WIDTH - (heartSize * MAX_HEALTH) - (spacing * (MAX_HEALTH - 1)) - 20;
    int startY = 20;

    Vector2 HEART_DIMENSIONS = { 1, 5 };
    
    int playerLives = gState.player->getLives();
    
    for (int i = 0; i < MAX_HEALTH; i++) 
    {
        Rectangle sourceRect;
        
        if (i < playerLives) {
            //full heart
            sourceRect = getUVRectangle(
                &gState.heartTexture,
                0,      
                HEART_DIMENSIONS.x, 
                HEART_DIMENSIONS.y      
            );
        } else {
            //empty heart
            sourceRect = getUVRectangle(
                &gState.heartTexture,
                4,     
                HEART_DIMENSIONS.x,     
                HEART_DIMENSIONS.y       
            );
        }
        
        Rectangle destRect = {
            static_cast<float>(startX + (heartSize + spacing) * i),
            static_cast<float>(startY),
            static_cast<float>(heartSize),
            static_cast<float>(heartSize)
        };
        
        DrawTexturePro(gState.heartTexture, sourceRect, destRect, 
                      {0, 0}, 0.0f, WHITE);
    }
}

void render()
{
    BeginDrawing();
    ClearBackground(ColorFromHex(BG_COLOUR));

    BeginMode2D(gState.camera);

    gState.player->render();
    gState.enemies->at(0).render();
    gState.enemies->at(1).render();
    gState.map->render();

    //gState.player->displayCollider();
    //gState.enemies->at(0).displayCollider();
    //gState.enemies->at(1).displayCollider();

    EndMode2D();

    renderUI();

    EndDrawing();
}

void shutdown() 
{
    delete gState.player;
    delete gState.enemies; 
    delete gState.map;

    UnloadTexture(gState.heartTexture);
    UnloadMusicStream(gState.bgm);
    UnloadSound(gState.jumpSound);

    CloseAudioDevice();
    CloseWindow();
}

int main(void)
{
    initialise();

    while (gAppStatus == RUNNING)
    {
        processInput();
        update();
        render();
    }

    shutdown();

    return 0;
}