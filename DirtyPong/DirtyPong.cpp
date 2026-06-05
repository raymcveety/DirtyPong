/*This source code is a modified version of the original code found in the SDL2 tutorial at lazyfoo.net
* created for learning purposes.

It is largely based on copyrighted by Lazy Foo' Productions 2004-2024
and may not be redistributed without written permission.*/

//Using SDL and standard IO
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <string>
#include <cmath>
#include <time.h>
#include <random>

//Screen dimension constexprants
constexpr int SCREEN_WIDTH = 640;
constexpr int SCREEN_HEIGHT = 480;
constexpr int MAX_POINTS = 10;

// misc constexprants
constexpr int DIVIDER_BLOCK_WIDTH = 10;
constexpr int DIVIDER_BLOCK_HEIGHT = DIVIDER_BLOCK_WIDTH;
constexpr int PLAYER_PADDLE_WIDTH = DIVIDER_BLOCK_WIDTH;
constexpr int PADDLE_MARGIN = 20;
constexpr int BALL_WIDTH = DIVIDER_BLOCK_WIDTH * 2;
constexpr int BALL_HEIGHT = DIVIDER_BLOCK_HEIGHT * 2;
constexpr int PLAYER_PADDLE_HEIGHT = SCREEN_HEIGHT / 4;
constexpr float PLAYER_PADDLE_SPEED = 1000.0;
constexpr float PLACEHOLDER_DELTA_TIME = 1.0 / 144.0;
constexpr float STARTING_BALL_SPEED = 1.5;
constexpr float BALL_STARTING_SPEED_SCALAR = 2.0;

constexpr float LEFT_PLAYER_PADDLE_Y_START = SCREEN_HEIGHT / 2.0;
constexpr float RIGHT_PLAYER_PADDLE_Y_START = SCREEN_HEIGHT / 2.0;

// Struct for vector2 representation
// for a project this small better learning
// experience to use my own data type. From
// research an open source alternative is OpenGL
// math library. (cannot find link anymore, got replaced with a github link, removing for now)
// provided 3/21/2026, not verified since
struct Vector2 {
	float x, y;
};

//Starts up SDL and creates window
bool init();

//Loads media
bool loadMedia();

//Frees media and shuts down SDL
void close();

// Frees text based textures
//void freeTextTexture();
void freeTextTexture(SDL_Texture*);

//Loads individual image
SDL_Surface* loadSurface(std::string path);

// Renders a rectangle using its center as the orign
void renderRect(SDL_Renderer* renderer,
	const SDL_Rect* rectPtr);

// Gives random value between -1 and 1 but excludes values that yield boring game start if ball
// is in center
Vector2 randomizeBallDirection();

// Spawns a ball in the center of the screen with a randomized direction and speed
void respawnBall(bool isServe, float* ballSpeedScalarPtr, float* ballXptr, float* ballYptr, float* ballDirectionPtrX, float* ballDirectionPtrY);

//Key press surfaces constexprants
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

TTF_Font* globalFont;

struct ScoreBoard 
{
	SDL_Texture* player1ScoreBoardTexture;
	SDL_Texture* player2ScoreBoardTexture;

	int player1ScoreBoardWidth;
	int player1ScoreBoardHeight;

	int player2ScoreBoardWidth;
	int player2ScoreBoardHeight;
};

ScoreBoard* scoreBoard;

SDL_Texture* loadFromRenderedText(std::string textureText, SDL_Color textColor);
SDL_Surface* gTextSurface;
SDL_Color textColor = { 255, 255, 255 };

SDL_Texture* startScreenTexture;

enum GameState
{
	PREGAME,
	PAUSED,
	PLAYING,
	SERVING,
	GAMEOVER
};

GameState currentGameState;

bool init()
{
	// Seed random number generator
	srand(time(NULL));

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

	if (TTF_Init() != 0)
	{
		printf("Could not initialize TTF library! TTF_Error: %s\n", TTF_GetError());
	}

	// init ScoreBoard
	scoreBoard = new ScoreBoard;
	scoreBoard->player1ScoreBoardTexture = NULL;
	scoreBoard->player2ScoreBoardTexture = NULL;
	
	return true;
}

