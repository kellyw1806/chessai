#include "board.h"
#include "graphicsdisplay.h"
#include <iostream>

void Board::flagReset(int x, int y)
{
    for (int i = 0; i < height; ++i)
    {
        for (int j = 0; j < width; ++j)
        {
            theBoard[i][j]->capture.reset();
            if (x == i && j == y)
                continue;

            if (theBoard[i][j]->identify().first == PieceType::Pawn)
            {
                Pawn *temp = dynamic_cast<Pawn *>(theBoard[i][j].get());
                if (temp->enPassant)
                {
                    temp->enPassant.reset();
                }
                if (i != 1 && i != 6) temp->firstMove = false;
                temp->justMadeDoubleStep = false;
            }
        }
    }
}

void Board::unMove(int sx, int sy, int ex, int ey) {

    //specifically unmake castling
    if (theBoard[sx][sy]->identify().first == PieceType::King &&
        ey - sy == 2 && sx == ex) {
            //swap rook back
            theBoard[sx][sy+1].swap(theBoard[ex][ey+1]);
            theBoard[ex][ey+1]->setPos(ex, ey+1);

    }

    //queenside castling, moving left
    if (theBoard[sx][sy]->identify().first == PieceType::King &&
        ey - sy == -2 && sx == ex) {
            //swap the rook back
            theBoard[sx][sy-1].swap(theBoard[ex][ey-2]);
            theBoard[ex][ey-2]->setPos(ex, ey-2);
    }
    
        theBoard[sx][sy].swap(theBoard[ex][ey]);
        theBoard[sx][sy]->setPos(sx,sy);
        theBoard[ex][ey]->setPos(ex,ey);
        int team = theBoard[ex][ey]->identify().second;
        //replace capture
        if (theBoard[ex][ey]->capture) {
            theBoard[sx][sy] = std::move(theBoard[ex][ey]->capture);
        }
        //reset piece specific flags, if necessary
        //pawn
        if (theBoard[ex][ey]->identify().first == PieceType::Pawn && (pawnFirst || pawnDouble)) {
            Pawn * temp = dynamic_cast<Pawn *>(theBoard[ex][ey].get());
            temp->firstMove = pawnFirst;
            temp->justMadeDoubleStep = pawnDouble;
            //enpassant
            if (temp->enPassant) {
                theBoard[temp->enPassant->getX()][temp->enPassant->getY()] = std::move(temp->enPassant);
            }
        } 
        //rook
        else if (theBoard[ex][ey]->identify().first == PieceType::Rook && rookFirst) {
            Rook * temp = dynamic_cast<Rook *>(theBoard[ex][ey].get());
            temp->setCastle(rookFirst);
        } 
        //king
        else if (theBoard[ex][ey]->identify().first == PieceType::King && kingFirst) {
            King * temp = dynamic_cast<King *>(theBoard[ex][ey].get());
            temp->setCastle(kingFirst);
        }

        //updating the king position
        if (theBoard[ex][ey]->identify().first == PieceType::King) {
            kingPos[team].first = ex;
            kingPos[team].second = ey;
        }
        pawnDouble = false;
        pawnFirst = false;
        kingFirst = false;
        rookFirst = false;
}

