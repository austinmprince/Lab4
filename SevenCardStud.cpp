#include "stdafx.h"
#include "SevenCardStud.h"
#include "Card.h"
#include "Deck.h"
#include "Hand.h"
#include<memory>
#include <algorithm>
#include <string>
#include <istream>
#include <iostream>
#include <vector>
#include <ostream>
#include <fstream>
#include <sstream>

using namespace std;

int SevenCardStud::turn(Player &p) {
	return 0;
}
int SevenCardStud::before_turn(Player &p) {
	if (currentRound == 1) {
		Card c1 = mainDeck.getTop();
		Card c2 = mainDeck.getTop();
		Card c3 = mainDeck.getTop();
		p.sevenCardHand.push_back(c1);
		p.sevenCardHand.push_back(c2);
		p.sevenCardHand.push_back(c3);
	}
	if (currentRound > 1) {
		Card c1 = mainDeck.getTop();
		
		p.sevenCardHand.push_back(c1);
		
	}
	return 0;
	
}
int SevenCardStud::after_turn(Player &p) {
	std::cout << p.playerName << " after turn!" << std::endl;
	return 0;
}

SevenCardStud::SevenCardStud()
{
	dealer = 0;
	mainDeck = Deck();
	discardDeck = Deck();
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 13; j++) {
			Card c = Card(Suit(i), Rank(j));
			mainDeck.add_card(c);
		}
	}
	pot = 0;
	currentBet = 0;
	currentRound = 0;
	mainDeck.shuffle();
}

int SevenCardStud::before_round() {
	if (currentRound == 0) {
		std::cout << "Taking ante's from all players" << std::endl;
		std::cout << std::endl;
		for (unsigned int i = 0; i < players.size(); i++) {
			(*players[i]).chips--;
			pot++;
		}
	}
	currentRound++;
	std::cout << "************************" << std::endl;
	std::cout << "The Current Round is " << currentRound << std::endl;
	std::cout << "************************" << std::endl;
	for (unsigned int i = 0; i < players.size(); i++) {
		Player &p = *players[i];
		before_turn(p);
	}
	return 0;
}


int SevenCardStud::round() {
	int numAlive = 0;
	for (unsigned int i = 0; i < players.size(); i++) {
		if ((*players[i]).isAlive) {
			numAlive++;
		}
	}
	if (numAlive == 1) {
		return 0;
	}
	for (unsigned int i = dealer + 1; i < players.size(); i++) {
		Player & p = *players[i];
		turn(p);
		//after_turn(p);
		std::cout << std::endl;
	}
	for (unsigned int i = 0; i <= dealer; i++) {
		Player & p = *players[i];
		turn(p);
		//after_turn(p);
		std::cout << std::endl;
	}
	
	bettingRound();
	return 0;
}

int SevenCardStud::after_round() {
	if (currentRound == 5) {
		std::cout << "*****************" << std::endl;
		std::cout << "All Rounds Over" << std::endl;
		std::cout << "*****************" << std::endl;
		vector<shared_ptr<Player>> copy = players;
		// ranking logic goes here
		for (unsigned int i = 0; i < copy.size(); i++) {
			(*copy[i]).playerHand = returnBestCombination((*copy[i]));
		}
		std::sort(copy.begin(), copy.end(), &Game::rank_player_hands);
		(*copy[0]).handsWon++;
		(*copy[0]).chips += pot;
		for (unsigned int i = 1; i < copy.size(); i++) {
			(*copy[i]).handsLost++;
		}
		std::cout << "*****************" << std::endl;
		std::cout << "SHOWING HANDS" << std::endl;
		std::cout << "*****************" << std::endl;
		printAllPlayers(copy);
		discardAllCards(players);
		string target;
		int state = 1;
		while (state != 0 && numPlayers() > 0) {	// while they haven't entered 'no' and there are still players thay can be removed
			std::cout << std::endl;
			std::cout << "Would you like to remove a player? (enter 'no' to continue): " << std::endl;
			std::cin >> target;
			std::cin.clear();
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			state = removePlayers(players, target);		// returns 1 if a player was removed, returns 0 if 'no' was entered
		}

		std::cout << "Done removing players" << std::endl;
		std::cout << std::endl;

		string newTarget;
		int newState = 1;
		while (newState != 0) {	// while they haven't entered no
			std::cout << std::endl;
			std::cout << "Would you like to add a new player? (enter 'no to continue): " << std::endl;
			std::cin >> newTarget;
			std::cin.clear();
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			newState = addNewPlayers(newTarget);	// returns 1 if a player was added, returns 0 if 'no' was entered

		}

		std::cout << "Done Adding Players" << std::endl;
		dealer++;
		if (dealer > numPlayers() - 1) {
			dealer = 0;
		}
		for (unsigned int i = 0; i < players.size(); i++) {
			Player &p = *players[i];
			p.isAlive = true;
			p.lastBet = 0;
			if (p.chips == 0) {
				std::cout << p.playerName << " has no chips, reseting to 20" << std::endl;
				p.chips = 20;
			}
		}
		pot = 0;
		currentBet = 0;
		currentRound = 0;

		return 0;


	}
	std::cout << "round over" << std::endl;
	return 0;
}

