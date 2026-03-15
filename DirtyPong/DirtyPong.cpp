/*This source code is a modified version of the original code found in the SDL2 tutorial at lazyfoo.net
* created for learning purposes.

It is largely based on copyrighted by Lazy Foo' Productions 2004-2024
and may not be redistributed without written permission.*/

//Using SDL and standard IO
#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <string>
#include <cmath>
#include <time.h>

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

// misc constants
const int DIVIDER_BLOCK_WIDTH = 10;
const int DIVIDER_BLOCK_HEIGHT = DIVIDER_BLOCK_WIDTH;
const int PLAYER_PADDLE_WIDTH = DIVIDER_BLOCK_WIDTH;
const int PLAYER_PADDLE_HEIGHT = SCREEN_HEIGHT / 4;
const float PLAYER_PADDLE_SPEED = 1000.0;
const float PLACEHOLDER_DELTA_TIME = 1.0 / 144.0;

//Starts up SDL and creates window
bool init();

//Loads media
bool loadMedia();

//Frees media and shuts down SDL
void close();


//Loads individual image
SDL_Surface* loadSurface(std::string path);

// Renders a rectangle using its center as the orign
void renderRect(SDL_Renderer* renderer,
	const SDL_Rect* rectPtr);

//Key press surfaces constants
enum KeyPressTextures
{
	KEY_PRESS_TEXTURE_DEFAULT,
	KEY_PRESS_TEXTURE_UP,
	KEY_PRESS_TEXTURE_DOWN,
	KEY_PRESS_TEXTURE_LEFT,
	KEY_PRESS_TEXTURE_RIGHT,
	KEY_PRESS_TEXTURE_TOTAL
};

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

// The window renderer
SDL_Renderer* gRenderer = NULL;

// Current displayed texture
SDL_Texture* gCurrentTexture = NULL;
	
//The surface contained by the window
SDL_Surface* gScreenSurface = NULL;

// The textures taht correspond to a keypress
SDL_Texture* gKeyPressTextures[KEY_PRESS_TEXTURE_TOTAL] = {};

SDL_Rect stretchRect;

bool init()
{
	// Initialize stretch rectangle to be the entire screen
	stretchRect.x = 0;
	stretchRect.y = 0;
	stretchRect.w = SCREEN_WIDTH;
	stretchRect.h = SCREEN_HEIGHT;

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		return false;
	}

	//Create window
	gWindow = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (gWindow == NULL)
	{
		printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		return false;
	}

	// Using rednerer accelerated which uses the GPU to render
	// wondering if it's possible to bitwise OR these and what the purpose 
	// if maybe this should have 0 passed so SDL will try to use renderer?
	// Is there automatic selection/fallback if I don't specify requirement via
	// flags?
	gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	if (!gRenderer)
	{
		gRenderer = SDL_CreateRenderer(gWindow, -1, 0);
	}

	if (!gRenderer)
	{
		printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
		return false;
	}

	// Initialize the rendering color
	SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

	// Initialize PNG loading
	int imgFlags = IMG_INIT_PNG;
	if (!IMG_Init(imgFlags) & imgFlags)
	{
		printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
		return false;
	}

	//Get window surface
	gScreenSurface = SDL_GetWindowSurface(gWindow);
	if (gScreenSurface == NULL)
	{
		printf("Could not get window surface! SDL_Error: %s\n", SDL_GetError());
		return false;
	}
	
	return true;
}

bool loadMedia()
{
	//Load default surface
	gKeyPressTextures[KEY_PRESS_TEXTURE_DEFAULT] = IMG_LoadTexture(gRenderer, "pngs/press.png");
	
	// Question to return to, is it better to ! for these?
	if (gKeyPressTextures[KEY_PRESS_TEXTURE_DEFAULT] == NULL)
	{
		printf("Failed to load default image!\n");
		return false;
	}

	//Load up surface
	gKeyPressTextures[KEY_PRESS_TEXTURE_UP] = IMG_LoadTexture(gRenderer, "pngs/up.png");
	if (gKeyPressTextures[KEY_PRESS_TEXTURE_UP] == NULL)
	{
		printf("Failed to load up image!\n");
		return false;
	}

	//Load down surface
	gKeyPressTextures[KEY_PRESS_TEXTURE_DOWN] = IMG_LoadTexture(gRenderer, "pngs/down.png");
	if (gKeyPressTextures[KEY_PRESS_TEXTURE_DOWN] == NULL)
	{
		printf("Failed to load down image!\n");
		return false;
	}

	//Load left surface
	gKeyPressTextures[KEY_PRESS_TEXTURE_LEFT] = IMG_LoadTexture(gRenderer, "pngs/left.png");
	if (gKeyPressTextures[KEY_PRESS_TEXTURE_LEFT] == NULL)
	{
		printf("Failed to load left image!\n");
		return false;
	}

	//Load right surface
	gKeyPressTextures[KEY_PRESS_TEXTURE_RIGHT] = IMG_LoadTexture(gRenderer, "pngs/right.png");
	if (gKeyPressTextures[KEY_PRESS_TEXTURE_RIGHT] == NULL)
	{
		printf("Failed to load right image!\n");
		return false;
	}

	return true;
}

