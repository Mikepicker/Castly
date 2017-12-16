// Using SDL and standard IO
#ifdef LINUX
#include <SDL2/SDL.h>
#elif WIN
#include <SDL.h>
#endif
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <string>
#include <stdlib.h> 
#include <time.h> 
#include <math.h>

#include "framework.cpp"
#include "particles.cpp"

// Constants
const Sint32 TILE_SIZE = 32;
const Sint32 CASTLE_SIZE = 8;
const Sint32 MAX_BALLS = 16;
const Sint32 MAX_CLOUDS = 32;
const Sint32 CLOUDS_MIN_SPEED = 10;
const Sint32 CLOUDS_MAX_SPEED = 50;
const Sint32 CLOUDS_MIN_LIFETIME = 6;
const Sint32 CLOUDS_MAX_LIFETIME = 12;
const Sint32 BALL_SPEED = 20;
const Sint32 MAX_SUPPLY = 100;
const Sint32 SUPPLY_TIME = 1000;
const Sint32 WALL_COST = 10;
const Sint32 CANNON_COST = 20;
const Sint32 MINE_COST = 30;
const Sint32 BALL_COST = 5;
const Uint32 SURPRISE_MIN_INTERVAL = 5000;
const Uint32 SURPRISE_MAX_INTERVAL = 20000;
const Sint32 SURPRISE_SUPPLY = 50; 
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
SDL_Texture* cloudTexture;
SDL_Texture* placeholderTexture;
SDL_Texture* grassTexture;
SDL_Texture* logoTexture;

// Font
TTF_Font* font = NULL;

// Tile types
enum TileType {
  WALL = 0,
  MINE = 1,
  CANNON = 2,
  KING = 3
};

// Team
enum Team {
  BLUE = 0,
  RED = 1
};

// Game entities
struct Tile {
  Sint32 x, y;
  SDL_Texture* texture;
  bool alive;
  TileType type; 
  Team team;
  double angle;
};

struct Emitter;
struct Ball {
  Sint32 x, y;
  float vx, vy;
  bool alive;
  double angle;
  bool collide;
  Tile* cannonTile;
  Emitter* emitter;
};

struct Player {
  Tile tile;
  Sint32 leftBound, rightBound;
  Tile castle[CASTLE_SIZE][CASTLE_SIZE];
  Ball balls[MAX_BALLS];
  Team team;
  Tile* king;
  Sint32 ore;
  SDL_Texture* cursorTexture;
  SDL_Texture* oreTexture;
  Sint32 castleOffset;
  bool winner;
  bool ai;
};

struct Surprise {
  Sint32 x, y;
  Uint8 alpha;
  bool alive;
  bool spawning;
  Uint32 lastSpawnTime;
  Uint32 nextSpawnTime;
} surprise;

struct Cloud {
  Sint32 x, y;
  float vx, vy;
  Sint32 size;
  Sint32 startTime;
  Sint32 lifeTime;
  float alpha;
};

struct World {
  float gravity = 0.5f;
} world;

// Variables
Player p1, p2;
Cloud clouds[MAX_CLOUDS];
Sint32 lastSupplyTime = 0;
SDL_Texture* winnerText;

// Menu
SDL_Texture* playersText;
SDL_Texture* startText;
SDL_Texture* exitText;
Sint32 menuEntrySelected = 0;

// Quit game
bool quit = false;

// Game state
enum GameState {
  STATE_MENU,
  STATE_GAME
} gameState;

#include "utils.cpp"
#include "ai.cpp"

bool load() {

    // Loading success flag
    bool success = true;
    
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
    cloudTexture = loadTexture("assets/cloud.png");
    placeholderTexture = loadTexture("assets/placeholder.png");
    grassTexture = loadTexture("assets/grass.png");
    logoTexture = loadTexture("assets/logo.png");

    initGame();
    initParticles();

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
    SDL_DestroyTexture(winnerText);
    SDL_DestroyTexture(p1.oreTexture);
    SDL_DestroyTexture(p2.oreTexture);
    SDL_DestroyTexture(cloudTexture);
    SDL_DestroyTexture(placeholderTexture);
    SDL_DestroyTexture(grassTexture);
    SDL_DestroyTexture(logoTexture);

    SDL_DestroyTexture(winnerText);
    SDL_DestroyTexture(playersText);
    SDL_DestroyTexture(startText);
    SDL_DestroyTexture(exitText);

    // Free font
    TTF_CloseFont(font);
    font = NULL;

    closeParticles();

    closeFramework();
}

