#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <list>
#include <stack>
#include <set>
#include <queue>
#include <algorithm>
#include <ctime>
#include <deque>
#include <math.h>
#include <climits>
#include <cstring>
#include <fstream>
#include <iterator>

using namespace std;

//#define OUTPUT_GAME_DATA
//#define REDIRECT_CIN_FROM_FILE
//#define REDIRECT_COUT_TO_FILE
//#define DEBUG_ONE_TURN
//#define DEBUG_BATTLE

const string INPUT_FILE_NAME = "input.txt";
const string OUTPUT_FILE_NAME = "output.txt";

const int INVALID_ID = -1;
const int INVALID_NODE_DEPTH = -1;
const int TREE_ROOT_NODE_DEPTH = 1;
const int ZERO_CHAR = '0';
const int DIRECTIONS_COUNT = 8;
const int BYTE_SIZE = 8;
const int OPPONENT_ATTCK = -1;
const int DRAFT_TURNS = 30;
const int MAX_GAME_CARDS = 60;
const int MAX_BOARD_CREATURES = 6;

const string EMPTY_STRING = "";
const string SUMMON = "SUMMON";
const string ATTACK = "ATTACK";
const string SPACE = " ";
const string END_EXPRESSION = "; ";

const char GUARD = 'G';

enum class GamePhase : int {
	INVALID = -1,
	DRAFT,
	BATTLE,
};

enum class Location : int {
	OPPONENT_BOARD = -1,
	PLAYER_HAND = 0,
	PLAYER_BOARD = 1,
};

//-------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------

class Card {
public:
	Card();

	Card(
		int id,
		int cardType,
		int cost
	);

	virtual ~Card();

	bool operator<(const Card& card) const {
		return (cost < card.cost);
	}

	int getId() { return id; }
	int getCardType() { return cardType; }
	int getCost() { return cost; }

	virtual void play(string& turnCommands) = 0;

private:
	int id;
	int cardType;
	int cost;
};

//*************************************************************************************************************
//*************************************************************************************************************

Card::Card() {

}

//*************************************************************************************************************
//*************************************************************************************************************

Card::Card(
	int id,
	int cardType,
	int cost
) :
	id(id),
	cardType(cardType),
	cost(cost)
{

}
//*************************************************************************************************************
//*************************************************************************************************************

Card::~Card() {

}

//-------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------

class Creature : public Card {
public:
	Creature();

	Creature(
		int id,
		int cardType,
		int cost,
		int attack,
		int defense,
		bool guard
	);

	~Creature();

	int getAttack() const { return attack; }
	int getDefense() const { return defense; }
	bool getGuard() const { return guard; }

	void setAttack(int attack) {
		this->attack = attack;
	}

	void setDefense(int defense) {
		this->defense = defense;
	}

	void setGuard(bool guard) {
		this->guard = guard;
	}

	void play(string& turnCommands) override;

	void attackDirectly(string& turnCommands);
	void attackCreature(Creature* oppCreture, string& turnCommands);

private:
	int attack;
	int defense;

	bool guard;
};

//*************************************************************************************************************
//*************************************************************************************************************

Creature::Creature() : Card(){

}

//*************************************************************************************************************
//*************************************************************************************************************

Creature::Creature(
	int id,
	int cardType,
	int cost,
	int attack,
	int defense,
	bool guard
) :
	Card(id, cardType, cost),
	attack(attack),
	defense(defense),
	guard(guard)
{

}

//*************************************************************************************************************
//*************************************************************************************************************

Creature::~Creature() {

}

//*************************************************************************************************************
//*************************************************************************************************************

void Creature::play(string& turnCommands) {
	turnCommands += SUMMON + SPACE + to_string(getId()) + END_EXPRESSION;
}

//*************************************************************************************************************
//*************************************************************************************************************

void Creature::attackDirectly(string& turnCommands) {
	turnCommands +=
		ATTACK +
		SPACE +
		to_string(getId()) +
		SPACE +
		to_string(OPPONENT_ATTCK) +
		END_EXPRESSION;
}

//*************************************************************************************************************
//*************************************************************************************************************

void Creature::attackCreature(Creature* oppCreture, string& turnCommands) {
	oppCreture->setDefense(oppCreture->getDefense() - attack);
	defense -= oppCreture->getAttack();

	turnCommands +=
		ATTACK +
		SPACE +
		to_string(getId()) +
		SPACE +
		to_string(oppCreture->getId()) +
		END_EXPRESSION;
}