SDL_Surface* loadSurface(std::string path)
{
	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
	if (loadedSurface == NULL)
	{
		printf("Unable to load image %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
		return NULL;
	}

	// Convert surface to screen format
	SDL_Surface* optimizedSurface = SDL_ConvertSurface(loadedSurface, gScreenSurface->format, 0);

	if (optimizedSurface == NULL)
	{
		printf("Unable to optimize image %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
		return NULL;
	}
	
	// Free the original loaded surface
	SDL_FreeSurface(loadedSurface);
	loadedSurface = optimizedSurface;

	// Set the optimized surface to NULL to avoid dangling pointer issues.
	optimizedSurface = NULL;

	return loadedSurface;
}

void close()
{
	// Free only the textures we explcititly loaded in to gKeyPressTextures
	for (int i = 0; i < KEY_PRESS_TEXTURE_TOTAL; i++)
	{
		if (gKeyPressTextures[i] != NULL)
		{
			// SDl_FreeSurface frees memory associated with the surface
			SDL_DestroyTexture(gKeyPressTextures[i]);
			// I manually set the pointer to NULL to avoid dangling pointer issues.
			gKeyPressTextures[i] = NULL;
		}
	}

	// gScreenSurface is owned by the window (returned by SDL_GetWindowSurface).
	// The lesson here is taht you only free memory that you allocate
	// So for the surfaces I allocate in loadMedia need me to free the memory, otherwise
	// that data stays there. Please note: even though we don't own the memory allocated for gScreenSurface
	// we still own the pointer 
	// Do not call SDL_FreeSurface on it.
	gScreenSurface = NULL;

	// Destroy Renderer
	if (gRenderer != NULL)
	{
		SDL_DestroyRenderer(gRenderer);
		gRenderer = NULL;
	}

	//Destroy window
	if (gWindow != NULL)
	{
		SDL_DestroyWindow(gWindow);
		gWindow = NULL;
	}

	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}

void renderRect(SDL_Renderer* renderer,
	const SDL_Rect* rectPtr)
{
	// x, y, w, h
	SDL_Rect renderRect = 
	{
		rectPtr->x - (rectPtr->w)/2, 
		rectPtr->y - (rectPtr->h)/2,
		rectPtr->w,
		rectPtr->h
	};

	SDL_RenderFillRect(gRenderer, &renderRect);
}

int main( int argc, char* args[] )
{
	//Start up SDL and create window
	if( !init() )
	{
		printf( "Failed to initialize!\n" );
		return -1;
	}

	//Load media
	if( !loadMedia() )
	{
		printf( "Failed to load media!\n" );
		return -1;
	}

	// Quit flag
	bool quit = false;

	//Event handler
	SDL_Event e;

	//Set default current texture
	gCurrentTexture = gKeyPressTextures[KEY_PRESS_TEXTURE_DEFAULT];

	// Paddle state init
	float leftPlayerPaddleY = SCREEN_HEIGHT / 2.0;
	float rightPlayerPaddleY = SCREEN_HEIGHT / 2.0;

	// Input state  init
	// In a better game this should be a bitmask
	bool upHeld = false;
	bool downHeld = false;
	bool wHeld = false;
	bool sHeld = false;

	float deltaTime = PLACEHOLDER_DELTA_TIME;

	// Game Loop
	while (!quit)
	{

		//int deltaTime = ts.tv_nsec
		// Game Loop 1. Collect input/events in queue
		// Handle events on queue
		// SDL_PollEvent() returns 1 if there are any events in the queue, otherwise it returns 0.
		while (SDL_PollEvent(&e) != 0)
		{
			//User requests quit
			if (e.type == SDL_QUIT)
			{
				quit = true;
				return 0;
			}
			
			// Handle keypress
			if (e.type == SDL_KEYDOWN)
			{
				switch (e.key.keysym.sym)
				{
					// left player input
					case SDLK_w:
						//gCurrentTexture = gKeyPressTextures[KEY_PRESS_TEXTURE_LEFT];
						wHeld = true;
						break;
					case SDLK_s:
						//gCurrentTexture = gKeyPressTextures[KEY_PRESS_TEXTURE_RIGHT];
						//leftPlayerPaddleY += PLAYER_PADDLE_SPEED*deltaTime;
						sHeld = true;
						break;

					// right player input
					case SDLK_UP:
						//gCurrentTexture = gKeyPressTextures[KEY_PRESS_TEXTURE_UP];
						upHeld = true;
						break;
					case SDLK_DOWN:
						//gCurrentTexture = gKeyPressTextures[KEY_PRESS_TEXTURE_DOWN];
						downHeld = true;
						break;
					default:
						//gCurrentTexture = gKeyPressTextures[KEY_PRESS_TEXTURE_DEFAULT];
						break;
				}
			} 
			else if (e.type == SDL_KEYUP)
			{
				switch (e.key.keysym.sym)
				{
					// left player input
				case SDLK_w:
					//gCurrentTexture = gKeyPressTextures[KEY_PRESS_TEXTURE_LEFT];
					wHeld = false;
					break;
				case SDLK_s:
					sHeld = false;
					break;

					// right player input
				case SDLK_UP:
					upHeld = false;
					//gCurrentTexture = gKeyPressTextures[KEY_PRESS_TEXTURE_UP];
					break;
				case SDLK_DOWN:
					downHeld = false;
					//gCurrentTexture = gKeyPressTextures[KEY_PRESS_TEXTURE_DOWN];
					break;
				default:
					//gCurrentTexture = gKeyPressTextures[KEY_PRESS_TEXTURE_DEFAULT];
					break;
				}
			}
		}


		// 2. Update the game state
		if (wHeld)
		{
			leftPlayerPaddleY -= (PLAYER_PADDLE_SPEED * deltaTime);
		}

		if (sHeld && (leftPlayerPaddleY <= (SCREEN_HEIGHT - PLAYER_PADDLE_HEIGHT / 2)))
		{
			leftPlayerPaddleY += PLAYER_PADDLE_SPEED*deltaTime;
		}

		if (upHeld)
		{
			rightPlayerPaddleY -= (PLAYER_PADDLE_SPEED * deltaTime);
		}

		if (downHeld)
		{
			rightPlayerPaddleY += PLAYER_PADDLE_SPEED * deltaTime;
		}

		// Clamp to boundaries
		if (leftPlayerPaddleY <= PLAYER_PADDLE_HEIGHT / 2)
		{
			leftPlayerPaddleY = PLAYER_PADDLE_HEIGHT / 2;
		}

		if (leftPlayerPaddleY >= (SCREEN_HEIGHT - PLAYER_PADDLE_HEIGHT / 2))
		{
			leftPlayerPaddleY = (SCREEN_HEIGHT - PLAYER_PADDLE_HEIGHT / 2);
		}

		if (rightPlayerPaddleY <= PLAYER_PADDLE_HEIGHT / 2)
		{
			rightPlayerPaddleY = PLAYER_PADDLE_HEIGHT / 2;
		}

		if (rightPlayerPaddleY >= (SCREEN_HEIGHT - PLAYER_PADDLE_HEIGHT / 2))
		{
			rightPlayerPaddleY = (SCREEN_HEIGHT - PLAYER_PADDLE_HEIGHT / 2);
		}

		// 3. Render the screen

		// Clear screen
		SDL_RenderClear(gRenderer);

		// Render texture to screen
		SDL_RenderCopy(gRenderer, gCurrentTexture, NULL, NULL);

		// Top left corner of screen is 0,0 and bottom right corner is 640,480
		// Render a filled quad
		//SDL_Rect fillRect = { SCREEN_WIDTH / 4, SCREEN_HEIGHT / 4, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
		//SDL_SetRenderDrawColor(gRenderer, 0xFF, 0x00, 0x00, 0xFF);
		//SDL_RenderFillRect(gRenderer, &fillRect);

		//// Render a green outlined quad
		//SDL_Rect outlineRect = { SCREEN_WIDTH / 6, SCREEN_HEIGHT / 6, SCREEN_WIDTH * 2 / 3, SCREEN_HEIGHT * 2 / 3 };
		//SDL_SetRenderDrawColor(gRenderer, 0x00, 0xFF, 0x00, 0xFF);
		//SDL_RenderDrawRect(gRenderer, &outlineRect);

		//// Draw a blue horizontal line
		//SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0xFF, 0xFF);
		//SDL_RenderDrawLine(gRenderer, 0, SCREEN_HEIGHT / 2, SCREEN_WIDTH, SCREEN_HEIGHT / 2);

		// Draw a vertical line of black dots
		SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 0xFF);
		
		for (int i = DIVIDER_BLOCK_HEIGHT; i < SCREEN_HEIGHT; i += (DIVIDER_BLOCK_HEIGHT*2))
		{
			SDL_Rect dotRect = { SCREEN_WIDTH / 2, i, DIVIDER_BLOCK_WIDTH, DIVIDER_BLOCK_HEIGHT };
			renderRect(gRenderer, &dotRect);
		}

		// Draw player rectangles
		// Player 1
		SDL_Rect fillRect = { PLAYER_PADDLE_WIDTH, (int)leftPlayerPaddleY, DIVIDER_BLOCK_WIDTH, PLAYER_PADDLE_HEIGHT };
		SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xFF);
		renderRect(gRenderer, &fillRect);

		// Player 2
		fillRect = { SCREEN_WIDTH - PLAYER_PADDLE_WIDTH,(int)rightPlayerPaddleY, DIVIDER_BLOCK_WIDTH, PLAYER_PADDLE_HEIGHT };
		SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xFF);
		renderRect(gRenderer, &fillRect);

		// Update screen
		SDL_RenderPresent(gRenderer);
	}

	//Free resources and close SDL
	close();

	return 0;
}
