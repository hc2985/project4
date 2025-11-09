#ifndef SCENE_H
#define SCENE_H

#include "Entity.h"

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
    Sound damage;
    Sound win;
    
    int nextSceneID;
};

class Scene 
{
protected:
    GameState mGameState;
    Vector2 mOrigin;
    const char *mBGColourHexCode = "#000000";
    
public:
    Scene();
    Scene(Vector2 origin, const char *bgHexCode);
    
    virtual void initialise() = 0;
    virtual void update(float deltaTime) = 0;
    virtual void render() = 0;
    virtual void shutdown() = 0;
    
    GameState&   getState()                { return mGameState; }
    Vector2     getOrigin()          const { return mOrigin;    }
    const char* getBGColourHexCode() const { return mBGColourHexCode; }
};

#endif // SCENE_H