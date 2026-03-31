#include "pong.h"
#include <string>

// Implementações das classes e funções

// Particle
void Particle::init() {
    speedX = rand() % 10, speedY = rand() % 10 * -1;
    r = 0.81, g = 0.07, b = 0.07;
}

void Particle::updatePosition() {
    if (abs(speedX) >= 3.5) {
        if (speedX < 0) speedX += gravityX / 10;
        else speedX -= gravityX / 10;
    } else speedX = 0;
    speedY += gravityY / 10;
    x += speedX;
    y += speedY;
    clr = C2D_Color32f(r, g, b, aliveTime / 60.0);
    aliveTime--;
}

void Particle::Draw() {
    if (aliveTime <= 0) {
        numParticles--;
        return;
    }
    C2D_DrawTriangle(x, y, clr, x + size, y, clr, x + (size / 2), y - size, clr, 0);
}

// Ball
bool Ball::CheckCollision(int colX1, int colX2, int colY1, int colY2) {
    if ((x >= colX1) && (x <= colX2) && (y >= colY1) && (y <= colY2)) {
        if (scored) return false;
        if (x < 200) speedX = abs(speedX);
        else {
            speedX = -abs(speedX);
            enemyHit = true;
        }
        boost = 1.5;
        ballHit = true;
        UpdatePosition();
        return true;
    }
    return false;
}

void Ball::Draw() {
    float sizeX = radius * 2 + abs(((x - 200) / 20));
    float sizeY = radius * 2 + abs(((y - 120) / 30));
    C2D_DrawEllipseSolid(x - (sizeX / 2) - ((x - 200) / 10), y - (sizeY / 2) - ((y - 120) / 10), 0, sizeX, sizeY, clrShadow);
    C2D_DrawCircleSolid(x, y, 0, radius, clrWhite);
}

void Ball::UpdatePosition() {
    scored = false;
    x += speedX * boost;
    y += speedY * boost;
    if (speedX != 0) speedXStore = speedX;
    if (speedY != 0) speedYStore = speedY;
    if (y + radius >= 240 || y - radius <= 0) {
        if (y + 120 < 0) speedY = abs(speedY);
        else speedY *= -1;
    }
    if (x + radius >= 400 || x - radius <= 0) {
        size_t amount = numParticles + 20;
        for (size_t i = numParticles; i < amount; i++) {
            Particle* ptcle = &particles[i];
            ptcle->aliveTime = 60;
            ptcle->x = x + rand() % 40 - 20;
            ptcle->y = y + rand() % 40 - 20;
            ptcle->init();
            if (x - 200 > 0) {
                ptcle->speedX *= -1;
                ptcle->r = 0.4, ptcle->g = 0.69, ptcle->b = 0.83;
            }
            numParticles++;
        }
        speedX *= -1;
        scored = true;
    }
    if (boost > 1) boost -= 0.015;
    else if (boost < 1) boost += 0.015;
    if (abs(boost - 1) < 0.05) boost = 1;
    clrWhite = C2D_Color32f(1, 1 - (boost - 1), 1, 1);
}

int Ball::sign(float num) {
    return (num >= 0) ? 1 : -1;
}

// Paddle
void Paddle::CheckPosition() {
    if (y + height > 240 || y < 0) y -= speedY;
}

void Paddle::getCollisionPoints() {
    colX1 = (x - 200 <= 0) ? x - 15 : x;
    colX2 = x + width;
    colY1 = y;
    colY2 = y + height;
}

void Paddle::Draw() {
    C2D_DrawRectangle(x + 3 * sign(x - 200), y + 3, 0, width + 2, height + 2, clrShadow, clrShadow, clrShadow, clrShadow);
    C2D_DrawRectangle(x, y, 0, width, height, clrWhite, clrWhite, clrWhite, clrWhite);
}

void Paddle::UpdatePosition() {
    y += speedY;
    CheckPosition();
    getCollisionPoints();
}

int Paddle::sign(float num) {
    return (num >= 0) ? 1 : -1;
}

// CpuPaddle
void CpuPaddle::UpdatePosition(int ballPosition, float ballSpeed) {
    if (ballSpeed > 0) {
        if (ballPosition < y + (height / 2)) speedY = -(speed) + speedDegredation;
        else speedY = speed - speedDegredation;
        y += speedY;
        CheckPosition();
        getCollisionPoints();
    } else IdleMovement();
}

