#ifndef _BOARD_H_
#define _BOARD_H_
#include "piece.h"
#include "graphicsdisplay.h"
#include <vector>
#include <memory>
#include <iostream>
using namespace std;

// class GraphicsDisplay;

class Board
{
    vector<vector<unique_ptr<Piece>>> theBoard;
    friend class Piece;
    int height, width;
    friend ostream& operator<<(ostream &out, const Board &b); // print board operator
    vector<pair<int,int>> kingPos;

    
    //special piece flags for unmaking moves
    bool kingFirst = false;
    bool rookFirst = false;
    bool pawnFirst = false;
    bool pawnDouble = false;

    // TODO: add the observer pattern stuff for displays
    std::unique_ptr<GraphicsDisplay> observer;

public:
    void reset();
    Board(int x, int y, int teams) : height{x}, width{y}
    {
        for (int i=0;i<height;++i) {
            theBoard.push_back(vector<unique_ptr<Piece>>());
            for (int j = 0; j < width; ++j)
            {
                theBoard[i].push_back(make_unique<Empty>(i, j, &theBoard));
            }
        }
        for (int i = 0; i <= teams; ++i)
        {
            kingPos.push_back(make_pair(-1, -1));
        }
    }
    // reset all capture pointers to null, removes enpassant flags
    // except for the piece that just moved
    void flagReset(int x, int y);

    // set piece at [x][y] to type under team
    void addPiece(int x, int y, int team, PieceType type);

    // forcibly unmakes a move
    void unMove(int sx, int sy, int ex, int ey);

    // moves piece at [sx][sy] to [ex][ey] if possible
    // performs capture if opposing team piece occupies that space
    bool movePiece(int sx, int sy, int ex, int ey);

    // piece getter
    Piece *getPiece(int x, int y) const;
    int getWidth();
    int getHeight();
    
    //  the team that piece is on to enter check
    // function to verify if the position is legal for the team that moved, used after move to verify legality
    // returns false if a piece can attack that square
    bool moveCheck(int kx, int ky, int team);
    // get kingpos function
    std::pair<int, int> getKingPosition(int team) const;

    // graphics
    void attach(std::unique_ptr<GraphicsDisplay> obs)
    {
        observer = std::move(obs);
    }
    void notifyObserver()
    {
        if (observer)
        {
            observer->notify(*this); // Notify the observer with a reference to this Board
        }
    }
};

#endif
