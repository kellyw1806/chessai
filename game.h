#ifndef _GAME_H_
#define _GAME_H_
#include "board.h"
#include "graphicsdisplay.h"
#include <memory>
using namespace std;
class AbstractGame {

    friend ostream& operator<< (ostream & out, const AbstractGame &ag);

    protected:
        unique_ptr<Board> board;
        vector<int> teams;

        Piece * getPiece (int x, int y) {
            return board->getPiece(x, y);
        }

        //tracks the current team;
        int activeTeam = 2;
        

        AbstractGame(int x, int y, int numTeams):
        board{make_unique<Board>(x,y,numTeams)} {
            for (int i=1;i<=numTeams;++i) {
                teams.push_back(i);
            }
        }
        AbstractGame(unique_ptr<Board> board, int numTeams, int activeTeam) : 
        board{std::move(board)}, activeTeam{activeTeam} {
            for (int i=1;i<=numTeams;++i) {
                teams.push_back(i);
            }
        }
    public:
        bool promoteFlag = false;
        void promote(PieceType pt, int x, int y) {
            int team;
            if (x == 0) team = 2;
            if (x == 7) team = 1;  
            board->addPiece(x, y, team, pt);
            promoteFlag = false;
            notify();
        } 

        //moving the board
        unique_ptr<Board> moveBoard() {
            return std::move(board);
        }

        void attachDisplay(Xwindow & xw) {
            board->attach(make_unique<GraphicsDisplay>(8, xw));
        }

        void notify() {
            board->notifyObserver();
        }

        //what init does depends on the type of chess game being initalized
        virtual void init() = 0;
        virtual ~AbstractGame();
        //move piece at [sx][sy] to [ex][ey], completing returning true is possible
        //returns false if not possible, and does not change the board and does not shift turn order
        //move piece logic
        //TODO: promotions on pawn moves 
        // make sure that moved piece has team == activeTeam
        bool move(int sx, int sy, int ex, int ey);
        // checks if current team is stalemated
        virtual bool stalemate();
        // checks if current team is checkmated
        virtual bool checkmate();
        //resign the game, active team loses
        virtual void resign();
        //cpu move, does nothing if not overridden
        virtual bool CPUMove();

        // true if active player is in check, false otherwise
        bool isCheck();

        //reset the board to be filled with empty
        virtual void reset();

        //outputs just the board, if needed
        void printBoard() {
            std::cout << *board;
        }

        //validate if the setup is ok
        bool validateSetup();

        bool addPiece (int x, int y, int team, PieceType pt);
        void setTeam (int team) { activeTeam = team; }
        int getActive () { return activeTeam; }
        
};

//player vs player chess on standard board with classic rules
class StandardPlayer : public AbstractGame {

    public:
        StandardPlayer() : AbstractGame{8, 8, 2} {}
        StandardPlayer(unique_ptr<Board> board, int teams, int activeTeam) : 
        AbstractGame{std::move(board), teams, activeTeam} {}
        //setup the board
        void init() override;
        void resign() override {
            string team;
            //reversed
            if (activeTeam == 2) { team = "Black"; }
            if (activeTeam == 1) { team = "White"; }
            std::cout << team << " wins!" << endl;
            reset();
        }
};

//helper strcut to contain moves
struct Move
{
    int startX, startY;
    int endX, endY;
    // Constructor, if needed
    Move(int sx, int sy, int ex, int ey) : startX(sx), startY(sy), endX(ex), endY(ey) {}
};


//game vs computer
class StandardCPU : public AbstractGame {

    int playerTeam = -1;

    //if both are cpus, then cpu2 is white and cpu1 is black
    int level = 1;
    int levelCpu2 = -1;

    // level two helpers
    vector<Move> availMoves(Board &board, vector<Move> &moves);
    int evaluateMove(Board &board, Move &mv);
    void generateLegalMoves(std::vector<Move> &legalMoves);
    //------------------

    void levelFour();
    double fourEval(Move mv);
    double tableVal(Move mv);
    //queen position values
    double pstQ [8][8] = { {-2.0, -1.0, -1.0, -0.5, -0.5, -1.0, -1.0, -2.0},
        {-1.0, -0.5, -0.5, 0.0, 0.0, -0.5, -0.5, -1.0}, 
        {-1.0, -0.5, -0.5, 0.0, 0.0, -0.5, -0.5, -1.0},
        {-0.5, 0, 0.25, 0.5, 0.5, 0.25, 0, -0.5},
        {-0.5, 0, 0.25, 0.5, 0.5, 0.25, 0, -0.5},
        {-1.0, -0.5, -0.5, 0.0, 0.0, -0.5, -0.5, -1.0},
        {-1.0, -0.5, -0.5, 0.0, 0.0, -0.5, -0.5, -1.0},
        {-2.0, -1.0, -1.0, -0.5, -0.5, -1.0, -1.0, -2.0}};

    //white king position table
    double pstWK [8][8] { 
        {-4.0, -4.0, -5.0, -6.0, -6.0, -5.0, -4.0, -4.0},
        {-4.0, -4.0, -5.0, -6.0, -6.0, -5.0, -4.0, -4.0},
        {-4.0, -4.0, -5.0, -6.0, -6.0, -5.0, -4.0, -4.0},
        {-4.0, -4.0, -5.0, -6.0, -6.0, -5.0, -4.0, -4.0},
        {-3.0, -3.0, -4.0, -5.0, -5.0, -4.0, -3.0, -3.0},
        {-2.0, -2.0, -3.0, -4.0, -4.0, -3.0, -2.0, -2.0},
        {1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0},
        {3.0, 3.0, 3.0, 0.0, 0.0, 2.0, 5.0, 3.0}
    };

