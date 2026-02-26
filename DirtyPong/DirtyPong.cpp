/*This source code is a modified version of the original code found in the SDL2 tutorial at lazyfoo.net
* created for learning purposes.

It is largely based on copyrighted by Lazy Foo' Productions 2004-2024
and may not be redistributed without written permission.*/

//Using SDL and standard IO
#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <string>

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

//Starts up SDL and creates window
bool init();

//Loads media
bool loadMedia();

//Frees media and shuts down SDL
void close();


//Loads individual image
SDL_Surface* loadSurface(std::string path);

//Key press surfaces constants
enum KeyPressSurfaces
{
	KEY_PRESS_SURFACE_DEFAULT,
	KEY_PRESS_SURFACE_UP,
	KEY_PRESS_SURFACE_DOWN,
	KEY_PRESS_SURFACE_LEFT,
	KEY_PRESS_SURFACE_RIGHT,
	KEY_PRESS_SURFACE_TOTAL
};

//The window we'll be rendering to
SDL_Window* gWindow = NULL;
	
//The surface contained by the window
SDL_Surface* gScreenSurface = NULL;

// Current displayed image
SDL_Surface* gCurrentSurface = NULL;

//The images that correspond to a keypress
SDL_Surface* gKeyPressSurfaces[KEY_PRESS_SURFACE_TOTAL] = {};

SDL_Rect stretchRect;


//bool init()
//{
//	// Initialize stretch rectangle to be the entire screen
//	stretchRect.x = 0;
//	stretchRect.y = 0;
//	stretchRect.w = SCREEN_WIDTH;
//	stretchRect.h = SCREEN_HEIGHT;
//
//	//Initialization flag
//	bool success = true;
//
//	//Initialize SDL
//	if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
//	{
//		printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
//		success = false;
//	}
//	else
//	{
//		//Create window
//		gWindow = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
//		if( gWindow == NULL )
//		{
//			printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
//			success = false;
//		}
//		else
//		{
//			// Initialize PNG loading
//			int imgFlags = IMG_INIT_PNG;
//
//			if (!IMG_Init(imgFlags) & imgFlags)
//			{
//				printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
//				success = false;
//			} 
//			else 
//			{
//				//Get window surface
//				gScreenSurface = SDL_GetWindowSurface(gWindow);
//				if (gScreenSurface == NULL)
//				{
//					printf("Could not get window surface! SDL_Error: %s\n", SDL_GetError());
//					success = false;
//				}
//			}
//		}
//	}
//
//	return success;
//}

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
	gKeyPressSurfaces[KEY_PRESS_SURFACE_DEFAULT] = loadSurface("pngs/press.png");
	if (gKeyPressSurfaces[KEY_PRESS_SURFACE_DEFAULT] == NULL)
	{
		printf("Failed to load default image!\n");
		return false;
	}

	//Load up surface
	gKeyPressSurfaces[KEY_PRESS_SURFACE_UP] = loadSurface("pngs/up.png");
	if (gKeyPressSurfaces[KEY_PRESS_SURFACE_UP] == NULL)
	{
		printf("Failed to load up image!\n");
		return false;
	}

	//Load down surface
	gKeyPressSurfaces[KEY_PRESS_SURFACE_DOWN] = loadSurface("pngs/down.png");
	if (gKeyPressSurfaces[KEY_PRESS_SURFACE_DOWN] == NULL)
	{
		printf("Failed to load down image!\n");
		return false;
	}

	//Load left surface
	gKeyPressSurfaces[KEY_PRESS_SURFACE_LEFT] = loadSurface("pngs/left.png");
	if (gKeyPressSurfaces[KEY_PRESS_SURFACE_LEFT] == NULL)
	{
		printf("Failed to load left image!\n");
		return false;
	}

	//Load right surface
	gKeyPressSurfaces[KEY_PRESS_SURFACE_RIGHT] = loadSurface("pngs/right.png");
	if (gKeyPressSurfaces[KEY_PRESS_SURFACE_RIGHT] == NULL)
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
	// Free only the surfaces we explicitly loaded into gKeyPressSurfaces.
	for (int i = 0; i < KEY_PRESS_SURFACE_TOTAL; i++)
	{
		if (gKeyPressSurfaces[i] != NULL)
		{
			// SDl_FreeSurface frees memory associated with the surface
			SDL_FreeSurface(gKeyPressSurfaces[i]);
			// I manually set the pointer to NULL to avoid dangling pointer issues.
			gKeyPressSurfaces[i] = NULL;
		}
	}

	// gCurrentSurface points into gKeyPressSurfaces; do not free it again.
	gCurrentSurface = NULL;

	// gScreenSurface is owned by the window (returned by SDL_GetWindowSurface).
	// The lesson here is taht you only free memory that you allocate
	// So for the surfaces I allocate in loadMedia need me to free the memory, otherwise
	// that data stays there. Please note: even though we don't own the memory allocated for gScreenSurface
	// we still own the pointer 
	// Do not call SDL_FreeSurface on it.
	gScreenSurface = NULL;

	//Destroy window
	if (gWindow != NULL)
	{
		SDL_DestroyWindow(gWindow);
		gWindow = NULL;
	}

	//Quit SDL subsystems
	SDL_Quit();
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

	//Set default current surface
	gCurrentSurface = gKeyPressSurfaces[KEY_PRESS_SURFACE_DEFAULT];


	// Game Loop
	while (!quit)
	{
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
					case SDLK_UP:
						gCurrentSurface = gKeyPressSurfaces[KEY_PRESS_SURFACE_UP];
						break;
					case SDLK_DOWN:
						gCurrentSurface = gKeyPressSurfaces[KEY_PRESS_SURFACE_DOWN];
						break;
					case SDLK_LEFT:
						gCurrentSurface = gKeyPressSurfaces[KEY_PRESS_SURFACE_LEFT];
						break;
					case SDLK_RIGHT:
						gCurrentSurface = gKeyPressSurfaces[KEY_PRESS_SURFACE_RIGHT];
						break;
					default:
						gCurrentSurface = gKeyPressSurfaces[KEY_PRESS_SURFACE_DEFAULT];
						break;
				}
			}
		}


		// 2. Update the game state
		// TBD

		// 3. Render the screen
		//Apply the image
		//SDL_BlitSurface(gCurrentSurface, NULL, gScreenSurface, NULL);
		SDL_BlitScaled(gCurrentSurface, NULL, gScreenSurface, &stretchRect);

		//Update the surface of the window
		SDL_UpdateWindowSurface(gWindow);
	}

	//Free resources and close SDL
	close();

	return 0;
}
