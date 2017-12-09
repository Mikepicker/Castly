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
bool canAfford(Player* p, TileType type) {

  switch(type) {

    case WALL:
      return p->ore >= WALL_COST;

    case CANNON:
      return p->ore >= CANNON_COST;

    case MINE:
      return p->ore >= MINE_COST;

    case KING:
      return true;
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
 
  // Tile occupied
  Tile* newTile = &p->castle[p->tile.x][p->tile.y];
  if (!newTile->alive) {

    // Check costs
    if (!canAfford(p, p->tile.type)) { return; }

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
void actionTile(Player* p) {

  Tile* castleTile = &p->castle[p->tile.x][p->tile.y];
  if (castleTile->alive) { 

    if (castleTile->type == CANNON) {

      Ball* ball = getDeadBall(p);

      castleToScreen(p, p->tile.x, p->tile.y, &(ball->x), &(ball->y));
      ball->x += TILE_SIZE / 2;
      ball->y += TILE_SIZE / 2;

      normalizeAndScale(cos(castleTile->angle * PI / 180), sin(castleTile->angle * PI / 180), &(ball->vx), &(ball->vy), BALL_SPEED);

      ball->cannonTile = castleTile;
      ball->collide = false;
      ball->alive = true;

    } else if (castleTile->type = MINE) {
      p->ore++;
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
        castleTile->alive = false;
        ball->alive = false;
      }
    }

  }
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
