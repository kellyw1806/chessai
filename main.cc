#include "game.h"
#include <iostream>
#include <string>
#include <sstream>
#include <ctime>

using namespace std;

PieceType charToType(char c)
{
    switch (c)
    {
    case 'B':
        return PieceType::Bishop;
    case 'R':
        return PieceType::Rook;
    case 'Q':
        return PieceType::Queen;
    case 'N':
        return PieceType::Knight;
    case 'K':
        return PieceType::King;
    case 'P':
        return PieceType::Pawn;
    default:
        return PieceType::Empty; // Assuming 'Empty' is used for an invalid or empty piece
    }
}

string teamToColour(int team)
{
    if (team == 1)
        return "Black";
    else
        return "White";
}

int main()
{
    // ajdfhjkasdhf
    Xwindow xw; // Ensure Xwindow is initialized correctly.
    // Board board(8, 8, 2); // Initialize the board.
    // Create GraphicsDisplay.

    // // Attach the observer to the board.
    // board.attach(make_unique<GraphicsDisplay>(8, xw));
    // -----

    // seed random gen for cpu movements
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    unique_ptr<AbstractGame> game = make_unique<StandardPlayer>();
    game->attachDisplay(xw);
    //game->getBoard().attach(make_unique<GraphicsDisplay>(8, xw));
    string line, command;
    int whiteWins = 0;
    int blackWins = 0;
    // int sx, sy, ex, ey, team;
    bool custom = false;
    bool gameRunning = false;

    while (getline(cin, line))
    {
        stringstream ss(line);
        ss >> command;

        // standard board if no init function is run
        if (command == "game")
        {
            if (!gameRunning)
            {
                string whitePlayer, blackPlayer;
                ss >> whitePlayer >> blackPlayer;
                // both players are human
                if (whitePlayer == "human" && blackPlayer == "human")
                {
                    game.reset(new StandardPlayer(game->moveBoard(), 2, game->getActive()));
                }
                // one or both are computer
                else if (whitePlayer == "human" && blackPlayer.find("computer") != std::string::npos && blackPlayer.length() >= 9)
                {
                    int level = blackPlayer[8] - '0';

                    if (level > 4 || level < 1)
                    {
                        cerr << "Computer level out of bounds" << endl;
                        continue;
                    }
                    game.reset(new StandardCPU(game->moveBoard(), 2, level, 2, game->getActive()));
                }
                else if (blackPlayer == "human" && whitePlayer.find("computer") != std::string::npos && whitePlayer.length() >= 9)
                {
                    int level = whitePlayer[8] - '0';

                    if (level > 4 || level < 1)
                    {
                        cerr << "Computer level out of bounds" << endl;
                        continue;
                    }
                    game.reset(new StandardCPU(game->moveBoard(), 1, level, 2, game->getActive()));
                }
                // both computers
                else if (whitePlayer.find("computer") != std::string::npos &&
                         blackPlayer.find("computer") != std::string::npos && whitePlayer.length() >= 9 && blackPlayer.length() >= 9)
                {
                    int levelWhite = whitePlayer[8] - '0';
                    int levelBlack = blackPlayer[8] - '0';
                    if (levelWhite > 4 || levelWhite < 1 || levelBlack > 4 || levelBlack < 1)
                    {
                        cerr << "Computer level out of bounds" << endl;
                        continue;
                    }

                    game.reset(new StandardCPU(game->moveBoard(), -1, levelBlack, levelWhite, 2, game->getActive()));
                }
                else
                {
                    cerr << "Invalid arguments for game" << endl;
                    continue;
                }
                // if setup not run, game is initialized to standard state
                if (!custom)
                {
                    game->init();
                }
                gameRunning = true;
                std::cout << *game;
            }
        }
        else if (command == "resign")
        {
            if (gameRunning)
            {
                // Handle resignation
                game->resign();
                if (game->getActive() == 1)
                    whiteWins++;
                else if (game->getActive() == 2)
                    blackWins++;
                gameRunning = false;
                custom = false;
            }
        }
        else if (command == "move")
        {
            if (gameRunning)
            {
                string from, to, promotion;
                ss >> from >> to;

                if (from.empty() || to.empty())
                {
                    // TODO: Computer moves
                    if (!game->CPUMove())
                    {
                        cerr << "Invalid request for computer move" << endl;
                    }
                }
                else
                {
                    // Convert from and to to board coordinates (e.g., e2 to (4, 1))
                    int fromY = from[0] - 'a';
                    int fromX = 8 - (from[1] - '0');
                    int toY = to[0] - 'a';
                    int toX = 8 - (to[1] - '0');

                    if (!game->move(fromX, fromY, toX, toY)) {
                        cerr << "Invalid Move" << endl;
                        continue;
                    }
                    if (ss >> promotion && game->promoteFlag)
                    {

                        game->promote(charToType(promotion[0]), toX, toY);
                        game->promoteFlag = false;
                    }
                }
                std::cout << *game;
                if (game->isCheck())
                {
                    std::cout << teamToColour(game->getActive()) << " is in check" << endl;
                    if (game->checkmate())
                    {
                        string colour;
                        // reversed
                        if (game->getActive() == 2)
                        {
                            colour = "Black";
                            blackWins++;
                        }
                        if (game->getActive() == 1)
                        {
                            colour = "White";
                            whiteWins++;
                        }
                        std::cout << "Checkmate! " << colour << " wins!" << endl;
                        game.reset();
                        custom = false;
                        gameRunning = false;
                    }
                }
                else if (game->stalemate())
                {
                    std::cout << "Stalemate! " << endl;
                    game.reset();
                    custom = false;
                    gameRunning = false;
                }
            }
            else
            {
                cerr << "Cannot make moves while game is not running" << endl;
            }
        }
        // if setup is run, use the board built in setup and not the standard
        else if (command == "setup")
        {
            if (!gameRunning)
            {
                string setupCommand;
                custom = true;
                while (getline(cin, setupCommand))
                {
                    stringstream setupStream(setupCommand);
                    setupStream >> command;

                    if (command == "+")
                    {
                        // change language to adhere to docs
                        char piece;
                        string pos;
                        int team;
                        setupStream >> piece >> pos;
                        // cout << pos << endl;
                        // capital is white, lowercase is black
                        if (piece >= 'A' && piece <= 'Z')
                        {
                            team = 2;
                        }
                        else if (piece >= 'a' && piece <= 'z')
                        {
                            team = 1;
                        }
                        // assuming all valid input will have lowercase
                        piece = std::toupper(piece);
                        pos[0] = pos[0] - 'a';
                        pos[1] = 8 - (pos[1] - '0');
                        // cout << piece << " " << pos[1] << pos[0] << " " << team << endl;

                        if (!game->addPiece(pos[1], pos[0], team, charToType(piece)))
                        {
                            std::cout << "Cannot add another king on same team as existing kings" << endl;
                        }
                        game->notify();
                        game->printBoard();
                    }
                    else if (command == "-")
                    {
                        string pos;
                        setupStream >> pos;
                        pos[0] = pos[0] - 'a';
                        pos[1] = pos[1] - '0';
                        pos[1] = 8 - pos[1];
                        // Remove piece from board
                        game->addPiece(pos[1], pos[0], -1, PieceType::Empty);
                        game->notify();
                    }
                    else if (command == "=")
                    { // Set next player's turn
                        // white is first team, black is second team
                        string color;
                        setupStream >> color;
                        if (color == "black")
                        {
                            game->setTeam(1);
                        }
                        if (color == "white")
                        {
                            game->setTeam(2);
                        }
                    }
                    else if (command == "done")
                    {
                        if ( game->validateSetup() ) {
                            break;
                        }      
                    }
                }
            }
            else
            {
                // no setup allowed while game is running
                // game already running error message
                cerr << "No setup while game is running." << endl;
            }
        }
        else
        {
            cerr << "Invalid command." << endl;
        }
    }
    std::cout << "Final Score:" << endl;
    std::cout << "White: " << whiteWins << endl;
    std::cout << "Black: " << blackWins << endl;
    return 0;
}
