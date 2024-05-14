#include "piece.h"
#include <cmath>
#include <iostream>
Piece::~Piece() {}

bool Piece::validate(int x, int y) { return false; }
bool Piece::mValidate(int x, int y) {
    return validate(x, y);
}

std::pair<PieceType, int> Piece::identify()  {
    return std::make_pair(type, team);
}


char typeToChar(PieceType pt) {
    switch (pt) {
        case PieceType::Bishop:
        return 'B';
        case PieceType::Rook:
        return 'R';
        case PieceType::Queen:
        return 'Q';
        case PieceType::Knight:
        return 'N';
        case PieceType::King:
        return 'K';
        case PieceType::Pawn:
        return 'P'; 
        default:
        return '0';
    }
}

std::ostream &operator<<(std::ostream &out, Piece &p) {
    out << typeToChar(p.identify().first) << (p.identify().second == -1 
    ? 0 : p.identify().second);
    return out;
}

// King Implmentation
bool King::validate(int x, int y)
{
    int len = (*board).size();
    int width = (*board)[0].size();
    if (std::abs(x - position.first) <= 1 &&
        std::abs(y - position.second) <= 1)
    {
        if (x < len && x >= 0 && y < width && y >= 0 && ((*board)[x][y]->identify().first == PieceType::Empty || ((*board)[x][y]->identify().second != team && (*board)[x][y]->identify().second != -1)))
        {
            // validate that move does not place king in check here later
            return true;
        }
    }
    // before verify anything
    else if (position.first == x && noMove && std::abs(y - position.second) == 2)
    {
        // castle kingside, rook 3 squares right of king
        if (position.second + 3 < width && y - position.second > 0)
        {
            std::pair<PieceType, int> temp = (*board)[position.first][position.second + 3]->identify();

            if (temp.first == PieceType::Rook && temp.second == team)
            {
                Rook *pr = dynamic_cast<Rook *>((*board)[position.first][position.second + 3].get());
                if (pr->Castle() && (*board)[position.first][position.second + 1]->identify().first == PieceType::Empty && (*board)[position.first][position.second + 2]->identify().first == PieceType::Empty)
                {
                    return true;
                }
            }
        }
        // castle queenside, rook 4 squares left of king
        else if (position.second - 4 >= 0 && position.second - y > 0)
        {
            std::pair<PieceType, int> temp = (*board)[position.first][position.second - 4]->identify();
            if (temp.first == PieceType::Rook && temp.second == team)
            {
                Rook *pr = dynamic_cast<Rook *>((*board)[position.first][position.second - 4].get());
                if (pr->Castle() && (*board)[position.first][position.second - 1]->identify().first == PieceType::Empty 
                && (*board)[position.first][position.second - 2]->identify().first == PieceType::Empty && (*board)[position.first][position.second - 3]->identify().first == PieceType::Empty)
                {
                    return true;
                }
            }
        }

    }

    // may need to add checks along y-axis for variants of chess + castling
    return false;
}

King::King(int x, int y, int team, std::vector<std::vector<std::unique_ptr<Piece>>> *board) : Piece{x, y, PieceType::King, team, board} {}

// king can move one square in any direction, including diagonals, except into check
// can also castle if not moved, moving two spaces towards an unmoved rook
// moving the rook in the opposite direction that the king moved
bool King::move(int x, int y)
{
    if (King::validate(x, y))
    {
        // set position of piece to new position
        position.first = x;
        position.second = y;
        noMove = false;
        return true;
    }

    return false;
}

//-----------------------------------

