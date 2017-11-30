#include "Player.h"
#include "Board.h"
#include "Game.h"
#include "globals.h"
#include <iostream>
#include <string>
#include <stack>

using namespace std;

void removePoint(Point p, vector<Point>& v)
{
    auto it = v.begin();
    for (; it != v.end(); )
    {
        if (it->r == p.r && it->c == p.c)
            it = v.erase(it);
        else
            it++;
    }
}

//*********************************************************************
//  AwfulPlayer
//*********************************************************************

class AwfulPlayer : public Player
{
public:
    AwfulPlayer(string nm, const Game& g);
    virtual bool placeShips(Board& b);
    virtual Point recommendAttack();
    virtual void recordAttackResult(Point p, bool validShot, bool shotHit,
                                    bool shipDestroyed, int shipId);
    virtual void recordAttackByOpponent(Point p);
private:
    Point m_lastCellAttacked;
};

AwfulPlayer::AwfulPlayer(string nm, const Game& g)
: Player(nm, g), m_lastCellAttacked(0, 0)
{}

bool AwfulPlayer::placeShips(Board& b)
{
    // Clustering ships is bad strategy
    for (int k = 0; k < game().nShips(); k++)
        if ( ! b.placeShip(Point(k,0), k, HORIZONTAL))
            return false;
    return true;
}

Point AwfulPlayer::recommendAttack()
{
    if (m_lastCellAttacked.c > 0)
        m_lastCellAttacked.c--;
    else
    {
        m_lastCellAttacked.c = game().cols() - 1;
        if (m_lastCellAttacked.r > 0)
            m_lastCellAttacked.r--;
        else
            m_lastCellAttacked.r = game().rows() - 1;
    }
    return m_lastCellAttacked;
}

void AwfulPlayer::recordAttackResult(Point /* p */, bool /* validShot */,
                                     bool /* shotHit */, bool /* shipDestroyed */,
                                     int /* shipId */)
{
    // AwfulPlayer completely ignores the result of any attack
}

void AwfulPlayer::recordAttackByOpponent(Point /* p */)
{
    // AwfulPlayer completely ignores what the opponent does
}

//*********************************************************************
//  HumanPlayer
//*********************************************************************

bool getLineWithTwoIntegers(int& r, int& c)
{
    bool result(cin >> r >> c);
    if (!result)
        cin.clear();  // clear error state so can do more input operations
    cin.ignore(10000, '\n');
    return result;
}

class HumanPlayer : public Player
{
public:
    HumanPlayer(string nm, const Game& g) : Player(nm, g) {}
    virtual ~HumanPlayer() {}
    virtual bool isHuman() const { return true; }
    virtual bool placeShips(Board& b);
    virtual Point recommendAttack();
    virtual void recordAttackResult(Point p, bool validShot, bool shotHit, bool shipDestroyed, int shipId) { }
    virtual void recordAttackByOpponent(Point p) { }
};

