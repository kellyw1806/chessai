#include "game.h"
#include "graphicsdisplay.h"
#include <ctime>
#include <vector>
#include <utility>

#include <algorithm>

AbstractGame::~AbstractGame() {}
void AbstractGame::resign() {}
bool AbstractGame::CPUMove() { return false; }

bool AbstractGame::move(int sx, int sy, int ex, int ey) {

    //piece is not on the caller's team
    if (board->getPiece(sx, sy)->identify().second != getActive()){
        return false;
    }
    if (board->movePiece(sx,sy,ex,ey)) {
                //cout << "team:" << activeTeam << endl;
                //swaps the active player to next team
                //cycles back to team 1 at the end of array
                if (activeTeam != teams.size()) {
                    activeTeam = teams[activeTeam]; 
                } else {
                    activeTeam = 1;
                }
                if((ex == 0 || ex == 7)&& getPiece(ex, ey)->identify().first == PieceType::Pawn) {
                    promoteFlag = true;
                }
                //reset flags everywhere, except for the piece that just moved
                //preserves pawn's enPassant vulnerability for an extra cycle
                //if pawn happens to moves twice in a row, Pawn class resets the flag itself
                board->flagReset(ex, ey);
                //cout << "team:" << activeTeam << endl;
                board->notifyObserver();
                return true;
    }
    return false;
}

void AbstractGame::reset() {
    board->reset();
    for (int i=0;i<board->getHeight();++i) {
        for (int j=0;j<board->getWidth(); ++j) {
            board->addPiece(i,j,0, PieceType::Empty);
        }
    }
}

bool AbstractGame::validateSetup() {
    int bKing = 0;
    int wKing = 0;
    bool valid = true;
                        //make sure board is in a valid state
                        //no pawns on edge ranks
    for (int i=0;i<8;++i) {
    if (board->getPiece(0,i)->identify().first == PieceType::Pawn ||
        board->getPiece(7,i)->identify().first == PieceType::Pawn) {
        cerr << "Invalid Setup: No pawns in first or last rows." << endl;
        valid = false;
        }
    }

    //check that there are exactly 1 White King and 1 Black King
    for (int i=0;i<8;++i) {
        for (int j=0;j<8;++j) {
            Piece * temp = board->getPiece(i,j);
            if (temp->identify().first == PieceType::King
                && temp->identify().second == 2) wKing++;
                if (temp->identify().first == PieceType::King
                     && temp->identify().second == 1) bKing++;
            }
        }

    if (bKing != 1 || wKing != 1) {
        cerr << "Number of kings != 1 for one or both players" << endl;
        return false;
        valid = false;
    } 

    //no kings can be in check
    for (int i=1;i<=2;++i) {
        std::pair<int,int> kPos = board->getKingPosition(i);
        if (!board->moveCheck(kPos.first, kPos.second, i)) {
            cerr << "One or more kings are in check" << endl;
            valid = false;
        }
    }
    if (valid == true) {
        return true;
    }
    return false;
}


bool AbstractGame::addPiece(int x, int y, int team, PieceType pt)
{
    if (pt == PieceType::King && board->getKingPosition(team).first != -1)
    {
        return false;
    }
    board->addPiece(x, y, team, pt);
    return true;
}

bool AbstractGame::stalemate()
{
    // Iterate over all squares on the board
    for (int x = 0; x < board->getHeight(); ++x)
    {
        for (int y = 0; y < board->getWidth(); ++y)
        {
            // Access the piece using getPiece method
            Piece *piece = board->getPiece(x, y);

            // Check if the piece belongs to the active team
            if (piece && piece->identify().second == activeTeam)
            {
                for (int dx = 0; dx < board->getHeight(); ++dx)
                {

                    for (int dy = 0; dy < board->getWidth(); ++dy)
                    {

                        if (board->movePiece(x, y, dx, dy))
                        {
                            board->unMove(dx, dy, x, y);
                            return false;
                        }
                    }
                }
            }
        }
    }
    return true; // No legal moves found, it's a stalemate
}

