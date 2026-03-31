#include <3ds.h>
#include <citro2d.h>
#include <citro3d.h>
#include <stdio.h>
#include <string>
#include "pong.h"

enum GameState {
    STATE_MENU,
    STATE_PONG,
    STATE_EXIT
};

struct MenuItem {
    const char* label;
    float x;
    float y;
    bool enabled;
    C2D_Sprite icon;
};

static GameState currentState = STATE_MENU;
static int selectedOption = 0;
static bool menuInitialized = false;
static bool pongPlaceholderInitialized = false;
static const int kMenuItemCount = 2;
static MenuItem menuItems[kMenuItemCount];
static C2D_SpriteSheet menuIconSheets[kMenuItemCount];

static void initMenuIcons() {
    menuIconSheets[0] = C2D_SpriteSheetLoad("romfs:/gfx/pong.t3x");
    if (!menuIconSheets[0]) svcBreak(USERBREAK_PANIC);

    menuIconSheets[1] = C2D_SpriteSheetLoad("romfs:/gfx/crush.t3x");
    if (!menuIconSheets[1]) svcBreak(USERBREAK_PANIC);

    menuItems[0] = {"Pong", 42.0f, 72.0f, true, {}};
    menuItems[1] = {"CandyCrush", 178.0f, 72.0f, false, {}};

    for (int i = 0; i < kMenuItemCount; i++) {
        C2D_SpriteFromSheet(&menuItems[i].icon, menuIconSheets[i], 0);
        C2D_SpriteSetCenter(&menuItems[i].icon, 0.0f, 0.0f);
        C2D_SpriteSetPos(&menuItems[i].icon, menuItems[i].x + 8.0f, menuItems[i].y + 8.0f);
        C2D_SpriteSetScale(&menuItems[i].icon, 0.2f, 0.2f);
    }
}

static void freeMenuIcons() {
    for (int i = 0; i < kMenuItemCount; i++) {
        if (menuIconSheets[i]) {
            C2D_SpriteSheetFree(menuIconSheets[i]);
            menuIconSheets[i] = nullptr;
        }
    }
}

static void initMenuTexts() {
    if (menuInitialized) return;
    textParse("SpaceHub", &Text[0], 18, 18, 0.85f);
    textParse(menuItems[0].label, &Text[1], menuItems[0].x + 10.0f, 176.0f, 0.65f);
    textParse(menuItems[1].label, &Text[2], menuItems[1].x - 2.0f, 176.0f, 0.65f);
    textParse("A: abrir   START: sair", &Text[3], 18, 214, 0.5f);
    menuInitialized = true;
}

static void initPongPlaceholderTexts() {
    if (pongPlaceholderInitialized) return;
    textParse("Pong desativado - em desenvolvimento", &Text[0], 40, 100, 1.5);
    textParse("Pressione SELECT para voltar", &Text[1], 40, 140, 1.2);
    textParse("", &Text[2], 0, 0, 0);
    textParse("", &Text[3], 0, 0, 0);
    pongPlaceholderInitialized = true;
}

static void resetMenuState() {
    pongPlaceholderInitialized = false;
    initMenuTexts();
}

static void drawMenuIcon(const MenuItem& item, bool selected) {
    u32 shadow = C2D_Color32(12, 16, 24, 180);
    u32 panel = item.enabled ? C2D_Color32(242, 245, 247, 255) : C2D_Color32(170, 178, 184, 255);
    u32 border = selected ? C2D_Color32(255, 183, 48, 255) : C2D_Color32(88, 102, 120, 255);
    u32 accent = selected ? C2D_Color32(255, 222, 130, 255) : C2D_Color32(208, 214, 220, 255);

    C2D_DrawRectSolid(item.x + 4.0f, item.y + 6.0f, 0.0f, 96.0f, 96.0f, shadow);
    C2D_DrawRectSolid(item.x, item.y, 0.0f, 96.0f, 96.0f, border);
    C2D_DrawRectSolid(item.x + 4.0f, item.y + 4.0f, 0.0f, 88.0f, 88.0f, panel);
    C2D_DrawRectSolid(item.x + 4.0f, item.y + 74.0f, 0.0f, 88.0f, 18.0f, accent);
    C2D_DrawSprite(&item.icon);
}

static void renderMenu(C3D_RenderTarget* top, C3D_RenderTarget* bottom) {
    C2D_SceneBegin(top);
    C2D_TargetClear(top, C2D_Color32(22, 30, 44, 255));
    C2D_DrawRectSolid(0.0f, 0.0f, 0.0f, 400.0f, 240.0f, C2D_Color32(22, 30, 44, 255));
    C2D_DrawRectSolid(0.0f, 178.0f, 0.0f, 400.0f, 62.0f, C2D_Color32(16, 22, 32, 255));

    C2D_SceneBegin(bottom);
    C2D_TargetClear(bottom, C2D_Color32(214, 225, 234, 255));
    C2D_DrawRectSolid(0.0f, 0.0f, 0.0f, 320.0f, 240.0f, C2D_Color32(214, 225, 234, 255));
    C2D_DrawRectSolid(0.0f, 0.0f, 0.0f, 320.0f, 48.0f, C2D_Color32(188, 204, 216, 255));

    for (int i = 0; i < kMenuItemCount; i++) {
        drawMenuIcon(menuItems[i], i == selectedOption);
    }

    renderText();
}

int main(int argc, char* argv[]) {
    romfsInit();
    cfguInit();
    gfxInitDefault();
    ndspInit();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();

    C3D_RenderTarget* top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    C3D_RenderTarget* bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

    menuInit();
    initMenuIcons();
    initMenuTexts();

    while (aptMainLoop() && currentState != STATE_EXIT) {
        hidScanInput();
        u32 keyJustPressed = hidKeysDown();

        switch (currentState) {
            case STATE_MENU:
                if (keyJustPressed & KEY_RIGHT) {
                    selectedOption = (selectedOption + 1) % kMenuItemCount;
                }
                if (keyJustPressed & KEY_LEFT) {
                    selectedOption = (selectedOption - 1 + kMenuItemCount) % kMenuItemCount;
                }
                if (keyJustPressed & KEY_START) {
                    currentState = STATE_EXIT;
                }
                if ((keyJustPressed & KEY_A) && menuItems[selectedOption].enabled) {
                    if (selectedOption == 0) {
                        pongPlaceholderInitialized = false;
                        currentState = STATE_PONG;
                    }
                }

                C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
                renderMenu(top, bottom);
                C3D_FrameEnd(0);
                break;

            case STATE_PONG:
                C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

                C2D_SceneBegin(top);
                C2D_TargetClear(top, C2D_Color32(24, 40, 56, 255));
                C2D_DrawRectSolid(0.0f, 0.0f, 0.0f, 400.0f, 240.0f, C2D_Color32(24, 40, 56, 255));

                C2D_SceneBegin(bottom);
                C2D_TargetClear(bottom, C2D_Color32(24, 40, 56, 255));
                C2D_DrawRectSolid(0.0f, 0.0f, 0.0f, 320.0f, 240.0f, C2D_Color32(24, 40, 56, 255));
                initPongPlaceholderTexts();
                renderText();
                C3D_FrameEnd(0);

                if (keyJustPressed & KEY_SELECT) {
                    resetMenuState();
                    currentState = STATE_MENU;
                }
                break;

            case STATE_EXIT:
                break;
        }
    }

    freeMenuIcons();
    menuExit();
    C2D_Fini();
    C3D_Fini();
    ndspExit();
    romfsExit();
    cfguExit();
    gfxExit();
    return 0;
}
