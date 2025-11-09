#ifndef ENTITY_H
#define ENTITY_H

#include "Map.h"

enum Direction    { LEFT, RIGHT, JUMP, FALL, STANDING, L_ATTACK, R_ATTACK, U_ATTACK, BLITZ}; // For walking / Actions
enum EntityStatus { ACTIVE, INACTIVE                   };
enum EntityType   { PLAYER, BLOCK, PLATFORM, NPC, NONE };
enum AIType       { WANDERER, FOLLOWER, FLYER, BOSS    };
enum AIState      { WALKING, IDLE, FOLLOWING, ATTACK_L, ATTACK_R, ATTACK_U, WANDER };

class Entity
{
private:
    Vector2 mPosition;
    Vector2 mMovement;
    Vector2 mVelocity;
    Vector2 mAcceleration;
    Vector2 mOrigin = {0, 0};

    Vector2 mScale;
    Vector2 mColliderDimensions;
    
    Texture2D mTexture;
    Texture2D mIdle;
    TextureType mTextureType;
    Vector2 mSpriteSheetDimensions;
    Vector2 mIdleDimension;
    
    std::map<Direction, std::vector<int>> mAnimationAtlas;
    std::vector<int> mAnimationIndices;
    Direction mDirection;
    int mFrameSpeed;

    int mCurrentFrameIndex = 0;
    float mAnimationTime = 0.0f;
    float mInvincibilityTimer = 0.0f;

    Vector2 mSpawnPoint;
    bool mReturningToSpawn = false;

    bool mIsJumping = false;
    float mJumpingPower = 0.0f;

    int mSpeed;
    float mAngle;
    int mLives;
    bool mlookingLeft = false;

    float mAttackCooldown = 0.0f; 
    float mAttackCooldownTime = 4.0f;


    bool mInAnimation = false;
    bool mAttacking = false;


    bool mIsCollidingTop    = false;
    bool mIsCollidingBottom = false;
    bool mIsCollidingRight  = false;
    bool mIsCollidingLeft   = false;

    EntityStatus mEntityStatus = ACTIVE;
    EntityType   mEntityType;

    AIType  mAIType;
    AIState mAIState;

    bool isColliding(Entity *other) const;

    void checkCollisionY(std::vector<Entity> *collidableEntities, int collisionCheckCount);
    void checkCollisionY(Map *map);

    void checkCollisionX(std::vector<Entity> *collidableEntities, int collisionCheckCount);
    void checkCollisionX(Map *map);
    
    void resetColliderFlags() 
    {
        mIsCollidingTop    = false;
        mIsCollidingBottom = false;
        mIsCollidingRight  = false;
        mIsCollidingLeft   = false;
    }

    void animate(float deltaTime);
    void AIActivate(Entity *target);
    void AIWander();
    void AIFlyer();
    void AIFollow(Entity *target);
    void AIBoss(Entity *target);

public:
    static constexpr int   DEFAULT_SIZE          = 250;
    static constexpr int   DEFAULT_SPEED         = 200;
    static constexpr int   DEFAULT_FRAME_SPEED   = 20;
    static constexpr float Y_COLLISION_THRESHOLD = 0.5f;

    Entity();
    Entity(Vector2 position, Vector2 scale, const char *textureFilepath, 
        EntityType entityType);
    Entity(Vector2 position, Vector2 scale, const char *textureFilepath, 
        TextureType textureType, Vector2 spriteSheetDimensions, 
        std::map<Direction, std::vector<int>> animationAtlas, EntityType entityType,
        const char *idleTextureFilepath = nullptr, Vector2 idleDimension = {0,0});
    ~Entity();

    void update(float deltaTime, Entity *player, Map *map, 
        std::vector<Entity> *collidableEntities, int collisionCheckCount);
    void render();
    void normaliseMovement() { Normalise(&mMovement); }

    void jump()       { mIsJumping = true;  }
    void activate()   { mEntityStatus  =    ACTIVE;   }
    void deactivate() { mEntityStatus  = INACTIVE; }
    void displayCollider();

    bool isActive() { return mEntityStatus == ACTIVE ? true : false; }

    void moveLeft()  { mMovement.x = -1.8f; mDirection = LEFT;  }
    void moveRight() { mMovement.x = 1.8f; mDirection = RIGHT;  }
    void blitz(Map *map);