bool AbstractGame::checkmate()
{
    std::pair<int, int> kingPosition = board->getKingPosition(activeTeam);
    // Piece *kingPiece = board->getPiece(kingPosition.first, kingPosition.second);

    // Check if the king itself can move to a safe square
    for (int dx = 0; dx < board->getWidth(); ++dx)
    {
        for (int dy = 0; dy < board->getHeight(); ++dy)
        {
            // if (kingPiece->move(dx, dy))
            //{
            bool moveLegal = board->movePiece(kingPosition.first, kingPosition.second, dx, dy);
            // bool kingInCheck = board->moveCheck(dx, dy, activeTeam);
            if (moveLegal)
            {
                board->unMove(dx, dy, kingPosition.first, kingPosition.second);
                return false; // King can move to a safe square
            }
            //}
        }
    }

    // Check if any other piece can capture the attacker or block the check
    for (int x = 0; x < board->getHeight(); ++x)
    {
        for (int y = 0; y < board->getWidth(); ++y)
        {
            Piece *piece = board->getPiece(x, y);
            if (piece && piece->identify().second == activeTeam && !(x == kingPosition.first && y == kingPosition.second))
            {
                for (int dx = 0; dx < board->getWidth(); ++dx)
                {
                    for (int dy = 0; dy < board->getHeight(); ++dy)
                    {
                        // if (piece->move(dx, dy))
                        //{
                        bool moveLegal = board->movePiece(x, y, dx, dy);
                        // bool kingInCheck = board->moveCheck(kingPosition.first, kingPosition.second, activeTeam);
                        if (moveLegal)
                        {
                            board->unMove(dx, dy, x, y);
                            return false; // Another piece can intercept the check
                        }
                        //}
                    }
                }
            }
        }
    }

    return true; // Checkmate, no legal moves to get the king out of check
}


bool AbstractGame::isCheck() {
    std::pair<int, int> kPos = board->getKingPosition(activeTeam);
    if (board->moveCheck(kPos.first, kPos.second, activeTeam))
    {
        return false;
    }
    return true;
}

ostream &operator<<(ostream &out, const AbstractGame &ag)
{
    string colour;
    if (ag.activeTeam == 1)
        colour = "Black";
    else if (ag.activeTeam == 2)
        colour = "White";
    out << colour << " to move" << endl;
    out << *ag.board.get();
    return out;
}

/*
ostream& operator<< (ostream & out, StandardPlayer &sp) {
    string colour;
    if (sp.activeTeam == 1) colour = "White";
    else if (sp.activeTeam == 2) colour = "Black";
    out << colour << " to move" << endl;
    out << sp.board;
    return out;
}*/

void StandardPlayer::init()
{
    activeTeam = 2;
    // init pawns
    for (int i = 0; i < board->getWidth(); ++i)
    {
        board->addPiece(1, i, 1, PieceType::Pawn);
        board->addPiece(6, i, 2, PieceType::Pawn);
    }
    // init rooks
    board->addPiece(0, 0, 1, PieceType::Rook);
    board->addPiece(0, 7, 1, PieceType::Rook);
    board->addPiece(7, 0, 2, PieceType::Rook);
    board->addPiece(7, 7, 2, PieceType::Rook);
    // init bishops
    board->addPiece(0, 2, 1, PieceType::Bishop);
    board->addPiece(0, 5, 1, PieceType::Bishop);
    board->addPiece(7, 2, 2, PieceType::Bishop);
    board->addPiece(7, 5, 2, PieceType::Bishop);
    // init knights
    board->addPiece(0, 1, 1, PieceType::Knight);
    board->addPiece(0, 6, 1, PieceType::Knight);
    board->addPiece(7, 1, 2, PieceType::Knight);
    board->addPiece(7, 6, 2, PieceType::Knight);
    // init queens
    board->addPiece(0, 3, 1, PieceType::Queen);
    board->addPiece(7, 3, 2, PieceType::Queen);
    // init kings
    board->addPiece(0, 4, 1, PieceType::King);
    board->addPiece(7, 4, 2, PieceType::King);

    board->notifyObserver();
}

