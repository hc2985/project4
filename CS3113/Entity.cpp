#include "Entity.h"

Entity::Entity() : mPosition {0.0f, 0.0f}, mMovement {0.0f, 0.0f}, 
                   mVelocity {0.0f, 0.0f}, mAcceleration {0.0f, 0.0f},
                   mScale {DEFAULT_SIZE, DEFAULT_SIZE},
                   mColliderDimensions {DEFAULT_SIZE, DEFAULT_SIZE}, 
                   mTexture {NULL}, mTextureType {SINGLE}, mAngle {0.0f},
                   mSpriteSheetDimensions {}, mDirection {LEFT}, 
                   mAnimationAtlas {{}}, mAnimationIndices {}, mFrameSpeed {0},
                   mEntityType {NONE} { }

Entity::Entity(Vector2 position, Vector2 scale, const char *textureFilepath, 
    EntityType entityType) : mPosition {position}, mVelocity {0.0f, 0.0f}, 
    mAcceleration {0.0f, 0.0f}, mScale {scale}, mMovement {0.0f, 0.0f}, 
    mColliderDimensions {scale}, mTexture {LoadTexture(textureFilepath)}, 
    mTextureType {SINGLE}, mDirection {LEFT}, mAnimationAtlas {{}}, 
    mAnimationIndices {}, mFrameSpeed {0}, mSpeed {DEFAULT_SPEED}, 
    mAngle {0.0f}, mEntityType {entityType} { }

Entity::Entity(Vector2 position, Vector2 scale, const char *textureFilepath,
    TextureType textureType, Vector2 spriteSheetDimensions,
    std::map<Direction, std::vector<int>> animationAtlas, EntityType entityType,
    const char *idleTextureFilepath, Vector2 idleDimension) :
        mPosition {position}, mVelocity {0.0f, 0.0f}, 
        mAcceleration {0.0f, 0.0f}, mMovement { 0.0f, 0.0f }, mScale {scale},
        mColliderDimensions {scale}, mTexture {LoadTexture(textureFilepath)}, 
        mIdle {LoadTexture(idleTextureFilepath)}, mIdleDimension {idleDimension},
        mTextureType {ATLAS}, mSpriteSheetDimensions {spriteSheetDimensions},
        mAnimationAtlas {animationAtlas}, mDirection {LEFT},
        mAnimationIndices {animationAtlas.at(LEFT)}, 
        mFrameSpeed {DEFAULT_FRAME_SPEED}, mAngle { 0.0f }, 
        mSpeed { DEFAULT_SPEED }, mEntityType {entityType} { }

Entity::~Entity() { UnloadTexture(mTexture); };

void Entity::checkCollisionY(std::vector<Entity> *collidableEntities, int collisionCheckCount)
{
    for (int i = 0; i < collisionCheckCount; i++)
    {
        // STEP 1: For every entity that our player can collide with...
        Entity *collidableEntity = &collidableEntities->at(i);
        
        if (isColliding(collidableEntity))
        {
            // STEP 2: Calculate the distance between its centre and our centre
            //         and use that to calculate the amount of overlap between
            //         both bodies.
            float yDistance = fabs(mPosition.y - collidableEntity->mPosition.y);
            float yOverlap  = fabs(yDistance - (mColliderDimensions.y / 2.0f) - 
                              (collidableEntity->mColliderDimensions.y / 2.0f));
            
            // STEP 3: "Unclip" ourselves from the other entity, and zero our
            //         vertical velocity.     

            if (mVelocity.y > 0) 
            {
                mPosition.y -= yOverlap;
                mVelocity.y  = 0;
                mIsCollidingBottom = true;

                if (collidableEntity->mEntityType == BLOCK)
                    collidableEntity->deactivate();
                        
            } else if (mVelocity.y < 0) 
            {
                mPosition.y += yOverlap;
                mVelocity.y  = 0;
                mIsCollidingTop = true;
                if (mEntityType == PLAYER) {
                    mLives--;
                    if (collidableEntity->getAIType() == BOSS) {
                        collidableEntity->setPosition(collidableEntity->getSpawn());
                        collidableEntity->setVelocity({0.0f, 0.0f});
                        collidableEntity->resetAttack();
                        collidableEntity->setFrame(0);
                        collidableEntity->setAIState(IDLE);
                    }
                }
            }
        }
    }
}

