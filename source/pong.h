#ifndef PONG_H
#define PONG_H

#include <3ds.h>
#include <citro2d.h>
#include <citro3d.h>
#include <string>
#include <cmath>

// Defines específicos do Pong
#define MAX_PARTICLES 100
#define MAX_SPRITES 768

// Estruturas e classes do Pong
typedef struct {
    C2D_Text StaticText;
    float x;
    float y;
    float scale;
} TextClass;

typedef struct {
    C2D_Sprite spr;
} Sprite;

// Declarações de classes
class Particle {
public:
    float r, g, b;
    u32 clr;
    float x, y;
    float gravityX, gravityY;
    float speedX, speedY;
    int aliveTime;
    float size;

    void init();
    void updatePosition();
    void Draw();
};

class Ball {
public:
    u32 clrWhite, clrShadow;
    float x, y;
    int radius;
    float speedX, speedY;
    float boost;
    bool scored, enemyHit, ballHit;
    float speedXStore, speedYStore;

    bool CheckCollision(int colX1, int colX2, int colY1, int colY2);
    void Draw();
    void UpdatePosition();

private:
    int sign(float num);
};

class Paddle {
protected:
    void CheckPosition();
    void getCollisionPoints();

public:
    u32 clrWhite, clrShadow;
    int colX1, colX2, colY1, colY2;
    int x, y;
    float speedY;
    int height, width;

    void Draw();
    void UpdatePosition();

private:
    int sign(float num);
};

class CpuPaddle : public Paddle {
public:
    float speedDegredation;
    enum IdleStates { STAY, MOVE_UP, MOVE_DOWN };
    IdleStates state;
    int stateTimer;
    float speed;

    void UpdatePosition(int ballPosition, float ballSpeed);
    void IdleMovement();
};

class GameManager {
public:
    float animFPS, animTick;
    int animationTimer;

    void Reset(Paddle* player, CpuPaddle* cpu, Ball* ball, int playerScore, int opScore, bool resetPositions = true);
    bool TickAnimation();
};

// Funções específicas do Pong
void pongInit();  // Inicializa tudo do Pong (mantido para compatibilidade)
void pongUpdate(u32 keyPressed, u32 keyJustPressed);  // Atualiza lógica do Pong
void pongRender(C3D_RenderTarget* top, C3D_RenderTarget* bottom);  // Renderiza o Pong
void pongExit();  // Limpa recursos do Pong

// Funções de menu/texto
void menuInit();
void menuExit();

// Funções de texto e renderização
void textInit();
void textParse(std::string txt, TextClass* staticText, float x, float y, float scale);
void renderText();
void freeText();
void initSprites(C2D_SpriteSheet sheet, float x, float y);

// Enum para estados de animação
enum animStates { STATE_HAPPY = 6, STATE_NORMAL = 0, STATE_SAD = 3 };

// Variáveis globais do Pong (extern para acessar de main.cpp)
extern TextClass Text[4];
extern C2D_TextBuf StaticTextBuffer;
extern C2D_Font font;
extern C2D_SpriteSheet nateSS1, nateSS2, nateSS3, spacSS1, spacSS2, spacSS3;
extern Sprite sprites[MAX_SPRITES];
extern Particle particles[MAX_PARTICLES];
extern size_t numParticles;
extern Ball ball;
extern Paddle player;
extern CpuPaddle cpu;
extern GameManager gameManager;
extern int yourScore, opScore, animationFrame;
extern bool gameStarted, paused;
extern animStates nateState, spacState;

#endif // PONG_H