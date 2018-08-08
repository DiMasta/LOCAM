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

#define OUTPUT_GAME_DATA
//#define REDIRECT_CIN_FROM_FILE
//#define REDIRECT_COUT_TO_FILE
//#define DEBUG_ONE_TURN
//#define DEBUG_BATTLE

static const string INPUT_FILE_NAME = "input.txt";
static const string OUTPUT_FILE_NAME = "output.txt";

static const int INVALID_ID = -1;
static const int INVALID_NODE_DEPTH = -1;
static const int TREE_ROOT_NODE_DEPTH = 1;
static const int ZERO_CHAR = '0';
static const int DIRECTIONS_COUNT = 8;
static const int BYTE_SIZE = 8;
static const int OPPONENT_ATTCK = -1;
static const int DRAFT_TURNS = 30;
static const int MAX_GAME_CARDS = 60;
static const int MAX_BOARD_CREATURES = 6;
static const int DRAFT_CARDS_COUNT = 3;
static const int STARTING_DECK_CARDS = 30;
static const int ALL_GAME_CARDS = 160;

static const string EMPTY_STRING = "";
static const string SUMMON = "SUMMON";
static const string ATTACK = "ATTACK";
static const string PICK = "PICK";
static const string PASS = "PASS";
static const string SPACE = " ";
static const string END_EXPRESSION = "; ";

static const char GUARD = 'G';
static const char WARD = 'W';
static const char LETHAL = 'L';

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

static const float CARDS_EVALUATIONS[ALL_GAME_CARDS] = {
	2.36905f,
	2.38474f,
	2.28950f,
	2.27110f,
	2.30357f,
	2.27760f,
	2.37284f,
	2.25162f,
	2.28517f,
	2.31548f,
	2.33712f,
	2.25920f,
	2.59091f,
	2.49405f,
	2.31872f,
	2.32522f,
	2.31872f,
	2.48755f,
	2.35227f,
	2.38474f,
	2.37825f,
	2.36634f,
	2.49080f,
	2.33929f,
	2.31548f,
	2.44426f,
	2.45617f,
	2.63474f,
	2.66071f,
	2.59903f,
	2.31548f,
	2.69426f,
	2.72781f,
	2.66396f,
	2.75379f,
	3.10660f,
	2.81439f,
	2.43019f,
	2.41071f,
	2.50162f,
	2.45617f,
	2.34903f,
	2.39015f,
	2.50487f,
	2.25325f,
	2.37392f,
	2.43777f,
	2.33929f,
	2.46807f,
	2.36093f,
	2.41396f,
	2.29708f,
	2.25595f,
	2.28950f,
	2.36634f,
	2.26677f,
	2.24080f,
	2.43561f,
	2.66558f,
	2.20509f,
	2.55790f,
	2.87500f,
	2.48755f,
	2.42262f,
	2.37284f,
	2.29167f,
	2.72348f,
	2.53301f,
	2.52327f,
	2.53734f,
	2.44426f,
	2.46591f,
	2.93994f,
	2.59470f,
	2.54491f,
	2.55682f,
	2.54058f,
	3.05682f,
	2.57413f,
	3.24080f,
	2.42370f,
	2.64015f,
	2.42262f,
	2.58929f,
	2.33496f,
	2.35444f,
	2.50920f,
	2.35660f,
	2.47024f,
	2.22348f,
	2.30141f,
	2.51786f,
	2.41071f,
	2.39232f,
	2.41829f,
	2.44426f,
	2.40639f,
	2.38041f,
	2.42587f,
	2.39989f,
	2.36851f,
	2.48972f,
	2.45942f,
	2.43994f,
	2.44751f,
	2.47348f,
	2.61472f,
	2.30465f,
	2.35227f,
	2.29816f,
	2.50703f,
	2.40963f,
	2.63041f,
	2.54058f,
	2.39015f,
	3.07413f,
	2.33929f,
	2.27543f,
	2.21807f,
	2.21050f,
	2.60877f,
	2.34686f,
	2.25812f,
	2.24405f,
	2.14232f,
	2.16829f,
	2.16180f,
	2.22781f,
	2.17587f,
	2.57846f,
	2.13690f,
	2.23972f,
	2.17478f,
	2.53950f,
	2.22348f,
	2.25595f,
	2.13907f,
	2.63907f,
	2.13907f,
	2.13907f,
	2.02219f,
	3.13907f,
	2.30574f,
	1.96483f,
	1.65530f,
	1.90530f,
	2.42695f,
	2.88149f,
	3.38907f,
	1.83604f,
	2.26786f,
	1.73755f,
	2.59740f,
	2.80574f,
	1.91937f,
	2.76407f,
	2.46861f,
	1.70725f,
	2.04437f,
	2.55574f
};

map<int, bool> wardShiledsDown;

//-------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------

class Card {
public:
	Card();

	Card(
		int cardNumber,
		int id,
		int cost,
		CardType cardType,
		CardLocation location,
		int att,
		int def,
		const string& abilities,
		int myHealthChange,
		int opponentHealthChange,
		int cardDraw,
		float evaluation
	);

	virtual ~Card();

	bool operator<(const Card& card) const {
		return (cost < card.cost);
	}

	int getCardNumber() const { return cardNumber; }
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
	float getEvaluation() const { return evaluation; };

	void setCardNUmber(int cardNumber) {
		this->cardNumber = cardNumber;
	}

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

	void setEvaluation(float evaluation) {
		this->evaluation = evaluation;
	}

	virtual void play(string& turnCommands) = 0;

private:
	int cardNumber;
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
	float evaluation;
};

