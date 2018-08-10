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

static const float INVALID_CARD_VALUE = -1.f;

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

static const float CARDS_VALUES[ALL_GAME_CARDS] = {
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

	int getNumber() const { return number; }
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
	float getValue() const { return evaluation; };

	void setNumber(int number) {
		this->number = number;
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

private:
	int number;
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
	int number,
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
	number(number),
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

struct CardValue {
	int cardNumber;
	float value;

	CardValue();

	CardValue(
		int cardNumber,
		float value
	);
};

//*************************************************************************************************************
//*************************************************************************************************************

CardValue::CardValue() :
	cardNumber(INVALID_ID),
	value(INVALID_CARD_VALUE)
{

}

//*************************************************************************************************************
//*************************************************************************************************************

CardValue::CardValue(
	int cardNumber,
	float value
) :
	cardNumber(cardNumber),
	value(value)
{

}

//-------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------

class Draft {
public:
	Draft();
	~Draft();

	void addCardForChoosing(const CardValue& card);
	void addCardInChosen(int pickedCardNumber);
	void outputTheChoice(int pickedCardId);
	void pick();

	void clearCardsToChooseFrom();

private:
	CardValue cardsToChooseFrom[DRAFT_CARDS_COUNT];
	int cardsToChooseFromCount; // Reset each draft turn

	int chosenCardsNumbers[STARTING_DECK_CARDS];
	int chosenCardsCount;
};

//*************************************************************************************************************
//*************************************************************************************************************

Draft::Draft() :
	cardsToChooseFromCount(0),
	chosenCardsCount(0)
{

}

//*************************************************************************************************************
//*************************************************************************************************************

Draft::~Draft() {
}

//*************************************************************************************************************
//*************************************************************************************************************

void Draft::addCardForChoosing(const CardValue& card) {
	cardsToChooseFrom[cardsToChooseFromCount++] = card;
}

//*************************************************************************************************************
//*************************************************************************************************************

void Draft::addCardInChosen(int pickedCardNumber) {
	chosenCardsNumbers[chosenCardsCount++] = pickedCardNumber;
}

//*************************************************************************************************************
//*************************************************************************************************************

void Draft::outputTheChoice(int pickedCardId) {
	cout << PICK + SPACE << pickedCardId << endl;
}

//*************************************************************************************************************
//*************************************************************************************************************

void Draft::pick() {
	int chosenIdx = 0;
	const CardValue* pickedCard = &cardsToChooseFrom[chosenIdx];

	for (int cardIdx = 1; cardIdx < DRAFT_CARDS_COUNT; ++cardIdx) {
		const CardValue* const card = &cardsToChooseFrom[cardIdx];

		if (card->value > pickedCard->value) {
			pickedCard = card;
			chosenIdx = cardIdx;
		}
	}

	addCardInChosen(pickedCard->cardNumber);
	outputTheChoice(chosenIdx);
}

//*************************************************************************************************************
//*************************************************************************************************************

void Draft::clearCardsToChooseFrom() {
	cardsToChooseFromCount = 0;
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
	void addDraftCard(const CardValue& draftCard);
	void addBattleCard(Card* card);
	void makeDraftTurn();

	Card* createCard(
		int cardNumber,
		int instanceId,
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
	);

private:
	int turnsCount;

	Draft draft;
	GamePhase gamePhase;

	// Hand ([8] cards; make playable card combinations; playCard())
	// Board ([6] [6] creatures; make attacks)
	// Player (health + mana)
	// Opponent (health + mana)
};

//*************************************************************************************************************
//*************************************************************************************************************

Game::Game() :
	turnsCount(0),
	draft(),
	gamePhase(GamePhase::INVALID)
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
			cost,
			CardType(cardType),
			CardLocation(location),
			attack,
			defense,
			abilities,
			myHealthChange,
			opponentHealthChange,
			cardDraw,
			CARDS_VALUES[cardNumber - 1]
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
	}
}

//*************************************************************************************************************
//*************************************************************************************************************

void Game::turnEnd() {
	++turnsCount;

	if (GamePhase::BATTLE == gamePhase) {
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
	gamePhase = GamePhase::BATTLE;
}

//*************************************************************************************************************
//*************************************************************************************************************

void Game::addCard(Card* card) {
	if (GamePhase::DRAFT == gamePhase) {
		CardValue draftCard(card->getNumber(), card->getValue());
		addDraftCard(draftCard);
	}
	else if (GamePhase::BATTLE == gamePhase) {
		addBattleCard(card);
	}
}

//*************************************************************************************************************
//*************************************************************************************************************

void Game::addDraftCard(const CardValue& draftCard) {
	draft.addCardForChoosing(draftCard);
}

//*************************************************************************************************************
//*************************************************************************************************************

void Game::addBattleCard(Card* card) {
	CardLocation location = card->getLocation();

	switch (location) {
		case CardLocation::PLAYER_HAND: {
			break;
		}
		case CardLocation::PLAYER_BOARD: {
			break;
		}
		case CardLocation::OPPONENT_BOARD: {
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
) {
	Card* card = new Card(
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

	switch (CardType(type)) {
		case CardType::CREATURE: {
			break;
		}
		case CardType::RED_ITEM:
		case CardType::GREEN_ITEM:
		case CardType::BLUE_ITEM: {
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