void Entity::checkCollisionX(std::vector<Entity> *collidableEntities, int collisionCheckCount)
{
    for (int i = 0; i < collisionCheckCount; i++)
    {
        Entity *collidableEntity = &collidableEntities->at(i);
        
        if (isColliding(collidableEntity))
        {            
            // When standing on a platform, we're always slightly overlapping
            // it vertically due to gravity, which causes false horizontal
            // collision detections. So the solution I dound is only resolve X
            // collisions if there's significant Y overlap, preventing the 
            // platform we're standing on from acting like a wall.
            float yDistance = fabs(mPosition.y - collidableEntity->mPosition.y);
            float yOverlap  = fabs(yDistance - (mColliderDimensions.y / 2.0f) - (collidableEntity->mColliderDimensions.y / 2.0f));

            // Skip if barely touching vertically (standing on platform)
            if (yOverlap < Y_COLLISION_THRESHOLD) continue;

            float xDistance = fabs(mPosition.x - collidableEntity->mPosition.x);
            float xOverlap  = fabs(xDistance - (mColliderDimensions.x / 2.0f) - (collidableEntity->mColliderDimensions.x / 2.0f));

            if (mVelocity.x >= 0) {
                mPosition.x     -= xOverlap;
                mVelocity.x      = 0;
                // Collision!
                mIsCollidingRight = true;

                if (mEntityType == PLAYER) {
                    mLives--;
                    if (collidableEntity->getAIType() == BOSS) {
                        collidableEntity->setPosition(collidableEntity->getSpawn());
                        collidableEntity->setVelocity({0.0f, 0.0f});
                        collidableEntity->resetAttack();
                        collidableEntity->setFrame(0);
                        collidableEntity->setAIState(IDLE);
                    }
                }
            } else if (mVelocity.x < 0) {
                mPosition.x    += xOverlap;
                mVelocity.x     = 0;
 
                // Collision!
                mIsCollidingLeft = true;
                if (mEntityType == PLAYER) {
                    mLives--;
                    if (collidableEntity->getAIType() == BOSS) {
                        collidableEntity->setPosition(collidableEntity->getSpawn());
                        collidableEntity->setVelocity({0.0f, 0.0f});
                        collidableEntity->resetAttack();
                        collidableEntity->setFrame(0);
                        collidableEntity->setAIState(IDLE);
                    }
                }
            }
        }
    }
}