bool loadMedia()
{
	bool success = true;
	//Load default surface
	//gKeyPressTextures[KEY_PRESS_TEXTURE_DEFAULT] = IMG_LoadTexture(gRenderer, "pngs/press.png");
	
	//// Question to return to, is it better to ! for these?
	//if (gKeyPressTextures[KEY_PRESS_TEXTURE_DEFAULT] == NULL)
	//{
	//	printf("Failed to load default image!\n");
	//	return false;
	//}

	////Load up surface
	//gKeyPressTextures[KEY_PRESS_TEXTURE_UP] = IMG_LoadTexture(gRenderer, "pngs/up.png");
	//if (gKeyPressTextures[KEY_PRESS_TEXTURE_UP] == NULL)
	//{
	//	printf("Failed to load up image!\n");
	//	return false;
	//}

	////Load down surface
	//gKeyPressTextures[KEY_PRESS_TEXTURE_DOWN] = IMG_LoadTexture(gRenderer, "pngs/down.png");
	//if (gKeyPressTextures[KEY_PRESS_TEXTURE_DOWN] == NULL)
	//{
	//	printf("Failed to load down image!\n");
	//	return false;
	//}

	////Load left surface
	//gKeyPressTextures[KEY_PRESS_TEXTURE_LEFT] = IMG_LoadTexture(gRenderer, "pngs/left.png");
	//if (gKeyPressTextures[KEY_PRESS_TEXTURE_LEFT] == NULL)
	//{
	//	printf("Failed to load left image!\n");
	//	return false;
	//}

	////Load right surface
	//gKeyPressTextures[KEY_PRESS_TEXTURE_RIGHT] = IMG_LoadTexture(gRenderer, "pngs/right.png");
	//if (gKeyPressTextures[KEY_PRESS_TEXTURE_RIGHT] == NULL)
	//{
	//	printf("Failed to load right image!\n");
	//	return false;
	//}

	//globalFont = TTF_OpenFont("./fonts/lazy.ttf", 28);
	globalFont = TTF_OpenFont("./fonts/Archivo/ttf/Archivo-Bold.ttf", 28);

	// Load global font
	if (globalFont == NULL)
	{
		printf("Could not load global font in loadMedia. TTF_Error: '%s'", TTF_GetError());
		return false;
	}

	// RenderText
	//if (!loadFromRenderedText("THIS IS SOME RENDERED TEXT", textColor))
	//{
	//	printf("Could not loadFromRenderedText in loadMedia. Returning false");
	//	return false;
	//}

	//if (scoreBoard->player1ScoreBoardTexture == NULL)
	//{
	//	printf("Unable to create texture from text surface. SDL_Error: %s", SDL_GetError());
	//	return false;
	//}

	//if (loadFromRenderedText("Player 1: ", textColor))
	//{
	//	printf("Could not loadFromRenderedText in loadMedia. Returning false");
	//	return false;
	//}
	freeTextTexture(scoreBoard->player1ScoreBoardTexture);
	freeTextTexture(scoreBoard->player2ScoreBoardTexture);
	scoreBoard->player1ScoreBoardWidth = 0;
	scoreBoard->player1ScoreBoardHeight = 0;
	scoreBoard->player1ScoreBoardTexture = loadFromRenderedText("0", textColor);

	if (scoreBoard->player1ScoreBoardTexture == NULL) {
		printf("Could not loadFromRenderedText in loadMedia. Returning false");
		return false;
	}
	
	scoreBoard->player1ScoreBoardWidth = gTextSurface->w;
	scoreBoard->player1ScoreBoardHeight = gTextSurface->h;

	SDL_FreeSurface(gTextSurface);

	scoreBoard->player2ScoreBoardTexture = loadFromRenderedText("0", textColor);

	if (scoreBoard->player2ScoreBoardTexture == NULL) {
		printf("Could not loadFromRenderedText in loadMedia. Returning false");
		return false;
	}

	scoreBoard->player2ScoreBoardWidth = gTextSurface->w;
	scoreBoard->player2ScoreBoardHeight = gTextSurface->h;

	SDL_FreeSurface(gTextSurface);

	return true;
}

