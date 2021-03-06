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
#define REDIRECT_COUT_TO_FILE
#define REDIRECT_CIN_FROM_FILE
#define DEBUG_ONE_TURN
#define DEBUG_BATTLE

static const string INPUT_FILE_NAME = "input.txt";
static const string OUTPUT_FILE_NAME = "output.txt";

static constexpr int INVALID_ID = -1;
static constexpr int INVALID_NODE_DEPTH = -1;
static constexpr int TREE_ROOT_NODE_DEPTH = 1;
static constexpr int ZERO_CHAR = '0';
static constexpr int DIRECTIONS_COUNT = 8;
static constexpr int BYTE_SIZE = 8;
static constexpr int OPPONENT_ATTCK = -1;
static constexpr int DRAFT_TURNS = 30;
static constexpr int MAX_GAME_CARDS = 60;
static constexpr int MAX_BOARD_CREATURES = 6;
static constexpr int DRAFT_CARDS_COUNT = 3;
static constexpr int STARTING_DECK_CARDS = 30;
static constexpr int ALL_GAME_CARDS_COUNT = 160;
static constexpr int MAX_CARDS_IN_HAND = 8;
static constexpr int DEFAULT_CARD_TEMPLATE = 0;
static constexpr int ABILITIES_COUNT = 6;
static constexpr int ITEM_GIVING_CHARGE = 140;

static constexpr int8_t PLAYER_TARGET = -1;
static constexpr int8_t MAX_ADDED_DEMAGE = 20;

static constexpr float INVALID_CARD_VALUE = -1.f;

static const string EMPTY_STRING = "";
static const string TAB = "\t";
static const string SUMMON = "SUMMON";
static const string ATTACK = "ATTACK";
static const string PICK = "PICK";
static const string PASS = "PASS";
static const string USE = "USE";
static const string SPACE = " ";
static const string END_EXPRESSION = "; ";

static constexpr char GUARD = 'G';
static constexpr char CHARGE = 'C';
static constexpr char WARD = 'W';
static constexpr char LETHAL = 'L';
static constexpr char DASH = '-';

enum class GamePhase : int8_t {
	INVALID = -1,
	DRAFT,
	BATTLE,
};

enum class CardLocation : int8_t {
	INVALID = -2,
	OPPONENT_BOARD = -1,
	PLAYER_HAND = 0,
	PLAYER_BOARD = 1,
};

enum class CardType : int8_t {
	INVALID = -1,
	CREATURE = 0,
	GREEN_ITEM = 1,
	RED_ITEM = 2,
	BLUE_ITEM = 3,
};

enum class Side : int8_t {
	INVALID = -1,
	PLAYER,
	OPPONENT,
};

enum class BoardSum : int8_t {
	INVALID = -1,
	ATT,
	DEF,
};

enum class StateSimulationType : int8_t {
	INVALID = -1,
	SUMMON_CHARGE_CREATURES,
	SUMMON_CREATURES_FOR_CHARGE, /// If the item that grants charge is in the combination
	PLAY_ITEMS,
	PERFORM_ATTACKS,
	FIRST_PERFORM_ATTACKS, /// If the hand combination is not changing the board state, start with attcks
	SUMMON_CREATURES,
	END,
	//EVALUATE,
};

enum class HandCombProperty : int8_t {
	INVALID = -1,
	NUMBER,
	ID,
};

namespace CardMasks {
	// HandCard masks
	static constexpr int NUMBER_OFFSET = 0;
	static constexpr int HAND_CARD_ID_OFFSET = 8;
	static constexpr int HAND_CARD_COMB_OFFSET = 8;

	static constexpr long long NUMBER			= 0b0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'1111'1111;
	static constexpr long long HAND_CARD_ID		= 0b0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0011'1111'0000'0000;

	// Boardcard masks
	static constexpr int ATTACK_OFFSET = 0;
	static constexpr int DEFENSE_OFFSET = 4;
	static constexpr int BOARD_CARD_ID_OFFSET = 8;
	static constexpr int ABILITIES_OFFSET = 14;
	
	static constexpr int ATTACK			= 0b0000'0000'0000'0000'0000'0000'0000'1111;
	static constexpr int DEFENSE		= 0b0000'0000'0000'0000'0000'0000'1111'0000;
	static constexpr int BOARD_CARD_ID	= 0b0000'0000'0000'0000'0011'1111'0000'0000;
	static constexpr int ABILITIES		= 0b0000'0000'0000'1111'1100'0000'0000'0000;
	static constexpr int CAN_ATTACK		= 0b0000'0000'0001'0000'0000'0000'0000'0000;

	// Boardcard abilities masks
	static constexpr int WARD			= 0b0000'0000'0000'0000'0100'0000'0000'0000;
	static constexpr int LETHAL			= 0b0000'0000'0000'0000'1000'0000'0000'0000;
	static constexpr int GUARD			= 0b0000'0000'0000'0001'0000'0000'0000'0000;
	static constexpr int DRAIN			= 0b0000'0000'0000'0010'0000'0000'0000'0000;
	static constexpr int CHARGE			= 0b0000'0000'0000'0100'0000'0000'0000'0000;
	static constexpr int BREAKTHROUGH	= 0b0000'0000'0000'1000'0000'0000'0000'0000;
};

namespace Weights {
	static constexpr int CREATURES_COUNT_DIFF = 300;
	static constexpr int HEALTH_DIFF = 100;
	static constexpr int ATTACKS_DIFF = 200;
	static constexpr int DEFENSES_DIFF = 200;
	static constexpr int HAND_CARDS = 150;
};

struct HandCombination {
	unsigned long long cardsNumbers = 0;
	unsigned long long cardsIds = 0;
	uint8_t playedCards = 0;
	map<uint8_t, vector<uint8_t>> itemsTargets;

	uint8_t extractProperty(uint8_t cardIdx, HandCombProperty propertyType) const {
		unsigned long long propertyValue = 0;

		switch (propertyType) {
			case HandCombProperty::NUMBER: {
				propertyValue = cardsNumbers;
				break;
			}
			case HandCombProperty::ID: {
				propertyValue = cardsIds;
				break;
			}
			default: {
				break;
			}
		}

		// Shift the mask for the current card, apply mask, shift back the result for valid card property
		const int cardOffset = cardIdx * CardMasks::HAND_CARD_COMB_OFFSET;
		return (propertyValue & (CardMasks::NUMBER << cardOffset)) >> cardOffset;
	}

	bool cardPlayed(uint8_t cardIdx) const {
		return playedCards & (1 << cardIdx);
	}

	void markPlayedCard(uint8_t cardIdx) {
		playedCards |= 1 << cardIdx;
	}

#ifdef DEBUG_BATTLE
	void debug() const {
		for (int8_t cardIdx = 0; cardIdx < MAX_CARDS_IN_HAND; ++cardIdx) {
			cout << static_cast<int>(extractProperty(cardIdx, HandCombProperty::ID)) << TAB;
		}
		cout << endl << endl;
	}
#endif
};

typedef vector<HandCombination> HandCombinations;

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

	int getBitsAbilities() const;

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

Card::Card() {

}

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

Card::~Card() {

}

// TODO: this function can be bypassed, make bits in adnvaced
int Card::getBitsAbilities() const {
	int bits = 0;

	int bitMask = 0b100'000;
	for (int abilityIdx = 0; abilityIdx < ABILITIES_COUNT; ++abilityIdx) {
		if (DASH != abilities[abilityIdx]) {
			bits |= bitMask;
		}

		bitMask >>= 1;
	}

	return bits;
}

struct AllCardsHolder {
	Card allGameCards[ALL_GAME_CARDS_COUNT + 1];

	void initCards();
}ALL_CARDS_HOLDER;