void Entity::checkCollisionY(Map *map)
{
    if (map == nullptr) return;

    Vector2 topCentreProbe    = { mPosition.x, mPosition.y - (mColliderDimensions.y / 2.0f) };
    Vector2 topLeftProbe      = { mPosition.x - (mColliderDimensions.x / 2.0f), mPosition.y - (mColliderDimensions.y / 2.0f) };
    Vector2 topRightProbe     = { mPosition.x + (mColliderDimensions.x / 2.0f), mPosition.y - (mColliderDimensions.y / 2.0f) };

    Vector2 bottomCentreProbe = { mPosition.x, mPosition.y + (mColliderDimensions.y / 2.0f) };
    Vector2 bottomLeftProbe   = { mPosition.x - (mColliderDimensions.x / 2.0f), mPosition.y + (mColliderDimensions.y / 2.0f) };
    Vector2 bottomRightProbe  = { mPosition.x + (mColliderDimensions.x / 2.0f), mPosition.y + (mColliderDimensions.y / 2.0f) };

    float xOverlap = 0.0f;
    float yOverlap = 0.0f;

    // COLLISION ABOVE (jumping upward)
    if ((map->isSolidTileAt(topCentreProbe, &xOverlap, &yOverlap) ||
         map->isSolidTileAt(topLeftProbe, &xOverlap, &yOverlap)   ||
         map->isSolidTileAt(topRightProbe, &xOverlap, &yOverlap)) 
         && mVelocity.y < 0.0f && xOverlap < map->getTileSize() * 0.5f)
    {
        printf("Stopping\n");
        mPosition.y += yOverlap * 1.01f;   // push down
        mVelocity.y  = 0.0f;
        mIsCollidingTop = true;
    }

    // COLLISION BELOW (falling downward)
    if ((map->isSolidTileAt(bottomCentreProbe, &xOverlap, &yOverlap) ||
         map->isSolidTileAt(bottomLeftProbe, &xOverlap, &yOverlap)   ||
         map->isSolidTileAt(bottomRightProbe, &xOverlap, &yOverlap)) && mVelocity.y > 0.0f)
    {
        mPosition.y -= yOverlap * 1.01f;   // push up
        mVelocity.y  = 0.0f;
        mIsCollidingBottom = true;
    } 
}

void Entity::checkCollisionX(Map *map)
{
    if (map == nullptr) return;

    Vector2 leftCentreProbe   = { mPosition.x - (mColliderDimensions.x / 2.0f), mPosition.y };
    Vector2 leftTopProbe      = { mPosition.x - (mColliderDimensions.x / 2.0f), mPosition.y - (mColliderDimensions.y / 2.0f) };
    Vector2 leftBottomProbe   = { mPosition.x - (mColliderDimensions.x / 2.0f), mPosition.y + (mColliderDimensions.y / 2.0f) };

    Vector2 rightCentreProbe  = { mPosition.x + (mColliderDimensions.x / 2.0f), mPosition.y };
    Vector2 rightTopProbe     = { mPosition.x + (mColliderDimensions.x / 2.0f), mPosition.y - (mColliderDimensions.y / 2.0f) };
    Vector2 rightBottomProbe  = { mPosition.x + (mColliderDimensions.x / 2.0f), mPosition.y + (mColliderDimensions.y / 2.0f) };

    float xOverlap = 0.0f;
    float yOverlap = 0.0f;

    // COLLISION ON RIGHT (moving right)
    if ((map->isSolidTileAt(rightCentreProbe, &xOverlap, &yOverlap) ||
         map->isSolidTileAt(rightTopProbe, &xOverlap, &yOverlap)    ||
         map->isSolidTileAt(rightBottomProbe, &xOverlap, &yOverlap)) 
         && mVelocity.x > 0.0f && yOverlap >= 0.5f)
    {
        mPosition.x -= xOverlap * 1.01f;   // push left
        mVelocity.x  = 0.0f;
        mIsCollidingRight = true;
    }

    // COLLISION ON LEFT (moving left)
    if ((map->isSolidTileAt(leftCentreProbe, &xOverlap, &yOverlap) ||
         map->isSolidTileAt(leftTopProbe, &xOverlap, &yOverlap)    ||
         map->isSolidTileAt(leftBottomProbe, &xOverlap, &yOverlap)) 
         && mVelocity.x < 0.0f && yOverlap >= 0.5f)
    {
        mPosition.x += xOverlap * 1.01;   // push right
        mVelocity.x  = 0.0f;
        mIsCollidingLeft = true;
    }
}

bool Entity::isColliding(Entity *other) const 
{
    if (!other->isActive() || other == this) return false;

    float xDistance = fabs(mPosition.x - other->getPosition().x) - 
        ((mColliderDimensions.x + other->getColliderDimensions().x) / 2.0f);
    float yDistance = fabs(mPosition.y - other->getPosition().y) - 
        ((mColliderDimensions.y + other->getColliderDimensions().y) / 2.0f);

    if (xDistance < 0.0f && yDistance < 0.0f) return true;

    return false;
}

