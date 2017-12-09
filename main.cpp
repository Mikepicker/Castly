// Using SDL and standard IO
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <string>
#include <stdlib.h> 
#include <time.h> 

// Constants
const Sint32 SCREEN_WIDTH = 768;
const Sint32 SCREEN_HEIGHT = 512;
const Sint32 TILE_SIZE = 32;
const Sint32 CASTLE_SIZE = 8;
const Sint32 MAX_BALLS = 16;
const Sint32 BALL_SPEED = 20;
const Sint32 MAX_SUPPLY = 100;
const Sint32 SUPPLY_TIME = 1000;
const Sint32 WALL_COST = 10;
const Sint32 CANNON_COST = 20;
const Sint32 MINE_COST = 30;
const Sint32 BALL_COST = 5;
const double PI = 3.14159265;

// Textures
SDL_Texture* cursorBlueTexture;
SDL_Texture* cursorRedTexture;
SDL_Texture* wallTexture;
SDL_Texture* cannonTexture;
SDL_Texture* cannonBaseTexture;
SDL_Texture* kingTexture;
SDL_Texture* mineTexture;
SDL_Texture* ballTexture;
SDL_Texture* oreTexture;

// Tile types
enum TileType {
  WALL = 0,
  CANNON = 1,
  MINE = 2,
  KING = 3,
};

// Game entities
struct Tile {
  Sint32 x, y;
  SDL_Texture* texture;
  bool alive;
  TileType type; 
  double angle;
};

struct Ball {
  Sint32 x, y;
  float vx, vy;
  bool alive;
  double angle;
  bool collide;
  Tile* cannonTile;
};

struct Player {
  Tile tile;
  Tile castle[CASTLE_SIZE][CASTLE_SIZE];
  Ball balls[MAX_BALLS];
  Tile* king;
  Sint32 ore;
  SDL_Texture* cursorTexture;
  SDL_Texture* oreTexture;
  Sint32 castleOffset;
};


struct World {
  float gravity = 0.5f;
} world;

// Variables
Player p1, p2;
bool keyDown = false;
Sint32 lastSupplyTime = 0;

#include "framework.cpp"
#include "utils.cpp"