int SevenCardStud::removePlayers(vector<shared_ptr<Player>> & players, string target) {
	if (target != "no") {
		shared_ptr<Player> p = find_player(target);	// find the pointer to the player
		if (p != 0) {


			string file_name = target + ".txt";	// generate file name and open it
			ofstream off(file_name);
			off << *p;	// output player info to file
			off.close();

			for (unsigned int i = 0; i < players.size(); i++) { // iterate through vector of pointers until we are at the pointer that matches find_player. Erase that pointer.
				if (p == players[i]) {
					players.erase(players.begin() + i);
					break;
				}
			}

		}
		return 1;
	}
	else return 0;

}

int SevenCardStud::addNewPlayers(string target) {
	if (target != "no") {
		try { // player might already be in the game
			add_player(target);
			std::cout << std::endl;
		}
		catch (game_errors &e) { // don't stop the game, just output that the player is already playing
			std::cout << "Player is already in the game" << std::endl;
			e;
		}
		return 1;
	}
	else return 0;


}

unsigned int bet(Player &p, SevenCardStud &g) {
	//string betInput;
	string garbage;
	std::cout << p.playerName << " your current hand is: " << std::endl;
	printNoHiddenCards(p);
	std::cout << std::endl;
	std::cout << "Everyone Else's Hand:" << std::endl;
	for (unsigned int i = 0; i < g.players.size(); i++) {
		Player x = *g.players[i];
		if (x.playerName != p.playerName) {
			printWithHiddenCards(*g.players[i]);
		}
	}
	std::cout << std::endl;
	std::cout << "Pot: " << g.pot << " , Current Bet: " << g.currentBet << ", Your Last Bet: " << p.lastBet << ", Your Chip count: " << p.chips << std::endl;
	bool validInput = false;

	std::cin.clear();
	while (!validInput) {
		string betInput;
		std::cout << "Please fold, check, call, bet, or raise (F, CH, C, B1 / B2, R1/R2)" << std::endl;
		std::cin >> betInput;
		cin.clear();
		cin.ignore(INT_MAX, '\n');
		//std::cout << betInput << std::endl;
		if (betInput == "") {
			std::cin.ignore(INT_MAX);
			std::cin.clear();
			getline(std::cin, betInput);
			std::cout << betInput << std::endl;
			std::cout << "yo" << std::endl;
		}
		if (betInput == "F") {
			p.isAlive = false;
			return g.currentBet;
		}
		if (betInput == "CH") {
			if (g.currentBet == p.lastBet) {
				return g.currentBet;
			}
			else {
				std::cout << "You cannot check right now" << std::endl;
			}
		}
		else if (betInput == "C") {
			unsigned int chipsNeeded = g.currentBet - p.lastBet;
			//std::cout << g.currentBet << "  " << p.lastBet << std::endl;
			//std::cout << chipsNeeded << std::endl;
			if (chipsNeeded <= p.chips) {
				p.chips -= chipsNeeded;
				g.pot += chipsNeeded;
				p.lastBet = g.currentBet;
				return g.currentBet;
			}
			else {
				std::cout << "Sorry, you don't have enough chips to call. Your max chips have been taken." << std::endl;
				g.pot += p.chips;
				p.chips -= p.chips;
				return g.currentBet;
			}


		}
		else if (betInput.at(0) == 'B') {
			unsigned int amount = betInput.at(1) - '0';
			if (amount == g.currentBet) {
				if (amount <= p.chips) {
					p.chips -= amount;
					g.pot += amount;
					p.lastBet = amount;
					return g.currentBet;
				}
				else {
					std::cout << "Sorry, you do not have enough chips to bet" << std::endl;
					return g.currentBet;
				}
			}
			if (amount < g.currentBet) {
				std::cout << "You must call or raise" << std::endl;
			}
			if (amount > g.currentBet && g.currentBet != 0) {
				std::cout << "It looks like you're trying to raise" << std::endl;
			}
			if (amount > g.currentBet && g.currentBet == 0) {
				p.chips -= amount;
				g.pot += amount;
				p.lastBet = amount;
				g.currentBet = amount;
				return g.currentBet;
			}
		}

		else if (betInput.at(0) == 'R') {
			int raise = betInput.at(1) - '0';
			unsigned int totalBet = g.currentBet + raise;
			if (totalBet <= p.chips) {
				p.chips -= totalBet;
				g.pot += totalBet;
				p.lastBet = totalBet;
				g.currentBet = totalBet;
				return g.currentBet;
			}
			else {
				std::cout << "Sorry, you don't have enough chips to do that" << std::endl;
			}
		}

		std::cout << "Invalid Input" << std::endl;
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}
	return 0;
}