// Queen Implmentation
bool Queen::validate(int x, int y) {
    const auto& currentPosition = position;
    if (position.first == x && position.second == y) return false;

    // Check if the move is horizontal, vertical, or diagonal
    bool queen_moves = (x == currentPosition.first || y == currentPosition.second ||
                        std::abs(x - currentPosition.first) == std::abs(y - currentPosition.second));

    if (queen_moves) {
        if (x != currentPosition.first && y != currentPosition.second) {
            // Diagonal movement

            // Determine direction of movement
            int col_step = (x > currentPosition.first) ? 1 : -1;
            int row_step = (y > currentPosition.second) ? 1 : -1;

            // Check the path one square away from the current position
            int cur_col = currentPosition.first + col_step;
            int cur_row = currentPosition.second + row_step;

            // Traverse the path until the target position
            while (cur_col != x && cur_row != y) {
                // If there is a piece in the path, it is invalid
                if ((*board)[cur_col][cur_row]->identify().first != PieceType::Empty ||
                    (*board)[cur_col][cur_row]->identify().second == team) {
                    return false; // Path is not empty
                }
                cur_col += col_step;
                cur_row += row_step;
            }
        } else {
            // Vertical or horizontal movement

            // Check the path in the vertical direction
            if (x == currentPosition.first) {
                int step = (y > currentPosition.second) ? 1 : -1;
                for (int cur_row = currentPosition.second + step; cur_row != y; cur_row += step) {
                    if ((*board)[x][cur_row]->identify().first != PieceType::Empty ||
                        (*board)[x][cur_row]->identify().second == team) {
                        return false; // Path is not empty
                    }
                }
            }

            // Check the path in the horizontal direction
            if (y == currentPosition.second) {
                int step = (x > currentPosition.first) ? 1 : -1;
                for (int cur_col = currentPosition.first + step; cur_col != x; cur_col += step) {
                    if ((*board)[cur_col][y]->identify().first != PieceType::Empty ||
                        (*board)[cur_col][y]->identify().second == team) {
                        return false; // Path is not empty
                    }
                }
            }
        }

        // Check if the destination square contains a piece of the same team
        if ((*board)[x][y]->identify().second == team) {
            return false;
        }

        return true; // Move is valid
    }

    return false;
}
  

Queen::Queen(int x, int y, int team, std::vector<std::vector<std::unique_ptr<Piece>>> *board) : Piece{x, y, PieceType::Queen, team, board} {}

bool Queen::move(int x, int y) {
    if (validate(x, y)) {
        position.first = x;
        position.second = y;
        return true;

    } else return false; 
}


//-----------------------------------

// Rook Implementation
bool Rook::validate(int x, int y)
{
    resetFlags();
    // up from position
    int tx = position.first-1;
    while (tx >= 0)
    {
        if ((*board)[tx][position.second]->identify().first == PieceType::Empty)
        {
            movable[tx][position.second] = true;
            tx--;
            continue;
        }
        else if ((*board)[tx][position.second]->identify().second != -1 &&
                 (*board)[tx][position.second]->identify().second != team)
        {
            movable[tx][position.second] = true;
            break;
        }
        else if ((*board)[tx][position.second]->identify().second == team)
        {
            break;
        }
    }

    // down from position
    tx = position.first+1;
    while (tx < blen)
    {
        if ((*board)[tx][position.second]->identify().first == PieceType::Empty)
        {
            movable[tx][position.second] = true;
            tx++;
            continue;
        }
        else if ((*board)[tx][position.second]->identify().second != -1 &&
                 (*board)[tx][position.second]->identify().second != team)
        {
            movable[tx][position.second] = true;
            break;
        }
        else if ((*board)[tx][position.second]->identify().second == team)
        {
            break;
        }
    }

    // left from position
    int ty = position.second-1;
    while (ty >= 0)
    {
        if ((*board)[position.first][ty]->identify().first == PieceType::Empty)
        {
            movable[position.first][ty] = true;
            ty--;
        }
        else if ((*board)[position.first][ty]->identify().second != -1 &&
                 (*board)[position.first][ty]->identify().second != team)
        {
            movable[position.first][ty] = true;
            break;
        }
        else if ((*board)[position.first][ty]->identify().second == team)
        {
            break;
        }
    }

    //right from position
    ty = position.second+1;
    while (ty < bwidth)
    {
        if ((*board)[position.first][ty]->identify().first == PieceType::Empty)
        {
            movable[position.first][ty] = true;
            ty++;
        }
        else if ((*board)[position.first][ty]->identify().second != -1 &&
                 (*board)[position.first][ty]->identify().second != team)
        {
            movable[position.first][ty] = true;
            break;
        }
        else if ((*board)[position.first][ty]->identify().second == team)
        {
            break;
        }
    }

    if (movable[x][y])
        return true;

    // TODO: Make sure that completing move will not place the corresponding king in check
    return false;
}

