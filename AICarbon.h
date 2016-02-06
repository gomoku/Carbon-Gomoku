/*
 *  AICarbon.h
 *  22.01.2002, Michal Czardybon
 *
 *  Klasa "firmowego" algorytmu sztucznej inteligencji
 *
 */ 

#ifndef _AICARBON
#define _AICARBON

#include "OXTypes.h"
#include "OXPlayer.h"

#include "AICarbonHash.h"

#include "CONFIG.CPP"
#include "COUNT5.CPP"
#include "STATUS1.CPP"
#include "PRIOR3.CPP"

enum LNUM {A = 8, B = 7, C = 6, D = 5, E = 4, F = 3, G = 2, H = 1};

// makro imitujace petle, w ktorej przypisuje wartoscia zmiannych x, y typu
// short wspolrzedne kolejnych kandydatow na ruch
#define FOR_EVERY_CAND(x, y) \
  for (y = upperLeftCand.y; y <= lowerRightCand.y; y++)\
    for (x = upperLeftCand.x; x <= lowerRightCand.x; x++)\
      if (cell[x][y].piece == EMPTY && (cell[x][y].adj1 || cell[x][y].adj2))

// wspolrzedne ruchu i jego wartosc / oszacowanie wartosci
struct OXMove
{
  OXMove() {}
  OXMove(int _x, int _y, int v) : x(_x), y(_y), value(v) {}
  operator OXPoint() {return OXPoint(x, y);}
  short x, y;
  int   value;
};

typedef OXMove OXCand; // kandydat na ruch

class AICarbon : public OXPlayer
{
  public:    
   
    void start(int size);
    void start(int width, int height);
    void move(int x, int y);
    void yourTurn(int &x, int &y, int depth = 0, int time = 0);
    void undo();
    int undo(int x, int y);
    void block(int x, int y);
    void setWho(OXPiece _who);
    const char* name() const { return "AICarbon"; }
    AICarbon(){ init(); }
#ifdef DEBUG_EVAL
    void eval(int x, int y);
#endif

  private:
    static const int WIN_MIN;  // wygrana w bardzo wielu ruchach
    static const int WIN_MAX;  // wygrana juz
    static const int INF;      // nieskonczonosc
    static const int MAX_CAND; // maksymalna liczba kandydatow na ruch

    struct OXCell
    {
      void    update4();
      void    update1(int k);

      OXPiece piece;           //  co jest na tym polu?
      UCHAR   pattern[4][2];   //  uklad dla 4 kierunkow i 2 graczy
      UCHAR   status1[4][2];   //  statusy jednego kierunku
      UCHAR   status4[2];      //  status pola
      
      char    adj1, adj2;      //  czy ma sasiada w odleglosci 1 i 2 pola

      short   prior();         //  zwraca wartosc priorytetu dla ustalania 
    };                         //  kolejnosci sprawdzania ruchow
    
    friend  struct OXCell;
    
    // ------------------------------------------------------------------------
    void    _move(int x, int y, bool updateHash = true);   // wykonanie ruchu, wspolrzedne w <4, boardSize + 3>

    // funkcje inicjujace
    void    init();                       // inicjacja tablic, korzysta z funkcji get*()
    int     getRank   (char cfg);   
    int     getPrior  (UCHAR a, UCHAR b);
    UCHAR   getStatus4(UCHAR s0, UCHAR s1, UCHAR s2, UCHAR s3);
    bool    databaseMove(int &x0, int &y0);


    // ------------------------------------------------------------------------
    // Funkcje AI

    // ocena biezacej sytuacji
    int     evaluate();

    // glowna funkcja przeszukujaca ruchy
    OXMove  minimax(int h, bool root, int alpha, int beta);   

    // szybkie znajdzowanie zakonczenia
    int     quickWinSearch();

    // tworzy liste kandydatow na ruch
    void    generateCand(OXCand *cnd, int &nCnd);
       
    // ------------------------------------------------------------------------
    // stan gry
    OXCell  cell[MAX_BOARD_WIDTH + 8][MAX_BOARD_HEIGHT + 8]; // tablica pol    
    int     boardWidth, boardHeight;  // rozmiar planszy
    int     moveCount;  // liczba wykonanych ruchow
    int     remCount;   // remCell length
    OXPiece who, opp;   // who, opp - aktualny i nastepny gracz 
    int     nSt[2][9];  // nSt[i][j] - liczba pol o statusie j gracza i
  
    // ------------------------------------------------------------------------
    // history stack
    OXPoint remMove[MAX_CELLS];   // zapamietuje kolejne ruchy
    OXCell* remCell[MAX_CELLS];   // zapamietuje kolejne ruchy
    OXPoint remULCand[MAX_CELLS]; // zapamietuje <upperLeftCand>
    OXPoint remLRCand[MAX_CELLS]; // zapamietuje <lowerRightCand> 

    // lewe-gorne i prawe-dolne ograniczenie dla przeszukiwanych ruchow
    OXPoint upperLeftCand, lowerRightCand;

    // tablica transpozycji
    HashTable table;

    // tablice heurystyczne
    static int   RANK[107];                 // punkty z konfiguracje
    static int   PRIOR[256][256];           // priorytet
    static UCHAR STATUS4[10][10][10][10];   // status4, indeksy: 4 x status1

    // liczba przeszukanych pozycji
    int totalSearched, nSearched; 

    // debugging - sprawdzanie spojnosci danych
    bool check();

    long start_time;
    long stopTime();
};

extern int info_timeout_turn, info_time_left;
extern int terminateAI;
extern long getTime();

#endif