// Game logic
void input(SDL_Event* e) {

  bool gamepad1Left = false;
  bool gamepad1Right = false;
  bool gamepad1Up = false;
  bool gamepad1Down = false;
  bool gamepad2Left = false;
  bool gamepad2Right = false;
  bool gamepad2Up = false;
  bool gamepad2Down = false;


  if (e->type == SDL_JOYAXISMOTION) {
    if (e->jaxis.which == 0) {

      // X axis
      if (e->jaxis.axis == 0) {

        if (e->jaxis.value > 0) {
          gamepad1Right = true;
        } else if (e->jaxis.value < 0) {
          gamepad1Left = true;
        }

      // Y axis
      } else if (e->jaxis.axis == 1) {
        
        if (e->jaxis.value > 0) {
          gamepad1Down = true;
        } else if (e->jaxis.value < 0) {
          gamepad1Up = true;
        }

      }
    } else if (e->jaxis.which == 1) {

      // X axis
      if (e->jaxis.axis == 0) {

        if (e->jaxis.value > 0) {
          gamepad2Right = true;
        } else if (e->jaxis.value < 0) {
          gamepad2Left = true;
        }

        // Y axis
      } else if (e->jaxis.axis == 1) {

        if (e->jaxis.value > 0) {
          gamepad2Down = true;
        } else if (e->jaxis.value < 0) {
          gamepad2Up = true;
        }

      }
    }
  }

  bool gamepad1Enter = false;
  bool gamepad1RotateLeft = false;
  bool gamepad1RotateRight = false;
  bool gamepad2Enter = false;
  bool gamepad2RotateLeft = false;
  bool gamepad2RotateRight = false;


  // Gamepad 1
  if (e->jaxis.which == 0) {

    if (e->type == SDL_JOYBUTTONDOWN) {

      if (e->jbutton.button == 4) { gamepad1RotateLeft = true; }
      else if (e->jbutton.button == 5) { gamepad1RotateRight = true; }

    } else if (e->type == SDL_JOYBUTTONUP) {
      if (e->jbutton.button == 3) { gamepad1Enter = true; }
    }

  } else if (e->jaxis.which == 1) {

    if (e->type == SDL_JOYBUTTONDOWN) {

      if (e->jbutton.button == 4) { gamepad2RotateLeft = true; }
      else if (e->jbutton.button == 5) { gamepad2RotateRight = true; }

    } else if (e->type == SDL_JOYBUTTONUP) {
      if (e->jbutton.button == 3) { gamepad2Enter = true; }
    }

  }

  if (e->type == SDL_JOYAXISMOTION || e->type == SDL_JOYBUTTONDOWN || e->type == SDL_KEYDOWN) {

    SDL_Scancode code = e->key.keysym.scancode; 

    if (p1.winner || p2.winner) {
      if (code == SDL_SCANCODE_R) {
        gameState = STATE_MENU;
        initGame();
        if (p2.ai) { initAI(); }
        return;
      }
    }

    //------------MENU----------\\

    if (gameState == STATE_MENU) {

      bool gamepadPressed = false;
      if (e->type == SDL_JOYBUTTONDOWN) {
        printf("%d\n", e->jbutton.button);
        gamepadPressed = true;
      }
      
      if ((gamepad1Up || code == SDL_SCANCODE_UP) && menuEntrySelected > 0) {
        menuEntrySelected--;
      } else if ((gamepad1Down || code == SDL_SCANCODE_DOWN) && menuEntrySelected < 2) {
        menuEntrySelected++;
      }

      if (gamepadPressed || code == SDL_SCANCODE_RETURN) {
        if (menuEntrySelected == 0) {
          p2.ai = !p2.ai;
        } else if (menuEntrySelected == 1) {
          initGame();
          if (p2.ai) { initAI(); }
          gameState = STATE_GAME;
        } else if (menuEntrySelected == 2) {
          quit = true;
        }
      }
    }

    if (gameState == STATE_GAME) {
      
      //-------------P1------------\\

      // Move cursor
      if (gamepad1Up || code == SDL_SCANCODE_W) {
        moveUp(&p1);
      } else if (gamepad1Down || code == SDL_SCANCODE_S) {
        moveDown(&p1);
      } else if (gamepad1Left || code == SDL_SCANCODE_A) {
        moveLeft(&p1);
      } else if (gamepad1Right || code == SDL_SCANCODE_D) {
        moveRight(&p1);
      }

      // Rotate cannon
      if (gamepad1RotateLeft || code == SDL_SCANCODE_G) {
        rotateCannon(&p1, -1);
      } else if (gamepad1RotateRight || code == SDL_SCANCODE_H) {
        rotateCannon(&p1, 1);
      }

      //-------------P2------------\\

      if (!p2.ai) {

        // Move cursor
        if (gamepad2Up || code == SDL_SCANCODE_UP) {
          moveUp(&p2);
        } else if (gamepad2Down || code == SDL_SCANCODE_DOWN) {
          moveDown(&p2);
        } else if (gamepad2Left || code == SDL_SCANCODE_LEFT) {
          moveLeft(&p2);
        } else if (gamepad2Right || code == SDL_SCANCODE_RIGHT) {
          moveRight(&p2);
        }

        // Rotate cannon
        if (gamepad2RotateLeft || code == SDL_SCANCODE_O) {
          rotateCannon(&p2, -1);
        } else if (gamepad2RotateRight || code == SDL_SCANCODE_P) {
          rotateCannon(&p2, 1);
        }

      }

    }     
  } else if (gameState == STATE_GAME && (e->type == SDL_JOYBUTTONUP || e->type == SDL_KEYUP)) {

      SDL_Scancode code = e->key.keysym.scancode; 

      //-------------P1------------\\

      // Place tile or fire cannon
      if (gamepad1Enter || code == SDL_SCANCODE_SPACE) {
        actionTile(&p1); // Execute action associated to this tile
        setTile(&p1);    // If unset, set new tile
      }

      //-------------P2------------\\

      if (!p2.ai) {

        // Place tile or fire cannon
        if (gamepad2Enter || code == SDL_SCANCODE_RETURN) {
          actionTile(&p2); // Execute action associated to this tile
          setTile(&p2);    // If unset, set new tile
        }

      }

  }

}

