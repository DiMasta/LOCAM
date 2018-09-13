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
const int DRAFT_CARDS_COUNT = 3;
const int STARTING_DECK_CARDS = 30;

const string EMPTY_STRING = "";
const string SUMMON = "SUMMON";
const string ATTACK = "ATTACK";
const string PICK = "PICK";
const string PASS = "PASS";
const string SPACE = " ";
const string END_EXPRESSION = "; ";

const char GUARD = 'G';

enum class GamePhase : int {
	INVALID = -1,
	DRAFT,
	BATTLE,
};

enum class CardLocation : int {
	OPPONENT_BOARD = -1,
	PLAYER_HAND = 0,
	PLAYER_BOARD = 1,
};

enum class CardType : int {
	INVALID = -1,
	CREATURE = 0,
	GREEN_ITEM = 1,
	RED_ITEM = 2,
	BLUE_ITEM = 3,
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
		int cost,
		CardType cardType,
		CardLocation location,
		int att,
		int def,
		const string& abilities,
		int myHealthChange,
		int opponentHealthChange,
		int cardDraw
	);

	virtual ~Card();

	bool operator<(const Card& card) const {
		return (cost < card.cost);
	}

	int getId() const { return id; }
	int getCost() const { return cost; }
	CardType getType() const { return type; }
	CardLocation getLocation() const { return location; }
	int getAtt() const { return att; }
	int getDef() const { return def; }
	string getAbilities() const { return abilities; };
	int getMyHealthChange() const { return myHealthChange; }
	int getOpponentHealthChange() const { return opponentHealthChange; }
	int getCardDraw() const { return cardDraw; }

	void setId(int id) {
		this->id = id;
	}

	void setCost(int cost) {
		this->cost = cost;
	}

	void settype(CardType type) {
		this->type = type;
	}

	void setLocation(CardLocation location) {
		this->location = location;
	}

	void setAtt(int att) {
		this->att = att;
	}

	void setDef(int def) {
		this->def = def;
	}

	void setAbilities(string abilities) {
		this->abilities = abilities;
	}

	void setMyHealthChange(int myHealthChange) {
		this->myHealthChange = myHealthChange;
	}

	void setOpponentHealthChange(int opponentHealthChange) {
		this->opponentHealthChange = opponentHealthChange;
	}

	void setCardDraw(int cardDraw) {
		this->cardDraw = cardDraw;
	}

	virtual void play(string& turnCommands) = 0;

private:
	int id;
	int cost;
	CardType type;
	CardLocation location;

	// In case of creature this is the creature's attack points
	// In case of item, this the item's attack modifier
	int att;

	// In case of creature, this is the creature's defense points
	// In case of item, this the item's defense modifier
	int def;

	string abilities;
	int myHealthChange;
	int opponentHealthChange;
	int cardDraw;
};

//*************************************************************************************************************
//*************************************************************************************************************

Card::Card() {

}

//*************************************************************************************************************
//*************************************************************************************************************

Card::Card(
	int id,
	int cost,
	CardType type,
	CardLocation location,
	int att,
	int def,
	const string& abilities,
	int myHealthChange,
	int opponentHealthChange,
	int cardDraw
) :
	id(id),
	cost(cost),
	type(type),
	location(location),
	att(att),
	def(def),
	abilities(abilities),
	myHealthChange(myHealthChange),
	opponentHealthChange(opponentHealthChange),
	cardDraw(cardDraw)
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
		int cost,
		CardType type,
		CardLocation location,
		int attack,
		int defense,
		const string& abilities,
		int myHealthChange,
		int opponentHealthChange,
		int cardDraw,
		bool guard
	);

	~Creature();

	bool getGuard() const { return guard; }

	void setGuard(bool guard) {
		this->guard = guard;
	}

	void play(string& turnCommands) override;

	void attackDirectly(string& turnCommands);
	void attackCreature(Creature* oppCreture, string& turnCommands);

private:
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
	int cost,
	CardType type,
	CardLocation location,
	int attack,
	int defense,
	const string& abilities,
	int myHealthChange,
	int opponentHealthChange,
	int cardDraw,
	bool guard
) :
	Card(
		id,
		cost,
		type,
		location,
		attack,
		defense,
		abilities,
		myHealthChange,
		opponentHealthChange,
		cardDraw
	),
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
	oppCreture->setDef(oppCreture->getDef() - getAtt());
	setDef(getDef() - oppCreture->getAtt());

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

class Item : public Card {
public:
	Item();
	~Item();