//*************************************************************************************************************
//*************************************************************************************************************

Card::Card() {

}

//*************************************************************************************************************
//*************************************************************************************************************

Card::Card(
	int cardNumber,
	int id,
	int cost,
	CardType type,
	CardLocation location,
	int att,
	int def,
	const string& abilities,
	int myHealthChange,
	int opponentHealthChange,
	int cardDraw,
	float evaluation
) :
	cardNumber(cardNumber),
	id(id),
	cost(cost),
	type(type),
	location(location),
	att(att),
	def(def),
	abilities(abilities),
	myHealthChange(myHealthChange),
	opponentHealthChange(opponentHealthChange),
	cardDraw(cardDraw),
	evaluation(evaluation)
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
		int cardNumber,
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
		float evaluation,
		bool guard,
		bool ward,
		bool lethal
	);

	~Creature();

	bool getGuard() const { return guard; }
	bool getWard() const { return ward; }
	bool getLethal() const { return lethal; }

	void setGuard(bool guard) {
		this->guard = guard;
	}

	void setWard(bool ward) {
		this->ward = ward;
	}

	void setLethal(bool lethal) {
		this->lethal = lethal;
	}

	void play(string& turnCommands) override;

	void attackDirectly(string& turnCommands);
	void attackCreature(Creature* oppCreture, string& turnCommands);

private:
	bool guard;
	bool ward;
	bool lethal;
};

//*************************************************************************************************************
//*************************************************************************************************************

Creature::Creature() : Card(){

}

//*************************************************************************************************************
//*************************************************************************************************************

Creature::Creature(
	int cardNumber,
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
	float evaluation,
	bool guard,
	bool ward,
	bool lethal
) :
	Card(
		cardNumber,
		id,
		cost,
		type,
		location,
		attack,
		defense,
		abilities,
		myHealthChange,
		opponentHealthChange,
		cardDraw,
		evaluation
	),
	guard(guard),
	ward(ward),
	lethal(lethal)
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
	if (oppCreture->getWard()) {
		oppCreture->setWard(false);
		wardShiledsDown[oppCreture->getId()] = true;
	}
	else if (lethal) {
		oppCreture->setDef(0);
	}
	else {
		oppCreture->setDef(oppCreture->getDef() - getAtt());
	}
	
	if (ward && oppCreture->getAtt() > 0) {
		ward = false;
		wardShiledsDown[getId()] = true;
	}
	else if (oppCreture->getLethal()) {
		setDef(0);
	}
	else {
		setDef(getDef() - oppCreture->getAtt());
	}

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
		int cardNumber,
		int id,
		int cost,
		CardType type,
		CardLocation location,
		int attackModifier,
		int defenseModifier,
		const string& abilities,
		int myHealthChange,
		int opponentHealthChange,
		int cardDraw,
		float evaluation
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
	int cardNumber,
	int id,
	int cost,
	CardType type,
	CardLocation location,
	int attackModifier,
	int defenseModifier,
	const string& abilities,
	int myHealthChange,
	int opponentHealthChange,
	int cardDraw,
	float evaluation
) :
	Card(
		cardNumber,
		id,
		cost,
		type,
		location,
		attackModifier,
		defenseModifier,
		abilities,
		myHealthChange,
		opponentHealthChange,
		cardDraw,
		evaluation
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
	void playCreatures(int playerMana, string& turnCommands);
	void playGuards(int playerMana, string& turnCommands);

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

void Hand::playGuards(int playerMana, string& turnCommands) {
	int manaToUse = playerMana;

	for (size_t cardIdx = 0; cardIdx < cards.size(); ++cardIdx) {
		Card* card = cards[cardIdx];
		int cardManaCost = card->getCost();

		if (CardType::CREATURE == card->getType() && manaToUse >= cardManaCost) {
			Creature* creature = dynamic_cast<Creature*>(card);

			if (creature->getGuard()) {
				card->play(turnCommands);
				manaToUse -= cardManaCost;
			}
		}
	}
}

void Hand::playCreatures(int playerMana, string& turnCommands) {
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
		hand.playGuards(mana, turnCommands);
		hand.playCreatures(mana, turnCommands);
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
	float cardEvaluation = 0;

	for (size_t cardIdx = 0; cardIdx < cardsToChooseFrom.size(); ++cardIdx) {
		const Card* card = cardsToChooseFrom[cardIdx];
		const CardType type = card->getType();
		const float evaluation = card->getEvaluation();

		if (CardType::CREATURE == type) {
			if (evaluation > cardEvaluation) {
				pickedCard = cardIdx;
				cardEvaluation = evaluation;
			}
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
		int cardDraw,
		float evaluation
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
			cardDraw,
			CARDS_EVALUATIONS[cardNumber - 1]
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
	int cardDraw,
	float evaluation
) {
	Card* card = nullptr;

	switch (CardType(type)) {
		case CardType::CREATURE: {
			bool guard = abilities.find(GUARD) != string::npos;
			bool ward = (abilities.find(WARD) != string::npos) && !wardShiledsDown[instanceId];
			bool lethal = abilities.find(LETHAL) != string::npos;

			card = new Creature(
				cardNumber,
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
				evaluation,
				guard,
				ward,
				lethal
			);

			break;
		}
		case CardType::RED_ITEM:
		case CardType::GREEN_ITEM:
		case CardType::BLUE_ITEM: {
			card = new Item(
				cardNumber,
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
				evaluation
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

// shufflePlayer0Seed = -781104924512814143
// seed = -388325619875001980
// draftChoicesSeed = 7503261107366343350
// shufflePlayer1Seed = -4630844502291892258
