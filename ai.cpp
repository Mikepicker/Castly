const Uint32 THINK_TIME = 100;
const Sint32 QUEUE_SIZE = 8;

enum ActionType {
  PLACE_KING,
  PLACE_MINE,
  PLACE_TILE,
  PLACE_CANNON,
  TO_KING,
  TO_MINE,
  TO_CANNON,
  SELECT_WALL,
  SELECT_CANNON,
  SELECT_MINE,
  DIG_FOR_WALL,
  DIG_FOR_CANNON,
  FIRE,
  TO_SURPRISE,
  GET_SURPRISE
};

struct Action {
  ActionType type;
  Sint32 destx, desty;
  bool alive;
};

struct Memory {
  Player* p;
  Tile* mine;
  Tile* cannons[3];
  Uint32 lastActionTime;
  Action queue[QUEUE_SIZE];
} memory;

// Utils
bool getFreeTile(TileType tileType, Sint32* x, Sint32* y){

  Player* p = memory.p;

  Sint32 minx = 0;
  Sint32 maxx = CASTLE_SIZE;
  if (tileType == CANNON) {
    minx = 0;
    maxx = 2;
  }

  Sint32 count = 0;
  for (Sint32 row = minx; row < maxx; row++) {
    for (Sint32 col = 0; col < CASTLE_SIZE; col++) {
     
      if (!p->castle[row][col].alive) {
        count++;
      }

    }
  }

  if (count == 0) {
    return false;
    printf("[AI] no free tiles!\n");
  }

  printf("[AI] there are %d free tiles\n", count);

  Tile* free[count];
  Sint32 i = 0;
  for (Sint32 row = minx; row < maxx; row++) {
    for (Sint32 col = 0; col < CASTLE_SIZE; col++) {
      
      if (!p->castle[row][col].alive) {
        free[i++] = &p->castle[row][col];
      }

    }
  }

  Sint32 rnd = randInRange(0, count-1);
  printf("[AI] RANDOM %d\n", rnd);
  *x = free[rnd]->x;
  *y = free[rnd]->y;

  return true;

}

// Queue management
void enqueueAction(ActionType actionType, Sint32 destx, Sint32 desty) {

  for (Sint32 i = 0; i < QUEUE_SIZE; i++) {

    if (!memory.queue[i].alive) {
      memory.queue[i].type = actionType;
      memory.queue[i].destx = destx;
      memory.queue[i].desty = desty;
      memory.queue[i].alive = true;
      return;
    }
    
  }

  printf("[AI] no space in queue!\n");
}

Action* getCurrentAction() {

  for (Sint32 i = 0; i < QUEUE_SIZE; i++) {
    
    if (memory.queue[i].alive) {
      return &memory.queue[i];
    }

  }

  //printf("[AI] no actions in queue!\n");
  return NULL;

}

Action* popAction() {

  for (Sint32 i = 0; i < QUEUE_SIZE; i++) {

    if (memory.queue[i].alive) {
      memory.queue[i].alive = false;

      if (i < QUEUE_SIZE-1 && memory.queue[i+1].alive) {
        return &memory.queue[i+1];
      } else {
        printf("[AI] no more actions!\n");
        return NULL;
      }
    }

  }

  //printf("[AI] no more actions!\n");
  return NULL;

}

void clearQueue() {

  for (Sint32 i = 0; i < QUEUE_SIZE; i++) {
    memory.queue[i].alive = false;
  }

}

// Place wall
void placeWall(Sint32 x, Sint32 y) {
  
  Player* p = memory.p;

  if (p->tile.type != WALL) {
    enqueueAction(TO_KING, p->king->x, p->king->y);
    enqueueAction(SELECT_WALL, x, y);
  }

  enqueueAction(PLACE_TILE, x, y);

}

// Enough cannons
bool enoughCannons() {
  
  for (Sint32 i = 0; i < 3; i++) {
    if (!memory.cannons[i] || !memory.cannons[i]->alive) {
      //printf(" %d NOT ENOUGH CANNONS\n", SDL_GetTicks());
      return false;
    }
  }

  //printf("%d ENOUGH CANNONS\n", SDL_GetTicks());
  return true;

}

