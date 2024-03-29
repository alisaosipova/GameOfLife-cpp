#include "Game.h"

const short Game::neighbors[8][2] = { {-1, -1}, {-1, 0}, {-1, 1}, {0,  1}, {1,  1}, {1,  0}, {1,  -1}, {0,  -1} };

void Game::update() {
    if (paused) {
        return;
    }
    bool newCells[rows][cols];
    for (auto& newCell : newCells)
        for (bool& x : newCell)
            x = false;
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            int neighborCount = countNeighbors(y, x);
            if (cells[y][x]) {
                if (neighborCount < 2 || neighborCount > 3) {
                    newCells[y][x] = false;
                }
                else if (neighborCount == 2 || neighborCount == 3) {
                    newCells[y][x] = true;
                }
            }
            else if (neighborCount == 3) {
                newCells[y][x] = true;
            }
        }
    }
    for (int y = 0; y < rows; y++)
        for (int x = 0; x < cols; x++)
            cells[y][x] = newCells[y][x];
}

int Game::countNeighbors(int y, int x) {
    int count = 0;

    for (auto n : neighbors) {
        int yy = y + n[0];
        int xx = x + n[1];

        if (wrap) {
            if (yy <= -1) yy = rows - 1;
            if (yy >= rows) yy = 0;
            if (xx <= -1) xx = rows - 1;
            if (xx >= cols) xx = 0;
        }
        else {
            if (yy < 0 || xx < 0 || yy >= rows || xx >= cols) { continue; }
        }

        if (yy < 0 || xx < 0 || yy >= rows || xx >= cols) { continue; }

        if (cells[yy][xx]) { count++; }
    }

    return count;
}

