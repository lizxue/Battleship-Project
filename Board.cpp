#include "Board.h"
#include "Game.h"
#include "globals.h"
#include <iostream>
#include <vector>
#include <map>

using namespace std;

class BoardImpl
{
public:
    BoardImpl(const Game& g);
    void clear();
    void block();
    void unblock();
    bool placeShip(Point topOrLeft, int shipId, Direction dir);
    bool unplaceShip(Point topOrLeft, int shipId, Direction dir);
    void display(bool shotsOnly) const;
    bool attack(Point p, bool& shotHit, bool& shipDestroyed, int& shipId);
    bool allShipsDestroyed() const { return m_shipsInPlay.empty(); }
    
private:
    vector<vector<char> > m_board;
    map<int, int> m_shipsInPlay;
    const Game& m_game;
};

BoardImpl::BoardImpl(const Game& g)
: m_game(g), m_shipsInPlay({})
{
    // This compiles, but may not be correct
    m_board.resize(g.rows());
    for (int r = 0; r < g.rows(); r++)
    {
        // Resize the inner vector to the number of columns in the game
        m_board[r].resize(g.cols());
        for (int c = 0; c < g.cols(); c++)
        {
            m_board[r][c] = '.';
        }
    }
}

void BoardImpl::clear()
{
    for (int r = 0; r < m_game.rows(); r++)
    {
        for (int c = 0; c < m_game.cols(); c++)
        {
            m_board[r][c] = '.';
        }
    }
}

void BoardImpl::block()
{
    // Block cells with 50% probability
    for (int r = 0; r < m_game.rows(); r++)
    {
        for (int c = 0; c < m_game.cols(); c++)
        {
            if (randInt(2) == 0)
            {
                m_board[r][c] = 'X';
            }
        }
    }
}

void BoardImpl::unblock()
{
    for (int r = 0; r < m_game.rows(); r++)
    {
        for (int c = 0; c < m_game.cols(); c++)
        {
            if (m_board[r][c] == 'X')
            {
                m_board[r][c] = '.';
            }
        }
    }
}

bool BoardImpl::placeShip(Point topOrLeft, int shipId, Direction dir)
{
    // If shipId is not valid return false
    if (shipId < 0 || shipId > m_game.nShips() - 1)
        return false;
    // If point is out of bounds return false
    if (topOrLeft.r < 0 || topOrLeft.r > m_game.rows() - 1 || topOrLeft.c < 0 || topOrLeft.c > m_game.cols() - 1)
        return false;
    // If ship is already in play return false
    if (m_shipsInPlay.find(shipId) != m_shipsInPlay.end())
        return false;
    int len = m_game.shipLength(shipId);
    // Horizontal placement
    if (dir == HORIZONTAL)
    {
        Point left = topOrLeft;
        // Check length to make sure ship fits
        if (left.c + len > m_game.cols())
            return false;
        for (int i = 0; i < len; i++)
        {
            // If slot for ship is not empty return false
            if (m_board[left.r][left.c+i] != '.')
                return false;
        }
        // Place ship on board
        for (int i = 0; i < len; i++)
            m_board[left.r][left.c+i] = shipId+'0';
    }
    // Vertical placement
    else // dir == VERTICAL
    {
        Point top = topOrLeft;
        // Check length to make sure ship fits
        if (top.r + len > m_game.rows())
            return false;
        for (int i = 0; i < len; i++)
        {
            // If slot for ship is not empty return false
            if (m_board[top.r+i][top.c] != '.')
                return false;
        }
        // Place ship on board
        for (int i = 0; i < len; i++)
            m_board[top.r+i][top.c] = shipId+'0';
    }
    // Add to ships in play and return true
    m_shipsInPlay.insert(make_pair(shipId, m_game.shipLength(shipId)));
    return true;
}