//-------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------

typedef vector<Card*> Cards;

class Hand {
public:
	Hand();
	~Hand();

	static bool comparePtrToCards(Card* a, Card* b);

	void addCard(Card* card);
	void clearCards();
	void sortCards();
	void getHighestCostCreatures(int playerMana, string& turnCommands);

private:
	Cards cards;
};

//*************************************************************************************************************
//*************************************************************************************************************

Hand::Hand() {

}

//*************************************************************************************************************
//*************************************************************************************************************

Hand::~Hand() {
	clearCards();
}

//*************************************************************************************************************
//*************************************************************************************************************

bool Hand::comparePtrToCards(Card* cardA, Card* cardB) {
	return (*cardA < *cardB);
}

//*************************************************************************************************************
//*************************************************************************************************************

void Hand::addCard(Card* card) {
	cards.push_back(card);
}

//*************************************************************************************************************
//*************************************************************************************************************

void Hand::clearCards() {
	for (size_t cardIdx = 0; cardIdx < cards.size(); ++cardIdx) {
		if (cards[cardIdx]) {
			delete cards[cardIdx];
			cards[cardIdx] = nullptr;
		}
	}

	cards.clear();
}

//*************************************************************************************************************
//*************************************************************************************************************

void Hand::sortCards() {
	sort(cards.rbegin(), cards.rend(), &comparePtrToCards);
}

//*************************************************************************************************************
//*************************************************************************************************************

void Hand::getHighestCostCreatures(int playerMana, string& turnCommands) {
	int manaToUse = playerMana;

	for (size_t cardIdx = 0; cardIdx < cards.size(); ++cardIdx) {
		Card* card = cards[cardIdx];
		int cardManaCost = card->getCost();

		if (manaToUse >= cardManaCost) {
			card->play(turnCommands);
			manaToUse -= cardManaCost;
		}
	}
}

//-------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------

class Board {
public:
	Board();
	~Board();

	void addCard(Card* card);
	void clearCards();
	void directCreaturesAttack(string& turnCommands);
	void attackCreaturesFirst(const Board* opponentBoard, string& turnCommands);

	int getCreturesCount() const;

private:
	Cards cards;
};

//*************************************************************************************************************
//*************************************************************************************************************

Board::Board() {

}

//*************************************************************************************************************
//*************************************************************************************************************

Board::~Board() {
	clearCards();
}

//*************************************************************************************************************
//*************************************************************************************************************

void Board::addCard(Card* card) {
	cards.push_back(card);
}

//*************************************************************************************************************
//*************************************************************************************************************

void Board::clearCards() {
	for (size_t cardIdx = 0; cardIdx < cards.size(); ++cardIdx) {
		if (cards[cardIdx]) {
			delete cards[cardIdx];
			cards[cardIdx] = nullptr;
		}
	}

	cards.clear();
}

//*************************************************************************************************************
//*************************************************************************************************************

void Board::directCreaturesAttack(string& turnCommands) {
	for (size_t cardIdx = 0; cardIdx < cards.size(); ++cardIdx) {
		Card* card = cards[cardIdx];
		Creature* creture = dynamic_cast<Creature*>(card);

		if (creture) {
			creture->attackDirectly(turnCommands);
		}
	}
}

//*************************************************************************************************************
//*************************************************************************************************************

void Board::attackCreaturesFirst(const Board* opponentBoard, string& turnCommands) {
	for (size_t cardIdx = 0; cardIdx < cards.size(); ++cardIdx) {
		Card* card = cards[cardIdx];
		Creature* creature = dynamic_cast<Creature*>(card);

		if (!creature) {
			continue;
		}

		if (creature->getAttack() <= 0) {
			continue;
		}

		Creature* targetOppCreature = nullptr;

		for (int oppCardIdx = 0; oppCardIdx < opponentBoard->getCreturesCount(); ++oppCardIdx) {
			Card* oppCard = opponentBoard->cards[oppCardIdx];
			Creature* oppCreature = dynamic_cast<Creature*>(oppCard);

			if (!oppCreature) {
				continue;
			}

			if (oppCreature->getDefense() > 0) {
				targetOppCreature = oppCreature;

				if (oppCreature->getGuard()) {
					break;
				}
			}
		}

		if (targetOppCreature) {
			creature->attackCreature(targetOppCreature, turnCommands);
		}
		else {
			creature->attackDirectly(turnCommands);
		}
	}
}

//*************************************************************************************************************
//*************************************************************************************************************