void StandardCPU::init()
{
    activeTeam = 2;
    // init pawns
    for (int i = 0; i < board->getWidth(); ++i)
    {
        board->addPiece(1, i, 1, PieceType::Pawn);
        board->addPiece(6, i, 2, PieceType::Pawn);
    }
    // init rooks
    board->addPiece(0, 0, 1, PieceType::Rook);
    board->addPiece(0, 7, 1, PieceType::Rook);
    board->addPiece(7, 0, 2, PieceType::Rook);
    board->addPiece(7, 7, 2, PieceType::Rook);
    // init bishops
    board->addPiece(0, 2, 1, PieceType::Bishop);
    board->addPiece(0, 5, 1, PieceType::Bishop);
    board->addPiece(7, 2, 2, PieceType::Bishop);
    board->addPiece(7, 5, 2, PieceType::Bishop);
    // init knights
    board->addPiece(0, 1, 1, PieceType::Knight);
    board->addPiece(0, 6, 1, PieceType::Knight);
    board->addPiece(7, 1, 2, PieceType::Knight);
    board->addPiece(7, 6, 2, PieceType::Knight);
    // init queens
    board->addPiece(0, 3, 1, PieceType::Queen);
    board->addPiece(7, 3, 2, PieceType::Queen);
    // init kings
    board->addPiece(0, 4, 1, PieceType::King);
    board->addPiece(7, 4, 2, PieceType::King);

    board->notifyObserver();
}



void StandardCPU::levelOne()
{
    // Seed the random number generator for different results each run
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // Use the helper function to generate all legal moves for the CPU's side
    std::vector<Move> legalMoves;
    generateLegalMoves(legalMoves);

    if (!legalMoves.empty())
    {
        
        // Randomly select a move from the legal moves
        int randomIndex = std::rand() % legalMoves.size();
        Move selectedMove = legalMoves[randomIndex];
       

        // Execute the selected move
        // cout << board->movePiece(1,0,2,0);
        move(selectedMove.startX, selectedMove.startY, selectedMove.endX, selectedMove.endY);
        if (promoteFlag) {
            promote(PieceType::Queen, selectedMove.endX, selectedMove.endY);
        }
    }
    else
    {
        cout << "No legal moves available." << endl;
    }
}


void StandardCPU::levelTwo()
{
    // Generate all legal moves for the CPU's side
    std::vector<Move> legal_moves;
    generateLegalMoves(legal_moves);

    // Evaluate and skim for preferred moves
    std::vector<Move> preferred_moves = availMoves(*board, legal_moves);

    // Execute one of the preferred moves
    if (!preferred_moves.empty())
    {
        // Randomly select one of the preferred moves
        std::srand(static_cast<unsigned int>(std::time(nullptr))); // Seed the random number generator
        int randint = std::rand() % preferred_moves.size();
        Move selectedMove = preferred_moves[randint];

        // Execute the selected move
        move(selectedMove.startX, selectedMove.startY, selectedMove.endX, selectedMove.endY);
        if (promoteFlag) {
            promote(PieceType::Queen, selectedMove.endX, selectedMove.endY);
        }
    }
    else
    {
        int randomIndex = std::rand() % legal_moves.size();
        Move selectedMove = legal_moves[randomIndex];

        // Execute the selected move
        move(selectedMove.startX, selectedMove.startY, selectedMove.endX, selectedMove.endY);
        if (promoteFlag) {
            promote(PieceType::Queen, selectedMove.endX, selectedMove.endY);
        }
    }
}

vector<Move> StandardCPU::availMoves(Board &board, vector<Move> &moves)
{
    vector<Move> preferred_moves;
    int max_score = 0;

    for (auto &mv : moves)
    {
        int score = evaluateMove(board, mv);
        if (score > max_score)
        {
            preferred_moves.clear();
            preferred_moves.push_back(mv);
            max_score = score;
        }
        else if (score == max_score)
        {
            preferred_moves.push_back(mv);
        }
    }

    return preferred_moves;
}

int StandardCPU::evaluateMove(Board &board, Move &mv)
{
    int score = 0;

    // Score for capturing moves
    Piece *target = board.getPiece(mv.endX, mv.endY);
    if (target && target->identify().second != activeTeam)
    {
        score += 10; // Base score for capturing any piece

        // Additional score based on the type of piece captured
        switch (target->identify().first)
        {
        case PieceType::Queen:
            score += 9;
            break;
        case PieceType::Rook:
        case PieceType::Bishop:
        case PieceType::Knight:
            score += 5;
            break;
        case PieceType::Pawn:
            score += 1;
            break;
        default:
            break; // No additional score for capturing an empty square
        }
    }

    return score;
}

