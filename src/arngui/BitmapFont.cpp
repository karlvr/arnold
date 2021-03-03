
#ifdef USE_OSD
#include "BitmapFont.h"

#include <SDL_image.h>

Uint32 getpixel(SDL_Surface *surface, int x, int y);

//*********************************************************************
//						class
//**********************************************************************

BitmapFont::BitmapFont()
{
    //Initialize variables
    TextureBitmapFont = NULL;
    newLine = 0;
    space = 0;
	WidthBase = 0;
	HeightBase = 0;
	WidthActual = 0;
	HeightActual = 0;
	Rx=1.0f;
	Ry=1.0f;
	Rg = 1;
	XPixelPadding = 1;
	YPixelPadding = 0;
}

BitmapFont::~BitmapFont()
{
	if (TextureBitmapFont) SDL_DestroyTexture(TextureBitmapFont);
}
 
void BitmapFont::apply_surface( int x, int y, SDL_Renderer *renderer, SDL_Rect* clip )
{

    SDL_Rect offset;

    //New offset calculs
    offset.x = int(x * Rx * Rg);
    offset.y = int(y * Ry * Rg);
	offset.w = int(clip->w * Rx * Rg);
	offset.h = int(clip->h * Ry * Rg);

	SDL_RenderCopy(renderer,TextureBitmapFont,clip,&offset);
}

void BitmapFont::RectConvert(SDL_Rect *rect)
{
	rect->x = int(rect->x * Rx * Rg);
	rect->y = int(rect->y * Ry * Rg);
	rect->w = int(rect->w * Rx * Rg);
	rect->h = int(rect->h * Ry * Rg);
}


bool BitmapFont::LoadFont( std::string filename,SDL_Renderer *renderer )
{
	//If a font is already loaded
	if (TextureBitmapFont) SDL_DestroyTexture(TextureBitmapFont);

	//The image that's loaded
    SDL_Surface* loadedImage = NULL;

    //Load the image
    loadedImage = IMG_Load( filename.c_str() );

	if (!loadedImage) return false;

	//Set the invisible color
 	SDL_SetColorKey( loadedImage, SDL_TRUE, SDL_MapRGB( loadedImage->format, 0, 0xff, 0xff ) );

    //Build the font
    build_font( loadedImage );

	TextureBitmapFont = SDL_CreateTextureFromSurface(renderer,loadedImage);

	SDL_FreeSurface(loadedImage);  // we got the texture now -> free surface

	//defaut options
	SetColor(0,0,0);
	SetAlpha(255);

	return true;
}

void BitmapFont::SetXPadding(int i)
{
	if (i < 10) XPixelPadding = i;
}

void BitmapFont::SetYPadding(int i)
{
	if (i < 10) YPixelPadding = i;
}

void BitmapFont::SetColor(Uint8 r,Uint8 g,Uint8 b)
{
	SDL_SetTextureColorMod(TextureBitmapFont,r,g,b);
}

void BitmapFont::SetAlpha(Uint8 a)
{
	SDL_SetTextureAlphaMod(TextureBitmapFont,a);
}

void BitmapFont::MemBaseScreen(short w,short h)
{
	WidthActual = w;
	HeightActual = h;

	if ((WidthBase * HeightBase) == 0)
	{
		ResetBaseScreen();
	}

	//ratio calcul
	Rx = 1.0f;
	Ry = 1.0f;
	if (WidthBase*HeightBase != 0) {
		Rx = ((float)WidthActual / WidthBase);
		Ry = ((float)HeightActual / HeightBase);
	}

}

void BitmapFont::Resize(int r)
{
	if ((r < 0) || (r > 100)) return;
	Rg = ((float)r) / 100.0f;
}

void BitmapFont::ResetBaseScreen(void)
{
	WidthBase = WidthActual;
	HeightBase = HeightActual;

}

