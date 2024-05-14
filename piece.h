#ifndef _PIECE_H_
#define _PIECE_H_
#include <utility>
#include <vector>
#include <memory>


enum class PieceType { King, Queen, Rook, Bishop, Knight, Pawn, Empty };

class Piece {

    //validate if possible to arrive at ending position
    
    virtual bool validate(int x, int y);
    friend std::ostream &operator<<(std::ostream &out, Piece &p);
    protected: 
        std::pair <int,int> position;
        PieceType type;
        int team;
        std::vector<std::vector<std::unique_ptr<Piece>>> * board;
    public:
        //store pointer to where capture occurs
        std::unique_ptr<Piece> capture;
        Piece(int x, int y, PieceType type, int team, std::vector<std::vector<std::unique_ptr<Piece>>> * board) : 
        position{std::make_pair(x,y)}, type{type}, team{team}, board{board} {}
        //publlic method called to move a piece
        virtual bool move(int x, int y) = 0; 
        virtual ~Piece();
        std::pair<PieceType, int> identify();
        int getX() { return position.first; }
        int getY() { return position.second; }
        //Unsafe to call in general case; 
        //call this function only to reset piece position on illegal move
        void setPos(int x, int y) {
            position.first = x;
            position.second = y;
        }
        virtual bool mValidate(int x, int y);
};

class King : public Piece {
    bool noMove = true;
    bool validate (int x, int y) override;
    public:
        King(int x, int y, int team,  std::vector<std::vector<std::unique_ptr<Piece>>> * board);
        bool move (int x, int y) override;
        bool Castle() { return noMove; }
        void setCastle(bool flag) { noMove = flag; }
};

class Queen : public Piece {
    bool validate (int x, int y) override;
    public :
        Queen(int x, int y, int team, std::vector<std::vector<std::unique_ptr<Piece>>>* board);
        bool move (int x, int y) override;

};

class Rook : public Piece {
    std::vector<std::vector<bool>> movable;
    int blen;
    int bwidth;
    void resetFlags() {
        for (int i=0;i<blen;++i) {
            for (int j=0;j<blen;++j) {
                movable[i][j] = false;
            }
        }
    }
    bool validate (int x, int y) override;
    
    protected : bool noMove = true;
    public:
        Rook(int x, int y, int team, std::vector<std::vector<std::unique_ptr<Piece>>>* board);
        bool Castle() { return noMove; }
        void setCastle(bool flag) { noMove = flag; }
        bool move (int x, int y) override;

};

class Bishop : public Piece {
    bool validate (int x, int y) override;
    public :
        Bishop(int x, int y, int team, std::vector<std::vector<std::unique_ptr<Piece>>>* board);
        bool move (int x, int y) override;

};

class Knight : public Piece {
    bool validate (int x, int y) override;
    public :
        Knight(int x, int y, int team, std::vector<std::vector<std::unique_ptr<Piece>>>* board);
        bool move (int x, int y) override;

};

class Pawn : public Piece {
    friend class Board;
    bool firstMove = true; // check if this is the pawn's first move
    bool justMadeDoubleStep = false;
    bool validate(int x, int y) override;

    //store pointer to where enpassant occured
    std::unique_ptr<Piece> enPassant;
    // check for valid capture -> this only checks if the pawn moved one square diagonally.
    // it does not check if there was another piece in the diagonal square to BE captured
    bool validateCapture(int x, int y);

    public:
        bool mValidate(int x, int y) override;
        Pawn(int x, int y, int team, std::vector<std::vector<std::unique_ptr<Piece>>>* board);
        bool move(int x, int y) override;

        bool canCaptureEnPassant(int x, int y, Pawn *opponentPawn);
        // only promotes this pawn to a queen. does not check whether it has reached the opposite side of the board
        bool promote();
};

//represents unoccupied tile
class Empty : public Piece {
    public :
        Empty(int x, int y, std::vector<std::vector<std::unique_ptr<Piece>>>* board) : 
        Piece{x, y, PieceType::Empty, -1, board} {}
        bool move(int x, int y) override {
            return false;
        }
};

#endif