SDL_Texture* loadFromRenderedText(std::string textureText, SDL_Color textColor) {

	//freeTextTexture();
	SDL_Texture* loadedTexture;

	gTextSurface = TTF_RenderText_Solid(globalFont, textureText.c_str(), textColor);

	if (gTextSurface == NULL)
	{
		printf("Unable to load text as surface from the string '%s', TTF_Error: '%s'", textureText.c_str(), TTF_GetError());
		return NULL;
	}

	//scoreBoard->player1ScoreBoardTexture = SDL_CreateTextureFromSurface(gRenderer, textSurface);
	loadedTexture = SDL_CreateTextureFromSurface(gRenderer, gTextSurface);

	//if (scoreBoard->player1ScoreBoardTexture == NULL)
	//{
	//	printf("Unable to create texture from text surface. SDL_Error: %s", SDL_GetError());
	//	return false;
	//}

	SDL_FreeSurface(gTextSurface);

	//return scoreBoard->player1ScoreBoardTexture != NULL;
	return loadedTexture;
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

//void freeTextTexture() 
//{
//	if (scoreBoard->player1ScoreBoardTexture != NULL)
//	{
//		SDL_DestroyTexture(scoreBoard->player1ScoreBoardTexture);
//		scoreBoard->player1ScoreBoardTexture = NULL;
//		scoreBoard->player1ScoreBoardWidth = 0;
//		scoreBoard->player1ScoreBoardHeight = 0;
//	}
//
//	if (scoreBoard->player2ScoreBoardTexture != NULL)
//	{
//		SDL_DestroyTexture(scoreBoard->player2ScoreBoardTexture);
//		scoreBoard->player2ScoreBoardTexture = NULL;
//		scoreBoard->player2ScoreBoardWidth = 0;
//		scoreBoard->player2ScoreBoardHeight = 0;
//	}
//}

void freeTextTexture(SDL_Texture* textureToFree)
{
	if (textureToFree != NULL)
	{
		SDL_DestroyTexture(textureToFree);
		textureToFree = NULL;
	}
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

	//Free global font
	TTF_CloseFont(globalFont);
	globalFont = NULL;

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

Vector2 randomizeBallDirection()
{
	/*float resultX; 
	do
	{
		resultX = 1.0 - (((float)(rand() % 20) + 1.0)/10.0);
		printf("result %f \n", resultX);
	} while (abs(resultX) < 0.7);

	float resultY;
	do
	{
		resultY = 1.0 - (((float)(rand() % 20) + 1.0) / 10.0);
		printf("result %f \n", resultY);
	} while (abs(resultY) > 0.4);*/

	Vector2 result;
	do
	{
		result.x = 1.0 - (((float)(rand() % 20) + 1.0) / 10.0);
		result.y = 1.0 - (((float)(rand() % 20) + 1.0) / 10.0);
	} while ((abs(result.x) < 0.7) || (abs(result.y) > 0.5));

	return result;
}

void respawnBall(bool isServe, float* ballSpeedScalarPtr, float* ballXptr, float* ballYptr, float* ballDirectionPtrX, float* ballDirectionPtrY)
{
	*ballXptr = SCREEN_WIDTH / 2.0;
	*ballYptr = SCREEN_HEIGHT / 2.0;
	*ballSpeedScalarPtr = 0.0;

	if (isServe)
	{
		Vector2 randomBallDirection = randomizeBallDirection();
		*ballDirectionPtrX = randomBallDirection.x;
		*ballDirectionPtrY = randomBallDirection.y;
		*ballSpeedScalarPtr = BALL_STARTING_SPEED_SCALAR;
	}
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

	//Event data structure
	SDL_Event e;

	//Set default current texture
	gCurrentTexture = gKeyPressTextures[KEY_PRESS_TEXTURE_DEFAULT];

	// Game state init
	bool ballReadyToServe = false;

	// Paddle state init
	float leftPlayerPaddleY = LEFT_PLAYER_PADDLE_Y_START;
	float rightPlayerPaddleY = RIGHT_PLAYER_PADDLE_Y_START;

	// Ball state init
	float ballX = SCREEN_WIDTH / 2.0;
	float ballY = SCREEN_HEIGHT / 2.0;
	int xDirection = 1;
	int yDirection = 1;
	float ballSpeedScalar = BALL_STARTING_SPEED_SCALAR;
	int winner = 0;

	// Old code. Trying to randomize the starting position
	/*float ballSpeedX = randomizeBallDirection();
	float ballSpeedY = randomizeBallDirection();*/
	Vector2 ballDirection = randomizeBallDirection();


	// Input state  init
	// In a better game this should be a bitmask
	bool upHeld = false;
	bool downHeld = false;
	bool wHeld = false;
	bool sHeld = false;
	bool respawnBallHeld = false;
	bool startPressed = false;
	int player1Score = 0;
	int player2Score = 0;

	float deltaTime = PLACEHOLDER_DELTA_TIME;

	currentGameState = PREGAME;

	// temporarily starting in playing isolate memory leak
	//currentGameState = PLAYING;

	// Game Loop
	while (!quit)
	{
		//// 1. Collect input
		//int deltaTime = ts.tv_nsec
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

					// general input
					case SDLK_r:
						respawnBallHeld = true;
						break;

					case SDLK_p:
						startPressed = true;
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

					// general input
					case SDLK_r:
						respawnBallHeld = false;
						break;

					case SDLK_p:
						startPressed = false;
					default:
						//gCurrentTexture = gKeyPressTextures[KEY_PRESS_TEXTURE_DEFAULT];
						break;
				}
			}
		}


		//// 2. Update the game state
		// Move ball
		//ballX = ballX + ballSpeedX;
		//ballY = ballY + ballSpeedY;
		//printf("ballspeedx, y (%f, %f) \n", ballSpeedX, ballSpeedY);

		// Handle player input
		if (startPressed)
		{
			// Reset game state
			if (currentGameState == GAMEOVER)
			{
				player1Score = 0;
				player2Score = 0;
			}
			if (currentGameState != SERVING)
			{
				currentGameState = PLAYING;
			}
		}

		if (currentGameState == PLAYING && wHeld)
		{
			leftPlayerPaddleY -= (PLAYER_PADDLE_SPEED * deltaTime);
		}

		if (currentGameState == PLAYING && sHeld && (leftPlayerPaddleY <= (SCREEN_HEIGHT - PLAYER_PADDLE_HEIGHT / 2)))
		{
			leftPlayerPaddleY += PLAYER_PADDLE_SPEED*deltaTime;
		}

		if (currentGameState == PLAYING && upHeld)
		{
			rightPlayerPaddleY -= (PLAYER_PADDLE_SPEED * deltaTime);
		}

		if (currentGameState == PLAYING && downHeld)
		{
			rightPlayerPaddleY += PLAYER_PADDLE_SPEED * deltaTime;
		}

		// Clamp to boundaries
		if (currentGameState == PLAYING && leftPlayerPaddleY <= PLAYER_PADDLE_HEIGHT / 2)
		{
			leftPlayerPaddleY = PLAYER_PADDLE_HEIGHT / 2;
		}

		if (currentGameState == PLAYING && leftPlayerPaddleY >= (SCREEN_HEIGHT - PLAYER_PADDLE_HEIGHT / 2))
		{
			leftPlayerPaddleY = (SCREEN_HEIGHT - PLAYER_PADDLE_HEIGHT / 2);
		}

		if (currentGameState == PLAYING && rightPlayerPaddleY <= PLAYER_PADDLE_HEIGHT / 2)
		{
			rightPlayerPaddleY = PLAYER_PADDLE_HEIGHT / 2;
		}

		if (currentGameState == PLAYING && rightPlayerPaddleY >= (SCREEN_HEIGHT - PLAYER_PADDLE_HEIGHT / 2))
		{
			rightPlayerPaddleY = (SCREEN_HEIGHT - PLAYER_PADDLE_HEIGHT / 2);
		}

		// Move ball
		if (currentGameState == PLAYING) 
		{
			ballX = ballX + ballSpeedScalar * ballDirection.x;
			ballY = ballY + ballSpeedScalar * ballDirection.y;
		}

		// Check collision with paddles
		// ball y check is if ball bottom edge is > top edge of paddle AND
		// ball top edge < bottom edge of paddle, collision can occur
		if (((ballX + BALL_WIDTH / 2.0) > (SCREEN_WIDTH - PADDLE_MARGIN - PLAYER_PADDLE_WIDTH/2.0)) 
			&& ((ballY + BALL_HEIGHT/2.0) > (rightPlayerPaddleY - PLAYER_PADDLE_HEIGHT/2.0))
				&& ((ballY - BALL_HEIGHT/2.0) < (rightPlayerPaddleY + PLAYER_PADDLE_HEIGHT/2.0)))
		{
			ballX = SCREEN_WIDTH - PADDLE_MARGIN - (PLAYER_PADDLE_WIDTH / 2.0) - (BALL_WIDTH / 2.0);
			ballDirection.x *= -1;
		}

		if (((ballX - BALL_WIDTH / 2.0) < (PADDLE_MARGIN + PLAYER_PADDLE_WIDTH / 2.0))
			&& ((ballY + BALL_HEIGHT / 2.0) > (leftPlayerPaddleY - PLAYER_PADDLE_HEIGHT / 2.0))
			&& ((ballY - BALL_HEIGHT / 2.0) < (leftPlayerPaddleY + PLAYER_PADDLE_HEIGHT / 2.0)))
		{
			ballX = PADDLE_MARGIN + (PLAYER_PADDLE_WIDTH / 2.0) + (BALL_WIDTH / 2.0);
			ballDirection.x *= -1;
		}

		// Clamp ball to screen borders
		if ((ballX - BALL_WIDTH/2.0) < 0.0)
		{
			// Player 2 scores!!!
			player2Score++;
			ballReadyToServe = true;
			leftPlayerPaddleY = LEFT_PLAYER_PADDLE_Y_START;
			rightPlayerPaddleY = RIGHT_PLAYER_PADDLE_Y_START;
			currentGameState = SERVING;
			respawnBall(false, &ballSpeedScalar, &ballX, &ballY, &ballDirection.x, &ballDirection.y);
		}
		
		if ((ballX + BALL_WIDTH/2.0) > SCREEN_WIDTH)
		{
			// Player 1 scores!!!
			player1Score++;
			ballReadyToServe = true;
			leftPlayerPaddleY = LEFT_PLAYER_PADDLE_Y_START;
			rightPlayerPaddleY = RIGHT_PLAYER_PADDLE_Y_START;
			currentGameState = SERVING;
			respawnBall(false, &ballSpeedScalar, &ballX, &ballY, &ballDirection.x, &ballDirection.y);
		}

		if ((ballY - BALL_HEIGHT/2.0) < 0.0)
		{
			ballY = 0.0 + BALL_HEIGHT/2.0;
			ballDirection.y *= -1;
		}

		if ((ballY + BALL_HEIGHT/2.0) > SCREEN_HEIGHT)
		{
			ballY = SCREEN_HEIGHT - BALL_HEIGHT/2.0;
			ballDirection.y *= -1;
		}

		if (respawnBallHeld && ballReadyToServe) {
			respawnBall(ballReadyToServe, &ballSpeedScalar, &ballX, &ballY, &ballDirection.x, &ballDirection.y);
			currentGameState = PLAYING;
			ballReadyToServe = false;
		}

		if (player1Score >= MAX_POINTS)
		{
			currentGameState = GAMEOVER;
			winner = 1;
		}

		if (player2Score >= MAX_POINTS)
		{
			currentGameState = GAMEOVER;
			winner = 2;
		}

		//// 3. Render the screen
		// Reset render draw color
		SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xFF);
		// Clear screen
		SDL_RenderClear(gRenderer);

		if (currentGameState == PREGAME)
		{ 
			SDL_RenderCopy(gRenderer, gCurrentTexture, NULL, NULL);

			// Make start screen texture
			startScreenTexture = loadFromRenderedText("DIRTY PONG", textColor);
			SDL_Texture* player1ControlsTexture = loadFromRenderedText("Player 1: w, s to move left paddle", textColor);
			SDL_Texture* player2ControlsTexture = loadFromRenderedText("Player 2:  up and down arrows to move right paddle", textColor);
			SDL_Texture* gameOverExplanationTexture = loadFromRenderedText("First player to 10 points wins", textColor);
			SDL_Texture* howToStartTexture = loadFromRenderedText("Press p to start", textColor);

			//SDL_Rect startScreenRenderQuad = { SCREEN_WIDTH - (scoreBoard->player1ScoreBoardWidth + BALL_WIDTH + 10.0), BALL_WIDTH, scoreBoard->player2ScoreBoardWidth, scoreBoard->player2ScoreBoardHeight };
			//SDL_Rect startScreenRenderQuad = { 0.0, 0.0, SCREEN_WIDTH, SCREEN_HEIGHT/6.0 };
			//SDL_Rect player1ControlsRenderQuad = { 0.0, SCREEN_HEIGHT / 6.0, SCREEN_WIDTH, SCREEN_HEIGHT / 12.0 };
			//SDL_Rect player2ControlsRenderQuad = { 0.0, 2 * (SCREEN_HEIGHT / 6.0), SCREEN_WIDTH, SCREEN_HEIGHT / 12.0};
			//SDL_Rect gameOverExplanationRenderQuad = { 0.0, 3 * (SCREEN_HEIGHT / 6.0), SCREEN_WIDTH, SCREEN_HEIGHT / 12.0};
			//SDL_Rect howToStartRenderQuad = { 0.0, 4 * (SCREEN_HEIGHT / 6.0), SCREEN_WIDTH, SCREEN_HEIGHT / 12.0 };
			SDL_Rect startScreenRenderQuad = { 20.0, 0.0, SCREEN_WIDTH - 40, SCREEN_HEIGHT / 6.0 };
			SDL_Rect player1ControlsRenderQuad = { 20.0, 2 * (SCREEN_HEIGHT / 12.0), SCREEN_WIDTH - 40, SCREEN_HEIGHT / 12.0 };
			SDL_Rect player2ControlsRenderQuad = { 20.0, 3 * (SCREEN_HEIGHT / 12.0), SCREEN_WIDTH - 40, SCREEN_HEIGHT / 12.0 };
			SDL_Rect gameOverExplanationRenderQuad = { 20.0, 4 * (SCREEN_HEIGHT / 12.0), SCREEN_WIDTH - 40, SCREEN_HEIGHT / 12.0 };
			SDL_Rect howToStartRenderQuad = { 20.0, 5 * (SCREEN_HEIGHT / 12.0), SCREEN_WIDTH - 40, SCREEN_HEIGHT / 12.0 };

			//Render start menu to screen
			SDL_RenderCopyEx(gRenderer, startScreenTexture, NULL, &startScreenRenderQuad, 0, NULL, SDL_FLIP_NONE);
			SDL_RenderCopyEx(gRenderer, player1ControlsTexture, NULL, &player1ControlsRenderQuad, 0, NULL, SDL_FLIP_NONE);
			SDL_RenderCopyEx(gRenderer, player2ControlsTexture, NULL, &player2ControlsRenderQuad, 0, NULL, SDL_FLIP_NONE);
			SDL_RenderCopyEx(gRenderer, gameOverExplanationTexture, NULL, &gameOverExplanationRenderQuad, 0, NULL, SDL_FLIP_NONE);
			SDL_RenderCopyEx(gRenderer, howToStartTexture, NULL, &howToStartRenderQuad, 0, NULL, SDL_FLIP_NONE);

			// Free texture memory
			freeTextTexture(startScreenTexture);
			freeTextTexture(player1ControlsTexture);
			freeTextTexture(player2ControlsTexture);
			freeTextTexture(gameOverExplanationTexture);
			freeTextTexture(howToStartTexture);
		}
		else if (currentGameState == PLAYING || currentGameState == SERVING) 
		{
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

			// Draw a vertical line of white dots
			SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

			for (int i = DIVIDER_BLOCK_HEIGHT; i < SCREEN_HEIGHT; i += (DIVIDER_BLOCK_HEIGHT * 2))
			{
				SDL_Rect dotRect = { SCREEN_WIDTH / 2, i, DIVIDER_BLOCK_WIDTH, DIVIDER_BLOCK_HEIGHT };
				renderRect(gRenderer, &dotRect);
			}

			// Draw player rectangles
			// Player 1
			SDL_Rect fillRect = { PADDLE_MARGIN, (int)leftPlayerPaddleY, DIVIDER_BLOCK_WIDTH, PLAYER_PADDLE_HEIGHT };
			SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
			renderRect(gRenderer, &fillRect);

			// Player 2
			fillRect = { SCREEN_WIDTH - PADDLE_MARGIN,(int)rightPlayerPaddleY, DIVIDER_BLOCK_WIDTH, PLAYER_PADDLE_HEIGHT };
			SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
			renderRect(gRenderer, &fillRect);

			// Draw ball
			fillRect = { (int)ballX, (int)ballY, 2 * DIVIDER_BLOCK_WIDTH, 2 * DIVIDER_BLOCK_HEIGHT };
			SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
			renderRect(gRenderer, &fillRect);

			// Test texture
			scoreBoard->player1ScoreBoardTexture = loadFromRenderedText(std::to_string(player1Score), textColor);
			scoreBoard->player2ScoreBoardTexture = loadFromRenderedText(std::to_string(player2Score), textColor);
			//Set rendering space and render to screen
			//SDL_Rect renderQuad = { BALL_WIDTH, BALL_WIDTH, SCREEN_WIDTH, SCREEN_HEIGHT / 20.0 };
			SDL_Rect player1ScoreBoardRenderQuad = { BALL_WIDTH + 10.0, BALL_WIDTH, scoreBoard->player1ScoreBoardWidth, scoreBoard->player1ScoreBoardHeight };
			SDL_Rect player2ScoreBoardRenderQuad = { SCREEN_WIDTH - (scoreBoard->player1ScoreBoardWidth + BALL_WIDTH + 10.0), BALL_WIDTH, scoreBoard->player2ScoreBoardWidth, scoreBoard->player2ScoreBoardHeight };

			//Set clip rendering dimensions
			//if (clip != NULL)
			//{
			//	renderQuad.w = clip->w;
			//	renderQuad.h = clip->h;
			//	renderQuad.h = clip->h;
			//}

			if (ballReadyToServe)
			{
				// Make start screen texture
				SDL_Texture* readyToServeTexture = loadFromRenderedText("Press r to serve the ball", textColor);
				SDL_Rect readyToServeTextureRenderQuad = { 20.0, 2 * (SCREEN_HEIGHT / 12.0), SCREEN_WIDTH - 40, SCREEN_HEIGHT / 12.0 };

				SDL_RenderCopyEx(gRenderer, readyToServeTexture, NULL, &readyToServeTextureRenderQuad, 0, NULL, SDL_FLIP_NONE);
				freeTextTexture(readyToServeTexture);
			}

			//Render scoreboard to screen
			SDL_RenderCopyEx(gRenderer, scoreBoard->player1ScoreBoardTexture, NULL, &player1ScoreBoardRenderQuad, 0, NULL, SDL_FLIP_NONE);
			SDL_RenderCopyEx(gRenderer, scoreBoard->player2ScoreBoardTexture, NULL, &player2ScoreBoardRenderQuad, 0, NULL, SDL_FLIP_NONE);

			freeTextTexture(scoreBoard->player1ScoreBoardTexture);
			freeTextTexture(scoreBoard->player2ScoreBoardTexture);
		}
		else if (currentGameState == GAMEOVER) {
			SDL_RenderCopy(gRenderer, gCurrentTexture, NULL, NULL);

			// Make start screen texture
			SDL_Texture* winner1AnnouncementTexture = loadFromRenderedText("Congratulations player 1!", textColor);
			SDL_Texture* winner2AnnouncementTexture = loadFromRenderedText("Congratulations player 2!", textColor);

			SDL_Texture* playAgainTexture = loadFromRenderedText("Press p to play again!", textColor);

			//SDL_Rect startScreenRenderQuad = { SCREEN_WIDTH - (scoreBoard->player1ScoreBoardWidth + BALL_WIDTH + 10.0), BALL_WIDTH, scoreBoard->player2ScoreBoardWidth, scoreBoard->player2ScoreBoardHeight };
			//SDL_Rect startScreenRenderQuad = { 0.0, 0.0, SCREEN_WIDTH, SCREEN_HEIGHT/6.0 };
			//SDL_Rect player1ControlsRenderQuad = { 0.0, SCREEN_HEIGHT / 6.0, SCREEN_WIDTH, SCREEN_HEIGHT / 12.0 };
			//SDL_Rect player2ControlsRenderQuad = { 0.0, 2 * (SCREEN_HEIGHT / 6.0), SCREEN_WIDTH, SCREEN_HEIGHT / 12.0};
			//SDL_Rect gameOverExplanationRenderQuad = { 0.0, 3 * (SCREEN_HEIGHT / 6.0), SCREEN_WIDTH, SCREEN_HEIGHT / 12.0};
			//SDL_Rect howToStartRenderQuad = { 0.0, 4 * (SCREEN_HEIGHT / 6.0), SCREEN_WIDTH, SCREEN_HEIGHT / 12.0 };
			SDL_Rect winnerAnnouncementrQuad = { 20.0, SCREEN_HEIGHT / 6.0, SCREEN_WIDTH - 40, SCREEN_HEIGHT / 6.0};
			SDL_Rect playAgainQuad = { 20.0, 2 * (SCREEN_HEIGHT / 6.0), SCREEN_WIDTH - 40, SCREEN_HEIGHT / 6.0 };

			//Render start menu to screen 
			if (winner == 1)
			{
				SDL_RenderCopyEx(gRenderer, winner1AnnouncementTexture, NULL, &winnerAnnouncementrQuad, 0, NULL, SDL_FLIP_NONE);
			}

			if (winner == 2)
			{
				SDL_RenderCopyEx(gRenderer, winner2AnnouncementTexture, NULL, &winnerAnnouncementrQuad, 0, NULL, SDL_FLIP_NONE);
			}

			SDL_RenderCopyEx(gRenderer, playAgainTexture, NULL, &playAgainQuad, 0, NULL, SDL_FLIP_NONE);

			// Free texture memory
			freeTextTexture(winner1AnnouncementTexture);
			freeTextTexture(winner2AnnouncementTexture);
			freeTextTexture(playAgainTexture);
		}

		//SDL_RenderCopyEx(gRenderer, mTexture, NULL, NULL, 0, NULL, SDL_FLIP_NONE);

		// Update screen
		SDL_RenderPresent(gRenderer);
	}

	//Free resources and close SDL
	close();

	return 0;
}