void update() {

  Uint32 now = SDL_GetTicks();

  // Update clouds
  for (Sint32 i = 0; i < MAX_CLOUDS; i++) {
    Sint32 lifeTime = now - clouds[i].startTime;
    if (lifeTime >= clouds[i].lifeTime) {
      clouds[i].size = randInRange(16, 64);
      clouds[i].vx = (float)randInRange(CLOUDS_MIN_SPEED, CLOUDS_MAX_SPEED) / 10.0f;
      clouds[i].x = randInRange(-400, SCREEN_WIDTH / 2);
      clouds[i].y = randInRange(60, 140);
      clouds[i].startTime = now;
      clouds[i].alpha = 0;
      clouds[i].lifeTime = randInRange(4000, 16000);
    }

    clouds[i].x += clouds[i].vx;
    if (lifeTime < 1000) {
      clouds[i].alpha = lerp(0, 255, ((float)lifeTime)/1000);
    } else if (lifeTime > 3000) {
      clouds[i].alpha = lerp(255, 0, ((float)(lifeTime-3000))/1000);
    }

    if (clouds[i].alpha < 0) { clouds[i].alpha = 0; }
  }
  
  if (gameState != STATE_GAME) {
    return;
  }

  // Increase ore
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
        p1.balls[i].emitter->alive = false;
        continue;
      }

      p1.balls[i].vy += world.gravity;
      p1.balls[i].x += p1.balls[i].vx;
      p1.balls[i].y += p1.balls[i].vy;

      p1.balls[i].emitter->x = p1.balls[i].x;
      p1.balls[i].emitter->y = p1.balls[i].y;
      
      handleBallCollision(&p1, &p1.balls[i]);
      handleBallCollision(&p2, &p1.balls[i]);

    }
    
    //-------------P2------------\\

    if (p2.balls[i].alive) {

      if (p2.balls[i].y > SCREEN_HEIGHT) {
        p2.balls[i].alive = false;
        p2.balls[i].emitter->alive = false;
        continue;
      }

      p2.balls[i].vy += world.gravity;
      p2.balls[i].x += p2.balls[i].vx;
      p2.balls[i].y += p2.balls[i].vy;

      p2.balls[i].emitter->x = p2.balls[i].x;
      p2.balls[i].emitter->y = p2.balls[i].y;

      handleBallCollision(&p2, &p2.balls[i]);
      handleBallCollision(&p1, &p2.balls[i]);
      
    }
  }

  // Surprise
  if (surprise.alive) {

    if (surprise.alpha <= 0) {
      surprise.alive = false;
      surprise.spawning = false;
    }
    else { surprise.alpha--; }

  } else if (!surprise.spawning && now - surprise.lastSpawnTime >= surprise.nextSpawnTime){
    spawnSurprise();
  }

  // Update particles
  updateParticles();

  // Update AI
  if (p2.ai) {
    updateAI();
  }
}

void render() {

    // Clear screen
    SDL_RenderClear(renderer);

    // Render suprise zone
    //renderSurpriseZone();

    // Bg color
    SDL_SetRenderDrawColor(renderer, 0x63, 0x9b, 0xff, 0xff);

    // Render clouds
    renderClouds();

    // Render castles
    renderCastle(&p1);
    renderCastle(&p2);

    if (gameState == STATE_GAME) {
      
      // Render surprise
      renderSurprise();

      // Render current tile
      renderTile(&p1);
      renderTile(&p2);

      // Render cursors
      renderCursor(&p1);
      renderCursor(&p2);

      // Render balls
      renderBalls(&p1); 
      renderBalls(&p2); 

      // Render particles
      renderParticles();

      // Render text
      renderOre(&p1);
      renderOre(&p2);

      // Render winner text
      renderWinnerText();

    } else if (gameState == STATE_MENU) {

      // Render menu
      renderMainMenu();

    }
   
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