int Board::getCreturesCount() const {
	return int(cards.size());
}

//-------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------

class Player {
public:
	Player();
	~Player();

	const Board* getBoard() const { return &board; }
	
	void setMana(int mana) {
		this->mana = mana;
	}

	void addCardToHand(Card* card);
	void addCardToBoard(Card* card);
	void reset();
	void sortHand();
	void makeBattleTurn(const Board* opponentBoard);
	void chooseHighestCostCreatures();
	void outputTurnCommands();
	void attack(const Board* opponentBoard);

private:
	Hand hand;
	Board board;
	int mana;
	string turnCommands;
};

//*************************************************************************************************************
//*************************************************************************************************************

Player::Player() :
	mana(0),
	turnCommands(EMPTY_STRING)
{

}

//*************************************************************************************************************
//*************************************************************************************************************

Player::~Player() {

}

//*************************************************************************************************************
//*************************************************************************************************************

void Player::addCardToHand(Card* card) {
	hand.addCard(card);
}

//*************************************************************************************************************
//*************************************************************************************************************

void Player::addCardToBoard(Card* card) {
	board.addCard(card);
}

//*************************************************************************************************************
//*************************************************************************************************************

void Player::reset() {
	hand.clearCards();
	board.clearCards();
	turnCommands = EMPTY_STRING;
}

//*************************************************************************************************************
//*************************************************************************************************************

void Player::sortHand() {
	hand.sortCards();
}

//*************************************************************************************************************
//*************************************************************************************************************

void Player::makeBattleTurn(const Board* opponentBoard) {
	sortHand();
	attack(opponentBoard);
	chooseHighestCostCreatures();

	outputTurnCommands();
}

//*************************************************************************************************************
//*************************************************************************************************************

void Player::chooseHighestCostCreatures() {
	if (MAX_BOARD_CREATURES > board.getCreturesCount()) {
		hand.getHighestCostCreatures(mana, turnCommands);
	}
}

//*************************************************************************************************************
//*************************************************************************************************************

void Player::outputTurnCommands() {
	cout << turnCommands << endl;
}

//*************************************************************************************************************
//*************************************************************************************************************

void Player::attack(const Board* opponentBoard) {
	if (0 == opponentBoard->getCreturesCount()) {
		board.directCreaturesAttack(turnCommands);
	}
	else {
		board.attackCreaturesFirst(opponentBoard, turnCommands);
	}
}

//-------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------

class GameState {
public:
	GameState();
	~GameState();

	//void addCard(
	//	int instanceId,
	//	int location,
	//	int cardType,
	//	int cost,
	//	int attack,
	//	int defense,
	//	const string& abilities,
	//	int myHealthChange,
	//	int opponentHealthChange,
	//	int cardDraw
	//);
private:
	int cards[MAX_GAME_CARDS];
	char abilities[MAX_GAME_CARDS];
};

//*************************************************************************************************************
//*************************************************************************************************************

GameState::GameState() {
	memset(cards, 0, sizeof(cards));
	memset(abilities, 0, sizeof(abilities));
}

//*************************************************************************************************************
//*************************************************************************************************************

GameState::~GameState() {

}

//-------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------

class Game {
public:
	Game();
	~Game();

	void initGame();
	void gameBegin();
	void gameEnd();
	void gameLoop();
	void getGameInput();
	void getTurnInput();
	void turnBegin();
	void makeTurn();
	void turnEnd();
	void play();

	void debug() const;

	void switchToBattlePhase();

private:
	int turnsCount;

	GamePhase gamePhase;
	Player player;
	Player opponent;
};

//*************************************************************************************************************
//*************************************************************************************************************

Game::Game() :
	turnsCount(0),
	gamePhase(GamePhase::INVALID),
	player(),
	opponent()
{

}

//*************************************************************************************************************
//*************************************************************************************************************

Game::~Game() {
}

//*************************************************************************************************************
//*************************************************************************************************************

void Game::initGame() {
#ifdef DEBUG_BATTLE
	gamePhase = GamePhase::BATTLE;
#else
	gamePhase = GamePhase::DRAFT;
#endif
}

//*************************************************************************************************************
//*************************************************************************************************************

void Game::gameBegin() {
}

//*************************************************************************************************************
//*************************************************************************************************************

void Game::gameEnd() {
}

//*************************************************************************************************************
//*************************************************************************************************************