void Game::clean() {
    delete[] cells;

    TTF_CloseFont(font);
    TTF_Quit();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

Game::Game(const char* title) {
    seed = std::chrono::system_clock::now().time_since_epoch().count();
    generator = std::default_random_engine(seed);
    distribution = std::uniform_int_distribution<int>(0, 99);

    if (SDL_Init(SDL_INIT_EVERYTHING)) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    int window_h = WINDOW_H + MENU_H;

    window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_W, window_h, 0);

    if (!window) {
        printf("Could not create window: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_SetWindowMinimumSize(window, WINDOW_W, window_h);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (!renderer) {
        printf("Count not get renderer! SDL Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    if (TTF_Init() == -1) {
        printf("TTF_Init failed: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }

    font = TTF_OpenFont(FONT, 21);
    if (font == nullptr) {
        printf("Failed to load font! Error: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }

    btnRects[BtnRun].x = 9;
    btnRects[BtnRun].y = WINDOW_H + 8;
    btnRects[BtnRun].w = 62;
    btnRects[BtnRun].h = 32;

    btnRects[BtnDraw].x = 9;
    btnRects[BtnDraw].y = WINDOW_H + 8;
    btnRects[BtnDraw].w = 62;
    btnRects[BtnDraw].h = 32;

    btnRects[BtnSpeed].x = 82;
    btnRects[BtnSpeed].y = WINDOW_H + 8;
    btnRects[BtnSpeed].w = 92;
    btnRects[BtnSpeed].h = 32;

    btnRects[BtnSize].x = 185;
    btnRects[BtnSize].y = WINDOW_H + 8;
    btnRects[BtnSize].w = 82;
    btnRects[BtnSize].h = 32;

    btnRects[BtnWrap].x = 278;
    btnRects[BtnWrap].y = WINDOW_H + 8;
    btnRects[BtnWrap].w = 62;
    btnRects[BtnWrap].h = 32;

    btnRects[BtnClear].x = 600;
    btnRects[BtnClear].y = WINDOW_H + 8;
    btnRects[BtnClear].w = 72;
    btnRects[BtnClear].h = 32;

    btnRects[BtnRand].x = 682;
    btnRects[BtnRand].y = WINDOW_H + 8;
    btnRects[BtnRand].w = 82;
    btnRects[BtnRand].h = 32;

   // btnRects[BtnPaused].x = (WINDOW_W - btnRects[BtnPaused].w) / 2;
   // btnRects[BtnPaused].y = (WINDOW_H - btnRects[BtnPaused].h) / 2;
   // btnRects[BtnPaused].w = 124;
   // btnRects[BtnPaused].h = 32;

    size = 10;
    cols = WINDOW_W / size;
    rows = WINDOW_H / size;

    cells = new bool* [rows];
    for (int y = 0; y < rows; y++) {
        cells[y] = new bool[cols];
        for (int x = 0; x < cols; x++) { cells[y][x] = false; }
    }

    addGlider();

    drawing = false;
    running = true;
}

void Game::togglePause() {
    paused = !paused;
}

void Game::render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    drawGrid();
    drawCells();
    drawMenu();
    if (paused) {
        drawPausedButton();
    }

    SDL_RenderPresent(renderer);
}

void Game::drawGrid() {
    SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);

    for (int x = 0; x <= WINDOW_W; x += size)
        SDL_RenderDrawLine(renderer, x, 0, x, WINDOW_H);

    for (int y = 0; y <= WINDOW_H; y += size)
        SDL_RenderDrawLine(renderer, 0, y, WINDOW_W, y);
}

void Game::drawCells() {
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            if (cells[y][x]) {
                Sint16 x0 = x * size;
                Sint16 x1 = (x + 1) * size;
                Sint16 y0 = y * size;
                Sint16 y1 = (y + 1) * size;

                const Sint16 vx[4] = { x0, x1, x1, x0 };
                const Sint16 vy[4] = { y0, y0, y1, y1 };
                filledPolygonRGBA(renderer, vx, vy, 4, 79, 0, 20, 255);
            }
        }
    }
}

void Game::drawMenu() {
    Sint16 x0 = 0;
    Sint16 x1 = WINDOW_W;
    Sint16 y0 = WINDOW_H;
    Sint16 y1 = WINDOW_H + MENU_H;

    const Sint16 vx[4] = { x0, x1, x1, x0 };
    const Sint16 vy[4] = { y0, y0, y1, y1 };

    filledPolygonRGBA(renderer, vx, vy, 4, 0xaa, 0xaa, 0xaa, 0xff);

    SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
    SDL_RenderDrawLine(renderer, 0, WINDOW_H, WINDOW_W, WINDOW_H);

    drawing ? drawRunButton() : drawDrawButton();
    drawSpeedButton();
    drawSizeButton();
    drawWrapButton();
    drawClearButton();
    drawRandButton();
    if (paused) { drawPausedButton(); }
}

void Game::drawRunButton() {
    drawBtn(BtnRun, "RUN", colorBlack, 24);
}

void Game::drawDrawButton() {
    drawBtn(BtnDraw, "DRAW", colorBlack, 18);
}

void Game::drawSpeedButton() {
    std::ostringstream label;
    label << "SPEED " << (int)(speed / 4);

    drawBtn(BtnSpeed, label.str().c_str(), colorBlack, 90);
}

void Game::drawSizeButton() {
    std::ostringstream label;
    label << "SIZE " << (int)(size / 5);

    drawBtn(BtnSize, label.str().c_str(), colorBlack, 193);
}

void Game::drawWrapButton() {
    SDL_Color color = wrap ? colorBlack : colorGrey;
    drawBtn(BtnWrap, "WRAP", color, 288);
}

void Game::drawClearButton() {
    drawBtn(BtnClear, "CLEAR", colorBlack, 611);
}

void Game::drawRandButton() {
    drawBtn(BtnRand, "RANDOM", colorBlack, 691);
}

void Game::drawPausedButton() {
    drawBtn(BtnPaused, "PAUSED", colorBlack, (WINDOW_W - 60) / 2);
    int textWidth, textHeight;
    TTF_SizeText(font, "Paused. Click SPACE for continue.", &textWidth, &textHeight);
    int textX = (WINDOW_W - textWidth) / 2;
    int textY = (WINDOW_H - textHeight) / 2;
    
    int rectWidth = textWidth + 20;
    int rectHeight = textHeight + 10;
    int rectX = (WINDOW_W - rectWidth) / 2;
    int rectY = (WINDOW_H - rectHeight) / 2;

    
    SDL_SetRenderDrawColor(renderer, 160, 160, 164, 255); // ����� ����
    SDL_Rect rect = { rectX, rectY, rectWidth, rectHeight };
    SDL_RenderFillRect(renderer, &rect);
    SDL_Color textColor = { 255, 255, 255, 255}; // ����� ���� ������
    writeText("Paused. Click SPACE for continue.", textX, textY, font, textColor);
}

void Game::drawBtn(Buttons button, const char* title, SDL_Color color, int xLoc) {
    Sint16 x0 = btnRects[button].x;
    Sint16 x1 = btnRects[button].x + btnRects[button].w;
    Sint16 y0 = btnRects[button].y;
    Sint16 y1 = WINDOW_H + btnRects[button].h;

    const Sint16 vx[4] = { x0, x1, x1, x0 };
    const Sint16 vy[4] = { y0, y0, y1, y1 };

    filledPolygonRGBA(renderer, vx, vy, 4, 160, 160, 164, 0xff);
    writeText(title, xLoc, WINDOW_H + 8, font, color);
}

void Game::handleEvents() {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            running = false;
            break;
        case SDL_MOUSEBUTTONUP:
            SDL_GetMouseState(&mouseX, &mouseY);
            handleClick();
            break;
        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_SPACE) {
                togglePause();
            }
            break;
        }
    }
}