bool StandardCPU::CPUMove() {
    //both cpus if cpu2 defined
    if (levelCpu2 != -1)  {
        if (activeTeam == 2) getMove(levelCpu2);
        else if (activeTeam == 1) getMove(level);
        return true;
    }

    // else only one cpu
    // function should only be called on cpus turn
    if (playerTeam == activeTeam)
        return false;
    else
        getMove(level);
    return true;

    board->notifyObserver();
}


// approach:
// if pieces are under attack, move the most valuable piece so that it is no longer under attack
// capture any favorable trade of depth one; i.e do not consider re captures
void StandardCPU::levelThree()
{
    std::vector<Move> moves;
    generateLegalMoves(moves);
    // move will be scored based on value of capture - value of piece
    // if move ending position is threatened by enemy, subtract value of piece
    // if move places opponenet in check, add some weight to that as well
    // value captures slightly more than non-capture, so cpu is not extremely passive
    std::vector<Move> ret;
    // most negative in this model will be forced losing queen at -9 score
    int max = -10;
    for (auto &move : moves)
    {
        int curr = threeEval(move);
        if (curr > max)
        {
            ret.clear();
            ret.emplace_back(move);
            max = curr;
        }
        else if (curr == max)
        {
            ret.emplace_back(move);
        }
    }
    // select random out of moves of equal score
    int id = std::rand() % ret.size();
    Move selectedMove = ret[id];
    move(selectedMove.startX, selectedMove.startY, selectedMove.endX, selectedMove.endY);
    if (promoteFlag) {
        promote(PieceType::Queen, selectedMove.endX, selectedMove.endY);
    }
    return;
}

int StandardCPU::threeEval(Move mv)
{
    // move will be scored based on value of capture - value of piece
    // if move ending position is threatened by enemy, subtract value of piece
    // if move places opponenet in check, add some weight to that as well
    // value captures slightly more than non-capture, so cpu is not extremely passive
    int score = 0;
    Piece *end = board->getPiece(mv.endX, mv.endY);
    // validate pointer, validate not on same team
    if (end && end->identify().second != activeTeam)
    {
        // not empty type implies capture,
        // encourage more aggressive CPU
        if (end->identify().first != PieceType::Empty)
            score++;

        switch (end->identify().first)
        {
        case PieceType::Queen:
            score += 9;
            break;
        case PieceType::Rook:
            score += 5;
            break;
        case PieceType::Knight:
        case PieceType::Bishop:
            score += 3;
            break;
        case PieceType::Pawn:
            score += 1;
            break;
        default:
            break;
        }
    }
    board->movePiece(mv.startX, mv.startY, mv.endX, mv.endY);
    end = board->getPiece(mv.endX, mv.endY);
    // score lost based on losing piece (only considers an immediate recapture)
    if (!board->moveCheck(mv.endX, mv.endY, activeTeam))
    {
        switch (end->identify().first)
        {
        case PieceType::Queen:
            score -= 9;
            break;
        case PieceType::Rook:
            score -= 5;
            break;
        case PieceType::Knight:
        case PieceType::Bishop:
            score -= 3;
            break;
        case PieceType::Pawn:
            score -= 1;
            break;
        default:
            break;
        }
    }

    int eTeam;
    if (activeTeam == 1)
        eTeam = 2;
    if (activeTeam == 2)
        eTeam = 1;
    std::pair<int, int> eKing = board->getKingPosition(eTeam);
    // add score for placing opponent in check
    if (!board->moveCheck(eKing.first, eKing.second, eTeam))
        score += 2;
    board->unMove(mv.endX, mv.endY, mv.startX, mv.startY);

    return score;
}


void StandardCPU::levelFour() {
    std::vector<Move> moves;
    generateLegalMoves(moves);
    std::vector<Move> ret;
    double epsilon = 0.0005;
    double max = -1000;
    for (auto &move : moves) {
        double curr = fourEval(move);
        if (curr > max) {
            ret.clear();
            ret.emplace_back(move);
            max = curr;
        }
        else if ((max - curr) < epsilon) {
            ret.emplace_back(move);
        }
    }
    //select random out of moves of equal score
    int id = std::rand() % ret.size();
    Move selectedMove = ret[id];
    move(selectedMove.startX, selectedMove.startY, selectedMove.endX, selectedMove.endY);
    if (promoteFlag) {
        promote(PieceType::Queen, selectedMove.endX, selectedMove.endY);
    }
    return;
}

