// Init game
void initGame() {

  // Init p1
  p1.tile.texture = kingTexture;
  p1.tile.type = KING;
  p1.tile.x = 0;
  p1.tile.y = 0;
  p1.cursorTexture = cursorBlueTexture;
  p1.castleOffset = 0;
  p1.king = NULL;
  p1.team = BLUE;
  p1.ore = MINE_COST;
  p1.winner = false;

  // Init p2
  p2.tile.texture = kingTexture;
  p2.tile.type = KING;
  p2.tile.x = 0;
  p2.tile.y = 0;
  p2.cursorTexture = cursorRedTexture;
  p2.castleOffset = SCREEN_WIDTH - CASTLE_SIZE * TILE_SIZE;
  p2.king = NULL;
  p2.team = RED;
  p2.ore = MINE_COST;
  p2.winner = false;

  // Init castles
  for (Sint32 row = 0; row < CASTLE_SIZE; row++) {
    for (Sint32 col = 0; col < CASTLE_SIZE; col++) {
      p1.castle[row][col].alive = false;
      p2.castle[row][col].alive = false;

      p1.castle[row][col].team = BLUE;
      p2.castle[row][col].team = RED;

    }
  }

  // Init balls
  for (Sint32 i = 0; i < CASTLE_SIZE; i++) {
    p1.balls[i].alive = false;
    p1.balls[i].collide = false;
    p2.balls[i].alive = false;
    p2.balls[i].collide = false;
  }

  // Init surprise
  surprise.alive = false;
  surprise.spawning = false;

}

// From castle to screen coordinates
void castleToScreen(Player* p, Sint32 x, Sint32 y, Sint32* resX, Sint32* resY) {

  Sint32 offY = SCREEN_HEIGHT - CASTLE_SIZE * TILE_SIZE;
  *resX = TILE_SIZE * x + p->castleOffset;
  *resY = TILE_SIZE * y + offY;

}

// From screen to castle coordinates
void screenToCastle(Player* p, Sint32 x, Sint32 y, Sint32* resX, Sint32* resY) {

  Sint32 offY = SCREEN_HEIGHT - CASTLE_SIZE * TILE_SIZE;
  *resX = (x - p->castleOffset) / TILE_SIZE;
  *resY = (y - offY) / TILE_SIZE;

}

// From surprise zone to screen coordinates
void surpriseToScreen(Sint32 x, Sint32 y, Sint32* resX, Sint32* resY) {

  Sint32 offY = SCREEN_HEIGHT - CASTLE_SIZE * TILE_SIZE;
  *resX = TILE_SIZE * x + CASTLE_SIZE * TILE_SIZE;
  *resY = TILE_SIZE * y + offY;
}

void normalizeAndScale(float x, float y, float* resX, float* resY, Sint32 scale) {

  float mag = sqrt(x*x + y*y);
  *resX = (x / mag) * scale;
  *resY = (y / mag) * scale;
}

// Set texture by type
void setTexture(Tile* tile) {

  switch(tile->type) {
    case WALL:
      tile->texture = wallTexture;
      break;
    case CANNON:
      tile->texture = cannonTexture;
      break;
    case KING:
      tile->texture = kingTexture;
      break;
    case MINE:
      tile->texture = mineTexture;
      break;
    default:
      break;
  }

}

// Get alive ball
Ball* getDeadBall(Player* p) {

  for (Sint32 i = 0; i < MAX_BALLS; i++) {
    if (!p->balls[i].alive) {
      return &p->balls[i];
    }
  }

  printf("No more balls!\n");
}

// Check if player can afford current tile
bool buyTile(Player* p, TileType type) {

  Sint32 cost = 0;

  switch(type) {

    case WALL:
      cost = WALL_COST;
      break;

    case CANNON:
      cost = CANNON_COST;
      break;

    case MINE:
      cost = MINE_COST;
      break;

    case KING:
      cost = 0;
      break;
  }

  if (p->ore >= cost) {
    p->ore -= cost;
    return true;
  } else {
    return false;
  }

}
// Change tile
void changeTile(Player* p) {

  if (!p->king) {
    p->tile.type = KING;
    setTexture(&p->tile);
    return;
  }

  p->tile.type = (TileType)((((TileType)p->tile.type) + 1) % 3);
  setTexture(&p->tile);
    
}