	Item(
		int id,
		int cost,
		CardType type,
		CardLocation location,
		int attackModifier,
		int defenseModifier,
		const string& abilities,
		int myHealthChange,
		int opponentHealthChange,
		int cardDraw
	);

	void play(string& turnCommands);

private:
};

//*************************************************************************************************************
//*************************************************************************************************************

Item::Item() : Card() {

}

//*************************************************************************************************************
//*************************************************************************************************************

Item::~Item() {

}

//*************************************************************************************************************
//*************************************************************************************************************

Item::Item(
	int id,
	int cost,
	CardType type,
	CardLocation location,
	int attackModifier,
	int defenseModifier,
	const string& abilities,
	int myHealthChange,
	int opponentHealthChange,
	int cardDraw
) :
	Card(
		id,
		cost,
		type,
		location,
		attackModifier,
		defenseModifier,
		abilities,
		myHealthChange,
		opponentHealthChange,
		cardDraw
	)
{

}

//*************************************************************************************************************
//*************************************************************************************************************

void Item::play(string& turnCommands) {
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

		if (creature->getAtt() <= 0) { 
			continue;
		}

		Creature* targetOppCreature = nullptr;

		for (int oppCardIdx = 0; oppCardIdx < opponentBoard->getCreturesCount(); ++oppCardIdx) {
			Card* oppCard = opponentBoard->cards[oppCardIdx];
			Creature* oppCreature = dynamic_cast<Creature*>(oppCard);

			if (!oppCreature) {
				continue;
			}

			if (oppCreature->getDef() > 0) {
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

class Draft {
public:
	Draft();
	~Draft();

	void addCardForChoosing(Card* card);
	void addCardInChosen(int pickedCard);
	void outputTheChoice(int pickedCard);
	void pick();
	void clear();

	// Two separate functions needed, because the two arrays will be deleted in different times by the Game
	void clearCardsToChooseFrom();
	void clearChosenCards();

private:
	Cards cardsToChooseFrom; // Deleted each draft turn
	Cards chosenCards; // Deleted at the end of the draft
};

//*************************************************************************************************************
//*************************************************************************************************************

Draft::Draft() {

}

//*************************************************************************************************************
//*************************************************************************************************************

Draft::~Draft() {
	clear();
}

//*************************************************************************************************************
//*************************************************************************************************************

void Draft::addCardForChoosing(Card* card) {
	cardsToChooseFrom.push_back(card);
}

//*************************************************************************************************************
//*************************************************************************************************************

void Draft::addCardInChosen(int pickedCard) {
	//chosenCards.push_back(cardsToChooseFrom[pickedCard]);
}

//*************************************************************************************************************
//*************************************************************************************************************

void Draft::outputTheChoice(int pickedCard) {
	cout << PICK + SPACE << pickedCard << endl;
}

//*************************************************************************************************************
//*************************************************************************************************************

void Draft::pick() {
	int pickedCard = 0;

	for (size_t cardIdx = 0; cardIdx < cardsToChooseFrom.size(); ++cardIdx) {
		Card* card = cardsToChooseFrom[cardIdx];
		CardType type = card->getType();

		if (CardType::CREATURE == type) {
			pickedCard = cardIdx;
			break;
		}
	}

	addCardInChosen(pickedCard);
	outputTheChoice(pickedCard);
}

//*************************************************************************************************************
//*************************************************************************************************************

void Draft::clear() {
	clearCardsToChooseFrom();
	//clearChosenCards();
}

//*************************************************************************************************************
//*************************************************************************************************************

void Draft::clearCardsToChooseFrom() {
	for (size_t cardIdx = 0; cardIdx < cardsToChooseFrom.size(); ++cardIdx) {
		if (cardsToChooseFrom[cardIdx]) {
			delete cardsToChooseFrom[cardIdx];
			cardsToChooseFrom[cardIdx] = nullptr;
		}
	}

	cardsToChooseFrom.clear();
}

//*************************************************************************************************************
//*************************************************************************************************************

void Draft::clearChosenCards() {
	for (size_t cardIdx = 0; cardIdx < chosenCards.size(); ++cardIdx) {
		if (chosenCards[cardIdx]) {
			delete chosenCards[cardIdx];
			chosenCards[cardIdx] = nullptr;
		}
	}

	chosenCards.clear();
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
	void addCard(Card* card);
	void addDraftCard(Card* card);
	void addBattleCard(Card* card);
	void makeDraftTurn();

	Card* createCard(
		int cardNumber,
		int instanceId,
		CardLocation location,
		CardType type,
		int cost,
		int attack,
		int defense,
		const string& abilities,
		int myHealthChange,
		int opponentHealthChange,
		int cardDraw
	);

private:
	int turnsCount;

	Draft draft;
	GamePhase gamePhase;
	Player player;
	Player opponent;
};

//*************************************************************************************************************
//*************************************************************************************************************

Game::Game() :
	turnsCount(0),
	draft(),
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
#endif
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
		int playerDraw;
		cin >> playerHealth >> playerMana >> playerDeck >> playerRune >> playerDraw; cin.ignore();

#ifdef OUTPUT_GAME_DATA
		cerr << playerHealth << " " << playerMana << " " << playerDeck << " " << playerRune << " " << playerDraw << endl;
#endif

		if (0 == i) {
			player.setMana(playerMana);
		}
	}

	int opponentHand;
	int opponentActions;
	cin >> opponentHand >> opponentActions; cin.ignore();
#ifdef OUTPUT_GAME_DATA
	cerr << opponentHand << " " << opponentActions << endl;
#endif
	for (int i = 0; i < opponentActions; i++) {
		string cardNumberAndAction;
		getline(cin, cardNumberAndAction);

#ifdef OUTPUT_GAME_DATA
		cerr << cardNumberAndAction << endl;
#endif
	}

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
		Card* card = createCard(
			cardNumber,
			instanceId,
			CardLocation(location),
			CardType(cardType),
			cost,
			attack,
			defense,
			abilities,
			myHealthChange,
			opponentHealthChange,
			cardDraw
		);

		addCard(card);
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
		makeDraftTurn();
	}
	else {
		player.makeBattleTurn(opponent.getBoard());
	}
}

