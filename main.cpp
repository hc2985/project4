/**
* Author: Hyeonung Cho
* Assignment: Rise of the AI
* Date due: 2025-11-08, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "CS3113/LevelD.h"


// Global Constants
constexpr int SCREEN_WIDTH     = 1000,
              SCREEN_HEIGHT    = 600,
              FPS              = 120,
              NUMBER_OF_LEVELS = 5;

constexpr char BG_COLOUR[] = "#011627";
constexpr char LIGHT_COLOUR[] = "#9db0e1ff";
constexpr Vector2 ORIGIN   = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
            
constexpr float FIXED_TIMESTEP = 1.0f / 60.0f;

// Global Variables
AppStatus gAppStatus   = RUNNING;
float gPreviousTicks   = 0.0f,
      gTimeAccumulator = 0.0f;

Scene *gCurrentScene = nullptr;
std::vector<Scene*> gLevels = {};

StartMenu *gMenu = nullptr;
LevelA *gLevelA = nullptr;
LevelB *gLevelB = nullptr;
LevelC *gLevelC = nullptr;
LevelD *gLevelD = nullptr;

// Global variable to preserve health across levels
int gPlayerHealth = MAX_HEALTH;

// Function Declarations
void switchToScene(Scene *scene);
void initialise();
void processInput();
void update();
void render();
void renderUI();
void shutdown();

void switchToScene(Scene *scene)
{   
    if (gCurrentScene != nullptr && gCurrentScene->getState().player != nullptr) {
        gPlayerHealth = gCurrentScene->getState().player->getLives();
    }

    // Switch
    gCurrentScene = scene;
    gCurrentScene->initialise();

    if (gCurrentScene->getState().player != nullptr) {
        gCurrentScene->getState().player->setLives(gPlayerHealth);
    }
}


void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Cursed Forest");
    InitAudioDevice();

    gMenu = new StartMenu(ORIGIN, LIGHT_COLOUR);
    gLevelA = new LevelA(ORIGIN, BG_COLOUR);
    gLevelB = new LevelB(ORIGIN, BG_COLOUR);
    gLevelC = new LevelC(ORIGIN, BG_COLOUR);
    gLevelD = new LevelD(ORIGIN, BG_COLOUR);

    gLevels.push_back(gMenu);
    gLevels.push_back(gLevelA);
    gLevels.push_back(gLevelB);
    gLevels.push_back(gLevelC);
    gLevels.push_back(gLevelD);

    switchToScene(gLevels[0]);

    SetTargetFPS(FPS);
}

void processInput() 
{
    
    if (gCurrentScene == gLevels[0]) {
        if (IsKeyPressed(KEY_ENTER)) gCurrentScene->getState().nextSceneID = 1;
        if (IsKeyPressed(KEY_Q) || WindowShouldClose()) gAppStatus = TERMINATED;
        return;
    }


    gCurrentScene->getState().player->resetMovement();

    if      (IsKeyDown(KEY_A)) gCurrentScene->getState().player->moveLeft();
    else if (IsKeyDown(KEY_D)) gCurrentScene->getState().player->moveRight();
    
    if (IsKeyDown(KEY_SPACE)) gCurrentScene->getState().player->blitz(gCurrentScene->getState().map);

    if (IsKeyPressed(KEY_W) && 
        gCurrentScene->getState().player->isCollidingBottom())
    {
        gCurrentScene->getState().player->jump();
        PlaySound(gCurrentScene->getState().jumpSound);
    }

    if (GetLength(gCurrentScene->getState().player->getMovement()) > 1.0f) 
        gCurrentScene->getState().player->normaliseMovement();

    if (IsKeyPressed(KEY_Q) || WindowShouldClose()) gAppStatus = TERMINATED;
}

void update() 
{
    if (gCurrentScene != gLevels[0]) {
        Entity *player = gCurrentScene->getState().player;
        if (player) {
            int livesNow = player->getLives();
            if (livesNow < gPlayerHealth) {
                PlaySound(gCurrentScene->getState().damage); 
                if (livesNow <= 0) {                    
                    gCurrentScene->getState().player->setLives(MAX_HEALTH);
                    switchToScene(gLevels[0]);
                } else {
                    gPlayerHealth = livesNow;
                }
            }
        }
    }
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
        gCurrentScene->update(FIXED_TIMESTEP);
        deltaTime -= FIXED_TIMESTEP;
    }

    gTimeAccumulator = deltaTime;
}

void renderUI()
{
    if (gCurrentScene == gLevels[0]) return;

    int heartSize = 50;
    int spacing = 10;
    int startX = SCREEN_WIDTH - (heartSize * MAX_HEALTH) - (spacing * (MAX_HEALTH - 1)) - 20;
    int startY = 20;

    Vector2 HEART_DIMENSIONS = { 1, 5 };
    
    int playerLives = gCurrentScene->getState().player->getLives();
    
    for (int i = 0; i < MAX_HEALTH; i++) 
    {
        Rectangle sourceRect;
        
        if (i < playerLives) {
            // Full heart
            sourceRect = getUVRectangle(
                &gCurrentScene->getState().heartTexture,
                0,      
                HEART_DIMENSIONS.x, 
                HEART_DIMENSIONS.y      
            );
        } else {
            // Empty heart
            sourceRect = getUVRectangle(
                &gCurrentScene->getState().heartTexture,
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
        
        DrawTexturePro(gCurrentScene->getState().heartTexture, sourceRect, destRect, 
                      {0, 0}, 0.0f, WHITE);
    }
}

void render()
{
    BeginDrawing();
    ClearBackground(ColorFromHex(BG_COLOUR));

    if (gCurrentScene != gLevels[0]) {
        BeginMode2D(gCurrentScene->getState().camera);
    }

    gCurrentScene->render();

    if (gCurrentScene != gLevels[0]) {
        EndMode2D();
    }

    renderUI();

    EndDrawing();
}

void shutdown() 
{
    delete gMenu;
    delete gLevelA;
    delete gLevelB;
    delete gLevelC;
    delete gLevelD;

    for (int i = 0; i < NUMBER_OF_LEVELS; i++) gLevels[i] = nullptr;

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

        // Check for scene transitions
        int nextID = gCurrentScene->getState().nextSceneID;
        if (nextID >= 0) {
            switchToScene(gLevels[nextID]);

            gCurrentScene->getState().nextSceneID = -1;
        }
        render();
    }

    shutdown();

    return 0;
}