const Sint32 MAX_EMITTERS = 64;
const Sint32 MAX_PARTICLES = 128;
const Sint32 PARTICLES_SPAWN_TIME = 0.1;

Sint32 w, h;
SDL_Texture* particleTexture;

struct Particle {
  Sint32 x, y;
  Sint32 angle;
  Uint8 alpha;
  bool alive;
};

struct Emitter {
  Sint32 x, y;
  Sint32 angle;
  Sint32 amount;
  Sint32 lastEmitTime;
  bool alive;
};

Emitter emitters[MAX_EMITTERS];
Particle particles[MAX_PARTICLES];

// Utils
Emitter* createEmitter(Sint32 x, Sint32 y) {

  for (Sint32 i = 0; i < MAX_EMITTERS; i++) {

    Emitter* emitter = &emitters[i];

    if (!emitter->alive) {

      emitter->x = x;
      emitter->y = y;
      emitter->alive = true;
      emitter->lastEmitTime = 0;
      return emitter;

    }
  }

  printf("[particles] no more emitters!\n");
  return NULL;

}

Particle* getDeadParticle() {

  for (Sint32 i = 0; i < MAX_PARTICLES; i++) {
    if (!particles[i].alive) { return &particles[i]; }
  }

  //printf("[particles] no more particles!\n");
  return NULL;
  
}

void emit(Emitter* emitter) {

  Particle* particle = getDeadParticle();

  if (particle) {
    particle->x = emitter->x; // + randInRange(-2, 2);
    particle->y = emitter->y; // + randInRange(-2, 2);
    particle->alpha = 100;
    particle->angle = emitter->angle;
    particle->alive = true;
    emitter->lastEmitTime = SDL_GetTicks();
  }
  
}

// Main
void initParticles() {
 
  // Load texture
  particleTexture = loadTexture("assets/particle.png");
  SDL_QueryTexture(particleTexture, NULL, NULL, &w, &h);

  for (Sint32 i = 0; i < MAX_EMITTERS; i++) {
    emitters[i].alive = false;
  }
 
  for (Sint32 i = 0; i < MAX_PARTICLES; i++) {
    particles[i].alive = false;
    particles[i].alpha = 0;
  }

}

void updateParticles() {

  Uint32 now = SDL_GetTicks();

  // Update emitters
  for (Sint32 i = 0; i < MAX_EMITTERS; i++) {
   
    Emitter* emitter = &emitters[i];

    if (!emitter->alive) { continue; }

    if (now - emitter->lastEmitTime >= PARTICLES_SPAWN_TIME) { emit(emitter); }

  }

  // Update particles
  for (Sint32 i = 0; i < MAX_PARTICLES; i++) {

    Particle* particle = &particles[i];

    if (!particle->alive) { continue; }

    particle->alpha--;

    if (particle->alpha <= 0) { particle->alive = false; }

  }

}

void renderParticles() {

  for (Sint32 i = 0; i < MAX_PARTICLES; i++) {

    Particle* particle = &particles[i];

    if (!particle->alive) { continue; }
    
    renderTexture(particleTexture, particle->x, particle->y, w, h, particle->angle, particle->alpha);

  }

}

void closeParticles() {

  SDL_DestroyTexture(particleTexture);
  particleTexture = NULL;

}