bool Board::movePiece(int sx, int sy, int ex, int ey)
{
    // out of bounds
    if (sx > height || sy > width)
        return false;
    if (ex > height || ey > width)
        return false;
    if (sx == ex && sy == ey)
        return false;

    if (theBoard[sx][sy]->identify().second == -1)
        return false;

    int team = theBoard[sx][sy]->identify().second;

    bool noMove = false;

    // pawn special flags with en passant
    bool justDouble = false;
    if (theBoard[sx][sy]->identify().first == PieceType::King)
    {
        King *temp = dynamic_cast<King *>(theBoard[sx][sy].get());
        noMove = temp->Castle();
        if (noMove)
            kingFirst = true;
    }
    else if (theBoard[sx][sy]->identify().first == PieceType::Rook)
    {
        Rook *temp = dynamic_cast<Rook *>(theBoard[sx][sy].get());
        noMove = temp->Castle();
        if (noMove)
            rookFirst = true;
    }
    else if (theBoard[sx][sy]->identify().first == PieceType::Pawn) {
        Pawn * temp = dynamic_cast<Pawn *>(theBoard[sx][sy].get());
        justDouble = temp->justMadeDoubleStep;
        if (temp->firstMove) pawnFirst = true;
        if (justDouble) pawnDouble = true;
    }

    //specific castling logic
    //verifies no check on king on spaces or intermediary spaces
    //kingside castling, moving right
    if (theBoard[sx][sy]->identify().first == PieceType::King &&
        ey - sy == 2 && sx == ex) {
            if (moveCheck(sx,sy,team) && moveCheck(sy,sy+1,team) && moveCheck(sx,sy+2,team)) {
                if(theBoard[sx][sy]->move(ex,ey)) {
                    kingPos[team].first = ex;
                    kingPos[team].second = ey;
                    theBoard[sx][sy].swap(theBoard[ex][ey]);
                    theBoard[sx][sy+1].swap(theBoard[ex][ey+1]);
                    theBoard[ex][ey+1]->setPos(ex, ey+1);
                    //flagReset();
                    return true;
                }
            }
        else return false;
    }

      //queenside castling, moving left
    if (theBoard[sx][sy]->identify().first == PieceType::King &&
        ey - sy == -2 && sx == ex) {
            if (moveCheck(sx,sy,team) && moveCheck(sy,sy-1,team) && moveCheck(sx,sy-2,team)) {
                if(theBoard[sx][sy]->move(ex,ey)) {
                    kingPos[team].first = ex;
                    kingPos[team].second = ey;
                    theBoard[sx][sy].swap(theBoard[ex][ey]);
                    theBoard[sx][sy-1].swap(theBoard[ex][ey-2]);
                    theBoard[ex][ey-2]->setPos(ex, ey-2);
                    //flagReset();
                    return true;
                }
            }
        else return false;
    }

    // check for capture
    // call unique_ptr::reset() on a capture, then update that object with a make_unique<Piece>(...) that is empty
    if (theBoard[sx][sy]->move(ex, ey))
    {
        // if ending position is not an empty square
        if (theBoard[ex][ey]->identify().first != PieceType::Empty
            // and not on the same team as piece moving
            && theBoard[ex][ey]->identify().second != theBoard[sx][sy]->identify().second && theBoard[ex][ey]->identify().second != -1)
        {
            theBoard[sx][sy]->capture = std::move(theBoard[ex][ey]);
            theBoard[ex][ey].reset();
            theBoard[ex][ey] = std::make_unique<Empty>(ex, ey, &theBoard);
        }

        // update kingPos vec
        if (theBoard[sx][sy]->identify().first == PieceType::King)
        {
            kingPos[team].first = ex;
            kingPos[team].second = ey;
        }

        // moves the positions
        theBoard[sx][sy].swap(theBoard[ex][ey]);
    }
    else
    {
        return false;
    }

    // does this move put king in check?
    // revert if below condition returns false
    if (moveCheck(kingPos[team].first, kingPos[team].second, team))
    {
        // flagReset();

        return true;
    }

    unMove(ex, ey, sx, sy);
    return false;
    // ---- Reverting ----
}

bool Board::moveCheck(int kx, int ky, int team)
{
    for (int i = 0; i < height; ++i)
    {
        for (int j = 0; j < width; ++j)
        {
            if (theBoard[i][j]->identify().second != team && theBoard[i][j]->identify().second != -1 && theBoard[i][j]->mValidate(kx, ky))
                return false;
        }
    }
    return true;
}

void Board::addPiece(int x, int y, int team, PieceType type) {
        if (theBoard[x][y]->identify().first == PieceType::King) {
            kingPos[theBoard[x][y]->identify().second] = make_pair(-1,-1);
        }
        theBoard[x][y].reset();
        if (type == PieceType::Pawn) {
            theBoard[x][y] = make_unique<Pawn>(x,y,team,&theBoard);
        }
        else if (type == PieceType::Knight) {
            theBoard[x][y] = make_unique<Knight>(x,y,team,&theBoard);
        }
        else if (type == PieceType::Bishop) {
            theBoard[x][y] = make_unique<Bishop>(x,y,team,&theBoard);
        }
        //set the castling flags if in correct position
        else if (type == PieceType::Rook) {
            theBoard[x][y] = make_unique<Rook>(x,y,team,&theBoard);
        }
        else if (type == PieceType::Queen) {
            theBoard[x][y] = make_unique<Queen>(x,y,team,&theBoard);
        }
        //set the castling flags if in right spots
        else if (type == PieceType::King) {
            theBoard[x][y] = make_unique<King>(x,y,team,&theBoard);
            kingPos[team].first = x;
            kingPos[team].second = y;
        }
        else if (type == PieceType::Empty) {
            theBoard[x][y] = make_unique<Empty>(x,y,&theBoard);
        }
    else if (type == PieceType::Empty)
    {
        theBoard[x][y] = make_unique<Empty>(x, y, &theBoard);
    }
}

int Board::getHeight()
{
    return height;
}

int Board::getWidth()
{
    return width;
}

ostream &operator<<(ostream &out, const Board &b)
{
    for (int i = 0; i < b.height; ++i)
    {
        for (int j = 0; j < b.width; ++j)
        {
            out << *(b.theBoard[i][j].get()) << " ";
        }
        out << std::endl;
    }
    return out;
}

Piece *Board::getPiece(int x, int y) const
{
    // Check if the coordinates are within the board boundaries
    if (x < 0 || x >= height || y < 0 || y >= width)
    {
        return nullptr; // Coordinates out of bounds
    }
    // Retrieve the piece at the specified position
    return theBoard[x][y].get();
}

void Board::reset() {
    for (int i=0;i<kingPos.size();i++) {
        kingPos[i] = make_pair(-1, -1);
    }
}

std::pair<int, int> Board::getKingPosition(int team) const {
    // Ensure team index is valid before accessing the vector
    if (team >= 1 && team < kingPos.size())
    {
        return kingPos[team];
    }
    // Return an invalid position or throw an exception if team index is invalid
    return std::make_pair(-1, -1);
}