void Entity::animate(float deltaTime)
{
    
    if (mDirection == LEFT || mDirection == RIGHT || mDirection == STANDING){
        mAnimationIndices = mAnimationAtlas.at(mDirection == RIGHT ? LEFT : mDirection);
    } else if (mDirection == U_ATTACK || mDirection == R_ATTACK || mDirection == L_ATTACK){
       mAnimationIndices = mAnimationAtlas.at(L_ATTACK);
    }

    mAnimationTime += deltaTime;
    float framesPerSecond = 0.8f / mFrameSpeed;

    if (mAnimationTime >= framesPerSecond)
    {
        mAnimationTime = 0.0f;
        mCurrentFrameIndex++;

        if (mCurrentFrameIndex >= mAnimationIndices.size() && (mDirection == L_ATTACK || mDirection == R_ATTACK || mDirection == U_ATTACK)){
            mInAnimation = false;
            mCurrentFrameIndex = 0;

            if (mDirection == L_ATTACK){
                mDirection = LEFT;
            }     
            else {
                mDirection = RIGHT; 
            }

            mAnimationIndices = mAnimationAtlas.at(mDirection == RIGHT ? LEFT : mDirection);

        }
        if (!mAnimationIndices.empty()){
            mCurrentFrameIndex %= mAnimationIndices.size();
        }
            
    }
}


void Entity::blitz(Map *map)
{ 
    if (mAttackCooldown > 0.0f) return;

    const float maxDistance = 200.0f;
    float step = 10.0f; 

    float dir = (mDirection == LEFT) ? -1.0f : 1.0f;

    Vector2 testPos = mPosition;
    Vector2 newPos = mPosition;

    for (float d = 1; d <= 20; d ++) {
        testPos.x = mPosition.x + dir * d * step;
        float Overlap = 0.0f;
        if (map->isSolidTileAt(testPos, &Overlap, &Overlap)) {
            newPos.x = mPosition.x + dir * (d*step);
            break;
        }
        newPos.x = testPos.x;
    }
    mPosition = newPos;

    mAttackCooldown = mAttackCooldownTime;
}


void Entity::AIWander() {  
        // Turn around when hitting walls
    if (mIsCollidingRight) {
        moveLeft();
    } 
    else if (mIsCollidingLeft) {
        moveRight();
    }
    // If not colliding with anything, keep current direction
    else if (mDirection == LEFT) {
        moveLeft();
    }
    else {
        moveRight();
    }                                
}

void Entity::AIFollow(Entity *target)
{
    switch (mAIState)
    {
    case IDLE:
        mMovement = {0.0f, 0.0f};
        if (Vector2Distance(mPosition, target->getPosition()) < 200.0f) 
            mAIState = WALKING;
        break;

    case WALKING:
        // Depending on where the player is in respect to their x-position
        // Change direction of the enemy
        if (Vector2Distance(mPosition, target->getPosition()) > 300.0f) {
            mAIState = IDLE;
            mMovement = {0.0f, 0.0f};
        } else if (mPosition.x > target->getPosition().x) moveLeft();
        else moveRight();
        break;                                     

    default:
        break;
    }
}

void Entity::AIFlyer()
{
    mAcceleration.y = 0.0f;
    if (mIsCollidingRight) {
        moveLeft();
    } 
    else if (mIsCollidingLeft) {
        moveRight();
    }
    // If not colliding with anything, keep current direction
    else if (mDirection == LEFT) {
        moveLeft();
    }
    else {
        moveRight();
    }  

}