// Place tile
void setTile(Player* p) {

  // Out of zone
  if (p->tile.x < 0 || p->tile.x >= CASTLE_SIZE) {
    return;
  }

  // Tile occupied
  Tile* newTile = &p->castle[p->tile.x][p->tile.y];
  if (!newTile->alive) {

    // Check costs
    if (!buyTile(p, p->tile.type)) { return; }

    newTile->x = p->tile.x;
    newTile->y = p->tile.y;
    newTile->type = p->tile.type;

    if (p == &p1) {
      newTile->angle = 0;
    } else {
      newTile->angle = 180;
    }

    newTile->alive = true;
    setTexture(newTile);

    if (p->tile.type == KING) {
      p->king = newTile;
      changeTile(p);
    }

  }

}

// Rotate cannon
void rotateCannon(Player* p, Sint32 dir) {

  Tile* castleTile = &p->castle[p->tile.x][p->tile.y];
  if (castleTile->alive && castleTile->type == CANNON) {
    castleTile->angle += dir * 10;
  }
}

// Fire cannon
void getSurprise(Player* p);
void actionTile(Player* p) {

  // Get surprise
  if (p->tile.x == surprise.x + CASTLE_SIZE && p->tile.y == surprise.y) {
    getSurprise(p);
    return;
  }

  Tile* castleTile = &p->castle[p->tile.x][p->tile.y];
  if (castleTile->alive) { 

    Ball* ball;

    switch(castleTile->type) {
      case CANNON:

        if (p->ore < BALL_COST) {
          return;
        }

        p->ore -= BALL_COST;

        ball = getDeadBall(p);

        castleToScreen(p, p->tile.x, p->tile.y, &(ball->x), &(ball->y));
        ball->x += TILE_SIZE / 2;
        ball->y += TILE_SIZE / 2;

        normalizeAndScale(cos(castleTile->angle * PI / 180), sin(castleTile->angle * PI / 180), &(ball->vx), &(ball->vy), BALL_SPEED);

        ball->cannonTile = castleTile;
        ball->collide = false;
        ball->alive = true;

        break;

      case MINE:
        p->ore++;
        break;

      case KING:
        changeTile(p);
        break;

      default:
        break;

    }

  }
}

// Ball collision
void handleBallCollision(Player* p, Ball* ball) {

  Sint32 x, y;
  screenToCastle(p, ball->x, ball->y, &x, &y);

  if (x >= 0 && x < CASTLE_SIZE && y >= 0 && y < CASTLE_SIZE) {

    Tile* castleTile = &p->castle[x][y];

    if (!ball->collide) {
      if (castleTile != ball->cannonTile) { ball->collide = true; }
    } else {
      if (castleTile->alive) {

        // Win condition
        if (castleTile->type == KING) {
          
          if (castleTile->team == RED) {
            p1.winner = true;
          } else {
            p2.winner = true;
          }

        }
        
        castleTile->alive = false;
        ball->alive = false;
      }
    }

  }
}

// Surprise
void spawnSurprise() {

  surprise.x = randInRange(0, CASTLE_SIZE-1);
  surprise.y = randInRange(0, CASTLE_SIZE-1);

  surprise.lastSpawnTime = SDL_GetTicks();
  surprise.nextSpawnTime = randInRange(SURPRISE_MIN_INTERVAL, SURPRISE_MAX_INTERVAL);

  surprise.alpha = 255;
  surprise.alive = true;
  surprise.spawning = true;
}

void getSurprise(Player* p) {

  p->ore += SURPRISE_SUPPLY;
  if (p->ore > MAX_SUPPLY) { p->ore = MAX_SUPPLY; }

  surprise.alive = false;
  surprise.spawning = false;
}

// Render tile
void renderTile(Player* p) {

  Sint32 x, y;
  castleToScreen(p, p->tile.x, p->tile.y, &x, &y);

  Sint32 angle = 0;
  if (p->tile.type == CANNON && p == &p2) {
    angle = 180;
  }

  renderTexture(p->tile.texture, x, y, TILE_SIZE, TILE_SIZE, angle, 100);

}

// Render castle
void renderCastle(Player* p) {

  for (Sint32 row = 0; row < CASTLE_SIZE; row++) {
    for (Sint32 col = 0; col < CASTLE_SIZE; col++) {
      Tile* tile = &p->castle[row][col];
      if (tile->alive) {

        double angle = 0;
        if (tile->type == CANNON) {
          angle = tile->angle;
          Sint32 x, y;
          castleToScreen(p, tile->x, tile->y, &x, &y);
          renderTexture(cannonBaseTexture, x, y, TILE_SIZE, TILE_SIZE, 0, 255);

        }

        Sint32 x, y;
        castleToScreen(p, tile->x, tile->y, &x, &y);
        
        renderTexture(tile->texture, x, y, TILE_SIZE, TILE_SIZE, angle, 255);
      }
    }
  }

}

