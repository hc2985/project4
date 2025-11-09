#include "StartMenu.h"

StartMenu::StartMenu()
    : Scene({0.0f}, nullptr) {}

StartMenu::StartMenu(Vector2 origin, const char *bgHexCode)
    : Scene(origin, bgHexCode) {}

StartMenu::~StartMenu() {
    shutdown();
}

void StartMenu::initialise() {
    mGameState.nextSceneID = 0;
    mGameState.map = nullptr;
    mGameState.player = nullptr;
    mGameState.enemies = nullptr;
}

void StartMenu::update(float deltaTime) {
}

void StartMenu::render() {
    DrawText("CURSED FOREST \n PRESS ENTER TO START", 300, 280, 30, WHITE);
}


void StartMenu::shutdown() {

}