void AllCardsHolder::initCards() {
	allGameCards[0] =	Card();
	allGameCards[1] =	Card(1,		INVALID_ID,	1,	CardType::CREATURE,		CardLocation::INVALID,	2,	1,		"------",	1	,0	,0	,2.36905f);
	allGameCards[2] =	Card(2,		INVALID_ID,	1,	CardType::CREATURE,		CardLocation::INVALID,	1,	2,		"------",	0	,-1	,0	,2.38474f);
	allGameCards[3] =	Card(3,		INVALID_ID,	1,	CardType::CREATURE,		CardLocation::INVALID,	2,	2,		"------",	0	,0	,0	,2.28950f);
	allGameCards[4] =	Card(4,		INVALID_ID,	2,	CardType::CREATURE,		CardLocation::INVALID,	1,	5,		"------",	0	,0	,0	,2.27110f);
	allGameCards[5] =	Card(5,		INVALID_ID,	2,	CardType::CREATURE,		CardLocation::INVALID,	4,	1,		"------",	0	,0	,0	,2.30357f);
	allGameCards[6] =	Card(6,		INVALID_ID,	2,	CardType::CREATURE,		CardLocation::INVALID,	3,	2,		"------",	0	,0	,0	,2.27760f);
	allGameCards[7] =	Card(7,		INVALID_ID,	2,	CardType::CREATURE,		CardLocation::INVALID,	2,	2,		"-----W",	0	,0	,0	,2.37284f);
	allGameCards[8] =	Card(8,		INVALID_ID,	2,	CardType::CREATURE,		CardLocation::INVALID,	2,	3,		"------",	0	,0	,0	,2.25162f);
	allGameCards[9] =	Card(9,		INVALID_ID,	3,	CardType::CREATURE,		CardLocation::INVALID,	3,	4,		"------",	0	,0	,0	,2.28517f);
	allGameCards[10] =	Card(10,	INVALID_ID,	3,	CardType::CREATURE,		CardLocation::INVALID,	3,	1,		"--D---",	0	,0	,0	,2.31548f);
	allGameCards[11] =	Card(11,	INVALID_ID,	3,	CardType::CREATURE,		CardLocation::INVALID,	5,	2,		"------",	0	,0	,0	,2.33712f);
	allGameCards[12] =	Card(12,	INVALID_ID,	3,	CardType::CREATURE,		CardLocation::INVALID,	2,	5,		"------",	0	,0	,0	,2.25920f);
	allGameCards[13] =	Card(13,	INVALID_ID,	4,	CardType::CREATURE,		CardLocation::INVALID,	5,	3,		"------",	1	,-1	,0	,2.59091f);
	allGameCards[14] =	Card(14,	INVALID_ID,	4,	CardType::CREATURE,		CardLocation::INVALID,	9,	1,		"------",	0	,0	,0	,2.49405f);
	allGameCards[15] =	Card(15,	INVALID_ID,	4,	CardType::CREATURE,		CardLocation::INVALID,	4,	5,		"------",	0	,0	,0	,2.31872f);
	allGameCards[16] =	Card(16,	INVALID_ID,	4,	CardType::CREATURE,		CardLocation::INVALID,	6,	2,		"------",	0	,0	,0	,2.32522f);
	allGameCards[17] =	Card(17,	INVALID_ID,	4,	CardType::CREATURE,		CardLocation::INVALID,	4,	5,		"------",	0	,0	,0	,2.31872f);
	allGameCards[18] =	Card(18,	INVALID_ID,	4,	CardType::CREATURE,		CardLocation::INVALID,	7,	4,		"------",	0	,0	,0	,2.48755f);
	allGameCards[19] =	Card(19,	INVALID_ID,	5,	CardType::CREATURE,		CardLocation::INVALID,	5,	6,		"------",	0	,0	,0	,2.35227f);
	allGameCards[20] =	Card(20,	INVALID_ID,	5,	CardType::CREATURE,		CardLocation::INVALID,	8,	2,		"------",	0	,0	,0	,2.38474f);
	allGameCards[21] =	Card(21,	INVALID_ID,	5,	CardType::CREATURE,		CardLocation::INVALID,	6,	5,		"------",	0	,0	,0	,2.37825f);
	allGameCards[22] =	Card(22,	INVALID_ID,	6,	CardType::CREATURE,		CardLocation::INVALID,	7,	5,		"------",	0	,0	,0	,2.36634f);
	allGameCards[23] =	Card(23,	INVALID_ID,	7,	CardType::CREATURE,		CardLocation::INVALID,	8,	8,		"------",	0	,0	,0	,2.49080f);
	allGameCards[24] =	Card(24,	INVALID_ID,	1,	CardType::CREATURE,		CardLocation::INVALID,	1,	1,		"------",	0	,-1	,0	,2.33929f);
	allGameCards[25] =	Card(25,	INVALID_ID,	2,	CardType::CREATURE,		CardLocation::INVALID,	3,	1,		"------",	-2	,-2	,0	,2.31548f);
	allGameCards[26] =	Card(26,	INVALID_ID,	2,	CardType::CREATURE,		CardLocation::INVALID,	3,	2,		"------",	0	,-1	,0	,2.44426f);
	allGameCards[27] =	Card(27,	INVALID_ID,	2,	CardType::CREATURE,		CardLocation::INVALID,	2,	2,		"------",	2	,0	,0	,2.45617f);
	allGameCards[28] =	Card(28,	INVALID_ID,	2,	CardType::CREATURE,		CardLocation::INVALID,	1,	2,		"------",	0	,0	,1	,2.63474f);
	allGameCards[29] =	Card(29,	INVALID_ID,	2,	CardType::CREATURE,		CardLocation::INVALID,	2,	1,		"------",	0	,0	,1	,2.66071f);
	allGameCards[30] =	Card(30,	INVALID_ID,	3,	CardType::CREATURE,		CardLocation::INVALID,	4,	2,		"------",	0	,-2	,0	,2.59903f);
	allGameCards[31] =	Card(31,	INVALID_ID,	3,	CardType::CREATURE,		CardLocation::INVALID,	3,	1,		"------",	0	,-1	,0	,2.31548f);
	allGameCards[32] =	Card(32,	INVALID_ID,	3,	CardType::CREATURE,		CardLocation::INVALID,	3,	2,		"------",	0	,0	,1	,2.69426f);
	allGameCards[33] =	Card(33,	INVALID_ID,	4,	CardType::CREATURE,		CardLocation::INVALID,	4,	3,		"------",	0	,0	,1	,2.72781f);
	allGameCards[34] =	Card(34,	INVALID_ID,	5,	CardType::CREATURE,		CardLocation::INVALID,	3,	5,		"------",	0	,0	,1	,2.66396f);
	allGameCards[35] =	Card(35,	INVALID_ID,	6,	CardType::CREATURE,		CardLocation::INVALID,	5,	2,		"B-----",	0	,0	,1	,2.75379f);
	allGameCards[36] =	Card(36,	INVALID_ID,	6,	CardType::CREATURE,		CardLocation::INVALID,	4,	4,		"------",	0	,0	,2	,3.10660f);
	allGameCards[37] =	Card(37,	INVALID_ID,	6,	CardType::CREATURE,		CardLocation::INVALID,	5,	7,		"------",	0	,0	,1	,2.81439f);
	allGameCards[38] =	Card(38,	INVALID_ID,	1,	CardType::CREATURE,		CardLocation::INVALID,	1,	3,		"--D---",	0	,0	,0	,2.43019f);
	allGameCards[39] =	Card(39,	INVALID_ID,	1,	CardType::CREATURE,		CardLocation::INVALID,	2,	1,		"--D---",	0	,0	,0	,2.41071f);
	allGameCards[40] =	Card(40,	INVALID_ID,	3,	CardType::CREATURE,		CardLocation::INVALID,	2,	3,		"--DG--",	0	,0	,0	,2.50162f);
	allGameCards[41] =	Card(41,	INVALID_ID,	3,	CardType::CREATURE,		CardLocation::INVALID,	2,	2,		"-CD---",	0	,0	,0	,2.45617f);
	allGameCards[42] =	Card(42,	INVALID_ID,	4,	CardType::CREATURE,		CardLocation::INVALID,	4,	2,		"--D---",	0	,0	,0	,2.34903f);
	allGameCards[43] =	Card(43,	INVALID_ID,	6,	CardType::CREATURE,		CardLocation::INVALID,	5,	5,		"--D---",	0	,0	,0	,2.39015f);
	allGameCards[44] =	Card(44,	INVALID_ID,	6,	CardType::CREATURE,		CardLocation::INVALID,	3,	7,		"--D-L-",	0	,0	,0	,2.50487f);
	allGameCards[45] =	Card(45,	INVALID_ID,	6,	CardType::CREATURE,		CardLocation::INVALID,	6,	5,		"B-D---",	-3	,0	,0	,2.25325f);
	allGameCards[46] =	Card(46,	INVALID_ID,	9,	CardType::CREATURE,		CardLocation::INVALID,	7,	7,		"--D---",	0	,0	,0	,2.37392f);
	allGameCards[47] =	Card(47,	INVALID_ID,	2,	CardType::CREATURE,		CardLocation::INVALID,	1,	5,		"--D---",	0	,0	,0	,2.43777f);
	allGameCards[48] =	Card(48,	INVALID_ID,	1,	CardType::CREATURE,		CardLocation::INVALID,	1,	1,		"----L-",	0	,0	,0	,2.33929f);
	allGameCards[49] =	Card(49,	INVALID_ID,	2,	CardType::CREATURE,		CardLocation::INVALID,	1,	2,		"---GL-",	0	,0	,0	,2.46807f);
	allGameCards[50] =	Card(50,	INVALID_ID,	3,	CardType::CREATURE,		CardLocation::INVALID,	3,	2,		"----L-",	0	,0	,0	,2.36093f);
	allGameCards[51] =	Card(51,	INVALID_ID,	4,	CardType::CREATURE,		CardLocation::INVALID,	3,	5,		"----L-",	0	,0	,0	,2.41396f);
	allGameCards[52] =	Card(52,	INVALID_ID,	4,	CardType::CREATURE,		CardLocation::INVALID,	2,	4,		"----L-",	0	,0	,0	,2.29708f);
	allGameCards[53] =	Card(53,	INVALID_ID,	4,	CardType::CREATURE,		CardLocation::INVALID,	1,	1,		"-C--L-",	0	,0	,0	,2.25595f);
	allGameCards[54] =	Card(54,	INVALID_ID,	3,	CardType::CREATURE,		CardLocation::INVALID,	2,	2,		"----L-",	0	,0	,0	,2.28950f);
	allGameCards[55] =	Card(55,	INVALID_ID,	2,	CardType::CREATURE,		CardLocation::INVALID,	0,	5,		"---G--",	0	,0	,0	,2.36634f);
	allGameCards[56] =	Card(56,	INVALID_ID,	4,	CardType::CREATURE,		CardLocation::INVALID,	2,	7,		"------",	0	,0	,0	,2.26677f);
	allGameCards[57] =	Card(57,	INVALID_ID,	4,	CardType::CREATURE,		CardLocation::INVALID,	1,	8,		"------",	0	,0	,0	,2.24080f);
	allGameCards[58] =	Card(58,	INVALID_ID,	6,	CardType::CREATURE,		CardLocation::INVALID,	5,	6,		"B-----",	0	,0	,0	,2.43561f);
	allGameCards[59] =	Card(59,	INVALID_ID,	7,	CardType::CREATURE,		CardLocation::INVALID,	7,	7,		"------",	1	,-1	,0	,2.66558f);
	allGameCards[60] =	Card(60,	INVALID_ID,	7,	CardType::CREATURE,		CardLocation::INVALID,	4,	8,		"------",	0	,0	,0	,2.20509f);
	allGameCards[61] =	Card(61,	INVALID_ID,	9,	CardType::CREATURE,		CardLocation::INVALID,	10,	10,		"------",	0	,0	,0	,2.55790f);
	allGameCards[62] =	Card(62,	INVALID_ID,	12,	CardType::CREATURE,		CardLocation::INVALID,	12,	12,		"B--G--",	0	,0	,0	,2.87500f);
	allGameCards[63] =	Card(63,	INVALID_ID,	2,	CardType::CREATURE,		CardLocation::INVALID,	0,	4,		"---G-W",	0	,0	,0	,2.48755f);
	allGameCards[64] =	Card(64,	INVALID_ID,	2,	CardType::CREATURE,		CardLocation::INVALID,	1,	1,		"---G-W",	0	,0	,0	,2.42262f);
	allGameCards[65] =	Card(65,	INVALID_ID,	2,	CardType::CREATURE,		CardLocation::INVALID,	2,	2,		"-----W",	0	,0	,0	,2.37284f);
	allGameCards[66] =	Card(66,	INVALID_ID,	5,	CardType::CREATURE,		CardLocation::INVALID,	5,	1,		"-----W",	0	,0	,0	,2.29167f);
	allGameCards[67] =	Card(67,	INVALID_ID,	6,	CardType::CREATURE,		CardLocation::INVALID,	5,	5,		"-----W",	0	,-2	,0	,2.72348f);
	allGameCards[68] =	Card(68,	INVALID_ID,	6,	CardType::CREATURE,		CardLocation::INVALID,	7,	5,		"-----W",	0	,0	,0	,2.53301f);
	allGameCards[69] =	Card(69,	INVALID_ID,	3,	CardType::CREATURE,		CardLocation::INVALID,	4,	4,		"B-----",	0	,0	,0	,2.52327f);
	allGameCards[70] =	Card(70,	INVALID_ID,	4,	CardType::CREATURE,		CardLocation::INVALID,	6,	3,		"B-----",	0	,0	,0	,2.53734f);
	allGameCards[71] =	Card(71,	INVALID_ID,	4,	CardType::CREATURE,		CardLocation::INVALID,	3,	2,		"BC----",	0	,0	,0	,2.44426f);
	allGameCards[72] =	Card(72,	INVALID_ID,	4,	CardType::CREATURE,		CardLocation::INVALID,	5,	3,		"B-----",	0	,0	,0	,2.46591f);
	allGameCards[73] =	Card(73,	INVALID_ID,	4,	CardType::CREATURE,		CardLocation::INVALID,	4,	4,		"B-----",	4	,0	,0	,2.93994f);
	allGameCards[74] =	Card(74,	INVALID_ID,	5,	CardType::CREATURE,		CardLocation::INVALID,	5,	4,		"B--G--",	0	,0	,0	,2.59470f);
	allGameCards[75] =	Card(75,	INVALID_ID,	5,	CardType::CREATURE,		CardLocation::INVALID,	6,	5,		"B-----",	0	,0	,0	,2.54491f);
	allGameCards[76] =	Card(76,	INVALID_ID,	6,	CardType::CREATURE,		CardLocation::INVALID,	5,	5,		"B-D---",	0	,0	,0	,2.55682f);
	allGameCards[77] =	Card(77,	INVALID_ID,	7,	CardType::CREATURE,		CardLocation::INVALID,	7,	7,		"B-----",	0	,0	,0	,2.54058f);
	allGameCards[78] =	Card(78,	INVALID_ID,	8,	CardType::CREATURE,		CardLocation::INVALID,	5,	5,		"B-----",	0	,-5	,0	,3.05682f);
	allGameCards[79] =	Card(79,	INVALID_ID,	8,	CardType::CREATURE,		CardLocation::INVALID,	8,	8,		"B-----",	0	,0	,0	,2.57413f);
	allGameCards[80] =	Card(80,	INVALID_ID,	8,	CardType::CREATURE,		CardLocation::INVALID,	8,	8,		"B--G--",	0	,0	,1	,3.24080f);
	allGameCards[81] =	Card(81,	INVALID_ID,	9,	CardType::CREATURE,		CardLocation::INVALID,	6,	6,		"BC----",	0	,0	,0	,2.42370f);
	allGameCards[82] =	Card(82,	INVALID_ID,	7,	CardType::CREATURE,		CardLocation::INVALID,	5,	5,		"B-D--W",	0	,0	,0	,2.64015f);
	allGameCards[83] =	Card(83,	INVALID_ID,	0,	CardType::CREATURE,		CardLocation::INVALID,	1,	1,		"-C----",	0	,0	,0	,2.42262f);
	allGameCards[84] =	Card(84,	INVALID_ID,	2,	CardType::CREATURE,		CardLocation::INVALID,	1,	1,		"-CD--W",	0	,0	,0	,2.58929f);
	allGameCards[85] =	Card(85,	INVALID_ID,	3,	CardType::CREATURE,		CardLocation::INVALID,	2,	3,		"-C----",	0	,0	,0	,2.33496f);
	allGameCards[86] =	Card(86,	INVALID_ID,	3,	CardType::CREATURE,		CardLocation::INVALID,	1,	5,		"-C----",	0	,0	,0	,2.35444f);
	allGameCards[87] =	Card(87,	INVALID_ID,	4,	CardType::CREATURE,		CardLocation::INVALID,	2,	5,		"-C-G--",	0	,0	,0	,2.50920f);
	allGameCards[88] =	Card(88,	INVALID_ID,	5,	CardType::CREATURE,		CardLocation::INVALID,	4,	4,		"-C----",	0	,0	,0	,2.35660f);
	allGameCards[89] =	Card(89,	INVALID_ID,	5,	CardType::CREATURE,		CardLocation::INVALID,	4,	1,		"-C----",	2	,0	,0	,2.47024f);
	allGameCards[90] =	Card(90,	INVALID_ID,	8,	CardType::CREATURE,		CardLocation::INVALID,	5,	5,		"-C----",	0	,0	,0	,2.22348f);
	allGameCards[91] =	Card(91,	INVALID_ID,	0,	CardType::CREATURE,		CardLocation::INVALID,	1,	2,		"---G--",	0	,1	,0	,2.30141f);
	allGameCards[92] =	Card(92,	INVALID_ID,	1,	CardType::CREATURE,		CardLocation::INVALID,	0,	1,		"---G--",	2	,0	,0	,2.51786f);
	allGameCards[93] =	Card(93,	INVALID_ID,	1,	CardType::CREATURE,		CardLocation::INVALID,	2,	1,		"---G--",	0	,0	,0	,2.41071f);
	allGameCards[94] =	Card(94,	INVALID_ID,	2,	CardType::CREATURE,		CardLocation::INVALID,	1,	4,		"---G--",	0	,0	,0	,2.39232f);
	allGameCards[95] =	Card(95,	INVALID_ID,	2,	CardType::CREATURE,		CardLocation::INVALID,	2,	3,		"---G--",	0	,0	,0	,2.41829f);
	allGameCards[96] =	Card(96,	INVALID_ID,	2,	CardType::CREATURE,		CardLocation::INVALID,	3,	2,		"---G--",	0	,0	,0	,2.44426f);
	allGameCards[97] =	Card(97,	INVALID_ID,	3,	CardType::CREATURE,		CardLocation::INVALID,	3,	3,		"---G--",	0	,0	,0	,2.40639f);
	allGameCards[98] =	Card(98,	INVALID_ID,	3,	CardType::CREATURE,		CardLocation::INVALID,	2,	4,		"---G--",	0	,0	,0	,2.38041f);
	allGameCards[99] =	Card(99,	INVALID_ID,	3,	CardType::CREATURE,		CardLocation::INVALID,	2,	5,		"---G--",	0	,0	,0	,2.42587f);
	allGameCards[100] = Card(100,	INVALID_ID,	3,	CardType::CREATURE,		CardLocation::INVALID,	1,	6,		"---G--",	0	,0	,0	,2.39989f);
	allGameCards[101] = Card(101,	INVALID_ID,	4,	CardType::CREATURE,		CardLocation::INVALID,	3,	4,		"---G--",	0	,0	,0	,2.36851f);
	allGameCards[102] = Card(102,	INVALID_ID,	4,	CardType::CREATURE,		CardLocation::INVALID,	3,	3,		"---G--",	0	,-1	,0	,2.48972f);
	allGameCards[103] = Card(103,	INVALID_ID,	4,	CardType::CREATURE,		CardLocation::INVALID,	3,	6,		"---G--",	0	,0	,0	,2.45942f);
	allGameCards[104] = Card(104,	INVALID_ID,	4,	CardType::CREATURE,		CardLocation::INVALID,	4,	4,		"---G--",	0	,0	,0	,2.43994f);
	allGameCards[105] = Card(105,	INVALID_ID,	5,	CardType::CREATURE,		CardLocation::INVALID,	4,	6,		"---G--",	0	,0	,0	,2.44751f);
	allGameCards[106] = Card(106,	INVALID_ID,	5,	CardType::CREATURE,		CardLocation::INVALID,	5,	5,		"---G--",	0	,0	,0	,2.47348f);
	allGameCards[107] = Card(107,	INVALID_ID,	5,	CardType::CREATURE,		CardLocation::INVALID,	3,	3,		"---G--",	3	,0	,0	,2.61472f);
	allGameCards[108] = Card(108,	INVALID_ID,	5,	CardType::CREATURE,		CardLocation::INVALID,	2,	6,		"---G--",	0	,0	,0	,2.30465f);
	allGameCards[109] = Card(109,	INVALID_ID,	5,	CardType::CREATURE,		CardLocation::INVALID,	5,	6,		"------",	0	,0	,0	,2.35227f);
	allGameCards[110] = Card(110,	INVALID_ID,	5,	CardType::CREATURE,		CardLocation::INVALID,	0,	9,		"---G--",	0	,0	,0	,2.29816f);
	allGameCards[111] = Card(111,	INVALID_ID,	6,	CardType::CREATURE,		CardLocation::INVALID,	6,	6,		"---G--",	0	,0	,0	,2.50703f);
	allGameCards[112] = Card(112,	INVALID_ID,	6,	CardType::CREATURE,		CardLocation::INVALID,	4,	7,		"---G--",	0	,0	,0	,2.40963f);
	allGameCards[113] = Card(113,	INVALID_ID,	6,	CardType::CREATURE,		CardLocation::INVALID,	2,	4,		"---G--",	4	,0	,0	,2.63041f);
	allGameCards[114] = Card(114,	INVALID_ID,	7,	CardType::CREATURE,		CardLocation::INVALID,	7,	7,		"---G--",	0	,0	,0	,2.54058f);
	allGameCards[115] = Card(115,	INVALID_ID,	8,	CardType::CREATURE,		CardLocation::INVALID,	5,	5,		"---G-W",	0	,0	,0	,2.39015f);
	allGameCards[116] = Card(116,	INVALID_ID,	12,	CardType::CREATURE,		CardLocation::INVALID,	8,	8,		"BCDGLW",	0	,0	,0	,3.07413f);
	allGameCards[117] = Card(117,	INVALID_ID,	1,	CardType::GREEN_ITEM,	CardLocation::INVALID,	1,	1,		"B-----",	0	,0	,0	,2.33929f);
	allGameCards[118] = Card(118,	INVALID_ID,	0,	CardType::GREEN_ITEM,	CardLocation::INVALID,	0,	3,		"------",	0	,0	,0	,2.27543f);
	allGameCards[119] = Card(119,	INVALID_ID,	1,	CardType::GREEN_ITEM,	CardLocation::INVALID,	1,	2,		"------",	0	,0	,0	,2.21807f);
	allGameCards[120] = Card(120,	INVALID_ID,	2,	CardType::GREEN_ITEM,	CardLocation::INVALID,	1,	0,		"----L-",	0	,0	,0	,2.21050f);
	allGameCards[121] = Card(121,	INVALID_ID,	2,	CardType::GREEN_ITEM,	CardLocation::INVALID,	0,	3,		"------",	0	,0	,1	,2.60877f);
	allGameCards[122] = Card(122,	INVALID_ID,	2,	CardType::GREEN_ITEM,	CardLocation::INVALID,	1,	3,		"---G--",	0	,0	,0	,2.34686f);
	allGameCards[123] = Card(123,	INVALID_ID,	2,	CardType::GREEN_ITEM,	CardLocation::INVALID,	4,	0,		"------",	0	,0	,0	,2.25812f);
	allGameCards[124] = Card(124,	INVALID_ID,	3,	CardType::GREEN_ITEM,	CardLocation::INVALID,	2,	1,		"--D---",	0	,0	,0	,2.24405f);
	allGameCards[125] = Card(125,	INVALID_ID,	3,	CardType::GREEN_ITEM,	CardLocation::INVALID,	1,	4,		"------",	0	,0	,0	,2.14232f);
	allGameCards[126] = Card(126,	INVALID_ID,	3,	CardType::GREEN_ITEM,	CardLocation::INVALID,	2,	3,		"------",	0	,0	,0	,2.16829f);
	allGameCards[127] = Card(127,	INVALID_ID,	3,	CardType::GREEN_ITEM,	CardLocation::INVALID,	0,	6,		"------",	0	,0	,0	,2.16180f);
	allGameCards[128] = Card(128,	INVALID_ID,	4,	CardType::GREEN_ITEM,	CardLocation::INVALID,	4,	3,		"------",	0	,0	,0	,2.22781f);
	allGameCards[129] = Card(129,	INVALID_ID,	4,	CardType::GREEN_ITEM,	CardLocation::INVALID,	2,	5,		"------",	0	,0	,0	,2.17587f);
	allGameCards[130] = Card(130,	INVALID_ID,	4,	CardType::GREEN_ITEM,	CardLocation::INVALID,	0,	6,		"------",	4	,0	,0	,2.57846f);
	allGameCards[131] = Card(131,	INVALID_ID,	4,	CardType::GREEN_ITEM,	CardLocation::INVALID,	4,	1,		"------",	0	,0	,0	,2.13690f);
	allGameCards[132] = Card(132,	INVALID_ID,	5,	CardType::GREEN_ITEM,	CardLocation::INVALID,	3,	3,		"B-----",	0	,0	,0	,2.23972f);
	allGameCards[133] = Card(133,	INVALID_ID,	5,	CardType::GREEN_ITEM,	CardLocation::INVALID,	4,	0,		"-----W",	0	,0	,0	,2.17478f);
	allGameCards[134] = Card(134,	INVALID_ID,	4,	CardType::GREEN_ITEM,	CardLocation::INVALID,	2,	2,		"------",	0	,0	,1	,2.53950f);
	allGameCards[135] = Card(135,	INVALID_ID,	6,	CardType::GREEN_ITEM,	CardLocation::INVALID,	5,	5,		"------",	0	,0	,0	,2.22348f);
	allGameCards[136] = Card(136,	INVALID_ID,	0,	CardType::GREEN_ITEM,	CardLocation::INVALID,	1,	1,		"------",	0	,0	,0	,2.25595f);
	allGameCards[137] = Card(137,	INVALID_ID,	2,	CardType::GREEN_ITEM,	CardLocation::INVALID,	0,	0,		"-----W",	0	,0	,0	,2.13907f);
	allGameCards[138] = Card(138,	INVALID_ID,	2,	CardType::GREEN_ITEM,	CardLocation::INVALID,	0,	0,		"---G--",	0	,0	,1	,2.63907f);
	allGameCards[139] = Card(139,	INVALID_ID,	4,	CardType::GREEN_ITEM,	CardLocation::INVALID,	0,	0,		"----LW",	0	,0	,0	,2.13907f);
	allGameCards[140] = Card(140,	INVALID_ID,	2,	CardType::GREEN_ITEM,	CardLocation::INVALID,	0,	0,		"-C----",	0	,0	,0	,2.13907f);
	allGameCards[141] = Card(141,	INVALID_ID,	0,	CardType::RED_ITEM,		CardLocation::INVALID,	-1,	-1,		"------",	0	,0	,0	,2.02219f);
	allGameCards[142] = Card(142,	INVALID_ID,	0,	CardType::RED_ITEM,		CardLocation::INVALID,	0,	0,		"BCDGLW",	0	,0	,0	,3.13907f);
	allGameCards[143] = Card(143,	INVALID_ID,	0,	CardType::RED_ITEM,		CardLocation::INVALID,	0,	0,		"---G--",	0	,0	,0	,2.30574f);
	allGameCards[144] = Card(144,	INVALID_ID,	1,	CardType::RED_ITEM,		CardLocation::INVALID,	0,	-2,		"------",	0	,0	,0	,1.96483f);
	allGameCards[145] = Card(145,	INVALID_ID,	3,	CardType::RED_ITEM,		CardLocation::INVALID,	-2,	-2,		"------",	0	,0	,0	,1.65530f);
	allGameCards[146] = Card(146,	INVALID_ID,	4,	CardType::RED_ITEM,		CardLocation::INVALID,	-2,	-2,		"------",	0	,-2	,0	,1.90530f);
	allGameCards[147] = Card(147,	INVALID_ID,	2,	CardType::RED_ITEM,		CardLocation::INVALID,	0,	-1,		"------",	0	,0	,1	,2.42695f);
	allGameCards[148] = Card(148,	INVALID_ID,	2,	CardType::RED_ITEM,		CardLocation::INVALID,	0,	-2,		"BCDGLW",	0	,0	,0	,2.88149f);
	allGameCards[149] = Card(149,	INVALID_ID,	3,	CardType::RED_ITEM,		CardLocation::INVALID,	0,	0,		"BCDGLW",	0	,0	,1	,3.38907f);
	allGameCards[150] = Card(150,	INVALID_ID,	2,	CardType::RED_ITEM,		CardLocation::INVALID,	0,	-3,		"------",	0	,0	,0	,1.83604f);
	allGameCards[151] = Card(151,	INVALID_ID,	5,	CardType::RED_ITEM,		CardLocation::INVALID,	0,	-10,	"BCDGLW",	0	,0	,0	,2.26786f);
	allGameCards[152] = Card(152,	INVALID_ID,	7,	CardType::RED_ITEM,		CardLocation::INVALID,	0,	-7,		"------",	0	,0	,1	,1.73755f);
	allGameCards[153] = Card(153,	INVALID_ID,	2,	CardType::BLUE_ITEM,	CardLocation::INVALID,	0,	0,		"------",	5	,0	,0	,2.59740f);
	allGameCards[154] = Card(154,	INVALID_ID,	2,	CardType::BLUE_ITEM,	CardLocation::INVALID,	0,	0,		"------",	0	,-2	,1	,2.80574f);
	allGameCards[155] = Card(155,	INVALID_ID,	3,	CardType::BLUE_ITEM,	CardLocation::INVALID,	0,	-3,		"------",	0	,-1	,0	,1.91937f);
	allGameCards[156] = Card(156,	INVALID_ID,	3,	CardType::BLUE_ITEM,	CardLocation::INVALID,	0,	0,		"------",	3	,-3	,0	,2.76407f);
	allGameCards[157] = Card(157,	INVALID_ID,	3,	CardType::BLUE_ITEM,	CardLocation::INVALID,	0,	-1,		"------",	1	,0	,1	,2.46861f);
	allGameCards[158] = Card(158,	INVALID_ID,	3,	CardType::BLUE_ITEM,	CardLocation::INVALID,	0,	-4,		"------",	0	,0	,0	,1.70725f);
	allGameCards[159] = Card(159,	INVALID_ID,	4,	CardType::BLUE_ITEM,	CardLocation::INVALID,	0,	-3,		"------",	3	,0	,0	,2.04437f);
	allGameCards[160] = Card(160,	INVALID_ID,	2,	CardType::BLUE_ITEM,	CardLocation::INVALID,	0,	0,		"------",	2	,-2	,0	,2.55574f);
}