void Entity::AIBoss(Entity *target)
{
    float dx = target->getPosition().x - mPosition.x; //how left boss is from player for positive
    float dy = target->getPosition().y - mPosition.y; //how up boss is from player for positive

    if (fabs(dx) > 150.0f || fabs(dy) > 150.0f) {
        setSpeed(160);  
    } else {
        setSpeed(110);    
    }
    
    switch (mAIState)
    {
    case IDLE:
        mMovement = {0.0f, 0.0f};
        if (Vector2Distance(mPosition, target->getPosition()) < 600.0f) 
            mAIState = WALKING;
        break;
    case WALKING:
        if (mIsCollidingBottom && dy > 150.0f) {
            mAIState = WANDER;
        }
        if (mAttackCooldown > 0.0f) {
            if (dx < 0.0f) {
                moveLeft();
            } else {
                moveRight();
            }
            break;
        }
        if (dy < 0.0f && dy > -250.0f && dx >= -80.0f && dx <= 80.0f) {
            mAIState = ATTACK_U;
            break;
        }
        if (dx > 0.0f && dx <= 200.0f && dy >= -100.0f && dy <= 100.0f) {
            mAIState = ATTACK_R;
            break;
        }
        if (dx < 0.0f && -dx <= 200.0f && dy >= -100.0f && dy <= 100.0f) {
            mAIState = ATTACK_L;
            break;
        }
        if (dx < 0.0f) {
            moveLeft();
        } else {
            moveRight();
        }
        break;

    case WANDER:
        if (dy <= 150.0f) {
            mAIState = WALKING;
            break;
        }
        if (mIsCollidingRight) {
            moveLeft();
        } 
        else if (mIsCollidingLeft) {
            moveRight();
        }
        // If not colliding with anything, keep current direction
        else if (mDirection == LEFT) {
            moveLeft();
        }
        else {
            moveRight();
        }
        break;
            
    case ATTACK_L:
        if (!mAttacking && !mInAnimation){
            mAttacking = true;
            mInAnimation = true;
            attackLeft();
            setFrameSpeed(5);
        } else if (!mInAnimation && mAttacking) {
            mAIState = WALKING;
            mAttacking = false;
            mAttackCooldown = mAttackCooldownTime;
            setFrameSpeed(10);
        }
        break;
    case ATTACK_R:
        if (!mAttacking && !mInAnimation){
            mAttacking = true;
            mInAnimation = true;
            mlookingLeft = true;
            attackRight();
            setFrameSpeed(5);
        } else if (!mInAnimation && mAttacking) {
            mAIState = WALKING;
            mAttacking = false;
            mAttackCooldown = mAttackCooldownTime;
            setFrameSpeed(10);
        }
        break;
    case ATTACK_U:
        if (!mAttacking && !mInAnimation){
            mAttacking = true;
            mInAnimation = true;
            attackUp();
            setFrameSpeed(5);
        } else if (!mInAnimation && mAttacking) {
            mAIState = WALKING;
            mAttacking = false;
            mAttackCooldown = mAttackCooldownTime;
            setFrameSpeed(10);
        }
        break;
    default:
        break;
    }
    
}


void Entity::AIActivate(Entity *target)
{
    switch (mAIType)
    {
    case WANDERER:
        AIWander();
        break;
    case FOLLOWER:
        AIFollow(target);
        break;
    case FLYER:
        AIFlyer();
        break;
    case BOSS:
        AIBoss(target);
        break;
    default:
        break;
    }
}

