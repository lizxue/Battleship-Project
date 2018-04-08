# Battleship-Project
Project for CS32 Spring 2017
This is a version of the classic Battleship game.
Battleship is a simple game where each player attempts to sink their opponent’s fleet of
ships before the opponent sinks theirs.
Here’s how you play it:
1. Each player has a number of different ships. In Standard Battleship, each player
has five ships:
a. An aircraft carrier which is 5 segments long: AAAAA
b. A battleship which is 4 segments long: BBBB
c. A destroyer which is 3 segments long: DDD
d. A submarine which is 3 segments long: SSS
e. A patrol boat which is 2 segments long: PP
2. Each player has their own up-to-10x10 board (with rows and columns numbered 0
through 9), in which they place their ships. Each ship may be placed horizontally
or vertically, but not diagonally. No ship may overlap another on the board. Each
player sees only their own board; they cannot see where their opponent has placed
their ships. A player's board in a 10x10 game might look like this:
0123456789
0 ....AAAAA.
1 .......P..
2 .......P..
3 ..........
4 ...D......
5 ...D......
6 ..SD......
7 ..S.......
8 ..S...BBBB
9 ..........
3. After both players have placed their ships, game play begins. The first player
picks a row and column coordinate to attack on the opponent’s board and
announces it to the opponent. The opponent then tells the player whether or not
the player hit a ship, and if so, whether or not the entire ship has been sunk. A
ship is sunk when all of its segments have been hit. If a player has sunk the entire
ship, the opponent must indicate which ship was sunk, e.g. “You sank my
battleship!”
4. After the first player attacks, the second player is given an opportunity to attack
the first player. Attacks alternate between players until one player has sunk all of
the other player’s ships.
5. The first player to sink all of their opponent’s ships wins the game.