bool HumanPlayer::placeShips(Board& b)
{
    string d;
    bool valid = false;
    int r = -1, c = -1;
    Direction dir = HORIZONTAL;
    for (int i = 0; i < game().nShips(); i++)
    {
        cout << Player::name() << " there are " << game().nShips()-i << " ships not been placed.";
        cout << endl;
        b.display(false);
        // Prompt user for direction until valid
        while (!valid)
        {
            cout << "Enter h or v for direction of " << game().shipName(i) << " (length " << game().shipLength(i) << "): ";
            cin >> d;
            if (d != "h" && d != "v")
                cout << "Direction must be h or v." << endl;
            else
                valid = true;
            // Clear buffer
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
        valid = false;
        if (d == "v")
            dir = VERTICAL;
        else
            dir = HORIZONTAL;
        // Prompt user for point of placement until valid
        while (!valid)
        {
            cout << "Enter row and column of ";
            if (dir == VERTICAL)
                cout << "topmost";
            else
                cout << "leftmost";
            cout << " cell (e.g. 3 5): ";
            cin >> r >> c;
            cin.clear();
            if (!b.placeShip(Point(r, c), i, dir))
                cout << "The ship cannot be placed there." << endl;
            else
                valid = true;
            // Clear buffer
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
        valid = false;
    }
    return true;
}

/**
 recommendAttack for Human Player
 
 Accepts user input for where to attack
 */
Point HumanPlayer::recommendAttack()
{
    int r = 0, c = 0;
    while (1)
    {
        cout << "Enter the row and column to attack (e.g. 3 5): ";
        cin >> r >> c;
        if (!cin.good())
        {
            cin.clear();
            cin.ignore(10000, '\n');
            //cout << endl;
            cout << "You must enter two integers." << endl;
            continue;
        }
        break;
    }
    Point p(r, c);
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    return p;
}

//*********************************************************************
//  MediocrePlayer
//*********************************************************************

class MediocrePlayer : public Player
{
public:
    MediocrePlayer(string nm, const Game& g);
    ~MediocrePlayer() {}
    virtual bool placeShips(Board& b);
    virtual Point recommendAttack();
    virtual void recordAttackResult(Point p, bool validShot, bool shotHit, bool shipDestroyed, int shipId);
    virtual void recordAttackByOpponent(Point p) { /* do nothing */ }
    
    bool auxPlaceShips(Board& b, int shipsLeft, Point p, Direction d, int id, bool backTrack, vector<Point> added, vector<Direction> dirs);
    Point calculateShot();
    void buildCalculatedPoints(Point p);
    
private:
    // Stores last cell HIT -- only set when a shot hits
    Point m_lastCellHit;
    // Stores state of player -- two states: randomly firing and calculated firing
    int m_state;
    // Stores the points available on the board to shoot
    vector<Point> m_points;
    // Stores the calculated points available when set to state 2
    vector<Point> m_calculatedPoints;
    // Stores history of shots -- misses and hits
    vector<vector<char> > m_hist;
    // Allows the player to know when to build the calculated points
    bool buildCPoints;
};

MediocrePlayer::MediocrePlayer(string nm, const Game& g)
: Player(nm, g), m_state(1), m_lastCellHit(0, 0), m_calculatedPoints({}), buildCPoints(false)
{
    m_hist.resize(game().rows());
    for (int r = 0; r < game().rows(); r++)
    {
        m_hist[r].resize(game().cols());
        for (int c = 0; c < game().cols(); c++)
        {
            m_points.push_back(Point(r,c));
            m_hist[r][c] = '.';
        }
    }
}

bool MediocrePlayer::placeShips(Board& b)
{
    bool valid = false;
    int counter = 0;
    // Attempt to place ships 50 times
    while (!valid && counter < 50)
    {
        // Block ~50% of board before placement
        b.block();
        valid = auxPlaceShips(b, game().nShips(), Point(0,0), HORIZONTAL, 0, false, {}, {});
        // Unblock board after attempting to place
        b.unblock();
        counter++;
    }
    return valid;
}

bool MediocrePlayer::auxPlaceShips(Board& b, int shipsLeft, Point p, Direction d, int id, bool backTrack, vector<Point> added, vector<Direction> dirs)
{
    // Set to true if ship is placed false otherwise
    bool valid;
    
    // all ships placed
    if (shipsLeft == 0) return true;
    // If c exceeds game columns set it to 0 and increment row count
    if (p.c > game().cols()-1)
    {
        p.c = 0;
        p.r++;
    }
    // If r exceeds game rows start backtracking
    if (p.r > game().rows()-1 && p.c == 0) backTrack = true;
    // If no ships in added and backtrack is true return false -- no viable arrangment for ships
    if (added.size() == 0 && backTrack) return false;
    
    // If not backtracking
    if (!backTrack)
    {
        valid = b.placeShip(p, id, d);
        if (valid)
        {
            // Add point of placement
            added.push_back(p);
            // Add direction or placedment
            dirs.push_back(d);
            return auxPlaceShips(b, shipsLeft-1, Point(0, 0), HORIZONTAL, id+1, backTrack, added, dirs);
        }
        else
        {
            if (d == HORIZONTAL)
            {
                d = VERTICAL;
                return auxPlaceShips(b, shipsLeft, p, d, id, backTrack, added, dirs);
            }
            else
            {
                return auxPlaceShips(b, shipsLeft, Point(p.r, p.c+1), HORIZONTAL, id, backTrack, added, dirs);
            }
        }
    }
    else // backtracking
    {
        // Get row and column of last added ship
        int r = added.back().r;
        int c = added.back().c;
        // Get direction of last added ship
        Direction dir = dirs.back();
        // Remove ship from added
        added.pop_back();
        // Remove direciton from added
        dirs.pop_back();
        // Unplace ship
        valid = b.unplaceShip(Point(r, c), id-1, dir);
        // No longer backtracking
        backTrack = false;
        if (dir == HORIZONTAL)
            return auxPlaceShips(b, shipsLeft + 1, Point(r, c), VERTICAL, id - 1, backTrack, added, dirs);
        else
            return auxPlaceShips(b, shipsLeft + 1, Point(r, c + 1), HORIZONTAL, id - 1, backTrack, added, dirs);
    }
}

Point MediocrePlayer::recommendAttack()
{
    // m_points should not be empty
    if (m_points.empty())
        cerr << "Error MediocrePlayer::recommendAttack() -- someone should have one" << endl;
    // Randomly select point on board to shoot
    if (m_state == 1)
    {
        // Randomly select point from points
        int i = randInt(m_points.size());
        Point p(m_points[i].r, m_points[i].c);
        removePoint(p, m_points);
        return p;
    }
    // Select point randomly from calculated points
    else // state 2
    {
        Point p = calculateShot();
        removePoint(p, m_points);
        return p;
    }
}

void MediocrePlayer::recordAttackResult(Point p, bool validShot, bool shotHit, bool shipDestroyed, int shipId)
{
    // If shot hit mark it in players data members
    if (shotHit)
        m_hist[p.r][p.c] = 'X';
    else
        m_hist[p.r][p.c] = 'o';
    
    if (!validShot)
        cerr << "Error MediocrePlayer::recordAttackResult -- computer should not be shooting invalid shots" << endl;
    
    // Switch to state 2 if shot hit but ship was not destroyed
    if (m_state == 1)
    {
        if (shotHit && !shipDestroyed)
        {
            m_state = 2;
            m_lastCellHit = p;
            buildCPoints = true;
        }
    }
    // Switch to state 1 if shot hit and ship was destroyed
    else // state 2
    {
        if (shotHit && shipDestroyed)
            m_state = 1;
    }
}

Point MediocrePlayer::calculateShot()
{
    if (buildCPoints)
        buildCalculatedPoints(m_lastCellHit);
    int i = randInt(m_calculatedPoints.size());
    Point r(m_calculatedPoints[i].r, m_calculatedPoints[i].c);
    removePoint(r, m_calculatedPoints);
    if (m_calculatedPoints.empty())
        m_state = 1;
    return r;
}

void MediocrePlayer::buildCalculatedPoints(Point p)
{
    // Clear old points before adding new ones
    m_calculatedPoints.clear();
    // Check points for validity and add them to vector
    for (int d = 1; d < 5; d++)
    {
        if (p.r-d >= 0 && m_hist[p.r-d][p.c] == '.')
            m_calculatedPoints.push_back(Point(p.r-d, p.c));
        if (p.r+d <= game().rows()-1 && m_hist[p.r+d][p.c] == '.')
            m_calculatedPoints.push_back(Point(p.r+d, p.c));
        if (p.c-d >= 0 && m_hist[p.r][p.c-d] == '.')
            m_calculatedPoints.push_back(Point(p.r, p.c-d));
        if (p.c+d <= game().cols()-1 && m_hist[p.r][p.c+d] == '.')
            m_calculatedPoints.push_back(Point(p.r, p.c+d));
    }
    buildCPoints = false;
}


//*********************************************************************
//  GoodPlayer
//*********************************************************************

class GoodPlayer : public Player
{
public:
    GoodPlayer(string nm, const Game& g);
    ~GoodPlayer() {}
    virtual bool placeShips(Board& b);
    virtual Point recommendAttack();
    virtual void recordAttackResult(Point p, bool validShot, bool shotHit, bool shipDestroyed, int shipId);
    virtual void recordAttackByOpponent(Point p) { }
    
    void addAttackPoints(Point p);
    
private:
    vector<Point> m_points;
    int m_state;
    // Stack storing the points surrounding a hit attack
    stack<Point> m_attackPoints;
    // Stores history of shots -- misses and hits
    vector<vector<char> > m_hist;
};

GoodPlayer::GoodPlayer(string nm, const Game& g)
: Player(nm, g), m_state(1)
{
    m_hist.resize(game().rows());
    for (int r = 0; r < game().rows(); r++)
    {
        m_hist[r].resize(game().cols());
        for (int c = 0; c < game().cols(); c++)
        {
            m_points.push_back(Point(r,c));
            m_hist[r][c] = '.';
        }
    }
}

bool GoodPlayer::placeShips(Board& b)
{
    int id = 0;
    bool valid;
    int shipsLeft = game().nShips();
    while (shipsLeft > 0)
    {
        int i = randInt(m_points.size());
        Point p(m_points[i].r, m_points[i].c);
        valid = b.placeShip(p, id, HORIZONTAL);
        if (!valid)
            valid = b.placeShip(p, id, VERTICAL);
        if (valid)
        {
            removePoint(p, m_points);
            shipsLeft--;
            id++;
        }
    }
    // Once ships are placed clear points and reinitiate them for attacking phase
    m_points.clear();
    for (int r = 0; r < game().rows(); r++)
        for (int c = 0; c < game().cols(); c++)
            m_points.push_back(Point(r,c));
    return true;
}

Point GoodPlayer::recommendAttack()
{
    // Randomly select one of the points left
    if (m_state == 1)
    {
        // Randomly select point from points
        int i = randInt(m_points.size());
        Point p(m_points[i].r, m_points[i].c);
        // Remove the selected point from points remaining
        removePoint(p, m_points);
        return p;
    }
    // Attack the next point on the stack
    else // m_state == 2
    {
        Point attack;
        if (!m_attackPoints.empty())
            attack = m_attackPoints.top();
        // Make sure stack is not empty
        else
            cerr << "Error GoodPlayer::reccomendAttack -- stack should not be empty" << endl;
        m_attackPoints.pop();
        // Remove the selected point from points remaining
        removePoint(attack, m_points);
        return attack;
    }
}

void GoodPlayer::recordAttackResult(Point p, bool validShot, bool shotHit, bool shipDestroyed, int shipId)
{
    // Check if shot was valid
    if (!validShot)
        cerr << "Error GoodPlayer::recordAttackResult -- computer should not be shooting invalid shots" << endl;
    
    // If shot hit mark it and add to the stack
    if (shotHit)
    {
        m_hist[p.r][p.c] = 'X';
        addAttackPoints(p);
    }
    // Mark if shot did not hit
    else
        m_hist[p.r][p.c] = 'o';
    
    // Switch to state 2 if shot hit
    if (m_state == 1)
    {
        if (shotHit) m_state = 2;
    }
    // Switch to state 1 if stack is empty
    else // m_state == 2
    {
        if (m_attackPoints.empty())
            m_state = 1;
    }
}

void GoodPlayer::addAttackPoints(Point p)
{
    // If cell above p is valid add it to the stack
    if (p.r-1 >= 0 && m_hist[p.r-1][p.c] == '.')
    {
        m_hist[p.r-1][p.c] = 'a';
        m_attackPoints.push(Point(p.r-1, p.c));
    }
    // If cell below p is valid add it to the stack
    if (p.r+1 <= game().rows()-1 && m_hist[p.r+1][p.c] == '.')
    {
        m_hist[p.r+1][p.c] = 'a';
        m_attackPoints.push(Point(p.r+1, p.c));
    }
    // If cell to the left of p is valid add it to the stack
    if (p.c-1 >= 0 && m_hist[p.r][p.c-1] == '.')
    {
        m_hist[p.r][p.c-1] = 'a';
        m_attackPoints.push(Point(p.r, p.c-1));
    }
    // If cell to the right of p is valid add it to the stack
    if (p.c+1 <= game().cols()-1 && m_hist[p.r][p.c+1] == '.')
    {
        m_hist[p.r][p.c+1] = 'a';
        m_attackPoints.push(Point(p.r, p.c+1));
    }
}

//*********************************************************************
//  createPlayer
//*********************************************************************

Player* createPlayer(string type, string nm, const Game& g)
{
    static string types[] = {
        "human", "awful", "mediocre", "good"
    };
    
    int pos;
    for (pos = 0; pos != sizeof(types)/sizeof(types[0])  &&
         type != types[pos]; pos++)
        ;
    switch (pos)
    {
        case 0:  return new HumanPlayer(nm, g);
        case 1:  return new AwfulPlayer(nm, g);
        case 2:  return new MediocrePlayer(nm, g);
        case 3:  return new GoodPlayer(nm, g);
        default: return nullptr;
    }
}