// Render surprise
void renderSurprise() {

  if (surprise.alive) {
    Sint32 x, y;
    surpriseToScreen(surprise.x, surprise.y, &x, &y);

    renderTexture(oreTexture, x, y, TILE_SIZE, TILE_SIZE, 0, surprise.alpha);
  }

}

// Render cursor
void renderCursor(Player* p) {

  Sint32 x, y;
  castleToScreen(p, p->tile.x, p->tile.y, &x, &y);

  renderTexture(p->cursorTexture, x - 2, y - 2, TILE_SIZE+4, TILE_SIZE+4, 0, 255);

}

// Render balls
void renderBalls(Player* p) {

  for (Sint32 i = 0; i < MAX_BALLS; i++) {
    if (p->balls[i].alive) {
      renderTexture(ballTexture, p->balls[i].x, p->balls[i].y, 4, 4, SDL_GetTicks(), 255);
    }
  }

}

// Render ore
void renderOre(Player* p) {

  SDL_Color textColor = { 0, 0, 0 };
  std::string text = std::to_string(p->ore);

  renderText(font, text, textColor, &p->oreTexture);
  Sint32 w, h;
  SDL_QueryTexture(p->oreTexture, NULL, NULL, &w, &h);

  Sint32 x;
  Sint32 y = 32;

  if (p == &p1) {
    x = 32;
  } else {
    x = SCREEN_WIDTH - w - 64;
  }

  // Render ore texture
  renderTexture(oreTexture, x, 32, 16, 16, 0, 255);

  // Render text
  SDL_Rect dstScore = { .x = x + 32, .y = 32 + 6 - h/2, .w = w, .h = h };
  SDL_RenderCopy(renderer, p->oreTexture, NULL, &dstScore);

}

// Render blue zone
void renderBlueZone() {
  
  SDL_Rect r;
  r.x = 0;
  r.y = SCREEN_HEIGHT - CASTLE_SIZE * TILE_SIZE;
  r.w = CASTLE_SIZE * TILE_SIZE;
  r.h = CASTLE_SIZE * TILE_SIZE;

  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
  SDL_SetRenderDrawColor(renderer, 0, 0, 255, 20);
  SDL_RenderFillRect(renderer, &r);

}

// Render red zone
void renderRedZone() {

  SDL_Rect r;
  r.x = p2.castleOffset;
  r.y = SCREEN_HEIGHT - CASTLE_SIZE * TILE_SIZE;
  r.w = CASTLE_SIZE * TILE_SIZE;
  r.h = CASTLE_SIZE * TILE_SIZE;

  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
  SDL_SetRenderDrawColor(renderer, 255, 0, 0, 20);
  SDL_RenderFillRect(renderer, &r);

}

// Render surprise zone
void renderSurpriseZone() {

  SDL_Rect r;
  r.x = TILE_SIZE * CASTLE_SIZE;
  r.y = SCREEN_HEIGHT - CASTLE_SIZE * TILE_SIZE;
  r.w = SCREEN_WIDTH - CASTLE_SIZE * TILE_SIZE * 2;
  r.h = CASTLE_SIZE * TILE_SIZE;

  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
  SDL_SetRenderDrawColor(renderer, 0, 255, 0, 20);
  SDL_RenderFillRect(renderer, &r);

}

// Render winner text
void renderWinnerText() {

  SDL_Color textColor = { 0, 0, 0 };
  std::string text;

  if (p1.winner) { 
    text = "Blue wins!";
    textColor.b = 255; 
  }
  else if (p2.winner) {
    text= "Red wins!";
    textColor.r = 255; 
  }
  else { return; }

  renderText(font, text, textColor, &winnerText);
  Sint32 w, h;
  SDL_QueryTexture(winnerText, NULL, NULL, &w, &h);

  SDL_Rect dstScore = { .x = (SCREEN_WIDTH / 2) - (w / 2), .y = 100, .w = w, .h = h };
  SDL_RenderCopy(renderer, winnerText, NULL, &dstScore);

}
