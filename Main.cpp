#include <raylib.h>
#include "Textures.h"

#define SCREEN_WIDTH 400
#define SCREEN_HEIGHT 600

class Background {
private:
    Vector2* fontDimensionsPtr;
    unsigned short *scorePtr, currColor = 0;
    static const unsigned short SIZE = 6;
    short int order = 1;
    const Color colors[SIZE]{
        {229, 255, 164, 255},
        {223, 246, 130, 255},
        {166, 254, 232, 255},
        {149, 192, 247, 255},
        {250, 195, 252, 255},
        {255, 155, 215, 255} };
    void NextColor() {
        if (currColor + order > SIZE - 1 || currColor + order < 0) {
            order *= -1;
        }
        currColor += order;
    }
public:
    Background(unsigned short * scorePtr, Vector2 * fontDimensionsPtr) {
        this->scorePtr = scorePtr;
        this->fontDimensionsPtr = fontDimensionsPtr;
    }
    bool UpdateScore() {
        ++*scorePtr;
        *fontDimensionsPtr = MeasureTextEx(GetFontDefault(), TextFormat("%i", *scorePtr), 100, 0);
        if (*scorePtr % 5 == 0) {
            NextColor();
            return true;
        }
        else
            return false;
    }
    Color GetColor() {
        return colors[currColor];
    }
    void ResetColor() {
        currColor = 0;
    }
};

class Ball {
private:
    Texture2D ballTexture;
    float posX, posY, speedX, speedY, rotation, rotationSpeed;
    const float textureSize = 60, radius = textureSize / 2,
        defaultX = SCREEN_WIDTH / 2, defaultY = SCREEN_HEIGHT / 4,
        defaultSpeedX = 300, defaultSpeedY = 300, defaultRotationSpeed = 100;
public:
    Ball(Image &ballImage) {
        ballTexture = LoadTextureFromImage(ballImage);
        Reset();
    }

    ~Ball() {
        UnloadTexture(ballTexture);
    }

    void Draw() {
        DrawTexturePro(ballTexture, Rectangle{ 0, 0, BALLTEXTURE_WIDTH, BALLTEXTURE_HEIGHT }, Rectangle{ posX, posY, textureSize, textureSize }, Vector2{ radius, radius }, rotation, WHITE);
    }

    void Update(Rectangle platform, Background &background, bool &gameOver) {
        float frameTime = GetFrameTime();
        posX += speedX * frameTime;
        posY += speedY * frameTime;
        rotation > 359 ? rotation = 0 : rotation += rotationSpeed * frameTime;

        if (posX - radius <= 0) {
            posX = radius;
            speedX *= -1;
        }
        else if (posX + radius >= SCREEN_WIDTH) {
            posX = SCREEN_WIDTH - radius;
            speedX *= -1;
        }
        else if (posY - radius <= 0) {
            posY = radius;
            speedY *= -1;
        }
        else if (posY + radius >= SCREEN_HEIGHT) {
            posY = SCREEN_HEIGHT - radius;
            gameOver = true;
        }
        else if (speedY > 0 && CheckCollisionCircleRec(Vector2{ posX, posY }, radius, platform)) {
            posY = platform.y - radius;
            speedY *= -1;
            if (background.UpdateScore()) {
                speedX *= 1.3f;
                speedY *= 1.3f;
            }
        }
    }

    void Reset() {
        posX = defaultX;
        posY = defaultY;
        speedX = defaultSpeedX;
        speedY = defaultSpeedY;
        rotation = 0;
        rotationSpeed = defaultRotationSpeed;
    }
};

class Platform {
private:
    float posX, posY, width, height;
    Texture2D mouseTexure;
public:
    Platform(float width, float height) {
        posX = (SCREEN_WIDTH - width) / 2;
        posY = SCREEN_HEIGHT - 60;
        this->width = width;
        this->height = height;
        Image mouseImage {0};
        mouseImage.data = MOUSETEXTURE_DATA;
        mouseImage.format = MOUSETEXTURE_FORMAT;
        mouseImage.height = MOUSETEXTURE_HEIGHT;
        mouseImage.width = MOUSETEXTURE_WIDTH;
        mouseImage.mipmaps = 1;
        mouseTexure = LoadTextureFromImage(mouseImage);
    }

    ~Platform() {
        UnloadTexture(mouseTexure);
    }

    void Draw(bool drawMouse) {
        DrawRectangleRounded(Rectangle{ posX, posY, width, height }, 0.9f, 0, BLACK);
        if(drawMouse) DrawTexture(mouseTexure, (int)posX + (int)width / 2 - 12, (int)posY + (int)height / 2 - 12, WHITE);
    }

    void Update(float x) {
        posX += x;
        if (posX < 0)
            posX = 0;
        else if (posX + width > SCREEN_WIDTH)
            posX = SCREEN_WIDTH - width;
    }

    Rectangle GetDimensions() {
        return Rectangle{posX, posY, width, height};
    }
};

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Emoji Ping Pong");
    Image ballImage = { 0 };
    ballImage.data = BALLTEXTURE_DATA;
    ballImage.width = BALLTEXTURE_WIDTH;
    ballImage.height = BALLTEXTURE_HEIGHT;
    ballImage.format = BALLTEXTURE_FORMAT;
    ballImage.mipmaps = 1;
    SetWindowIcon(ballImage);

    bool gameOver = true, pause = false, showFps = false;
    unsigned short score = 0;
    Ball ball(ballImage);
    Platform platform(100, 30);
    Vector2 fontDimensions = MeasureTextEx(GetFontDefault(), TextFormat("%i", score), 100, 0);
    Background background(&score, &fontDimensions);

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_F))
            showFps = !showFps;
        if (!gameOver && !pause) {
            if (IsMouseButtonPressed(1))
                pause = !pause;
            ball.Update(platform.GetDimensions(), background, gameOver);
            platform.Update(GetMouseDelta().x);
        }
        else if (IsMouseButtonPressed(0)) {
            if (gameOver) {
                background.ResetColor();
                ball.Reset();
                score = 0;
                fontDimensions = MeasureTextEx(GetFontDefault(), TextFormat("%i", score), 100, 0);
            }
            gameOver = pause = false;
            HideCursor();
        }
        else
            ShowCursor();

        BeginDrawing();
        ClearBackground(background.GetColor());
        DrawText(TextFormat("%i", score), (SCREEN_WIDTH - fontDimensions.x) / 2, (SCREEN_HEIGHT - fontDimensions.y) / 2, 100, Color {0, 0, 0, 128});
        gameOver || pause ? platform.Draw(true) : platform.Draw(false);
        ball.Draw();
        if(showFps) DrawFPS(10, 10);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}