    //black king position table
    double pstBK [8][8] { 
        {3.0, 3.0, 3.0, 0.0, 0.0, 2.0, 5.0, 3.0},
        {1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0},
        {-2.0, -2.0, -3.0, -4.0, -4.0, -3.0, -2.0, -2.0},
        {-3.0, -3.0, -4.0, -5.0, -5.0, -4.0, -3.0, -3.0},
        {-4.0, -4.0, -5.0, -6.0, -6.0, -5.0, -4.0, -4.0},
        {-4.0, -4.0, -5.0, -6.0, -6.0, -5.0, -4.0, -4.0},
        {-4.0, -4.0, -5.0, -6.0, -6.0, -5.0, -4.0, -4.0},
        {-4.0, -4.0, -5.0, -6.0, -6.0, -5.0, -4.0, -4.0}
    };

    //knight position table
    double pstK [8][8] { 
        {-5.0, -3.0, -3.0, -3.0, -3.0, -3.0, -3.0, -5.0},
        {-4.0, -3.0, -3.0, -2.0, -2.0, -3.0, -3.0, -4.0},
        {-2.0, -1.0, 0.0, 1.0, 1.0, 0.0, -1.0, -2.0},
        {-2.0, 0.0, 1.5, 3.0, 3.0, 1.5, 0.0, -2.0},
        {-2.0, 0.0, 1.5, 3.0, 3.0, 1.5, 0.0, -2.0},
        {-2.0, -1.0, 0.0, 1.0, 1.0, 0.0, -1.0, -2.0},
        {-4.0, -3.0, -3.0, -2.0, -2.0, -3.0, -3.0, -4.0},
        {-5.0, -3.0, -3.0, -3.0, -3.0, -3.0, -3.0, -5.0}
    };

    //white pawn table
    double pstWP [8][8] { 
        {15.0, 15.0, 15.0, 15.0, 15.0, 15.0, 15.0, 15.0},
        {5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0},
        {2.0, 2.0, 3.0, 4.0, 4.0, 3.0, 2.0, 2.0},
        {1.0, 1.0, 2.0, 3.0, 3.0, 2.0, 1.0, 1.0},
        {0.0, 0.0, 0.0, 5.0, 5.0, 0.0, 0.0, 0.0},
        {1.0, -1.0, -1.0, 1.0, 1.0, -1.0, -1.0, 1.0},
        {0.0, 2.0, 2.0, 0.0, 0.0, 2.0, 2.0, 0.0},
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}
    };

    //black pawn table
    double pstBP [8][8] { 
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
        {1.0, -1.0, -1.0, 1.0, 1.0, -1.0, -1.0, 1.0},
        {0.0, 2.0, 2.0, 0.0, 0.0, 2.0, 2.0, 0.0},
        {0.0, 0.0, 0.0, 5.0, 5.0, 0.0, 0.0, 0.0},
        {1.0, 1.0, 2.0, 3.0, 3.0, 2.0, 1.0, 1.0},
        {2.0, 2.0, 3.0, 4.0, 4.0, 3.0, 2.0, 2.0},        
        {5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0},
        {15.0, 15.0, 15.0, 15.0, 15.0, 15.0, 15.0, 15.0}
    };

    //bishop table
    double pstB [8][8] { 
        {-5.0, -3.0, -3.0, -3.0, -3.0, -3.0, -3.0, -5.0},
        {-4.0, -3.0, -3.0, -2.0, -2.0, -3.0, -3.0, -4.0},
        {-2.0, -1.0, 0.0, 3.0, 3.0, 0.0, -1.0, -2.0},
        {-2.0, 0.0, 2.5, 5.0, 5.0, 2.5, 0.0, -2.0},
        {-2.0, 0.0, 2.5, 5.0, 5.0, 2.5, 0.0, -2.0},
        {-2.0, -1.0, 0.0, 3.0, 3.0, 0.0, -1.0, -2.0},
        {-4.0, -3.0, -3.0, -2.0, -2.0, -3.0, -3.0, -4.0},
        {-5.0, -3.0, -3.0, -3.0, -3.0, -3.0, -3.0, -5.0}
    };

    void levelThree();
    int threeEval(Move mv);

    void levelTwo();
    void levelOne();



    public:
        StandardCPU (int playerTeam, int level) : AbstractGame {8,8,2}, playerTeam{playerTeam}, level{level} {}
        StandardCPU(unique_ptr<Board> board, int playerTeam, int level, int teams, int activeTeam) : 
        AbstractGame{std::move(board), teams, activeTeam}, playerTeam{playerTeam}, level{level} {}
        
        StandardCPU(unique_ptr<Board> board, int playerTeam, int level, int levelCpu2, int teams, int activeTeam) : 
        AbstractGame{std::move(board), teams, activeTeam}, playerTeam{}, level{level}, levelCpu2{levelCpu2} {}

        void init() override;
        void resign() override {
            string team;
            //reversed
            if (activeTeam == 2) { team = "Black"; }
            if (activeTeam == 1) { team = "White"; }
            std::cout << team << " wins!" << endl;
            reset();
        }

        bool CPUMove() override;

        //tentative, but should be something like this
        //call this to make the cpu move after the player makes a move
        //use game.move() instead of board.move() to finalize and change turn order
        void getMove(int level) {
            if (level == 4) levelFour();
            else if (level == 3) levelThree();
            else if (level == 2) levelTwo();
            else levelOne();
        }
};


#endif
