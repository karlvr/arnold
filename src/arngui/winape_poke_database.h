#ifndef __WINAPE_POKE_DATABASE2__
#define __WINAPE_POKE_DATABASE2__

#include <wx/string.h>
#include <wx/dynarray.h>

typedef enum
{
    WINAPE_POKE_DATABASE_TYPE_DECIMAL = 0,
    WINAPE_POKE_DATABASE_TYPE_HEXADECIMAL,
    WINAPE_POKE_DATABASE_TYPE_BCD,
    WINAPE_POKE_DATABASE_TYPE_NUMERIC,          /* Zero based */
    WINAPE_POKE_DATABASE_TYPE_NUMERIC_ASCII,    /* ASCII based */
    WINAPE_POKE_DATABASE_TYPE_LONG,
    WINAPE_POKE_DATABASE_TYPE_STRING
} WINAPE_POKE_DATABASE_TYPE;

/* Zero based means each digit is stored as a byte (00..09), ASCII means they are stored as the ASCII
characters (30 ('0')..39 ('9')) */


//pokes needed by cheat
typedef struct
{
	//address
	unsigned short address;
	//Bytes, pointer list
	unsigned short * byte;
	//nbre of bytes
	int NbrBytes;

 }POKEINFO;

//poke/cheat list by game
typedef struct
{
	//Descripttion
	char Desc[255];
	//Comment
	char Commment[255];
	//Nbre of poke
	int npokes;

	//type of values
	char type;
	//reversed
	bool reversed;
	//ram bank
	char ram_bank;

	//poke needed by cheat
	POKEINFO * PokeInfo;

 } POKEINFOBYGAME ;

//Game
class POKEGAMEINFO
{

public:
	//Game name
	wxString Name;
	//Nbre poke
	int NbrePoke;
	//pokes info
	POKEINFOBYGAME *PokeInfoByGame;

	~POKEGAMEINFO(void);
	POKEGAMEINFO(void);

 };



class WinapePokeDatabase
{
public:
	~WinapePokeDatabase(void);
	WinapePokeDatabase(void);
	bool Init(const char *pWinapePokeDatabase,long size);
	int GetNumberGame();
	bool GetNameofGame(int,wxString*);
	bool SetGame(int id,POKEGAMEINFO *);
	
private:
	//GAMEPOKEINFO *GamePokeInfo;

	int SelGame;

	int NbreGame;
	const char *m_pWinapePokeDatabase;
	const char **pGamePtrs;

};



void WinapePokeDatabase_Init(const char *pWinapePokeDatabase,long size);

void WinapePokeDatabase_ApplyPoke(int nGame, int nPoke, const char *pPoke);



#endif