Rook::Rook(int x, int y, int team, std::vector<std::vector<std::unique_ptr<Piece>>> *board) : Piece{x, y, PieceType::Rook, team, board}
{
    blen = (*board).size();
    bwidth = (*board)[0].size();
    for (int i = 0; i < blen; ++i)
    {
        movable.push_back(std::vector<bool>(bwidth, false));
    }
}
bool Rook::move(int x, int y)
{
    if (Rook::validate(x, y))
    {
        position.first = x;
        position.second = y;
        this->noMove = false;
        return true;
    }
    return false;
}

//-----------------------------------

// Bishop Implementation

bool Bishop::validate(int x, int y) {
    // Bishop moves diagonally
    const auto& currentPosition = position;
    bool bishop_moves = (std::abs(x - currentPosition.first) == std::abs(y - currentPosition.second));
    if (position.first == x && position.second == y) return false;

    if (bishop_moves) {

        // Determines the direction of movement
        int col_step = (x > currentPosition.first) ? 1 : -1;
        int row_step = (y > currentPosition.second) ? 1 : -1;

        // Check the path one square away from the current position
        int cur_col = currentPosition.first + col_step;
        int cur_row = currentPosition.second + row_step;

        // Traverse the path until the target position
        while (cur_col != x && cur_row != y) {
            // If there is a piece in the path, it is invalid
            if (((*board)[cur_col][cur_row]->identify().first != PieceType::Empty ||
                  (*board)[cur_col][cur_row]->identify().second == team)) {
                return false;  // Path is not empty or has a piece of the same team
            }
            cur_col += col_step;
            cur_row += row_step;
        }

        // If the target position is empty or has an opponent's piece, the move is valid
        return ((*board)[x][y]->identify().first == PieceType::Empty || (*board)[x][y]->identify().second != team);
    }

    return false;
}

Bishop::Bishop(int x, int y, int team, std::vector<std::vector<std::unique_ptr<Piece>>>* board)
    : Piece{x, y, PieceType::Bishop, team, board} {}

bool Bishop::move(int x, int y) {
    if (validate(x, y)) {
        position.first = x;
        position.second = y;
        return true;
    }

    return false;
}


//-----------------------------------

// Knight Implementation


bool Knight::validate(int x, int y) {
    if (position.first == x && position.second == y) return false;
    const auto& currentPosition = position;

    // Calculate the horizontal and vertical distances
    int hozDistance = std::abs(x - currentPosition.first);
    int verDistance = std::abs(y - currentPosition.second);

    // Check if the move is an L-shaped knight move
    bool isLShaped = (hozDistance == 2 && verDistance == 1) || (hozDistance == 1 && verDistance == 2);

    // Check if the destination is empty or contains an opponent's piece
    if (isLShaped && ((*board)[x][y]->identify().first == PieceType::Empty || (*board)[x][y]->identify().second != team)) {
        return true;
    }

    return false;
}


Knight::Knight(int x, int y, int team, std::vector<std::vector<std::unique_ptr<Piece>>>* board)
    : Piece{x, y, PieceType::Knight, team, board} {}

bool Knight::move(int x, int y) {
    if (Knight::validate(x, y)) {
        position.first = x;
        position.second = y;
        return true;
    }
    return false;
}    

//-----------------------------------

// Pawn Implmentation

Pawn::Pawn(int x, int y, int team, std::vector<std::vector<std::unique_ptr<Piece>>> *board) : Piece{x, y, PieceType::Pawn, team, board} {}