struct CardValue {
	int cardNumber;
	float value;

	CardValue();

	CardValue(
		int cardNumber,
		float value
	);
};

CardValue::CardValue() :
	cardNumber(INVALID_ID),
	value(INVALID_CARD_VALUE)
{

}

CardValue::CardValue(
	int cardNumber,
	float value
) :
	cardNumber(cardNumber),
	value(value)
{

}

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

Draft::Draft() :
	cardsToChooseFromCount(0),
	chosenCardsCount(0)
{

}

Draft::~Draft() {
}

void Draft::addCardForChoosing(const CardValue& card) {
	cardsToChooseFrom[cardsToChooseFromCount++] = card;
}

void Draft::addCardInChosen(int pickedCardNumber) {
	chosenCardsNumbers[chosenCardsCount++] = pickedCardNumber;
}

void Draft::outputTheChoice(int pickedCardId) {
	cout << PICK + SPACE << pickedCardId << endl;
}

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

void Draft::clearCardsToChooseFrom() {
	cardsToChooseFromCount = 0;
}

class HandCard {
public:
	HandCard();

	HandCard(
		int number,
		int id
	);

	~HandCard();

	uint8_t extractNumber() const;
	int8_t extractId() const;

	HandCard& operator=(const HandCard& handCard);

	void create(
		int number,
		int id
	);

	void erase();

