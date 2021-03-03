
#ifdef USE_SDL2

#ifdef USE_OSD

#ifdef _MSC_VER
// both sdl and sdl2
#include <SDL_image.h>
#else
// choose sdl or sdl2 directory
#ifdef USE_SDL
#include <SDL/SDL_image.h>
#endif
#ifdef USE_SDL2
#include <SDL2/SDL_image.h>
#endif
#endif
#include <string>

//Our bitmap font
class BitmapFont
{
    private:
		//the bitmap font texture
		SDL_Texture *TextureBitmapFont;

		//The individual characters in the surface
		SDL_Rect chars[ 256 ];

		//Spacing Variables
		int newLine, space;

		//Generates the font
		void build_font( SDL_Surface *surface );

		//To keep size ratio
		short WidthBase,HeightBase;
		short WidthActual,HeightActual;
		float Rx,Ry;

		//number of Pixel for Padding
		int XPixelPadding;
		int YPixelPadding;

		int CharHeight;

		//New ratio for resise
		float Rg;

		void apply_surface( int x, int y, SDL_Renderer *renderer, SDL_Rect* clip = NULL );

    public:
		//The default constructor
		BitmapFont();

		//the defaut destructor
		~BitmapFont();

		//Rect convertor to adapt size
		void RectConvert(SDL_Rect *);

		//Generates the font when the object is made
		bool LoadFont( std::string filename ,SDL_Renderer *renderer);

		//Set Padding
		void SetXPadding(int);
		void SetYPadding(int);

		//Set color
		void SetColor(Uint8,Uint8,Uint8);

		//Set alpha
		void SetAlpha(Uint8);

		//Memorize Base screen
		void MemBaseScreen(short,short);
		void ResetBaseScreen(void);

		//Shows the text
		void show_text( int x, int y, std::string text, SDL_Renderer*);

		//calc size
		int CalcSize(std::string text);

		//Get infos
		int GetHeight();

		//Resize font (1->100)
		void Resize(int);
};

#endif
#endif