// Store cannon in memory
void memorizeCannon(Tile* cannon) {

  for (Sint32 i = 0; i < 3; i++) {
    if (!memory.cannons[i] || !memory.cannons[i]->alive) {
      memory.cannons[i] = cannon;
      return;
    }
  }

}

// Get random cannon
Tile* getRandomCannon() {

  Sint32 count = 0;
  for (Sint32 i = 0; i < 3; i++) {
    if (memory.cannons[i] || memory.cannons[i]->alive) {
      count++;
    }
  }
 
  Tile* cannons[count];
  Sint32 j = 0;
  for (Sint32 i = 0; i < 3; i++) {
    if (memory.cannons[i] || memory.cannons[i]->alive) {
      cannons[j++] = memory.cannons[i];
    }
  }

  return cannons[randInRange(0, count-1)];

}

// Check if king is protected
bool kingProtected() {

  Player* p = memory.p;
  return p->castle[p->king->x - 1][p->king->y].alive &&
    p->castle[p->king->x - 1][p->king->y - 1].alive &&
    p->castle[p->king->x][p->king->y - 1].alive;

}

// Decision maker
void setNextAction() {
  
  Player* p = memory.p;

  // Get next action in queue
  Action* nextAction = popAction();
  if (nextAction) {
    return;
  }

  // Place the king
  if (!p->king) {
    enqueueAction(PLACE_KING, randInRange(3, CASTLE_SIZE-2), randInRange(3, CASTLE_SIZE-1));
    return;
  }

  // Decide whether to take bonus or not
  if (surprise.alive && randInRange(0, 1) < 0.5) {
    enqueueAction(TO_SURPRISE, surprise.x - CASTLE_SIZE, surprise.y);
    enqueueAction(GET_SURPRISE, surprise.x - CASTLE_SIZE, surprise.y);
    return;
  }

  // Place a mine
  if (!memory.mine || !memory.mine->alive) {

    Sint32 x, y;
    if (getFreeTile(MINE, &x, &y)) {
      if (p->tile.type != MINE) {
        enqueueAction(TO_KING, p->king->x, p->king->y);
        enqueueAction(SELECT_MINE, p->king->x, p->king->y);
      }
      enqueueAction(PLACE_MINE, x, y);
      return;
    }
  }

  // Protect king
  if (!kingProtected()) {
    if (p->ore >= WALL_COST) {
      if (!p->castle[p->king->x - 1][p->king->y].alive) {
        placeWall(p->king->x - 1, p->king->y);
        return;
      }

      if (!p->castle[p->king->x - 1][p->king->y - 1].alive) {
        placeWall(p->king->x - 1, p->king->y - 1);
        return;
      } 

      if (!p->castle[p->king->x][p->king->y - 1].alive) {
        placeWall(p->king->x, p->king->y - 1);
        return;
      }
    } else {
      enqueueAction(TO_MINE, memory.mine->x, memory.mine->y);
      enqueueAction(DIG_FOR_WALL, memory.mine->x, memory.mine->y);
      return;
    }
  }
  
  // Go mine
  if (!enoughCannons() && p->ore < CANNON_COST) {
    enqueueAction(TO_MINE, memory.mine->x, memory.mine->y);
    enqueueAction(DIG_FOR_CANNON, memory.mine->x, memory.mine->y);
    return;
  }

  // Place cannon
  Sint32 x, y;
  if (!enoughCannons() && getFreeTile(CANNON, &x, &y)) {
    printf("[AI] place cannon to: %d %d\n", x, y);

    if (p->tile.type != CANNON) {
      enqueueAction(TO_KING, p->king->x, p->king->y);
      enqueueAction(SELECT_CANNON, p->king->x, p->king->y);
    }

    enqueueAction(PLACE_TILE, x, y);
    return;
  }

  // Fire cannon
  if (p->ore >= BALL_COST) {
    Tile* cannon = getRandomCannon();
    if (cannon) {
      enqueueAction(TO_CANNON, cannon->x, cannon->y);
      enqueueAction(FIRE, cannon->x, cannon->y);
    }
    return;
  } else {
    enqueueAction(TO_MINE, memory.mine->x, memory.mine->y);
    enqueueAction(DIG_FOR_CANNON, memory.mine->x, memory.mine->y);
  }

}