void Game::gameLoop() {
	while (true) {
		turnBegin();
		getTurnInput();
		makeTurn();
		turnEnd();

#ifdef DEBUG_ONE_TURN
		break;
#endif // DEBUG_ONE_TURN
	}
}

//*************************************************************************************************************
//*************************************************************************************************************

void Game::getGameInput() {
}

//*************************************************************************************************************
//*************************************************************************************************************

void Game::getTurnInput() {
	for (int i = 0; i < 2; i++) {
		int playerHealth;
		int playerMana;
		int playerDeck;
		int playerRune;
		cin >> playerHealth >> playerMana >> playerDeck >> playerRune; cin.ignore();

#ifdef OUTPUT_GAME_DATA
		cerr << playerHealth << " " << playerMana << " " << playerDeck << " " << playerRune << endl;
#endif

		if (0 == i) {
			player.setMana(playerMana);
		}
	}

	int opponentHand;
	cin >> opponentHand; cin.ignore();
	int cardCount;
	cin >> cardCount; cin.ignore();

#ifdef OUTPUT_GAME_DATA
	cerr << opponentHand << endl;
	cerr << cardCount << endl;
#endif

	for (int i = 0; i < cardCount; i++) {
		int cardNumber;
		int instanceId;
		int location;
		int cardType;
		int cost;
		int attack;
		int defense;
		string abilities;
		int myHealthChange;
		int opponentHealthChange;
		int cardDraw;
		cin >> cardNumber >> instanceId >> location >> cardType >> cost >> attack >> defense >> abilities >> myHealthChange >> opponentHealthChange >> cardDraw; cin.ignore();

#ifdef OUTPUT_GAME_DATA
		cerr << cardNumber << " " << instanceId << " " << location << " " << cardType << " " << cost << " " << attack << " " << defense << " " << abilities << " " << myHealthChange << " " << opponentHealthChange << " " << cardDraw << endl;
#endif
		bool guard = abilities.find(GUARD) != string::npos;
		if (GamePhase::BATTLE == gamePhase) {
			Card* card = new Creature(instanceId, cardType, cost, attack, defense, guard);

			if (Location::PLAYER_HAND == Location(location)) {
				player.addCardToHand(card);
			}

			if (Location::PLAYER_BOARD == Location(location)) {
				player.addCardToBoard(card);
			}

			if (Location::OPPONENT_BOARD == Location(location)) {
				opponent.addCardToBoard(card);
			}
		}
	}
}

//*************************************************************************************************************
//*************************************************************************************************************

void Game::turnBegin() {
}

//*************************************************************************************************************
//*************************************************************************************************************

void Game::makeTurn() {
	if (GamePhase::DRAFT == gamePhase) {
		cout << "PASS" << endl;
	}
	else {
		player.makeBattleTurn(opponent.getBoard());
	}
}

//*************************************************************************************************************
//*************************************************************************************************************

void Game::turnEnd() {
	++turnsCount;

	if (turnsCount >= DRAFT_TURNS) {
		switchToBattlePhase();
	}

	if (GamePhase::BATTLE == gamePhase) {
		player.reset();
		opponent.reset();
	}
}

//*************************************************************************************************************
//*************************************************************************************************************

void Game::play() {
	initGame();
	getGameInput();
	gameBegin();
	gameLoop();
	gameEnd();
}

//*************************************************************************************************************
//*************************************************************************************************************

void Game::debug() const {
}

//*************************************************************************************************************
//*************************************************************************************************************

void Game::switchToBattlePhase() {
	gamePhase = GamePhase::BATTLE;
}

//-------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------

#ifdef TESTS
#include "debug.h"
#endif // TESTS

int main(int argc, char** argv) {
#ifdef TESTS
	doctest::Context context;
	int res = context.run();
#else

#ifdef REDIRECT_CIN_FROM_FILE
	ifstream in(INPUT_FILE_NAME);
	streambuf *cinbuf = cin.rdbuf();
	cin.rdbuf(in.rdbuf());
#endif // REDIRECT_CIN_FROM_FILE

#ifdef REDIRECT_COUT_TO_FILE
	ofstream out(OUTPUT_FILE_NAME);
	streambuf *coutbuf = cout.rdbuf();
	cout.rdbuf(out.rdbuf());
#endif // REDIRECT_COUT_TO_FILE

	Game game;
	game.play();
#endif // TESTS

	return 0;
}

// shufflePlayer0Seed = -8945160174642498262
// seed = 2260229151254511100
// draftChoicesSeed = -1769648689152082079
// shufflePlayer1Seed = 1926399767800745180

