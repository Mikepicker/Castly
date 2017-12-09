// Renderer
SDL_Renderer* renderer = NULL;

// The window we'll be rendering to
SDL_Window* window = NULL;

// Font
TTF_Font* font = NULL;

// Init
bool initFramework() {

  // Initialization flag
  bool success = true;

  // Initialize SDL
  if(SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    success = false;
  }
  else {

    // Create window
    window = SDL_CreateWindow("Zombies", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
      printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
      success = false;
    }
    else {
      // Create renderer
      renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
      if (renderer == NULL) {
        printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
        success = false;
      } else {

        // Init renderer color
        SDL_SetRenderDrawColor(renderer, 0xdf, 0xda, 0xd2, 0xff);

        // Init png loading
        int imgFlags = IMG_INIT_PNG;
        if (!(IMG_Init(imgFlags) & imgFlags)) {
          printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
          success = false;
        }

        //Initialize SDL_ttf
        if(TTF_Init() == -1) {
          printf( "SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError() );
          success = false;
        }
      }
    }
  }

  return success;
}

// Init
void closeFramework() {

  //Destroy window
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  renderer = NULL;
  window = NULL;

  // Free font
  TTF_CloseFont(font);
  font = NULL;

  // Quit SDL subsystems
  TTF_Quit();
  IMG_Quit();
  SDL_Quit();
}

// Utils
SDL_Texture* loadTexture(std::string path) {

  SDL_Texture* newTexture = NULL;

  // Load image
  SDL_Surface* loadedSurface = IMG_Load(path.c_str());
  if (loadedSurface == NULL) {
    printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError());
  } else {

    newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
    if (newTexture == NULL) {
      printf("Unable to create texture from %s! SDL_Error: %s\n", path.c_str(), SDL_GetError());
    }

    SDL_SetTextureBlendMode(newTexture, SDL_BLENDMODE_BLEND);

    SDL_FreeSurface(loadedSurface);
  }

  return newTexture;
}

void renderTexture(SDL_Texture* texture, Sint32 x, Sint32 y, Sint32 w, Sint32 h, double angle, Uint8 alpha) {

  SDL_SetTextureAlphaMod(texture, alpha);
  SDL_Rect dst = { .x = x, .y = y, .w = w, .h = h };
  SDL_RenderCopyEx(renderer, texture, NULL, &dst, angle, NULL, SDL_FLIP_NONE);

}

bool renderText(TTF_Font* font, std::string textureText, SDL_Color textColor, SDL_Texture** texture) {

  // Free old texture
  SDL_DestroyTexture(*texture);

  SDL_Texture* newTexture;

  SDL_Surface* textSurface = TTF_RenderText_Solid(font, textureText.c_str(), textColor);
  if (textSurface == NULL) {
    printf("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
  } else {
    newTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

    if (newTexture == NULL) {
      printf("Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError());
    }

    SDL_FreeSurface(textSurface);
  }

  *texture = newTexture;
  return newTexture != NULL;
}

bool collision(SDL_Rect a, SDL_Rect b) {
  if (a.y + a.h <= b.y) {
    return false;
  }

  if (a.y >= b.y + b.h) {
    return false;
  }

  if (a.x + a.w <= b.x) {
    return false;
  }

  if (a.x >= b.x + b.w) {
    return false;
  }

  return true;
}

int randInRange(int min, int max) {
  return rand() % (max + 1 - min) + min;
}