// Goals
bool moveToDest() {

  Player* p = memory.p;
  Action* currentAction = getCurrentAction();

  if (p->tile.x < currentAction->destx) {
    moveRight(p);
    return false;
  } else if (p->tile.x > currentAction->destx) {
    moveLeft(p);
    return false;
  }

  if (p->tile.y < currentAction->desty) {
    moveDown(p);
    return false;
  } else if (p->tile.y > currentAction->desty) {
    moveUp(p);
    return false;
  }

  return true;

}


// When on king tile -> select desired tile
bool selectTile(TileType tileType) {

  Player* p = memory.p;

  if (p->tile.type == tileType) {
    return true;
  }

  actionTile(p);
  return false;

}

void initAI() {

  memory.p = &p2;
  memory.p->ai = true;
  memory.mine = NULL;
  memory.lastActionTime = 0;

  clearQueue();

  setNextAction();
}

void updateAI() {
 
  Uint32 now = SDL_GetTicks();
  Player* p = memory.p;

  if (now - memory.lastActionTime < THINK_TIME) {
    return;
  }

  Action* currentAction = getCurrentAction();
  
  if (!currentAction || !currentAction->alive) {
    setNextAction();
    return;
  }

  Sint32 dir;
  Tile* cannon;

  switch(currentAction->type) {

    case PLACE_KING:
      printf("[AI] PLACE_KING\n");
      if (moveToDest()) {
        setTile(memory.p);
        printf("KING: %d %d\n", p->king->x, p->king->y);
        setNextAction();
      } 
      break;
      
    case PLACE_MINE:
      printf("[AI] PLACE_MINE\n");
      if (moveToDest()) {
        Tile* mine = setTile(memory.p);
        if (mine) {
          memory.mine = mine;
        }
        setNextAction();
      }
      break;

    case PLACE_TILE:
      printf("[AI] PLACE_TILE\n");
      if (moveToDest()) {
        Tile* tile = setTile(memory.p);
        if (tile) {
          if (tile->type == CANNON) {
            printf("[AI] memorize %d %d\n", tile->x, tile->y);
            memorizeCannon(tile);
          }
        }
        setNextAction();        
      }
      break;
    
    case TO_KING:
      printf("[AI] TO_KING\n");
      if (moveToDest()) {
        setNextAction();        
      }
      break;

    case TO_MINE:
      printf("[AI] TO_MINE\n");
      if (moveToDest()) {
        setNextAction();        
      }
      break;
    
    case TO_CANNON:
      printf("[AI] TO_CANNON\n");
      if (moveToDest()) {
        setNextAction();        
      }
      break;

    case SELECT_WALL:
      printf("[AI] SELECT_WALL\n");
      if (selectTile(WALL)) {
        setNextAction();        
      }
      break;

    case SELECT_CANNON:
      printf("[AI] SELECT_CANNON\n");
      if (selectTile(CANNON)) {
        setNextAction();
      }
      break;
    
    case SELECT_MINE:
      printf("[AI] SELECT_MINE\n");
      if (selectTile(MINE)) {
        setNextAction();
      }
      break;

    case DIG_FOR_WALL:
      printf("[AI] DIG_FOR_WALL\n");
      if (p->ore >= WALL_COST) {
        setNextAction();
      } else {
        actionTile(p);
      }
      break;
    
    case DIG_FOR_CANNON:
      printf("[AI] DIG_FOR_CANNON\n");
      if (p->ore >= CANNON_COST) {
        setNextAction();
      } else {
        actionTile(p);
      }
      break;

    case FIRE:
      printf("[AI] FIRE\n");
      cannon = getTile(p, p->tile.x, p->tile.y);
      dir = randInRange(0, 1) < 0.5 ? -1 : 1;
      printf("DIR: %f\n", cannon->angle);
      
      if (cannon->angle >= 260) { dir = -1; }
      else if (cannon->angle <= 140) { dir = 1; }

      rotateCannon(p, dir);
      actionTile(p);
      setNextAction();
      break;

    case TO_SURPRISE:
      printf("[AI] TO_SURPRISE\n");
      if (moveToDest()) {
        setNextAction();
      }
      break;

    case GET_SURPRISE:
      printf("[AI] GET_SURPRISE\n");
      actionTile(p);
      setNextAction();
      break;
  } 

  memory.lastActionTime = now;
}