	bool isErased();

private:
	int card;
};

HandCard::HandCard() :
	card(DEFAULT_CARD_TEMPLATE)
{

}

HandCard::HandCard(
	int number,
	int id
) :
	card(DEFAULT_CARD_TEMPLATE)
{
	create(number, id);
}

HandCard::~HandCard() {

}

void HandCard::create(
	int number,
	int id
) {
	card = number;

	id <<= CardMasks::HAND_CARD_ID_OFFSET;
	card |= id;
}

void HandCard::erase() {
	card = 0;
}

bool HandCard::isErased() {
	return !card;
}

uint8_t HandCard::extractNumber() const {
	return card & CardMasks::NUMBER;
}

int8_t HandCard::extractId() const {
	return (card & CardMasks::HAND_CARD_ID) >> CardMasks::HAND_CARD_ID_OFFSET;
}

HandCard& HandCard::operator=(const HandCard& handCard) {
	if (this != &handCard) {
		card = handCard.card;
	}

	return *this;
}

class Hand {
public:
	Hand();
	Hand(const Hand& hand);
	~Hand();

	int8_t getCardsCount() const {
		return cardsCount;
	}

	HandCard& getCard(int8_t cardIdx);

	Hand& operator=(const Hand& hand);

	void copy(const Hand& hand);
	void addCard(const HandCard& card);
	void removeCard(int8_t cardId);

	void getAllCombinations(
		int8_t mana,
		StateSimulationType simType,
		HandCombinations& handCombinations
	) const;

	bool uniqueCombination(
		const HandCombinations& handCombinations,
		const HandCombination& combination
	) const;

	void reset();

private:
	HandCard cards[MAX_CARDS_IN_HAND];
	int8_t cardsCount;
};

Hand::Hand() :
	cardsCount(0)
{

}

Hand::Hand(const Hand& hand) {
	copy(hand);
}

Hand::~Hand() {

}

HandCard& Hand::getCard(int8_t cardIdx) {
	return cards[cardIdx];
}

Hand& Hand::operator=(const Hand& hand) {
	if (this != &hand) {
		copy(hand);
	}

	return *this;
}

void Hand::copy(const Hand& hand) {
	cardsCount = hand.cardsCount;

	for (int cardIdx = 0; cardIdx < MAX_CARDS_IN_HAND; ++cardIdx) {
		cards[cardIdx] = hand.cards[cardIdx];
	}
}

void Hand::addCard(const HandCard& card) {
	cards[cardsCount++] = card;
}

bool Hand::uniqueCombination(
	const HandCombinations& handCombinations,
	const HandCombination& combination
) const {
	bool unique = true;

	for (const HandCombination& handCombination : handCombinations) {
		if (handCombination.cardsNumbers == combination.cardsNumbers) {
			unique = false;
			break;
		}
	}

	return unique;
}

void Hand::getAllCombinations(
	int8_t mana,
	StateSimulationType simType,
	HandCombinations& handCombinations
) const {
	int maxCombinations = static_cast<int>(pow(2, cardsCount));

	// Start from ..000001 until..1111111 bit 1 represents card in set
	for (int comb = 1; comb <= maxCombinations; ++comb) {
		int8_t combinationCost = 0;
		HandCombination combination;

		for (int8_t cardIdx = 0; cardIdx < cardsCount; ++cardIdx) {
			if (comb & (1 << cardIdx)) {
				const HandCard& handCard = cards[cardIdx];
				long long number = static_cast<long long>(handCard.extractNumber());
				long long id = static_cast<long long>(handCard.extractId());
				const Card& card = ALL_CARDS_HOLDER.allGameCards[number];

				combinationCost += card.getCost();
				combination.cardsNumbers |= number << (CardMasks::HAND_CARD_COMB_OFFSET * cardIdx);
				combination.cardsIds |= id << (CardMasks::HAND_CARD_COMB_OFFSET * cardIdx);
			}
		}

		if (combination.cardsNumbers > 0 &&
			combinationCost <= mana
		) {
			handCombinations.push_back(combination);
		}
	}
}

inline void Hand::reset() {
	cardsCount = 0;
}

void Hand::removeCard(int8_t cardId) {
	for (int8_t cardIdx = 0; cardIdx < MAX_CARDS_IN_HAND; ++ cardIdx) {
		HandCard* card = &cards[cardIdx];

		if (card->extractId() == cardId) {
			card->erase();
			--cardsCount;

			for (int8_t nextCardIdx = cardIdx; nextCardIdx < MAX_CARDS_IN_HAND - 1; ++nextCardIdx) {
				cards[nextCardIdx] = cards[nextCardIdx + 1];
			}
			
			break;
		}
	}
}

typedef vector<int8_t> AttackTargets;

class BoardCard {
public:
	BoardCard();

	BoardCard(
		int id,
		int att,
		int def,
		int abilities
	);

	~BoardCard();

	void create(
		int id,
		int att,
		int def,
		int abilitiesBits
	);

	inline const AttackTargets& getTargets() const;

	uint8_t extractId() const;
	uint8_t extractAttack() const;
	uint8_t extractDefense() const;
	uint8_t extractAbilitiesBits() const;

	/// Returns true if the creature is destroyed
	bool applyItemEffect(const Card& item);

	inline bool hasAbility(int abilityFlag) const;
	
	inline void setAbility(int abilityFlag);
	inline void unsetAbility(int abilityFlag);
	inline void setTargets(const AttackTargets& targets);
	inline void setAttack(int attack);
	inline void setDefense(int defense);

private:
	int card;
	AttackTargets targets;
};

typedef vector<BoardCard> BoardCardList;

BoardCard::BoardCard() :
	card(0),
	targets()
{

}

BoardCard::BoardCard(
	int id,
	int attack,
	int defense,
	int abilities
) :
	targets()
{
	create(id, attack, defense, abilities);
}

BoardCard::~BoardCard() {
}

void BoardCard::create(
	int id,
	int attack,
	int defense,
	int abilitiesBits
) {
	card = attack;

	defense <<= CardMasks::DEFENSE_OFFSET;
	card |= defense;

	id <<= CardMasks::BOARD_CARD_ID_OFFSET;
	card |= id;

	abilitiesBits <<= CardMasks::ABILITIES_OFFSET;
	card |= abilitiesBits;

	if (hasAbility(CardMasks::CHARGE)) {
		setAbility(CardMasks::CAN_ATTACK);
	}
}

inline const AttackTargets & BoardCard::getTargets() const {
	return targets;
}

uint8_t BoardCard::extractId() const {
	return (card & CardMasks::BOARD_CARD_ID) >> CardMasks::BOARD_CARD_ID_OFFSET;
}

uint8_t BoardCard::extractAttack() const {
	return card & CardMasks::ATTACK;
}

uint8_t BoardCard::extractDefense() const {
	return (card & CardMasks::DEFENSE) >> CardMasks::DEFENSE_OFFSET;
}

uint8_t BoardCard::extractAbilitiesBits() const {
	return (card & CardMasks::ABILITIES) >> CardMasks::ABILITIES_OFFSET;
}

bool BoardCard::applyItemEffect(const Card& item) {
	bool creatureDead = true;

	int newDefence = extractDefense() + item.getDef();
	if (newDefence > 0) {
		creatureDead = false;

		uint8_t abilities = extractAbilitiesBits();
		uint8_t newAbilities = abilities | item.getBitsAbilities();

		if (CardType::RED_ITEM == item.getType()) {
			newAbilities = abilities & item.getBitsAbilities();
		}

		create(
			extractId(),
			extractAttack() + item.getAtt(),
			extractDefense() + item.getDef(),
			newAbilities
		);
	}

	return creatureDead;
}

inline bool BoardCard::hasAbility(int abilityFlag) const {
	return abilityFlag & card;
}

inline void BoardCard::setAbility(int abilityFlag) {
	card |= abilityFlag;
}

inline void BoardCard::unsetAbility(int abilityFlag) {
	card &= (~abilityFlag);
}

inline void BoardCard::setTargets(const AttackTargets& targets) {
	this->targets = targets;
}

inline void BoardCard::setAttack(int attack) {
	card |= attack;
}

inline void BoardCard::setDefense(int defense) {
	if (defense < 0) {
		defense = 0;
	}

	defense <<= CardMasks::DEFENSE_OFFSET;
	card = (card & (~CardMasks::DEFENSE)) | defense;
}

class Board {
public:
	Board();
	Board(const Board& board);
	~Board();

	int8_t getPlayerCardsCount() const {
		return playerCardsCount;
	}

	int8_t getOpponentCardsCount() const {
		return opponentCardsCount;
	}

	const BoardCard& getPlayerBoardCard(int8_t cardIdx) const {
		return playerBoard[cardIdx];
	}

	const BoardCard& getOpponentBoardCard(int8_t cardIdx) const {
		return opponentBoard[cardIdx];
	}

	BoardCard (&getPlayerBoard())[MAX_BOARD_CREATURES] {
		return playerBoard;
	}

	BoardCard (&getOpponentBoard())[MAX_BOARD_CREATURES] {
		return opponentBoard;
	}

	Board& operator=(const Board& board);

	void copy(const Board& board);
	void addCard(const BoardCard& card, Side side);
	void playItem(const Card& item, uint8_t target);
	void destroyCreature(int8_t creatureId);
	void removeCard(int8_t cardIdx, Side side);

	void performAttack(
		const int8_t attCreatureId,
		const int8_t defCreatureId,
		int8_t& attackingPlayerHealthChange,
		int8_t& defendingPlayerHealthChange
	);

	void fight(
		BoardCard& attackCreature,
		BoardCard& defenseCreature,
		int8_t& attackingPlayerHealthChange,
		int8_t& defendingPlayerHealthChange
	);

	void applyDrain(
		BoardCard& attackCreature,
		BoardCard& defenseCreature,
		int8_t& attackingPlayerHealthChange,
		int8_t& defendingPlayerHealthChange
	);

	void applyBreakthrough(
		BoardCard& attackCreature,
		BoardCard& defenseCreature,
		int8_t& attackingPlayerHealthChange,
		int8_t& defendingPlayerHealthChange
	);

	void applyLethal(
		BoardCard& attackCreature,
		BoardCard& defenseCreature,
		bool& attackerDestroyed,
		bool& defenderDestroyed
	);

	void applyDemage(
		BoardCard& attackCreature,
		BoardCard& defenseCreature,
		bool& attackerDestroyed,
		bool& defenderDestroyed
	);

	void applyWard(
		BoardCard& attackCreature,
		BoardCard& defenseCreature
	);

	int8_t setAttackCreaturesTargets();
	int8_t attackingCreaturesCount(Side side) const;

	int getBoardSum(Side side, BoardSum sumType) const;

	void reset();
	int8_t playerDemageToLethal(const int8_t opponentHealth) const;
	void checkForBlockers(BoardCardList& blockers) const;
	void executeDirectAttacks() const;

private:
	BoardCard playerBoard[MAX_BOARD_CREATURES];
	int8_t playerCardsCount;

	BoardCard opponentBoard[MAX_BOARD_CREATURES];
	int8_t opponentCardsCount;
};

Board::Board() :
	playerCardsCount(0),
	opponentCardsCount(0)
{

}

Board::Board(const Board& board) {
	copy(board);
}

Board::~Board() {

}

Board& Board::operator=(const Board& board) {
	if (this != &board) {
		copy(board);
	}

	return *this;
}

void Board::copy(const Board& board) {
	playerCardsCount = board.playerCardsCount;
	opponentCardsCount = board.opponentCardsCount;

	for (int cardIdx = 0; cardIdx < MAX_BOARD_CREATURES; ++cardIdx) {
		playerBoard[cardIdx] = board.playerBoard[cardIdx];
		opponentBoard[cardIdx] = board.opponentBoard[cardIdx];
	}
}

void Board::addCard(const BoardCard& card, Side side) {
	if (Side::PLAYER == side) {
		playerBoard[playerCardsCount++] = card;
	}
	else {
		opponentBoard[opponentCardsCount++] = card;
	}
}

void Board::playItem(const Card& item, uint8_t target) {
	bool playerCardTarget = false;

	// Based on the card type choose which array to iterate
	for (int8_t cardIdx = 0; cardIdx < MAX_BOARD_CREATURES; ++cardIdx) {
		BoardCard& playerCard = playerBoard[cardIdx];
		if (playerCard.extractId() == target) {
			if (playerCard.applyItemEffect(item)) {
				removeCard(cardIdx, Side::PLAYER);
			}
			playerCardTarget = true;
			break;
		}
	}

	if (!playerCardTarget) {
		for (int8_t cardIdx = 0; cardIdx < MAX_BOARD_CREATURES; ++cardIdx) {
			BoardCard& opponentCard = opponentBoard[cardIdx];
			if (opponentCard.extractId() == target) {
				if (opponentCard.applyItemEffect(item)) {
					removeCard(cardIdx, Side::PLAYER);
				}
				break;
			}
		}
	}
}