bool load() {

    // Loading success flag
    bool success = true;
    
    // Seed random number generator
    srand(time(NULL));

    // Init font
    font = TTF_OpenFont("assets/PixelText.ttf", 32);
    if (font == NULL) {
        printf("Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
    }

    // Init textures
    cursorBlueTexture = loadTexture("assets/cursor_blue.png");
    cursorRedTexture = loadTexture("assets/cursor_red.png");
    wallTexture = loadTexture("assets/wall.png");
    cannonTexture = loadTexture("assets/cannon.png");
    cannonBaseTexture = loadTexture("assets/cannon_base.png");
    mineTexture = loadTexture("assets/mine.png");
    kingTexture = loadTexture("assets/king.png");
    ballTexture = loadTexture("assets/ball.png");
    oreTexture = loadTexture("assets/ore.png");

    // Init p1
    p1.tile.texture = kingTexture;
    p1.tile.type = KING;
    p1.tile.x = 0;
    p1.tile.y = 0;
    p1.cursorTexture = cursorBlueTexture;
    p1.castleOffset = 0;
    p1.king = NULL;
    p1.ore = 0;

    // Init p2
    p2.tile.texture = kingTexture;
    p2.tile.type = KING;
    p2.tile.x = 0;
    p2.tile.y = 0;
    p2.cursorTexture = cursorRedTexture;
    p2.castleOffset = SCREEN_WIDTH - CASTLE_SIZE * TILE_SIZE;
    p2.king = NULL;
    p2.ore = 0;

    // Init castles
    for (Sint32 row = 0; row < CASTLE_SIZE; row++) {
      for (Sint32 col = 0; col < CASTLE_SIZE; col++) {
        p1.castle[row][col].alive = false;
        p2.castle[row][col].alive = false;
      }
    }

    // Init balls
    for (Sint32 i = 0; i < CASTLE_SIZE; i++) {
      p1.balls[i].alive = false;
      p1.balls[i].collide = false;
      p2.balls[i].alive = false;
      p2.balls[i].collide = false;
    }

    return success;
}

void close() {

    //Free loaded image
    SDL_DestroyTexture(cursorBlueTexture);
    SDL_DestroyTexture(cursorRedTexture);
    SDL_DestroyTexture(wallTexture);
    SDL_DestroyTexture(cannonTexture);
    SDL_DestroyTexture(cannonBaseTexture);
    SDL_DestroyTexture(kingTexture);
    SDL_DestroyTexture(mineTexture);
    SDL_DestroyTexture(ballTexture);
    SDL_DestroyTexture(oreTexture);

    closeFramework();
}


// Game logic
void input(SDL_Event* e) {

  if (e->type == SDL_KEYDOWN) {

    SDL_Scancode code = e->key.keysym.scancode;

    //-------------P1------------\\

    // Move cursor
    if (code == SDL_SCANCODE_W) {
      if (p1.tile.y > 0) { p1.tile.y--; }
    } else if (code == SDL_SCANCODE_S) {
      if (p1.tile.y < CASTLE_SIZE-1) { p1.tile.y++; }
    } else if (code == SDL_SCANCODE_A) {
      if (p1.tile.x > 0) { p1.tile.x--; }
    } else if (code == SDL_SCANCODE_D) {
      if (p1.tile.x < CASTLE_SIZE-1) { p1.tile.x++; }
    }

    // Change tile
    if (code == SDL_SCANCODE_J) {
      changeTile(&p1);
    }

    // Place tile or fire cannon
    if (code == SDL_SCANCODE_SPACE) {
      actionTile(&p1); // Execute action associated to this tile
      setTile(&p1);    // If unset, set new tile
    }

    // Rotate cannon
    if (code == SDL_SCANCODE_K) {
      rotateCannon(&p1, -1);
    } else if (code == SDL_SCANCODE_L) {
      rotateCannon(&p1, 1);
    }

    //-------------P2------------\\

    // Move cursor
    if (code == SDL_SCANCODE_UP) {
      if (p2.tile.y > 0) { p2.tile.y--; }
    } else if (code == SDL_SCANCODE_DOWN) {
      if (p2.tile.y < CASTLE_SIZE-1) { p2.tile.y++; }
    } else if (code == SDL_SCANCODE_LEFT) {
      if (p2.tile.x > 0) { p2.tile.x--; }
    } else if (code == SDL_SCANCODE_RIGHT) {
      if (p2.tile.x < CASTLE_SIZE-1) { p2.tile.x++; }
    }

    // Change tile
    if (code == SDL_SCANCODE_I) {
      changeTile(&p2);
    }

    // Place tile or fire cannon
    if (code == SDL_SCANCODE_RETURN) {
      actionTile(&p2); // Execute action associated to this tile
      setTile(&p2);    // If unset, set new tile
    }

    // Rotate cannon
    if (code == SDL_SCANCODE_O) {
      rotateCannon(&p2, -1);
    } else if (code == SDL_SCANCODE_P) {
      rotateCannon(&p2, 1);
    }

  }
}

void update() {

  // Increase ore
  Uint32 now = SDL_GetTicks();
  if (now - lastSupplyTime >= SUPPLY_TIME) {
    p1.ore = p1.ore < MAX_SUPPLY ? p1.ore + 1 : MAX_SUPPLY;
    p2.ore = p2.ore < MAX_SUPPLY ? p2.ore + 1 : MAX_SUPPLY;
    lastSupplyTime = now;
  }

  // Update balls
  for (Sint32 i = 0; i < MAX_BALLS; i++) {

    //-------------P1------------\\

    if (p1.balls[i].alive) {

      if (p1.balls[i].y > SCREEN_HEIGHT) {
        p1.balls[i].alive = false;
        continue;
      }

      p1.balls[i].vy += world.gravity;
      p1.balls[i].x += p1.balls[i].vx;
      p1.balls[i].y += p1.balls[i].vy;

      handleBallCollision(&p1, &p1.balls[i]);
      handleBallCollision(&p2, &p1.balls[i]);

    }
    
    //-------------P2------------\\

    if (p2.balls[i].alive) {

      if (p2.balls[i].y > SCREEN_HEIGHT) {
        p2.balls[i].alive = false;
        continue;
      }

      p2.balls[i].vy += world.gravity;
      p2.balls[i].x += p2.balls[i].vx;
      p2.balls[i].y += p2.balls[i].vy;

      handleBallCollision(&p2, &p2.balls[i]);
      handleBallCollision(&p1, &p2.balls[i]);
      
    }
  }
}

void render() {

    // Clear screen
    SDL_RenderClear(renderer);

    // Render castles
    renderCastle(&p1);
    renderCastle(&p2);

    // Render current tile
    renderTile(&p1);
    renderTile(&p2);

    // Render cursors
    renderCursor(&p1);
    renderCursor(&p2);
    
    // Render balls
    renderBalls(&p1); 
    renderBalls(&p2); 
    
    /*SDL_RendererFlip flip = zombies[i].dir == 1 ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
    SDL_RenderCopyEx(renderer, zombieTexture, &srcZombie, &dstZombie, 0, NULL, flip);*/

    // Render text
    renderOre(&p1);
    renderOre(&p2);

    // Update the screen
    SDL_RenderPresent(renderer);
}

int main(Sint32 argc, char* args[]) {

	//Start up SDL and create window
	if(!initFramework()) {
		printf("Failed to initialize!\n");
	}
	else {
		// Load media
		if(!load()) {
			printf("Failed to load media!\n");
		}
		else {

      // Main loop flag
      bool quit = false;

      // Event handler
      SDL_Event e;

      Sint32 countedFrames = 0;

      // While application is running
      while(!quit) {

        // Handle events on queue
        while(SDL_PollEvent(&e) != 0) {

          // User requests quit
          if(e.type == SDL_QUIT) {
            quit = true;
          }

          input(&e);
        }

        float avgFPS = countedFrames / (SDL_GetTicks() / 1000.f);
        if(avgFPS > 2000000) {
          avgFPS = 0;
        }

        // Update game
        update();


        // Render game
        render();

        countedFrames++;
      }
    }
	}

	//Free resources and close SDL
	close();

	return 0;
}