void CpuPaddle::IdleMovement() {
    if (stateTimer > 0) {
        if (state == MOVE_UP) speedY = -(speed) + speedDegredation;
        else if (state == MOVE_DOWN) speedY = (speed) - speedDegredation;
        y += speedY;
        CheckPosition();
        getCollisionPoints();
    } else {
        stateTimer = rand() % 30;
        if (rand() % 2 == 0) state = MOVE_UP;
        else if (rand() % 2 == 0) state = MOVE_DOWN;
        else state = STAY;
    }
    stateTimer--;
}

// GameManager
void GameManager::Reset(Paddle* player, CpuPaddle* cpu, Ball* ball, int playerScore, int opScore, bool resetPositions) {
    ball->speedX = ball->speedXStore;
    ball->speedY = ball->speedYStore;
    ball->radius = 10;
    cpu->speedDegredation = 0;
    cpu->speed = 3.6;
    ball->boost = 0.5;
    textParse(std::to_string(playerScore), &Text[0], 150, 8, 1.5);
    textParse(std::to_string(opScore), &Text[1], 250, 8, 1.5);
    if (resetPositions) {
        if (rand() % 10 >= 5) {
            ball->speedX = 3; ball->speedY = 3;
        } else {
            ball->speedX = -3; ball->speedY = -3;
        }
        ball->x = 200; ball->y = 120;
        player->x = 15; player->y = 95;
        cpu->x = 385; cpu->y = 70;
    }
}

bool GameManager::TickAnimation() {
    if (animTick <= 0) {
        animTick = 60.0 / animFPS;
        return true;
    }
    animationTimer--;
    animTick--;
    return false;
}

// Funções globais do Pong
void textInit() {
    StaticTextBuffer = C2D_TextBufNew(4096);
    font = C2D_FontLoadSystem(CFG_REGION_USA);
}

void textParse(std::string txt, TextClass* staticText, float x, float y, float scale) {
    const char* text1 = txt.c_str();
    C2D_TextFontParse(&staticText->StaticText, font, StaticTextBuffer, text1);
    C2D_TextOptimize(&staticText->StaticText);
    staticText->x = x;
    staticText->y = y;
    staticText->scale = scale;
}

void renderText() {
    for (int i = 0; i < 4; i++) {
        TextClass txt = Text[i];
        C2D_DrawText(&txt.StaticText, 0, txt.x, txt.y, 0, txt.scale, txt.scale);
    }
}

void freeText() {
    C2D_TextBufDelete(StaticTextBuffer);
    C2D_FontFree(font);
}

void initSprites(C2D_SpriteSheet sheet, float x, float y) {
    size_t numImages = C2D_SpriteSheetCount(sheet);
    for (size_t i = 0; i < numImages; i++) {
        Sprite* sprite = &sprites[i];
        C2D_SpriteFromSheet(&sprite->spr, sheet, i);
        C2D_SpriteSetCenter(&sprite->spr, 0.0f, 0.0f);
        C2D_SpriteSetPos(&sprite->spr, x, y);
    }
}

void pongInit() {
    textInit();
    // Inicializar todos os textos com strings vazias
    textParse("", &Text[0], 0, 0, 0);
    textParse("", &Text[1], 0, 0, 0);
    textParse("", &Text[2], 0, 0, 0);
    textParse("", &Text[3], 0, 0, 0);
    textParse("press any button to start", &Text[2], 48, 208, 1);
    gameManager.Reset(&player, &cpu, &ball, yourScore, opScore);
    // Carregar spritesheets aqui se necessário
    nateSS1 = C2D_SpriteSheetLoad("romfs:/gfx/Nate1.t3x");
    if (!nateSS1) svcBreak(USERBREAK_PANIC);
    nateSS2 = C2D_SpriteSheetLoad("romfs:/gfx/Nate2.t3x");
    if (!nateSS2) svcBreak(USERBREAK_PANIC);
    nateSS3 = C2D_SpriteSheetLoad("romfs:/gfx/Nate3.t3x");
    if (!nateSS3) svcBreak(USERBREAK_PANIC);
    spacSS1 = C2D_SpriteSheetLoad("romfs:/gfx/Spac1.t3x");
    if (!spacSS1) svcBreak(USERBREAK_PANIC);
    spacSS2 = C2D_SpriteSheetLoad("romfs:/gfx/Spac2.t3x");
    if (!spacSS2) svcBreak(USERBREAK_PANIC);
    spacSS3 = C2D_SpriteSheetLoad("romfs:/gfx/Spac3.t3x");
    if (!spacSS3) svcBreak(USERBREAK_PANIC);
}