void printNoHiddenCards(Player &p) {
	vector<Card> v = p.sevenCardHand;
	std::sort(v.begin(), v.end());
	printDeck(v, false);
}

void printWithHiddenCards(Player &p) {
	vector<Card> v = p.sevenCardHand;
	for (unsigned int i = 0; i < v.size(); i++) {
		if (i == 0 || i == 1 || i == 6) {
			std::cout << "* " ;
		}
		else {
			std::cout << printCard(v[i]) << " ";
		}

	}
	std::cout << std::endl;
}
int SevenCardStud::bettingRound() {
	bool allPlayersDone = false;
	while (!allPlayersDone) {
		for (unsigned int i = dealer + 1; i < players.size(); i++) {	// call before_turn from spot just after dealer to end of list
			Player & p = *players[i];
			if (p.isAlive && (p.lastBet < currentBet || currentBet == 0)) {
				bet(p, *this);
			}
			std::cout << std::endl;
		}
		for (unsigned int i = 0; i <= dealer; i++) {	// call before turn up to the dealer
			Player & p = *players[i];
			if (p.isAlive && (p.lastBet < currentBet || currentBet == 0)) {
				bet(p, *this);
			}
			std::cout << std::endl;
		}
		bool check = true;
		int numAlive = 0;
		for (unsigned int i = 0; i < players.size(); i++) {
			Player p = *players[i];
			if (p.isAlive) {
				numAlive++;
				if (p.lastBet == currentBet || p.chips == 0) {

				}
				else {

					check = false;
				}
			}

			else check = true;
		}


		if (numAlive == 1) {
			return 1;
		}
		allPlayersDone = check;
	}

	for (unsigned int i = 0; i < players.size(); i++) {
		Player &p = *players[i];
		p.lastBet = 0;
	}
	currentBet = 0;
	return 0;
}

void SevenCardStud::discardAllCards(vector<shared_ptr<Player>> & players) {
	vector<shared_ptr<Player>>::iterator p;
	for (p = players.begin(); p != players.end(); ++p) {
		vector<Card> &h = (**p).sevenCardHand;
		int size = h.size();
		for (int i = 0; i < size; i++) {
			discardDeck.add_card(h.back());
			h.pop_back();
		}
	}
	for (int i = 0; i < discardDeck.size(); ++i) {
		mainDeck.add_card(discardDeck.getTop());
	}
}

Hand SevenCardStud::returnBestCombination(Player &p) {
	vector<Card> copy = p.sevenCardHand;
	std::sort(copy.begin(), copy.end());
	Hand max = Hand();
	int locArray[] = { 0, 0, 0, 0, 0, 1 ,1 };
	do {
		Hand temp = Hand();
		for (unsigned int i = 0; i < copy.size(); i++) {
			if (locArray[i] == 0) {
				temp << copy[i];
			}
		}
		if (poker_rank(temp, max)) {
			max = temp;
		}
	} while (std::next_permutation( locArray, locArray + 7));
	return max;
}

void printAllPlayers(vector<shared_ptr<Player>> copy) {
	for (unsigned int i = 0; i < copy.size(); i++) {
		if ((*copy[i]).isAlive) {
			std::cout << "Player " << (*copy[i]).playerName << " : " << (*copy[i]).playerHand << " Rank(" << (*copy[i]).playerHand.getRank() << ") -> " << "Wins: "
				<< (*copy[i]).handsWon
				<< ", Loses: " << (*copy[i]).handsLost << " , Chips: " << (*copy[i]).chips << std::endl;
		}
		else {
			std::cout << "Player " << (*copy[i]).playerName << " : " << "FOLD" << " Rank(-1) " << "Wins: "
				<< (*copy[i]).handsWon
				<< ", Loses: " << (*copy[i]).handsLost << " , Chips: " << (*copy[i]).chips << std::endl;
		}

	}

}