//*************************************************************************************************************
//*************************************************************************************************************

void Game::turnEnd() {
	++turnsCount;

	if (GamePhase::BATTLE == gamePhase) {
		player.reset();
		opponent.reset();
	}
	else {
		draft.clearCardsToChooseFrom();
	}

	if (turnsCount >= DRAFT_TURNS) {
		switchToBattlePhase();
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
	//draft.clearChosenCards();
	gamePhase = GamePhase::BATTLE;
}

//*************************************************************************************************************
//*************************************************************************************************************

void Game::addCard(Card* card) {
	if (GamePhase::DRAFT == gamePhase) {
		addDraftCard(card);
	}
	else if (GamePhase::BATTLE == gamePhase) {
		addBattleCard(card);
	}
}

//*************************************************************************************************************
//*************************************************************************************************************

void Game::addDraftCard(Card* card) {
	draft.addCardForChoosing(card);
}

//*************************************************************************************************************
//*************************************************************************************************************

void Game::addBattleCard(Card* card) {
	CardLocation location = card->getLocation();

	switch (location) {
		case CardLocation::PLAYER_HAND: {
			player.addCardToHand(card);
			break;
		}
		case CardLocation::PLAYER_BOARD: {
			player.addCardToBoard(card);
			break;
		}
		case CardLocation::OPPONENT_BOARD: {
			opponent.addCardToBoard(card);
			break;
		}
		default: {
			break;
		}
	}
}

//*************************************************************************************************************
//*************************************************************************************************************

void Game::makeDraftTurn() {
	draft.pick();
}

//*************************************************************************************************************
//*************************************************************************************************************

Card* Game::createCard(
	int cardNumber,
	int instanceId,
	CardLocation location,
	CardType type,
	int cost,
	int att,
	int def,
	const string& abilities,
	int myHealthChange,
	int opponentHealthChange,
	int cardDraw
) {
	Card* card = nullptr;

	switch (CardType(type)) {
		case CardType::CREATURE: {
			bool guard = abilities.find(GUARD) != string::npos;

			card = new Creature(
				instanceId,
				cost,
				type,
				location,
				att,
				def,
				abilities,
				myHealthChange,
				opponentHealthChange,
				cardDraw,
				guard
			);

			break;
		}
		case CardType::RED_ITEM:
		case CardType::GREEN_ITEM:
		case CardType::BLUE_ITEM: {
			card = new Item(
				instanceId,
				cost,
				type,
				location,
				att,
				def,
				abilities,
				myHealthChange,
				opponentHealthChange,
				cardDraw
			);

			break;
		}
		default: {
		}
	}

	return card;
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

//shufflePlayer0Seed = -4112529282944933822
//seed = -6096490858822098900
//draftChoicesSeed = 9115280734189523327
//shufflePlayer1Seed = -8932282316776585208
