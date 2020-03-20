#ifndef PLAYER_H
#define PLAYER_H
#include <vector>
#include <gtkmm.h>

using namespace std;

class Player
{
    public:
        Player(char* playerIP, char* portNumber);
        void Fold(); 
        void Check();
        void Call();
        void Raise();
        void Bet();
        void All_In();
        void Timer();
        bool TimeOut();
	vector<int> getHandOrganized();
        vector<int> getSuitHand();
        vector<int> getValueHand();
	vector<int> rankingHand();
    private:
        vector<int> Hand;
        int PlayerID;
        char* PlayerIP;
        int Balance;
        int Bid;
	vector<int> OrganizedHand;
        vector<int> SuitHand;
        vector<int> ValueHand;
	vector<int> Rankings;
};

#endif