void Entity::update(float deltaTime, Entity *player, Map *map, 
    std::vector <Entity> *collidableEntities, int collisionCheckCount)
{
    if (mEntityStatus == INACTIVE) return;
    

    if (mAttackCooldown > 0.0f) {
        mAttackCooldown -= deltaTime;
    }
    
    if (mEntityType == NPC) AIActivate(player);

    resetColliderFlags();

    if(mMovement.x == 0 && mMovement.y == 0 && mIdleDimension.x > 0){
        if (mDirection != STANDING) {
            if (mDirection == LEFT){
                mlookingLeft = true;
            }
            mDirection = STANDING;
            mCurrentFrameIndex = 0; 
        }
    }

    mVelocity.x = mMovement.x * mSpeed;

    mVelocity.x += mAcceleration.x * deltaTime;
    mVelocity.y += mAcceleration.y * deltaTime;

    // ––––– JUMPING ––––– //
    if (mIsJumping)
    {
        // STEP 1: Immediately return the flag to its original false state
        mIsJumping = false;
        
        // STEP 2: The player now acquires an upward velocity
        mVelocity.y -= mJumpingPower;
    }

    mPosition.x += mVelocity.x * deltaTime;
    checkCollisionX(collidableEntities, collisionCheckCount);
    checkCollisionX(map);
    
    mPosition.y += mVelocity.y * deltaTime;
    checkCollisionY(collidableEntities, collisionCheckCount);
    checkCollisionY(map);    

    if (mTextureType == ATLAS){
        if (mEntityType == NPC || mIsCollidingBottom) {
            animate(deltaTime);
        }
    }
    
        
}

void Entity::render()
{
    if(mEntityStatus == INACTIVE) return;

    Rectangle textureArea;

    switch (mTextureType)
    {
        case SINGLE:
            // Whole texture (UV coordinates)
            textureArea = {
                // top-left corner
                0.0f, 0.0f,

                // bottom-right corner (of texture)
                static_cast<float>(mTexture.width),
                static_cast<float>(mTexture.height)
            };
            break;
        case ATLAS:
            textureArea = getUVRectangle(
                &mTexture, 
                mAnimationIndices[mCurrentFrameIndex], 
                mSpriteSheetDimensions.x, 
                mSpriteSheetDimensions.y
            );     

        default: break;
    }

    

    // Destination rectangle – centred on gPosition
    Rectangle destinationArea = {
        mPosition.x,
        mPosition.y,
        static_cast<float>(mScale.x),
        static_cast<float>(mScale.y)
    };
    
    Vector2 originOffset;
    // Origin inside the source texture (centre of the texture)
    if (mOrigin.x == 0 && mOrigin.y == 0){
        originOffset = {
            static_cast<float>(mScale.x) / 2.0f,
            static_cast<float>(mScale.y) / 2.0f //1.7f for main character
        };
    }else{
        originOffset = mOrigin;
    }

    if (mDirection == STANDING && mIdleDimension.x > 0){
        destinationArea.width  *= 0.6f;
        destinationArea.height *= 0.6f;

        destinationArea.x += 20.0f;

        originOffset.x *= 0.6f;  // Scale origin with the sprite
        originOffset.y *= 0.5f; 

        textureArea = getUVRectangle(
            &mIdle, 
            mAnimationIndices[mCurrentFrameIndex], 
            mIdleDimension.x, 
            mIdleDimension.y
        );
        

        if (mlookingLeft) {
            textureArea.width = -textureArea.width;  // flip horizontally
            destinationArea.x -= 40.0f;
        }

        DrawTexturePro(
            mIdle, 
            textureArea, destinationArea, originOffset,
            mAngle, WHITE
        );

        return;
    }else{
        mlookingLeft = false;
    }
    

    if (mDirection == RIGHT) {
        textureArea.width = -textureArea.width;  // flip horizontally
    }

    // Render the texture on screen
    DrawTexturePro(
        mTexture, 
        textureArea, destinationArea, originOffset,
        mAngle, WHITE
    );

    // displayCollider();
}

void Entity::displayCollider() 
{
    // draw the collision box
    Rectangle colliderBox = {
        mPosition.x - mColliderDimensions.x / 2.0f,  
        mPosition.y - mColliderDimensions.y / 2.0f,  
        mColliderDimensions.x,                        
        mColliderDimensions.y                        
    };

    DrawRectangleLines(
        colliderBox.x,      // Top-left X
        colliderBox.y,      // Top-left Y
        colliderBox.width,  // Width
        colliderBox.height, // Height
        GREEN               // Color
    );
}