bool BoardImpl::unplaceShip(Point topOrLeft, int shipId, Direction dir)
{
    if (shipId < 0 || shipId > m_game.nShips() - 1)
        return false;
    // If out of bounds return false
    if (dir == VERTICAL && topOrLeft.r + m_game.shipLength(shipId) > m_game.rows())
        return false;
    // If out of bounds return false
    if (dir == HORIZONTAL && topOrLeft.c + m_game.shipLength(shipId) > m_game.cols())
        return false;
    // If out of bounds return false
    if (topOrLeft.r < 0 || topOrLeft.r > m_game.rows() - 1 || topOrLeft.c < 0 || topOrLeft.c > m_game.cols() - 1)
        return false;
    // If ship is not in play return false
    if (m_shipsInPlay.find(shipId) == m_shipsInPlay.end())
        return false;
    int len = m_game.shipLength(shipId);
    // Direction is horizontal
    if (dir == HORIZONTAL)
    {
        Point left = topOrLeft;
        for (int i = 0; i < len; i++)
        {
            // If slot does not equal the shipId return false
            if (m_board[left.r][left.c+i]-48 != shipId)
                return false;
        }
        // Remove ship from board
        for (int i = 0; i < len; i++)
            m_board[left.r][left.c+i] = '.';
    }
    // Direction is vertical
    else // dir == VERTICAL
    {
        Point top = topOrLeft;
        for (int i = 0; i < len; i++)
        {
            // If slot does not equal shipId return false
            if (m_board[top.r+i][top.c]-48 != shipId)
                return false;
        }
        // Remove ship from board
        for (int i = 0; i < len; i++)
            m_board[top.r+i][top.c] = '.';
    }
    // Erase ship from ships in play and return true
    m_shipsInPlay.erase(shipId);
    return true;
}

void BoardImpl::display(bool shotsOnly) const
{
    // Output top row
    cout << "  ";
    for (int n = 0; n < m_game.cols(); n++)
        cout << n;
    cout << endl;
    
    // Output remainder of the board
    for (int r = 0; r < m_game.rows(); r++)
    {
        cout << r << " ";
        for (int c = 0; c < m_game.cols(); c++)
        {
            if (m_board[r][c] == '.' || m_board[r][c] == 'o' || m_board[r][c] == 'X')
                cout << m_board[r][c];
            else // board cell contains an unattacked boat segment
            {
                if (shotsOnly)
                    cout << '.';
                else
                    cout << m_game.shipSymbol(m_board[r][c]-48);
            }
        }
        cout << endl;
    }
}

bool BoardImpl::attack(Point p, bool& shotHit, bool& shipDestroyed, int& shipId)
{
    // If shot is out of bounds return false
    if (p.r < 0 || p.r > m_game.rows() || p.c < 0 || p.c > m_game.rows())
    {
        // Used to let Game::play() know user wasted shot
        shipId = -1;
        return false;
    }
    
    // If cell already shot return false
    if (m_board[p.r][p.c] == 'o' || m_board[p.r][p.c] == 'X')
    {
        shipId = -1;
        return false;
    }
    // If cell is empty mark with missed
    else if (m_board[p.r][p.c] == '.')
    {
        shotHit = false;
        shipDestroyed = false;
        m_board[p.r][p.c] = 'o';
    }
    // Hit a ship
    else
    {
        // Set variables to shipId and mark board as hit
        shotHit = true;
        shipId = m_board[p.r][p.c]-48;
        m_board[p.r][p.c] = 'X';
        // Decrement ships health
        m_shipsInPlay[shipId]--;
        // If the ships health is zero shipDestroyed is true
        if (m_shipsInPlay[shipId] == 0)
        {
            shipDestroyed = true;
            // Erase ship from ships in play
            m_shipsInPlay.erase(shipId);
        }
    }
    // Return true
    return true;
}

//******************** Board functions ********************************

// These functions simply delegate to BoardImpl's functions.
// You probably don't want to change any of this code.

Board::Board(const Game& g)
{
    m_impl = new BoardImpl(g);
}

Board::~Board()
{
    delete m_impl;
}

void Board::clear()
{
    m_impl->clear();
}

void Board::block()
{
    return m_impl->block();
}

void Board::unblock()
{
    return m_impl->unblock();
}

bool Board::placeShip(Point topOrLeft, int shipId, Direction dir)
{
    return m_impl->placeShip(topOrLeft, shipId, dir);
}

bool Board::unplaceShip(Point topOrLeft, int shipId, Direction dir)
{
    return m_impl->unplaceShip(topOrLeft, shipId, dir);
}

void Board::display(bool shotsOnly) const
{
    m_impl->display(shotsOnly);
}

bool Board::attack(Point p, bool& shotHit, bool& shipDestroyed, int& shipId)
{
    return m_impl->attack(p, shotHit, shipDestroyed, shipId);
}

bool Board::allShipsDestroyed() const
{
    return m_impl->allShipsDestroyed();
}