bool Pawn::validate(int x, int y)
{

    int changeX = position.first - x;
    int direction = (team == 1) ? -1 : 1; // team 1 goes down, team 2 goes up

    // Move one square forward
    if (y == position.second && changeX == direction &&
        (*board)[x][y]->identify().first == PieceType::Empty)
    {
        justMadeDoubleStep = false;
        return true;
    }
    // Move two squares forward on first move
    else if (firstMove && y == position.second && changeX == 2 * direction &&
             (*board)[x+direction][y]->identify().first == PieceType::Empty &&
             (*board)[x][y]->identify().first == PieceType::Empty && firstMove)
    {
        justMadeDoubleStep = true;
        return true;
    }
    return false;
}

bool Pawn::validateCapture(int x, int y)
{
    int changeX = position.first - x;
    int changeY = std::abs(y - position.second);
    int direction = (team == 1) ? -1 : 1; //team 1 goes down, team 2 goes up

    if (changeX == direction && changeY == 1)
    {
        auto &targetPiece = (*board)[x][y];
        if (targetPiece->identify().first != PieceType::Empty && targetPiece->identify().second != team)
        {
            return true;
        }
    }
    return false;
}

bool Pawn::move(int x, int y)
{
    auto &destinationPiece = (*board)[x][y];
    bool isCapture = validateCapture(x, y);
    bool isEnPassantCapture = false;
    Pawn *pawn = nullptr;

    if (position.first != 1 && position.first != 6) firstMove = false;

    int column = y - position.second;

    // check if its left or right column of p1 that has an adjacent p2
    if ((column > 0 && position.second + 1 < 8) || (column < 0 && position.second - 1 >= 0)) {
       
        // find the pawn adjacent to the current piece
        auto& adjacentPiece = (*board)[position.first][position.second+column];
        // check if its both a pawn AND in the opposing team
        if (adjacentPiece->identify().first == PieceType::Pawn && adjacentPiece->identify().second != team) {
            pawn = dynamic_cast<Pawn*>(adjacentPiece.get()); // dynamic casting to make it a pawn
        }
    }

    if (pawn && pawn->justMadeDoubleStep) { 
        int changeX = position.first - x;
        int changeY = std::abs(y - position.second);
        int direction = (team == 1) ? -1 : 1; //team 1 goes down, team 2 goes up

        if (changeX == direction && changeY == 1) {
            isEnPassantCapture = true;
        }

    }
    

    if (isCapture || isEnPassantCapture)
    {
        // Special handling for en passant
        if (isEnPassantCapture){ // ALL OF THIS WORKS ! JUST THE DOUBLESTEP FLAG DAT IS WRONG
            int capturedPawnRow = position.first;
            int capturedPawnColumn = y;
            enPassant = std::move((*board)[capturedPawnRow][capturedPawnColumn]);
            (*board)[capturedPawnRow][capturedPawnColumn] = std::make_unique<Empty>(capturedPawnRow, capturedPawnColumn, board);
        }
        
        // Perform capture logic
        
        position.first = x;
        position.second = y;
        firstMove = false;

        return true;
    }

    else if (validate(x, y) && (!destinationPiece || destinationPiece->identify().first == PieceType::Empty))
    {
        // Perform standard move logic
        position.first = x;
        position.second = y;
        firstMove = false;
        return true;
    }
    
    return false;
}

bool Pawn::mValidate(int x, int y) {
    std::pair<int, int> pos = position;
    bool dStep = justMadeDoubleStep;
    bool first = firstMove;
    bool ret = move(x, y);
    position.first = pos.first;
    position.second = pos.second;
    justMadeDoubleStep = dStep;
    firstMove = first;
    //enpassant
    if (enPassant) {
        (*board)[enPassant->getX()][enPassant->getY()] = std::move(enPassant);
    }
    return ret;
}

bool Pawn::promote()
{
    if (position.first == 0 || position.first == 7) {
        return true;
    }
    return false;
}
//-----------------------------------