void pongUpdate(u32 keyPressed, u32 keyJustPressed) {
    if (!gameStarted) {
        ball.speedX = 0; ball.speedY = 0; cpu.speed = 0;
        if (keyJustPressed) {
            gameStarted = true;
            gameManager.Reset(&player, &cpu, &ball, yourScore, opScore);
            textParse("", &Text[2], 0, 0, 0);
            paused = false;
        }
    }
    if (ball.scored) {
        if (ball.x + ball.radius >= 400) {
            yourScore++;
            nateState = STATE_SAD; spacState = STATE_HAPPY; gameManager.animationTimer = 160;
        } else {
            opScore++;
            nateState = STATE_HAPPY; spacState = STATE_SAD; gameManager.animationTimer = 160;
        }
        gameManager.Reset(&player, &cpu, &ball, yourScore, opScore);
    }
    if (keyPressed & KEY_UP) player.speedY = -4.2;
    if (keyPressed & KEY_DOWN) player.speedY = 4.2;
    player.UpdatePosition(); player.Draw(); player.speedY = 0;
    cpu.UpdatePosition(ball.y, ball.speedX); cpu.Draw();
    ball.UpdatePosition();
    ball.CheckCollision(player.colX1, player.colX2, player.colY1, player.colY2);
    ball.CheckCollision(cpu.colX1, cpu.colX2, cpu.colY1, cpu.colY2);
    ball.Draw();
    if (ball.enemyHit) {
        cpu.speedDegredation += 0.025 + (0.025 * (rand() % 8));
        ball.enemyHit = false;
    }
    for (size_t i = 0; i < numParticles; i++) {
        particles[i].updatePosition();
        particles[i].Draw();
    }
}

void pongRender(C3D_RenderTarget* top, C3D_RenderTarget* bottom) {
    // u32 clrClear = C2D_Color32f(0.83, 0.45, 0.35, 1);
    u32 clrWhite = C2D_Color32f(0.94, 0.9, 0.89, 1);
    u32 clrBlack = C2D_Color32f(0, 0, 0, 1);

    C2D_SceneBegin(top);
    C2D_DrawLine(200, 0, clrWhite, 200, 240, clrWhite, 3, 0);
    renderText();

    C2D_SceneBegin(bottom);
    if (gameManager.TickAnimation()) {
        animationFrame++;
        animationFrame %= 3;
        if (gameManager.animationTimer <= 0) {
            nateState = STATE_NORMAL; spacState = STATE_NORMAL;
        }
    }
    if (nateState == STATE_HAPPY) initSprites(nateSS3, 160, 0);
    else if (nateState == STATE_SAD) initSprites(nateSS2, 160, 0);
    else initSprites(nateSS1, 160, 0);
    C2D_DrawSprite(&sprites[animationFrame].spr);
    if (spacState == STATE_HAPPY) initSprites(spacSS3, 0, 0);
    else if (spacState == STATE_SAD) initSprites(spacSS2, 0, 0);
    else initSprites(spacSS1, 0, 0);
    C2D_DrawSprite(&sprites[animationFrame].spr);
    C2D_DrawLine(160, 0, clrBlack, 160, 240, clrBlack, 4, 0);
}

void pongExit() {
    freeText();
    C2D_SpriteSheetFree(nateSS1);
    C2D_SpriteSheetFree(nateSS2);
    C2D_SpriteSheetFree(nateSS3);
    C2D_SpriteSheetFree(spacSS1);
    C2D_SpriteSheetFree(spacSS2);
    C2D_SpriteSheetFree(spacSS3);
}

void menuInit() {
    textInit();
    // Inicializa os textos para o menu
    for (int i = 0; i < 4; i++) {
        textParse("", &Text[i], 0, 0, 0);
    }
}

void menuExit() {
    freeText();
}

// Definições de variáveis globais
TextClass Text[4];
C2D_TextBuf StaticTextBuffer;
C2D_Font font;
C2D_SpriteSheet nateSS1, nateSS2, nateSS3, spacSS1, spacSS2, spacSS3;
Sprite sprites[MAX_SPRITES];
Particle particles[MAX_PARTICLES];
size_t numParticles = 0;
Ball ball;
Paddle player;
CpuPaddle cpu;
GameManager gameManager;
int yourScore = 0, opScore = 0, animationFrame = 0;
bool gameStarted = false, paused = false;
animStates nateState = STATE_NORMAL, spacState = STATE_NORMAL;