void Game::handleClick() {
    if (drawing && mouseY < WINDOW_H) {
        int x = mouseX / size;
        int y = mouseY / size;
        cells[y][x] = !cells[y][x];
        return;
    }

    if (drawing && insideRect(btnRects[BtnRun], mouseX, mouseY)) {
        drawing = false;
        return;
    }

    if (!drawing && insideRect(btnRects[BtnDraw], mouseX, mouseY)) {
        drawing = true;
        return;
    }

    if (insideRect(btnRects[BtnSpeed], mouseX, mouseY)) {
        float newSpeed = speed + 4;
        if (newSpeed > 36) { newSpeed = 4; }
        speed = newSpeed;

        return;
    }

    if (insideRect(btnRects[BtnSize], mouseX, mouseY)) {
        int newSize = size + 5;
        if (newSize > 45) { newSize = 5; }
        size = newSize;

        cols = WINDOW_W / size;
        rows = WINDOW_H / size;

        delete[] cells;
        cells = new bool* [rows];

        for (int y = 0; y < rows; y++) {
            cells[y] = new bool[cols];
            for (int x = 0; x < cols; x++) { cells[y][x] = false; }
        }

        addGlider();
        return;
    }
    if (insideRect(btnRects[BtnPaused], mouseX, mouseY)) {
        togglePause();
        return;
    }

    if (!wrap && insideRect(btnRects[BtnWrap], mouseX, mouseY)) {
        wrap = true;
        return;
    }

    if (wrap && insideRect(btnRects[BtnWrap], mouseX, mouseY)) {
        wrap = false;
        return;
    }

    if (insideRect(btnRects[BtnClear], mouseX, mouseY)) {
        bool oldDrawing = drawing;
        drawing = false;

        for (int y = 0; y < rows; y++)
            for (int x = 0; x < cols; x++) { cells[y][x] = false; }

        drawing = oldDrawing;
        return;
    }

    if (insideRect(btnRects[BtnRand], mouseX, mouseY)) {
        bool oldDrawing = drawing;
        drawing = false;

        for (int y = 0; y < rows; y++)
            for (int x = 0; x < cols; x++)
                cells[y][x] = distribution(generator) > 50;

        drawing = oldDrawing;
        return;
    }
}

bool Game::insideRect(SDL_Rect rect, int x, int y) {
    return x > rect.x &&
        x < rect.x + rect.w &&
        y > rect.y &&
        y < rect.y + rect.h;
}

bool Game::isRunning() {
    return running;
}

bool Game::isDrawing() {
    return drawing;
}

void Game::delay() {
    frameTime = SDL_GetTicks() - frameStart;
    if (frameTime < getDelayTime()) { SDL_Delay((int)(getDelayTime() - frameTime)); }
}

void Game::setFrameStart() {
    frameStart = SDL_GetTicks();
}

void Game::writeText(const char* text, int x, int y, TTF_Font* font, SDL_Color color) {
    int w, h;

    SDL_Surface* surface = TTF_RenderText_Blended(font, text, color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

    TTF_SizeText(font, text, &w, &h);
    SDL_Rect rect = { .x = x, .y = y, .w = w, .h = h };

    SDL_RenderCopy(renderer, texture, nullptr, &rect);

    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

Uint32 Game::getDelayTime() {
    return (Uint32)(1000.0f / speed);
}

void Game::addGlider() {
    cells[0][2] = true;
    cells[1][2] = true;
    cells[2][2] = true;
    cells[2][1] = true;
    cells[1][0] = true;
}