    void attackLeft() { mMovement.x = -5.0f; mDirection = L_ATTACK; mCurrentFrameIndex = 0; }
    void attackRight() {mMovement.x = 5.0f; mDirection = R_ATTACK; mCurrentFrameIndex = 0; }
    void attackUp() {  mVelocity.y -= 600.0f; mDirection = U_ATTACK; mCurrentFrameIndex = 0; }

    void resetMovement() { mMovement = { 0.0f, 0.0f }; }

    Vector2     getPosition()              const { return mPosition;              }
    Vector2     getMovement()              const { return mMovement;              }
    Vector2     getVelocity()              const { return mVelocity;              }
    Vector2     getAcceleration()          const { return mAcceleration;          }
    Vector2     getScale()                 const { return mScale;                 }
    Vector2     getColliderDimensions()    const { return mColliderDimensions;    }
    Vector2     getSpriteSheetDimensions() const { return mSpriteSheetDimensions; }
    Vector2     getSpawn()                 const { return mSpawnPoint;            }
    Texture2D   getTexture()               const { return mTexture;               }
    TextureType getTextureType()           const { return mTextureType;           }
    Direction   getDirection()             const { return mDirection;             }
    int         getLives()                 const { return mLives;                 }
    int         getFrameSpeed()            const { return mFrameSpeed;            }
    float       getJumpingPower()          const { return mJumpingPower;          }
    bool        isJumping()                const { return mIsJumping;             }
    int         getSpeed()                 const { return mSpeed;                 }
    float       getAngle()                 const { return mAngle;                 }
    EntityType  getEntityType()            const { return mEntityType;            }
    AIType      getAIType()                const { return mAIType;                }
    AIState     getAIState()               const { return mAIState;               }
    

    
    bool isCollidingTop()    const { return mIsCollidingTop;    }
    bool isCollidingBottom() const { return mIsCollidingBottom; }

    std::map<Direction, std::vector<int>> getAnimationAtlas() const { return mAnimationAtlas; }

    void setPosition(Vector2 newPosition)
        { mPosition = newPosition;                 }
    void setFrame(int frame)
        { mCurrentFrameIndex = frame;              }
    void setMovement(Vector2 newMovement)
        { mMovement = newMovement;                 }
    void setVelocity(Vector2 velocity)
        { mVelocity = velocity;                    }
    void setAcceleration(Vector2 newAcceleration)
        { mAcceleration = newAcceleration;         }
    void setScale(Vector2 newScale)
        { mScale = newScale;                       }
    void setTexture(const char *textureFilepath)
        { mTexture = LoadTexture(textureFilepath); }
    void setColliderDimensions(Vector2 newDimensions) 
        { mColliderDimensions = newDimensions;     }
    void setSpriteSheetDimensions(Vector2 newDimensions) 
        { mSpriteSheetDimensions = newDimensions;  }
    void setSpeed(int newSpeed)
        { mSpeed  = newSpeed;                      }
    void setFrameSpeed(int newSpeed)
        { mFrameSpeed = newSpeed;                  }
    void setJumpingPower(float newJumpingPower)
        { mJumpingPower = newJumpingPower;         }
    void setAngle(float newAngle) 
        { mAngle = newAngle;                       }
    void setEntityType(EntityType entityType)
        { mEntityType = entityType;                }
    void setDirection(Direction newDirection)
    { 
        mDirection = newDirection;

        if (mTextureType == ATLAS) mAnimationIndices = mAnimationAtlas.at(mDirection);
    }
    void setAIState(AIState newState)
        { mAIState = newState;                     }
    void setAIType(AIType newType)
        { mAIType = newType;                       }
    void setCoolDown(float cooldown)      
        { mAttackCooldownTime = cooldown;          }
    void setOrigin(Vector2 origin)
        { mOrigin = origin;                        }
    void setLives(int lives)
        { mLives = lives;                          }
    void setSpawn(Vector2 spawn)
        { mSpawnPoint = spawn;
          mReturningToSpawn = true; 
        }
    void resetAttack(){
        mAttacking = false;
        mInAnimation = false;
    }
};

#endif // ENTITY_CPP