void Board::performAttack(
	const int8_t attCreatureId,
	const int8_t defCreatureId,
	int8_t& attackingPlayerHealthChange,
	int8_t& defendingPlayerHealthChange
) {
	BoardCard* attackCreature = nullptr;
	BoardCard* defenseCreature = nullptr;

	for (int8_t playerCreatureIdx = 0; playerCreatureIdx < playerCardsCount; ++playerCreatureIdx) {
		BoardCard& playerBoardCard = playerBoard[playerCreatureIdx];
		if (attCreatureId == playerBoardCard.extractId()) {
			attackCreature = &playerBoardCard;
			break;
		}
		else if (defCreatureId == playerBoardCard.extractId()) {
			defenseCreature = &playerBoardCard;
			break;
		}
	}

	for (int8_t opponentCreatureIdx = 0; opponentCreatureIdx < opponentCardsCount; ++opponentCreatureIdx) {
		BoardCard& opponentBoardCard = opponentBoard[opponentCreatureIdx];
		if (attCreatureId == opponentBoardCard.extractId()) {
			attackCreature = &opponentBoardCard;
			break;
		}
		else if (defCreatureId == opponentBoardCard.extractId()) {
			defenseCreature = &opponentBoardCard;
			break;
		}
	}

	if (PLAYER_TARGET != defCreatureId) {
		fight(*attackCreature, *defenseCreature, attackingPlayerHealthChange, defendingPlayerHealthChange);
	}
	else {
		defendingPlayerHealthChange -= attackCreature->extractAttack();
		attackCreature->unsetAbility(CardMasks::CAN_ATTACK);
	}
}

void Board::fight(
	BoardCard& attackCreature,
	BoardCard& defenseCreature,
	int8_t& attackingPlayerHealthChange,
	int8_t& defendingPlayerHealthChange
) {
	bool attackerDestroyed = false;
	bool defenderDestroyed = false;

	applyDrain(attackCreature, defenseCreature, attackingPlayerHealthChange, defendingPlayerHealthChange);
	applyBreakthrough(attackCreature, defenseCreature, attackingPlayerHealthChange, defendingPlayerHealthChange);
	applyLethal(attackCreature, defenseCreature, attackerDestroyed, defenderDestroyed);
	applyDemage(attackCreature, defenseCreature, attackerDestroyed, defenderDestroyed);
	applyWard(attackCreature, defenseCreature);

	if (attackerDestroyed) {
		destroyCreature(attackCreature.extractId());
	}
	else {
		attackCreature.unsetAbility(CardMasks::CAN_ATTACK);
	}

	if (defenderDestroyed) {
		destroyCreature(defenseCreature.extractId());
	}
}

void Board::destroyCreature(int8_t creatureId) {
	for (int8_t playerCreatureIdx = 0; playerCreatureIdx < playerCardsCount; ++playerCreatureIdx) {
		if (creatureId == playerBoard[playerCreatureIdx].extractId()) {
			removeCard(playerCreatureIdx, Side::PLAYER);
			break;
		}
	}

	for (int8_t opponentCreatureIdx = 0; opponentCreatureIdx < opponentCardsCount; ++opponentCreatureIdx) {
		if (creatureId == opponentBoard[opponentCreatureIdx].extractId()) {
			removeCard(opponentCreatureIdx, Side::OPPONENT);
			break;
		}
	}
}

void Board::removeCard(int8_t cardIdx, Side side) {
	if (MAX_BOARD_CREATURES - 1 == cardIdx) {
		if (Side::PLAYER == side) {
			playerBoard[cardIdx] = BoardCard();
		}
		else {
			opponentBoard[cardIdx] = BoardCard();
		}
	}
	else {
		for (; cardIdx < MAX_BOARD_CREATURES - 1; ++cardIdx) {
			if (Side::PLAYER == side) {
				playerBoard[cardIdx] = playerBoard[cardIdx + 1];
			}
			else {
				opponentBoard[cardIdx] = opponentBoard[cardIdx + 1];
			}
		}
	}

	if (Side::PLAYER == side) {
		--playerCardsCount;
	}
	else {
		--opponentCardsCount;
	}
}

void Board::applyDrain(
	BoardCard& attackCreature,
	BoardCard& defenseCreature,
	int8_t& attackingPlayerHealthChange,
	int8_t& defendingPlayerHealthChange
){
	if (attackCreature.hasAbility(CardMasks::DRAIN) && !defenseCreature.hasAbility(CardMasks::WARD)) {
		attackingPlayerHealthChange += attackCreature.extractAttack();
	}

	if (defenseCreature.hasAbility(CardMasks::DRAIN) && !attackCreature.hasAbility(CardMasks::WARD)) {
		defendingPlayerHealthChange += defenseCreature.extractAttack();
	}
}

void Board::applyBreakthrough(
	BoardCard& attackCreature,
	BoardCard& defenseCreature,
	int8_t& attackingPlayerHealthChange,
	int8_t& defendingPlayerHealthChange
) {
	int8_t attack = attackCreature.extractAttack();
	int8_t defense = defenseCreature.extractDefense();

	if (attackCreature.hasAbility(CardMasks::BREAKTHROUGH) &&
		attack > defense &&
		!defenseCreature.hasAbility(CardMasks::WARD)) {
		defendingPlayerHealthChange -= attack - defense;
	}

	if (defenseCreature.hasAbility(CardMasks::BREAKTHROUGH) &&
		defense > attack &&
		!attackCreature.hasAbility(CardMasks::WARD)) {
		attackingPlayerHealthChange -= defense - attack;
	}
}

void Board::applyLethal(
	BoardCard& attackCreature,
	BoardCard& defenseCreature,
	bool& attackerDestroyed,
	bool& defenderDestroyed
) {
	if (attackCreature.hasAbility(CardMasks::LETHAL) && !defenseCreature.hasAbility(CardMasks::WARD)) {
		defenderDestroyed = true;
	}

	if (defenseCreature.hasAbility(CardMasks::LETHAL) && !attackCreature.hasAbility(CardMasks::WARD)) {
		attackerDestroyed = true;
	}
}

void Board::applyDemage(
	BoardCard& attackCreature,
	BoardCard& defenseCreature,
	bool& attackerDestroyed,
	bool& defenderDestroyed
) {
	if (!defenseCreature.hasAbility(CardMasks::WARD) && !defenderDestroyed) {
		defenseCreature.setDefense(defenseCreature.extractDefense() - attackCreature.extractAttack());

		if (defenseCreature.extractDefense() <= 0) {
			defenderDestroyed = true;
		}
	}

	if (!attackCreature.hasAbility(CardMasks::WARD) && !attackerDestroyed) {
		attackCreature.setDefense(attackCreature.extractDefense() - defenseCreature.extractAttack());

		if (attackCreature.extractDefense() <= 0) {
			attackerDestroyed = true;
		}
	}
}

void Board::applyWard(
	BoardCard& attackCreature,
	BoardCard& defenseCreature
) {
	if (attackCreature.extractAttack() > 0) {
		defenseCreature.unsetAbility(CardMasks::WARD);
	}

	if (defenseCreature.extractAttack() > 0) {
		attackCreature.unsetAbility(CardMasks::WARD);
	}
}

int8_t Board::setAttackCreaturesTargets() {
	int8_t allTargetsCount = 0;

	for (int8_t attCreatureIdx = 0; attCreatureIdx < playerCardsCount; ++attCreatureIdx) {
		BoardCard& attCreature = playerBoard[attCreatureIdx];
		if (attCreature.hasAbility(CardMasks::CAN_ATTACK) && attCreature.extractAttack() > 0) {
			AttackTargets guardTargets;
			AttackTargets targets;

			for (int8_t defCreatureIdx = 0; defCreatureIdx < opponentCardsCount; ++defCreatureIdx) {
				const BoardCard& defCreature = opponentBoard[defCreatureIdx];
				int8_t defCreatureId = defCreature.extractId();
				targets.push_back(defCreatureId);

				if (defCreature.hasAbility(CardMasks::GUARD)) {
					guardTargets.push_back(defCreatureId);
				}
			}

			if (0 < guardTargets.size()) {
				attCreature.setTargets(guardTargets);
			}
			else {
				targets.push_back(PLAYER_TARGET);
				attCreature.setTargets(targets);
			}

			allTargetsCount += static_cast<int8_t>(attCreature.getTargets().size());
		}
	}

	return allTargetsCount;
}

int8_t Board::attackingCreaturesCount(Side side) const {
	int8_t count = 0;

	const BoardCard* boardCardsArr = playerBoard;

	if (Side::OPPONENT == side) {
		boardCardsArr = opponentBoard;
	}

	for (int8_t attCreatureIdx = 0; attCreatureIdx < playerCardsCount; ++attCreatureIdx) {
		const BoardCard& attCreature = boardCardsArr[attCreatureIdx];
		if (attCreature.hasAbility(CardMasks::CAN_ATTACK) && attCreature.extractAttack() > 0) {
			++count;
		}
	}

	return count;
}

int Board::getBoardSum(Side side, BoardSum sumType) const {
	int sum = 0;

	int8_t count = playerCardsCount;
	const BoardCard* boardCards = playerBoard;

	if (Side::OPPONENT == side) {
		count = opponentCardsCount;
		boardCards = opponentBoard;
	}

	for (int8_t cardIdx = 0; cardIdx < count; ++cardIdx) {
		const BoardCard& card = boardCards[cardIdx];
		if (BoardSum::ATT == sumType) {
			sum += card.extractAttack();
		}
		else {
			sum += card.extractDefense();
		}
	}

	return sum;
}

void Board::reset() {
	playerCardsCount = 0;
	opponentCardsCount = 0;
}

int8_t Board::playerDemageToLethal(const int8_t opponentHealth) const {
	int8_t playerCombinedAttack = getBoardSum(Side::PLAYER, BoardSum::ATT);

	return opponentHealth - playerCombinedAttack;
}

void Board::checkForBlockers(BoardCardList& blockers) const {
	bool canAttack = true;

	for (int8_t opponentCreatureIdx = 0; opponentCreatureIdx < opponentCardsCount; ++opponentCreatureIdx) {
		const BoardCard& opponentBoardCard = opponentBoard[opponentCreatureIdx];

		if (opponentBoardCard.hasAbility(CardMasks::GUARD)) {
			blockers.push_back(opponentBoardCard);
		}
	}
}

void Board::executeDirectAttacks() const {
	for (int8_t playerCreatureIdx = 0; playerCreatureIdx < playerCardsCount; ++playerCreatureIdx) {
		const BoardCard& playerBoardCard = playerBoard[playerCreatureIdx];

		cout << playerBoardCard.extractId() << SPACE << OPPONENT_ATTCK << endl;
	}
}

class Player {
public:
	Player();

	Player(
		int8_t mana,
		int8_t health,
		int8_t additionalCards
	);

	Player(const Player& player);

	~Player();

	Player& operator=(const Player& player);

	int8_t getMana() const { return mana; }
	int8_t getHealth() const { return health; }
	int8_t getAdditionalCards() const { return additionalCards; }

	void setMana(int8_t mana) {
		this->mana = mana;
	}

	void setHealth(int8_t health) {
		this->health = health;
	}

	void setAdditionalCards(int8_t additionalCards) {
		this->additionalCards = additionalCards;
	}

private:
	int8_t mana;
	int8_t health;
	int8_t additionalCards;
};

Player::Player() :
	mana(0),
	health(0),
	additionalCards(0)
{

}

Player::Player(
	int8_t mana,
	int8_t health,
	int8_t additionalCards
) :
	mana(mana),
	health(health),
	additionalCards(additionalCards)
{

}

Player::Player(const Player& player) :
	mana(player.mana),
	health(player.health),
	additionalCards(player.additionalCards)
{
}

Player::~Player() {

}

Player& Player::operator=(const Player& player) {
	if (this != &player) {
		mana = player.mana;
		health = player.health;
		additionalCards = player.additionalCards;
	}

	return *this;
}

class GameState {
public:
	GameState();

	GameState(
		StateSimulationType simType,
		int8_t opponentHealth,
		const Player& player,
		const Hand& playerHand,
		const Board& board,
		const HandCombination& handCombination,
		const string& move
	);

	GameState(const GameState& gameState);

	~GameState();

	GameState& operator=(const GameState& gameState);

	StateSimulationType getSimType() const { return simType; }
	int8_t getOpponentHealth() const { return opponentHealth; }
	Player getPlayer() const { return player; }
	Hand getPlayerHand() const { return playerHand; }
	Board getBoard() const { return board; };
	HandCombination getHandCombinaiton() const { return handCombination; }
	string getMove() const { return move; }

	void setSimType(StateSimulationType simType) {
		this->simType = simType;
	}

	void setOpponentHealth(int8_t opponentHealth) {
		this->opponentHealth = opponentHealth;
	}

	void setPlayer(const Player& player) {
		this->player = player;
	}

	void setPlayerHand(const Hand& playerHand) {
		this->playerHand = playerHand;
	}

	void setBoard(const Board& board) {
		this->board = board;
	}

	void setHandCombination(const HandCombination& handCombination) {
		this->handCombination = handCombination;
	}

	void setMove(const string& move) {
		this->move = move;
	}

	void getAllHandCombinations(HandCombinations& cardCombination) const;
	void decideSimulationType();
	void playCards();
	void playCreature(const Card& creatureCard, uint8_t cardId);
	void playItem(const Card& item, uint8_t target);
	void setItemTargets();
	void setItemTargets(const Card& card, uint8_t cardId, const BoardCard (&board)[MAX_BOARD_CREATURES]);
	void setPlayedCardInHandCombination(uint8_t cardIdx);
	void checkForItemsToPlay();
	void setPlayerHealth(const int8_t health);
	void checkForLethal();
	void overcomeBlockers(const BoardCardList& blockers);

	bool checkIfPlayerDiesNextTurn() const;
	bool validTarget(const Card& card, const BoardCard& boardCard) const;
	bool handCombinationChangesBoard(const HandCombination& handCombination) const;
	
	void performAttack(
		const int8_t attCreatureId,
		const int8_t defCreatureId,
		int8_t& attackingPlayerHealthChange,
		int8_t& defendingPlayerHealthChange
	);
	