//this technic is coming from Lazy Foo
//http://lazyfoo.net/SDL_tutorials/lesson30/index.php
//
void BitmapFont::build_font( SDL_Surface *bitmap )
{
    //If surface is NULL
    if( bitmap == NULL ) return;

    //Set the background color
    Uint32 bgColor = SDL_MapRGB( bitmap->format, 0, 0xFF, 0xFF );

    //Set the cell dimensions
    int cellW = bitmap->w / 16;
    int cellH = bitmap->h / 16;

	//memorize
	CharHeight = cellH;

    //New line variables
    int top = cellH;
    int baseA = cellH;

    //The current character we're setting
    int currentChar = 0;

    //Go through the cell rows
    for( int rows = 0; rows < 16; rows++ )
    {
        //Go through the cell columns
        for( int cols = 0; cols < 16; cols++ )//=9
        {

            //Set the character offset
            chars[ currentChar ].x = cellW * cols;
            chars[ currentChar ].y = cellH * rows;

            //Set the dimensions of the character
            chars[ currentChar ].w = cellW;
            chars[ currentChar ].h = cellH;

            //Find Left Side
            //Go through pixel columns
            for( int pCol = 0; pCol < cellW; pCol++ )
            {
                //Go through pixel rows
                for( int pRow = 0; pRow < cellH; pRow++ )
                {
                    //Get the pixel offsets
                    int pX = ( cellW * cols ) + pCol;
                    int pY = ( cellH * rows ) + pRow;

                    //If a non colorkey pixel is found
					if( getpixel( bitmap, pX, pY ) != bgColor )
                    {
                        //Set the x offset
                        chars[ currentChar ].x = pX;

                        //Break the loops
                        pCol = cellW;
                        pRow = cellH;
                    }
                }
            }

            //Find Right Side
            //Go through pixel columns
            for( int pCol_w = cellW - 1; pCol_w >= 0; pCol_w-- )
            {
                //Go through pixel rows
                for( int pRow_w = 0; pRow_w < cellH; pRow_w++ )
                {
                    //Get the pixel offsets
                    int pX = ( cellW * cols ) + pCol_w;
                    int pY = ( cellH * rows ) + pRow_w;

                    //If a non colorkey pixel is found
					if( getpixel( bitmap, pX, pY ) != bgColor )
                    {
                        //Set the width
                        chars[ currentChar ].w = ( pX - chars[ currentChar ].x ) + 1;

                        //Break the loops
                        pCol_w = -1;
                        pRow_w = cellH;
                    }
                }
            }

            //Find Top
            //Go through pixel rows
            for( int pRow = 0; pRow < cellH; pRow++ )
            {
                //Go through pixel columns
                for( int pCol = 0; pCol < cellW; pCol++ )
                {
                    //Get the pixel offsets
                    int pX = ( cellW * cols ) + pCol;
                    int pY = ( cellH * rows ) + pRow;

                    //If a non colorkey pixel is found
					if( getpixel( bitmap, pX, pY ) != bgColor )
                    {
                        //If new top is found
                        if( pRow < top )
                        {
                            top = pRow;
                        }

                        //Break the loops
                        pCol = cellW;
                        pRow = cellH;
                    }
                }
            }

            //Find Bottom of A
            if( currentChar == 'A' )
            {
                //Go through pixel rows
                for( int pRow = cellH - 1; pRow >= 0; pRow-- )
                {
                    //Go through pixel columns
                    for( int pCol = 0; pCol < cellW; pCol++ )
                    {
                        //Get the pixel offsets
                        int pX = ( cellW * cols ) + pCol;
                        int pY = ( cellH * rows ) + pRow;

                        //If a non colorkey pixel is found
						if( getpixel( bitmap, pX, pY ) != bgColor )
                        {
                            //Bottom of a is found
                            baseA = pRow;

                            //Break the loops
                            pCol = cellW;
                            pRow = -1;
                        }
                    }
                }
            }

            //Go to the next character
            currentChar++;
        }
    }

    //Calculate space
    //space = cellW / 2;
	//For space we take the char "t" value
	space = chars['t'].w;

    //Calculate new line
    newLine = baseA - top;

    //Lop off excess top pixels
    for( int t = 0; t < 256; t++ )
    {
        chars[ t ].y += top;
        chars[ t ].h -= top;
    }
}

int BitmapFont::GetHeight(void)
{
	return (int)(CharHeight*Ry);
}

int BitmapFont::CalcSize(std::string text)
{
	int l = 0;
	int ascii;
	for( int car = 0; car < text.length(); car++ )
	{
		ascii = (unsigned char)text[ car ];
		if (ascii == ' ')
		{
			l = l + space;
		}
		else
		{
			l = l + chars[ ascii ].w + 1;
		}
	}
	return (int)(l*Rx*Rg);
}

void BitmapFont::show_text( int x, int y, std::string text, SDL_Renderer * renderer )
{
    //Temp offsets
    int X = x, Y = y;

    //If the font has been built
    if( TextureBitmapFont != NULL )
    {
        //Go through the text
        for( int show = 0; show < text.length(); show++ )
        {

			//if we are and end of line, I use the 'W' char to check because it s the larger character
			if (X + chars['W'].w > WidthBase / Rg)
			{
				//Move down
                Y += newLine + YPixelPadding;

                //Move back
                X = x;
			}

            //If the current character is a space
            if( text[ show ] == ' ' )
            {
                //Move over
                X += space;
            }
            //If the current character is a newline
            else if( text[ show ] == '\n' )
            {
                //Move down
                Y += newLine + YPixelPadding;

                //Move back
                X = x;
            }
            else
            {
                //Get the ASCII value of the character
                int ascii = (unsigned char)text[ show ];

                //Show the character
                apply_surface( X, Y, renderer,  &chars[ ascii ] );

                //Move over the width of the character with X pixel of padding
                X += chars[ ascii ].w + XPixelPadding;
            }
        }
    }
}



//**********************************************************************
// functions
//*********************************************************************

Uint32 getpixel(SDL_Surface *surface, int x, int y)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to retrieve */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        return *p;
        break;

    case 2:
        return *(Uint16 *)p;
        break;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
            return p[0] << 16 | p[1] << 8 | p[2];
        else
            return p[0] | p[1] << 8 | p[2] << 16;
        break;

    case 4:
        return *(Uint32 *)p;
        break;

    default:
        return 0;       /* shouldn't happen, but avoids warnings */
    }
}
#endif