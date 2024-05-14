#include "graphicsdisplay.h"
#include "board.h"

GraphicsDisplay::GraphicsDisplay(int n, Xwindow &xw) : xw(xw)
{
    // Initialize the display, such as drawing an empty chessboard
    // n could be the size of the board (e.g., 8 for an 8x8 chessboard)
    int squareSize = 500 / n; // Assuming window size is 500x500

    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < n; ++j)
        {
            int color = (i + j) % 2 == 0 ? Xwindow::White : Xwindow::LightGreen;
            xw.fillRectangle(j * squareSize, i * squareSize, squareSize, squareSize, color);
        }
    }
}

void GraphicsDisplay::notify(Board &board)
{
    int n = board.getHeight();
    int squareSize = 500 / n; // Adjust as per your window size

    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < n; ++j)
        {
            int color = (i + j) % 2 == 0 ? Xwindow::White : Xwindow::LightGreen;
            xw.fillRectangle(j * squareSize, i * squareSize, squareSize, squareSize, color);
            Piece *piece = board.getPiece(i, j);
            std::string pieceChar = " "; // Default to empty space

            if (piece)
            {
                pieceChar = getPieceSymbol(*piece);
                // redraw board if it can be fast
                // or make a previous position field
            }

            // Draw the piece character on the board
            xw.drawString(j * squareSize + squareSize / 2, i * squareSize + squareSize / 2, pieceChar);
        }
    }
}

std::string GraphicsDisplay::getPieceSymbol(Piece &piece)
{
    PieceType type = piece.identify().first;
    int team = piece.identify().second;
    std::string symbol;

    switch (type)
    {
    case PieceType::King:
        symbol = "K";
        break;
    case PieceType::Queen:
        symbol = "Q";
        break;
    case PieceType::Rook:
        symbol = "R";
        break;
    case PieceType::Bishop:
        symbol = "B";
        break;
    case PieceType::Knight:
        symbol = "N"; // N for Knight
        break;
    case PieceType::Pawn:
        symbol = "P";
        break;
    case PieceType::Empty:
        symbol = " "; // Empty space for no piece
        break;
    default:
        symbol = " "; // Fallback for unrecognized piece
    }

    if(team == 2)
    {
        symbol += "2";
    }
    else if (team == 1)
    {
        symbol += "1";
    }
    return symbol;
}

GraphicsDisplay::~GraphicsDisplay() {}