	inline int8_t setAttackCreaturesTargets();

	int evaluate() const;

private:
	StateSimulationType simType;
	int8_t opponentHealth;
	Player player;
	Hand playerHand;
	Board board;
	HandCombination handCombination;
	string move; // TODO: may be it would be better to use const char* or some different mapping
};

GameState::GameState() :
	opponentHealth(0),
	player(),
	playerHand(),
	board(),
	move(EMPTY_STRING)
{

}

GameState::GameState(
	StateSimulationType simType,
	int8_t opponentHealth,
	const Player& player,
	const Hand& playerHand,
	const Board& board,
	const HandCombination& handCombination,
	const string& move
) :
	simType(simType),
	opponentHealth(opponentHealth),
	player(player),
	playerHand(playerHand),
	board(board),
	handCombination(handCombination)
{
}

GameState::GameState(const GameState& gameState) :
	simType(gameState.simType),
	opponentHealth(gameState.opponentHealth),
	player(gameState.player),
	playerHand(gameState.playerHand),
	board(gameState.board),
	handCombination(gameState.handCombination)
{
}

GameState::~GameState() {

}

GameState& GameState::operator=(const GameState& gameState) {
	if (this != &gameState) {
		simType = gameState.simType;
		opponentHealth = gameState.opponentHealth;
		player = gameState.player;
		playerHand = gameState.playerHand;
		board = gameState.board;
		handCombination = gameState.handCombination;
	}

	return *this;
}

void GameState::getAllHandCombinations(HandCombinations& cardCombination) const {
	playerHand.getAllCombinations(player.getMana(), simType, cardCombination);
}

void GameState::decideSimulationType() {
	bool itemGivingCharge = false;
	bool nonChargeCreaturesToSummon = false;
	bool chargeCreaturesToSummon = false;
	bool itemsToPlay = false;
	bool attacksToPerform = board.attackingCreaturesCount(Side::PLAYER) > 0;;

	// TODO: perform this for only for the Sim types that make sense
	for (int8_t cardIdx = 0; cardIdx < MAX_CARDS_IN_HAND; ++cardIdx) {
		uint8_t cardNumber = handCombination.extractProperty(cardIdx, HandCombProperty::NUMBER);

		if (0 == cardNumber || handCombination.cardPlayed(cardIdx)) {
			continue;
		}

		const Card& cardToPlay = ALL_CARDS_HOLDER.allGameCards[cardNumber];

		if (ITEM_GIVING_CHARGE == cardNumber) {
			itemGivingCharge = true;
		}

		if (CardType::CREATURE == cardToPlay.getType()) {
			if (CHARGE == cardToPlay.getAbilities()[1]) {
				chargeCreaturesToSummon = true;
			}
			else {
				nonChargeCreaturesToSummon = true;
			}
		}
		else {
			itemsToPlay = true;
		}
	}

	if (itemGivingCharge && nonChargeCreaturesToSummon) {
		simType = StateSimulationType::SUMMON_CREATURES_FOR_CHARGE;
	}
	else if (!itemGivingCharge && chargeCreaturesToSummon) {
		simType = StateSimulationType::SUMMON_CHARGE_CREATURES;
	}
	else if (StateSimulationType::FIRST_PERFORM_ATTACKS == simType) {
		// If thare are no attacks to perform, summon creatures, otherwise stay on FIRST_PERFORM_ATTACKS
		if (!attacksToPerform) {
			simType = StateSimulationType::SUMMON_CREATURES;
		}
	}
	else if (StateSimulationType::SUMMON_CREATURES == simType) {
		if (!nonChargeCreaturesToSummon) {
			simType = StateSimulationType::END;
		}
	}
	else if (itemsToPlay) {
		simType = StateSimulationType::PLAY_ITEMS;
	}
	else if (attacksToPerform) {
		simType = StateSimulationType::PERFORM_ATTACKS;
	}
	else if (nonChargeCreaturesToSummon) {
		simType = StateSimulationType::SUMMON_CREATURES;
	}
	else {
		simType = StateSimulationType::INVALID;
	}
}

void GameState::playCards() {
	for (int8_t cardIdx = 0; cardIdx < MAX_CARDS_IN_HAND; ++cardIdx) {
		uint8_t cardNumber = handCombination.extractProperty(cardIdx, HandCombProperty::NUMBER);
		uint8_t cardId = handCombination.extractProperty(cardIdx, HandCombProperty::ID);

		if (0 == cardNumber) {
			continue;
		}

		const Card& cardToPlay = ALL_CARDS_HOLDER.allGameCards[cardNumber];

		if (StateSimulationType::SUMMON_CREATURES == simType &&
			CardType::CREATURE == cardToPlay.getType()
		) {
			playCreature(cardToPlay, cardId);
			handCombination.markPlayedCard(cardIdx);
		}
	}
}

void GameState::playCreature(const Card& creatureCard, uint8_t cardId) {
	if (board.getPlayerCardsCount() < MAX_BOARD_CREATURES) {
		BoardCard boardCard(
			cardId, // game id is not stored in global array cards
			creatureCard.getAtt(),
			creatureCard.getDef(),
			creatureCard.getBitsAbilities()
		);

		board.addCard(boardCard, Side::PLAYER);
		player.setMana(player.getMana() - creatureCard.getCost());
		playerHand.removeCard(cardId);

		// apply creature effect
		player.setHealth(player.getHealth() + creatureCard.getMyHealthChange());
		player.setAdditionalCards(player.getAdditionalCards() + creatureCard.getCardDraw());
		opponentHealth += creatureCard.getOpponentHealthChange();

		move = SUMMON + SPACE + to_string(cardId) + END_EXPRESSION + SPACE;
	}
}

void GameState::playItem(const Card& item, uint8_t target) {
	board.playItem(item, target);

	player.setMana(player.getMana() - item.getCost());
	player.setHealth(player.getHealth() + item.getMyHealthChange());
	player.setAdditionalCards(player.getAdditionalCards() + item.getCardDraw());
	opponentHealth += item.getOpponentHealthChange();
}

void GameState::setItemTargets() {
	for (uint8_t cardIdx = 0; cardIdx < MAX_CARDS_IN_HAND; ++cardIdx) {
		const uint8_t number = handCombination.extractProperty(cardIdx, HandCombProperty::NUMBER);
		if (0 == number) {
			continue;
		}

		const uint8_t id = handCombination.extractProperty(cardIdx, HandCombProperty::ID);

		const Card& card = ALL_CARDS_HOLDER.allGameCards[number];

		const CardType type = card.getType();

		switch (type) {
			case CardType::RED_ITEM: {
				setItemTargets(card, id, board.getOpponentBoard());
				break;
			}
			case CardType::GREEN_ITEM: {
				setItemTargets(card, id, board.getPlayerBoard());
				break;
			}
			case CardType::BLUE_ITEM: {
				// TODO: player targets and no creture!!!
				// if player target item
				setItemTargets(card, id, board.getOpponentBoard());
				setItemTargets(card, id, board.getPlayerBoard());
				break;
			}
			default: {
				break;
			}
		}
	}
}

void GameState::setItemTargets(const Card& card, uint8_t cardId, const BoardCard(&board)[MAX_BOARD_CREATURES]) {
	for (const BoardCard& boardCard : board) {
		const uint8_t id = boardCard.extractId();

		handCombination.itemsTargets[cardId];

		if (0 == id || !validTarget(card, boardCard)) {
			break;
		}

		handCombination.itemsTargets[cardId].push_back(id);
	}
}

void GameState::setPlayedCardInHandCombination(uint8_t cardIdx) {
	handCombination.markPlayedCard(cardIdx);
}

void GameState::checkForItemsToPlay() {
	bool itemToBePlyed = false;

	for (uint8_t cardIdx = 0; cardIdx < MAX_CARDS_IN_HAND; ++cardIdx ) {
		uint8_t number = handCombination.extractProperty(cardIdx, HandCombProperty::NUMBER);
		
		if (number > 0 &&
			CardType::CREATURE != ALL_CARDS_HOLDER.allGameCards[number].getType() &&
			!handCombination.cardPlayed(cardIdx)
		) {
			itemToBePlyed = true;
			break;
		}
	}

	// If there are no items to be played, simulate attack otherwise stay on PLAY_ITEMS
	if (!itemToBePlyed) {
		simType = StateSimulationType::PERFORM_ATTACKS;
	}
}

void GameState::setPlayerHealth(const int8_t health) {
	player.setHealth(health);
}

void GameState::checkForLethal() {
	int8_t demageToLethal = board.playerDemageToLethal(opponentHealth);

	if (demageToLethal <= 0) {
		//check if lethal could be execute
		BoardCardList blockers;
		board.checkForBlockers(blockers);

		if (0 == blockers.size()) {
			board.executeDirectAttacks();
		}
		else {
			//check if hand cards could destroy the blockers
			overcomeBlockers(blockers);
		}
	}
	else if (demageToLethal <= MAX_ADDED_DEMAGE) {
		//check if cards in hand could add demage for lethal (charges or buffs or summon + charge buff)
	}
}

void GameState::overcomeBlockers(const BoardCardList& blockers) {
	// First choose the cards which can overcome a blocker
	// If the Charge buff is in hand and creture with lethal may be granded charge
	// Then Check f there is enough mana to play them

	for (const BoardCard& card : blockers) {
		// Silence
		// Demage if no ward
		// Monster charge 
	}
}

bool GameState::checkIfPlayerDiesNextTurn() const {
	const int opponentAttacks = board.getBoardSum(Side::OPPONENT, BoardSum::ATT);

	return opponentAttacks >= static_cast<int>(player.getHealth());
}

bool GameState::validTarget(const Card& item, const BoardCard& boardCard) const {
	bool notDemaging = 0 == item.getAtt() && 0 == item.getDef() && 0 == item.getMyHealthChange() && 0 == item.getOpponentHealthChange();

	int itemAbilities = item.getBitsAbilities();
	int boardCardAbilities = static_cast<int>(boardCard.extractAbilitiesBits());

	bool changesAbilities = itemAbilities & boardCardAbilities;

	return !notDemaging || changesAbilities;
}

bool GameState::handCombinationChangesBoard(const HandCombination& handCombination) const {
	bool changesBoard = false;

	for (uint8_t cardIdx = 0; cardIdx < MAX_CARDS_IN_HAND; ++cardIdx) {
		uint8_t number = handCombination.extractProperty(cardIdx, HandCombProperty::NUMBER);

		if (number == 0) {
			continue;
		}

		const Card& cardToPlay = ALL_CARDS_HOLDER.allGameCards[number];

		switch(cardToPlay.getType()) {
			case CardType::RED_ITEM: {
				for (int8_t cardIdx = 0; cardIdx < board.getOpponentCardsCount(); ++cardIdx) {
					if (validTarget(cardToPlay, board.getOpponentBoardCard(cardIdx))) {
						changesBoard = true;
						break;
					}
				}
				break;
			}
			case CardType::GREEN_ITEM: {
				for (int8_t cardIdx = 0; cardIdx < board.getPlayerCardsCount(); ++cardIdx) {
					if (validTarget(cardToPlay, board.getPlayerBoardCard(cardIdx))) {
						changesBoard = true;
						break;
					}
				}
				break;
			}
			case CardType::BLUE_ITEM: {
				for (int8_t cardIdx = 0; cardIdx < board.getPlayerCardsCount(); ++cardIdx) {
					if (validTarget(cardToPlay, board.getPlayerBoardCard(cardIdx))) {
						changesBoard = true;
						break;
					}
				}

				if (!changesBoard) {
					for (int8_t cardIdx = 0; cardIdx < board.getOpponentCardsCount(); ++cardIdx) {
						if (validTarget(cardToPlay, board.getOpponentBoardCard(cardIdx))) {
							changesBoard = true;
							break;
						}
					}
				}
				break;
			}
			default: {
				break;
			}
		}
	}

	return changesBoard;
}
 
void GameState::performAttack(
	const int8_t attCreatureId,
	const int8_t defCreatureId,
	int8_t& attackingPlayerHealthChange,
	int8_t& defendingPlayerHealthChange
) {
	board.performAttack(attCreatureId, defCreatureId, attackingPlayerHealthChange, defendingPlayerHealthChange);
}

inline int8_t GameState::setAttackCreaturesTargets() {
	return board.setAttackCreaturesTargets();
}

int GameState::evaluate() const {
	int evaluation = 0;
	const int playerHealth = player.getHealth();

	if ((opponentHealth <= 0) && (playerHealth > 0)) {
		evaluation = INT_MAX;
	}
	else if ((opponentHealth > 0) && (playerHealth <= 0)) {
		evaluation = INT_MIN;
	}
	else if (checkIfPlayerDiesNextTurn()) {
		evaluation = INT_MIN;
	}
	else {
		const int healthDiff = playerHealth - opponentHealth;
		evaluation += healthDiff * Weights::HEALTH_DIFF;

		const int creaturesCountDiff = board.getPlayerCardsCount() - board.getOpponentCardsCount();
		evaluation += creaturesCountDiff * Weights::CREATURES_COUNT_DIFF;

		const int playerAttacks = board.getBoardSum(Side::PLAYER, BoardSum::ATT);
		const int opponentAttacks = board.getBoardSum(Side::OPPONENT, BoardSum::DEF);
		const int attacksDiff = playerAttacks - opponentAttacks;
		evaluation += attacksDiff * Weights::ATTACKS_DIFF;

		const int playerDefenses = board.getBoardSum(Side::PLAYER, BoardSum::ATT);
		const int oppenentDefenses = board.getBoardSum(Side::OPPONENT, BoardSum::DEF);
		const int defensesDiff = playerDefenses - oppenentDefenses;
		evaluation += defensesDiff * Weights::DEFENSES_DIFF;

		evaluation += player.getAdditionalCards() * Weights::HAND_CARDS;
	}

	return evaluation;
}

