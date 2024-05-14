#ifndef __GRAPHICSDISPLAY_H__
#define __GRAPHICSDISPLAY_H__
#include "window.h"
#include "piece.h"
#include <vector>

class Board;

class GraphicsDisplay
{
    Xwindow &xw;
    std::string getPieceSymbol(Piece &piece);

public:
    ~GraphicsDisplay();
    GraphicsDisplay(int n, Xwindow &xw);
    void notify(Board &board);
};

#endif