double StandardCPU::tableVal (Move mv) {
    Piece * p = board->getPiece(mv.startX, mv.startY);
    double ret = 0;
    int team = p->identify().second;
    PieceType pt = p->identify().first;
    //white team tables
    if (team == 2) {
        switch (pt) {
            case PieceType::Queen: 
            ret += pstQ[mv.endX][mv.endY];
            break;
            case PieceType::King: 
            ret += pstWK[mv.endX][mv.endY];
            break;
            case PieceType::Knight: 
            ret += pstK[mv.endX][mv.endY];
            break;
            case PieceType::Pawn: 
            ret += pstWP[mv.endX][mv.endY];
            break;
            case PieceType::Bishop:
            ret += pstB[mv.endX][mv.endY];
            default:
            break;
        }
    }
    //black team tables
    else if (team == 1) {
        switch (pt) {
            case PieceType::Queen: 
            ret += pstQ[mv.endX][mv.endY];
            break;
            case PieceType::King: 
            ret += pstBK[mv.endX][mv.endY];
            break;
            case PieceType::Knight: 
            ret += pstK[mv.endX][mv.endY];
            break;
            case PieceType::Pawn: 
            ret += pstBP[mv.endX][mv.endY];
            break;
            case PieceType::Bishop:
            ret += pstB[mv.endX][mv.endY];
            default:
            break;
        }
    }
    return ret;
}

//evaluates the position for the given team
double StandardCPU::fourEval(Move mv) {
    int ret = 0;
    ret += tableVal(mv);
    Piece *end = board->getPiece(mv.endX, mv.endY);
    if (end && end->identify().second != activeTeam) {
        if (end->identify().first != PieceType::Empty) ret++;
        
        switch (end->identify().first) {
            case PieceType::Queen:
                ret+=90;
                break;
            case PieceType::Rook:
                ret+=50;
                break;
            case PieceType::Knight:
                ret+=30;
                break;
            case PieceType::Bishop:
                ret+=35;
                break;
            case PieceType::Pawn:
                ret+=10;
                break;
            default:
                break;
        }
    } 
    board->movePiece(mv.startX, mv.startY, mv.endX, mv.endY);
    end = board->getPiece(mv.endX, mv.endY);
    if (!board->moveCheck(mv.endX, mv.endY, activeTeam)) {
        switch (end->identify().first) {
            case PieceType::Queen:
                ret-=90;
                break;
            case PieceType::Rook:
                ret-=50;
                break;
            case PieceType::Knight:
            case PieceType::Bishop:
                ret-=30;
                break;
            case PieceType::Pawn:
                ret-=10;
                break;
            default:
                break;
        }
    }

    int eTeam;
    int team = activeTeam;
    if (activeTeam == 1) eTeam = 2;
    if (activeTeam == 2) eTeam = 1;
    std::pair<int,int> eKing = board->getKingPosition(eTeam);
    //add score for placing opponent in check
    setTeam(eTeam);
    if (!board->moveCheck(eKing.first, eKing.second, eTeam)) {
        ret += 7;
        //if checkmate available, always go for it
        if (checkmate()) ret += 500;
    }
    setTeam(team);
    board->unMove(mv.endX, mv.endY, mv.startX, mv.startY);

    return ret;
}

void StandardCPU::generateLegalMoves(std::vector<Move> &legalMoves)
{
    for (int startX = 0; startX < board->getHeight(); ++startX)
    {
        for (int startY = 0; startY < board->getWidth(); ++startY)
        {
            Piece *piece = board->getPiece(startX, startY);
            if (piece && piece->identify().second == activeTeam)
            {
                for (int endX = 0; endX < board->getHeight(); ++endX)
                {
                    for (int endY = 0; endY < board->getWidth(); ++endY)
                    {
                        if (board->movePiece(startX, startY, endX, endY))
                        {
                            // cout << *board << endl;
                            board->unMove(endX, endY, startX, startY);
                            // cout << *board << endl;
                            legalMoves.emplace_back(startX, startY, endX, endY);
                        }
                    }
                }
            }
        }
    }
}