typedef int NodeId;
static constexpr NodeId INVALID_NODE_ID = -1;

typedef queue<NodeId> NodesQueue;
typedef vector<NodeId> NodesVector;

class Node {
public:
	Node();

	Node(
		NodeId nodeId,
		NodeId parentId,
		const GameState& gameState
	);

	~Node();

	NodeId getId() const {
		return id;
	}

	NodeId getParentId() const {
		return parentId;
	}

	GameState* getGameState() {
		return &gameState;
	}

	void setId(NodeId id) { this->id = id; }
	void setParentId(NodeId parentId) { this->parentId = parentId; }
	void setGameState(const GameState& gameState) { this->gameState = gameState; }

private:
	NodeId id;
	NodeId parentId;

	GameState gameState;
};

Node::Node() :
	id(INVALID_ID),
	parentId(INVALID_ID),
	gameState()
{

}

Node::Node(
	NodeId id,
	NodeId parentId,
	const GameState& gameState
) :
	id(id),
	parentId(parentId),
	gameState(gameState)
{

}

Node::~Node() {

}

typedef vector<NodeId> ChildrenList;
typedef map<NodeId, ChildrenList> GraphMap;
typedef map<NodeId, Node*> IdNodeMap;

class Graph {
public:
	Graph();
	~Graph();

	int getNodesCount() const {
		return nodesCount;
	}

	GraphMap getGraph() const {
		return graph;
	}

	IdNodeMap getIdNodeMap() const {
		return idNodeMap;
	}

	Node* getNode(NodeId nodeId) const {
		return idNodeMap.at(nodeId);
	}

	void setNodesCount(int nodesCount) { this->nodesCount = nodesCount; }
	void setGraph(GraphMap graph) { this->graph = graph; }
	void setIdNodeMap(IdNodeMap idNodeMap) { this->idNodeMap = idNodeMap; }

	void addEdge(NodeId parentId, NodeId childId);

	NodeId createNode(
		NodeId parentId,
		const GameState& gameState
	);

	void clear();
	bool nodeCreated(NodeId nodeId) const;
	void deleteAllNodes();
	bool edgeExists(NodeId parent, NodeId child) const;
	vector<NodeId> backtrack(NodeId from, NodeId to) const;

private:
	int nodesCount;
	GraphMap graph;

	// Map used to store all nodes, used to check if node is already created
	// and for easy accesss when deleteing memory pointed by each node
	IdNodeMap idNodeMap;
};

Graph::Graph() :
	nodesCount(0),
	graph(),
	idNodeMap()
{

}

Graph::~Graph() {
	deleteAllNodes();
	graph.clear();
}

void Graph::deleteAllNodes() {
	for (IdNodeMap::iterator it = idNodeMap.begin(); it != idNodeMap.end(); ++it) {
		Node* node = it->second;

		if (node) {
			delete node;
			node = NULL;
		}
	}

	idNodeMap.clear();
	nodesCount = 0;
}

bool Graph::edgeExists(NodeId parent, NodeId child) const {
	bool res = false;

	if (nodeCreated(parent) && nodeCreated(child)) {
		ChildrenList children = graph.at(parent); // TODO: copying do not copy use * for children
		res = find(children.begin(), children.end(), child) != children.end();
	}

	return res;
}

vector<NodeId> Graph::backtrack(NodeId from, NodeId to) const {
	vector<NodeId> path;

	while (from != to) {
		path.push_back(from);
		from = idNodeMap.at(from)->getParentId();
	}

	path.push_back(to);

	reverse(path.begin(), path.end());

	return path;
}

void Graph::addEdge(NodeId parentId, NodeId childId) {
	graph[parentId].push_back(childId);
}

NodeId Graph::createNode(
	NodeId parentId,
	const GameState& gameState
) {
	NodeId nodeId = nodesCount;

	if (!nodeCreated(nodeId)) {
		Node* node = new Node(nodeId, parentId, gameState);
		idNodeMap[nodeId] = node;
		graph[nodeId];

		if (INVALID_NODE_ID != parentId) {
			graph[parentId].push_back(nodeId);
		}
		++nodesCount;
	}

	return nodeId;
}

void Graph::clear() {
	deleteAllNodes();
	graph.clear();
}

bool Graph::nodeCreated(NodeId nodeId) const {
	return idNodeMap.end() != idNodeMap.find(nodeId);
}

class GameTree {
public:
	GameTree();

	~GameTree();

	GameState getTurnState() const { return turnState; }

	void setTurnState(const GameState& turnState) {
		this->turnState = turnState;
	}

	void build();
	void createHandCombinationsChildren(Node* parent, GameState* parentState, NodesQueue& children);
	void createSummonChargeChildren(Node* parent, GameState* parentState, NodesQueue& children);
	void createSummonCreaturesChildren(Node* parent, GameState* parentState, NodesQueue& children);
	void createPlayItemsChildren(Node* parent, GameState* parentState, NodesQueue& children);
	void createPerformAttacksChildren(Node* parent, GameState* parentState, NodesQueue& children);
	void createSummonCreaturesFromHandCombChildren(Node* parent, GameState* parentState, NodesQueue& children);
	void createSummonCreaturesChildren(const HandCombination& handCombination, Node* parent, GameState* parentState, NodesQueue& children);
	void createPlayedCardsChildren(NodeId parentId, NodesQueue& children);
	void playItems();
	void performAttacks();
	void summonCreatures();
	void reset();
	void evaluateState(const GameState& state, const NodeId nodeId);

	string getBestMoves() const;

private:
	GameState turnState;
	NodeId bestNode;
	int bestEvaluation;
	HandCombinations cardCombinations;

	Graph gameTree;
};

GameTree::GameTree() :
	turnState(),
	bestNode(INVALID_NODE_ID),
	bestEvaluation(INT_MIN),
	cardCombinations(),
	gameTree()
{

}

GameTree::~GameTree() {

}

void GameTree::build() {
	const NodeId rootId = gameTree.createNode(INVALID_NODE_ID, turnState);

	NodesQueue nodesQueue;
	nodesQueue.push(rootId);

	while (!nodesQueue.empty()) {
		const NodeId parentId = nodesQueue.front();
		nodesQueue.pop();

		createPlayedCardsChildren(parentId, nodesQueue);
	}
}

void GameTree::createHandCombinationsChildren(Node* parent, GameState* parentState, NodesQueue& children) {
	turnState.getAllHandCombinations(cardCombinations);

#ifdef DEBUG_BATTLE
	for (const HandCombination& handCombination : cardCombinations) {
		handCombination.debug();
	}
#endif

	bool nonChangingBoardState = false;

	for (const HandCombination& handCombination : cardCombinations) {
		if (parentState->handCombinationChangesBoard(handCombination)) {
			NodeId childNodeId = gameTree.createNode(parent->getId(), *parentState);
			GameState* childState = gameTree.getNode(childNodeId)->getGameState();

			childState->setMove(EMPTY_STRING);
			childState->setHandCombination(handCombination);
			childState->decideSimulationType();

			children.push(childNodeId);
		}
		else {
			nonChangingBoardState = true;
		}
	}

	if (nonChangingBoardState) {
		NodeId childNodeId = gameTree.createNode(parent->getId(), *parentState);
		GameState* childState = gameTree.getNode(childNodeId)->getGameState();

		childState->setMove(EMPTY_STRING);
		childState->setSimType(StateSimulationType::FIRST_PERFORM_ATTACKS);

		children.push(childNodeId);
	}
}

void GameTree::createSummonChargeChildren(Node* parent, GameState* parentState, NodesQueue& children) {
	const HandCombination& handCombination = parentState->getHandCombinaiton();

	for (uint8_t cardIdx = 0; cardIdx < MAX_CARDS_IN_HAND; ++cardIdx) {
		if (handCombination.cardPlayed(cardIdx)) {
			continue;
		}

		uint8_t cardNumber = handCombination.extractProperty(cardIdx, HandCombProperty::NUMBER);
		uint8_t cardId = handCombination.extractProperty(cardIdx, HandCombProperty::ID);

		if (0 == cardNumber) {
			continue;
		}

		const Card& card = ALL_CARDS_HOLDER.allGameCards[cardNumber];

		if (CHARGE == card.getAbilities()[1]) {
			NodeId childNodeId = gameTree.createNode(parent->getId(), *parentState);
			GameState* childState = gameTree.getNode(childNodeId)->getGameState();

			childState->playCreature(card, cardId);
			childState->setPlayedCardInHandCombination(cardIdx);
			childState->decideSimulationType();

			children.push(childNodeId);
		}
	}
}

void GameTree::createSummonCreaturesChildren(Node* parent, GameState* parentState, NodesQueue& children) {
	GameState* parentParent = gameTree.getNode(parent->getParentId())->getGameState();
	if (StateSimulationType::FIRST_PERFORM_ATTACKS == parentParent->getSimType()) {
		createSummonCreaturesFromHandCombChildren(parent, parentState,children);
	}
	else {
		const HandCombination& handCombination = parentState->getHandCombinaiton();
		createSummonCreaturesChildren(handCombination, parent, parentState, children);
	}
}

void GameTree::createSummonCreaturesFromHandCombChildren(Node* parent, GameState* parentState, NodesQueue& children) {
	HandCombinations handCombinations;
	parentState->getAllHandCombinations(handCombinations);

	for (const HandCombination& handCombination : handCombinations) {
		createSummonCreaturesChildren(handCombination, parent, parentState, children);
	}
}

void GameTree::createPlayItemsChildren(Node* parent, GameState* parentState, NodesQueue& children) {
	const HandCombination& handCombination = parentState->getHandCombinaiton();
	
	for (uint8_t cardIdx = 0; cardIdx < MAX_CARDS_IN_HAND; ++cardIdx) {
		if (handCombination.cardPlayed(cardIdx)) {
			continue;
		}
	
		uint8_t cardNumber = handCombination.extractProperty(cardIdx, HandCombProperty::NUMBER);
		uint8_t cardId = handCombination.extractProperty(cardIdx, HandCombProperty::ID);
	
		if (0 == cardNumber) {
			continue;
		}

		const Card& card = ALL_CARDS_HOLDER.allGameCards[cardNumber];
		if (card.getType() != CardType::CREATURE) {
			const vector<uint8_t>& itemTargets = handCombination.itemsTargets.at(cardId);
			// each target, for item, makes new state
			for (uint8_t target : itemTargets) {
				NodeId childNodeId = gameTree.createNode(parent->getId(), *parentState);
				GameState* childState = gameTree.getNode(childNodeId)->getGameState();
	
				childState->playItem(card, target);
				childState->setHandCombination(handCombination);
				childState->setPlayedCardInHandCombination(cardIdx);
				childState->decideSimulationType();
				childState->setMove(USE + SPACE + to_string(cardId) + SPACE + to_string(target) + END_EXPRESSION);
	
				evaluateState(*childState, childNodeId);
				children.push(childNodeId);
			}
		}
	}
}

void GameTree::createPerformAttacksChildren(Node* parent, GameState* parentState, NodesQueue& children) {
	parentState->setAttackCreaturesTargets();
	const Board& board = parentState->getBoard(); // TODO: remove copying

	// for all attack creatures and all targets make states 
	for (int8_t attCreatureIdx = 0; attCreatureIdx < board.getPlayerCardsCount(); ++attCreatureIdx) {
		const BoardCard& playerBoardCard = board.getPlayerBoardCard(attCreatureIdx);
	
		if (playerBoardCard.hasAbility(CardMasks::CAN_ATTACK)) {
			const AttackTargets& targets = playerBoardCard.getTargets();
			for (int8_t targetId : targets) {
				NodeId childNodeId = gameTree.createNode(parent->getId(), *parentState);
				GameState* childState = gameTree.getNode(childNodeId)->getGameState();
	
				int8_t attackingPlayerHealthChange = 0;
				int8_t defendingPlayerHealthChange = 0;
				int8_t attCreatureId = playerBoardCard.extractId();
				childState->performAttack(attCreatureId, targetId, attackingPlayerHealthChange, defendingPlayerHealthChange);
				childState->setPlayerHealth(childState->getPlayer().getHealth() + attackingPlayerHealthChange);
				childState->setOpponentHealth(childState->getOpponentHealth() + defendingPlayerHealthChange);
				childState->decideSimulationType();
				childState->setMove(ATTACK + SPACE + to_string(attCreatureId) + SPACE + to_string(targetId) + END_EXPRESSION);
	
				evaluateState(*childState, childNodeId);
				children.push(childNodeId);
			}
		}
	}
}

void GameTree::createSummonCreaturesChildren(const HandCombination& handCombination, Node* parent, GameState* parentState, NodesQueue& children) {
	for (uint8_t cardIdx = 0; cardIdx < MAX_CARDS_IN_HAND; ++cardIdx) {
		if (handCombination.cardPlayed(cardIdx)) {
			continue;
		}

		uint8_t cardNumber = handCombination.extractProperty(cardIdx, HandCombProperty::NUMBER);
		uint8_t cardId = handCombination.extractProperty(cardIdx, HandCombProperty::ID);

		if (0 == cardNumber) {
			continue;
		}

		const Card& card = ALL_CARDS_HOLDER.allGameCards[cardNumber];

		if (CardType::CREATURE == card.getType()) {
			NodeId childNodeId = gameTree.createNode(parent->getId(), *parentState);
			GameState* childState = gameTree.getNode(childNodeId)->getGameState();

			childState->playCreature(card, cardId);
			childState->setHandCombination(handCombination);
			childState->setPlayedCardInHandCombination(cardIdx);
			childState->decideSimulationType();

			evaluateState(*childState, childNodeId);
			children.push(childNodeId);
		}
	}
}

void GameTree::createPlayedCardsChildren(NodeId parentId, NodesQueue& children) {
	Node* parent = gameTree.getNode(parentId);
	GameState* parentState = parent->getGameState();

	switch (parentState->getSimType()) {
		case StateSimulationType::INVALID: {
			createHandCombinationsChildren(parent, parentState, children);
			break;
		}
		case StateSimulationType::SUMMON_CHARGE_CREATURES: {
			createSummonChargeChildren(parent, parentState, children);
			break;
		}
		case StateSimulationType::SUMMON_CREATURES_FOR_CHARGE: {
			createSummonCreaturesChildren(parent, parentState, children);
			break;
		}
		case StateSimulationType::PLAY_ITEMS: {
			createPlayItemsChildren(parent, parentState, children);
			break;
		}
		case StateSimulationType::PERFORM_ATTACKS: {
			createPerformAttacksChildren(parent, parentState, children);
			break;
		}
		case StateSimulationType::FIRST_PERFORM_ATTACKS: {
			createPerformAttacksChildren(parent, parentState, children);
			break;
		}
		case StateSimulationType::SUMMON_CREATURES: {
			createSummonCreaturesChildren(parent, parentState, children);
			break;
		}
		case StateSimulationType::END: {
			break; // End of simulation reached
		}
		default: {
			evaluateState(*parentState, parentId);
			break;
		}
	}

	//if (StateSimulationType::PLAY_CREATURES == parentState->getSimType()) {
	//	HandCombinations cardCombinations;
	//	parentState->getAllHandCombinations(cardCombinations);
	//
	//	for (size_t combIdx = 0; combIdx < cardCombinations.size(); ++combIdx) {
	//		NodeId childNodeId = gameTree.createNode(parent->getId(), *parentState);
	//		GameState* childState = gameTree.getNode(childNodeId)->getGameState();
	//
	//		childState->setMove(EMPTY_STRING);
	//		childState->setHandCombination(cardCombinations[combIdx]);
	//		childState->playCards();
	//		childState->setItemTargets();
	//		childState->setSimType(StateSimulationType::PLAY_ITEMS);
	//
	//		children.push(childNodeId);
	//	}
	//}
	//else if (StateSimulationType::PLAY_ITEMS == parentState->getSimType()) {
	//	const HandCombination& handCombination = parentState->getHandCombinaiton();
	//	bool itemPlayed = false;
	//
	//	for (uint8_t cardIdx = 0; cardIdx < MAX_CARDS_IN_HAND; ++cardIdx) {
	//		if (handCombination.cardPlayed(cardIdx)) {
	//			continue;
	//		}
	//
	//		uint8_t cardNumber = handCombination.extractProperty(cardIdx, HandCombProperty::NUMBER);
	//		uint8_t cardId = handCombination.extractProperty(cardIdx, HandCombProperty::ID);
	//
	//		if (cardNumber > 0) {
	//			const Card& card = ALL_CARDS_HOLDER.allGameCards[cardNumber];
	//			if (card.getType() != CardType::CREATURE) {
	//				const vector<uint8_t>& itemTargets = handCombination.itemsTargets.at(cardId);
	//				// each target, for item, makes new state
	//				for (uint8_t target : itemTargets) {
	//					NodeId childNodeId = gameTree.createNode(parent->getId(), *parentState);
	//					GameState* childState = gameTree.getNode(childNodeId)->getGameState();
	//
	//					childState->playItem(card, target);
	//					childState->setHandCombination(handCombination);
	//					childState->setPlayedCardInHandCombination(cardIdx);
	//					childState->checkForItemsToPlay();
	//					childState->setMove(USE + SPACE + to_string(cardId) + SPACE + to_string(target) + END_EXPRESSION);
	//
	//					children.push(childNodeId);
	//
	//					itemPlayed = true;
	//				}
	//			}
	//		}
	//	}
	//
	//	if (!itemPlayed) {
	//		// Dummy state to enforece performing attacks
	//		NodeId childNodeId = gameTree.createNode(parent->getId(), *parentState);
	//		GameState* childState = gameTree.getNode(childNodeId)->getGameState();
	//		childState->setSimType(StateSimulationType::PERFORM_ATTACKS);
	//		childState->setMove(EMPTY_STRING);
	//		children.push(childNodeId);
	//	}
	//}
	//else if (StateSimulationType::PERFORM_ATTACKS == parentState->getSimType()) {
	//	// first fill all creatures targets
	//	int8_t allTargetsCount = parentState->setAttackCreaturesTargets();
	//
	//	if (0 == allTargetsCount) {
	//		// No targets to attack
	//		parentState->setSimType(StateSimulationType::EVALUATE);
	//
	//		int evaluation = parentState->evaluate();
	//		if (evaluation > bestEvaluation) {
	//			bestEvaluation = evaluation;
	//			bestNode = parentId;
	//		}
	//	}
	//	else {
	//		const Board& board = parentState->getBoard();
	//
	//		// for all attack creatures and all targets make states 
	//		for (int8_t attCreatureIdx = 0; attCreatureIdx < board.getPlayerCardsCount(); ++attCreatureIdx) {
	//			const BoardCard& playerBoardCard = board.getPlayerBoardCard(attCreatureIdx);
	//
	//			if (playerBoardCard.hasAbility(CardMasks::CAN_ATTACK)) {
	//				const AttackTargets& targets = playerBoardCard.getTargets();
	//				for (int8_t targetId : targets) {
	//					NodeId childNodeId = gameTree.createNode(parent->getId(), *parentState);
	//					GameState* childState = gameTree.getNode(childNodeId)->getGameState();
	//
	//					int8_t attackingPlayerHealthChange = 0;
	//					int8_t defendingPlayerHealthChange = 0;
	//					int8_t attCreatureId = playerBoardCard.extractId();
	//					childState->performAttack(attCreatureId, targetId, attackingPlayerHealthChange, defendingPlayerHealthChange);
	//					childState->setPlayerHealth(childState->getPlayer().getHealth() + attackingPlayerHealthChange);
	//					childState->setOpponentHealth(childState->getOpponentHealth() + defendingPlayerHealthChange);
	//					childState->setMove(ATTACK + SPACE + to_string(attCreatureId) + SPACE + to_string(targetId) + END_EXPRESSION);
	//
	//					children.push(childNodeId);
	//				}
	//			}
	//		}
	//	}
	//}
}

void GameTree::playItems() {

}

void GameTree::performAttacks() {

}

void GameTree::summonCreatures() {

}

void GameTree::reset() {
	gameTree.clear();

	bestNode = INVALID_NODE_ID;
	bestEvaluation = INT_MIN;
}

void GameTree::evaluateState(const GameState& state, const NodeId nodeId) {
	const int evaluation = state.evaluate();
	if (evaluation > bestEvaluation) {
		bestEvaluation = evaluation;
		bestNode = nodeId;
	}
}

string GameTree::getBestMoves() const {
	string bestMoves = EMPTY_STRING;

	if (INVALID_NODE_ID != bestNode) {
		Node* currentNode = gameTree.getNode(bestNode);
		NodeId parentId = currentNode->getParentId();

		while (INVALID_NODE_ID != parentId) {
			bestMoves = currentNode->getGameState()->getMove() + bestMoves;
			currentNode = gameTree.getNode(parentId);
			parentId = currentNode->getParentId();
		}
	}

	return bestMoves;
}

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
	void addCard(const Card& card);
	void addDraftCard(const CardValue& draftCard);
	void addBattleCard(const Card& card);
	void makeDraftTurn();
	void makeBattleTurn();
	void createAllGameCards();

	// Provide information(player hand and board) for which to build the tree
	void initGameTree();

	Card createCard(
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

	GameState tunrState;

	Hand hand;

	// Board ([6] [6] creatures; make attacks)
	Board board;

	Player player;
	Player opponent;

	GameTree gameTree;
};

Game::Game() :
	turnsCount(0),
	draft(),
	gamePhase(GamePhase::INVALID),
	hand(),
	board(),
	gameTree()
{

}

Game::~Game() {
}

void Game::initGame() {
#ifdef DEBUG_BATTLE
	gamePhase = GamePhase::BATTLE;
#else
	gamePhase = GamePhase::DRAFT;
#endif

	createAllGameCards();
}

void Game::gameBegin() {
}

void Game::gameEnd() {
}

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

void Game::getGameInput() {
}

void Game::getTurnInput() {
	for (int i = 0; i < 2; i++) {
		int playerHealth;
		int playerMana;
		int playerDeck;
		int playerRune;
		int playerDraw;
		cin >> playerHealth >> playerMana >> playerDeck >> playerRune >> playerDraw; cin.ignore();

		if (Side::PLAYER == static_cast<Side>(i)) {
			player.setHealth(playerHealth);
			player.setMana(playerMana);
		}
		else {
			opponent.setHealth(playerHealth);
			opponent.setMana(playerMana);
		}

#ifdef OUTPUT_GAME_DATA
		cerr << playerHealth << TAB << playerMana << TAB << playerDeck << TAB << playerRune << TAB << playerDraw << endl;
#endif
	}

	int opponentHand;
	int opponentActions;
	cin >> opponentHand >> opponentActions; cin.ignore();
#ifdef OUTPUT_GAME_DATA
	cerr << opponentHand << TAB << opponentActions << endl;
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
		cerr << cardNumber << TAB << instanceId << TAB << location << TAB << cardType << TAB << cost << TAB << attack << TAB << defense << TAB << abilities << TAB << myHealthChange << TAB << opponentHealthChange << TAB << cardDraw << endl;
#endif
		const Card& card = createCard(
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
			ALL_CARDS_HOLDER.allGameCards[cardNumber].getValue()
		);

		addCard(card);
	}
}

void Game::turnBegin() {
	hand.reset();
	board.reset();
}

void Game::makeTurn() {
	if (GamePhase::DRAFT == gamePhase) {
		makeDraftTurn();
	}
	else {
		makeBattleTurn();
	}
}

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

void Game::play() {
	initGame();
	getGameInput();
	gameBegin();
	gameLoop();
	gameEnd();
}

void Game::debug() const {
}

void Game::switchToBattlePhase() {
	gamePhase = GamePhase::BATTLE;
}

void Game::addCard(const Card& card) {
	// Set card valid targets at some point, may be not here

	if (GamePhase::DRAFT == gamePhase) {
		CardValue draftCard(card.getNumber(), card.getValue());
		addDraftCard(draftCard);
	}
	else if (GamePhase::BATTLE == gamePhase) {
		addBattleCard(card);
	}
}

void Game::addDraftCard(const CardValue& draftCard) {
	draft.addCardForChoosing(draftCard);
}

void Game::addBattleCard(const Card& card) {
	CardLocation location = card.getLocation();

	switch (location) {
		case CardLocation::PLAYER_HAND: {
			HandCard handCard(card.getNumber(), card.getId());
			hand.addCard(handCard);

			break;
		}
		case CardLocation::PLAYER_BOARD: {
			BoardCard boardCard(card.getId(), card.getAtt(), card.getDef(), card.getBitsAbilities());
			boardCard.setAbility(CardMasks::CAN_ATTACK); // Creatures on board can attack
			board.addCard(boardCard, Side::PLAYER);

			break;
		}
		case CardLocation::OPPONENT_BOARD: {
			BoardCard boardCard(card.getId(), card.getAtt(), card.getDef(), card.getBitsAbilities());
			boardCard.setAbility(CardMasks::CAN_ATTACK); // Creatures on board can attack
			board.addCard(boardCard, Side::OPPONENT);
			
			break;
		}
		default: {
			break;
		}
	}
}

void Game::makeDraftTurn() {
	draft.pick();
}

void Game::makeBattleTurn() {
	GameState turnState(
		StateSimulationType::INVALID,
		opponent.getHealth(),
		player,
		hand,
		board,
		HandCombination(),
		EMPTY_STRING
	);

	turnState.checkForLethal();

	//initGameTree();
	//gameTree.build();
	//
	//string bestMoves = gameTree.getBestMoves();
	//
	//if (EMPTY_STRING == bestMoves) {
	//	cout << PASS << endl;
	//}
	//else {
	//	cout << bestMoves << endl;
	//}
}

void Game::createAllGameCards() {
	ALL_CARDS_HOLDER.initCards();
}
void Game::initGameTree() {
	GameState turnState(
		StateSimulationType::INVALID,
		opponent.getHealth(),
		player,
		hand,
		board,
		HandCombination(),
		EMPTY_STRING
	);

	gameTree.setTurnState(turnState);
	gameTree.reset();
}

Card Game::createCard(
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
	Card card(
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

int main(int argc, char** argv) {
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

	return 0;
}

//First victory
//shufflePlayer0Seed = 7357325505330546760
//seed = -8778666055931850800
//draftChoicesSeed = -3000373932237871214
//shufflePlayer1Seed = 8230258112429480157

//shufflePlayer0Seed = 5403642507680023789
//seed = -8026924741729352700
//draftChoicesSeed = 5240035254843309687
//shufflePlayer1Seed = -4395887682863324828
