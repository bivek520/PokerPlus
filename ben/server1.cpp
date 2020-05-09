//
// chat_server.cpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2018 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "dealer.h"
#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <memory>
#include <set>
#include <utility>
#include "asio.hpp"
#include "chat_message.hpp"
#include "json.hpp"
#include <string>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

using namespace std;
using asio::ip::tcp;

dealer Dealer;
//Dealer.createDeck();
//vector<int>Deck = Dealer.getDeck();
//int card1, card2, card3, card4, card5, card6, card7, card8, card9, card10;
//int hand1[5], hand2[5], hand3[5], hand4[5], hand5[5];
int hand[6][5];
int ehand[6][5];
int rankHand[6][6];
//----------------------------------------------------------------------

typedef std::deque<chat_message> chat_message_queue;

//----------------------------------------------------------------------

class player
{
public:
    virtual ~player() {}
    virtual void deliver(const chat_message& msg) = 0;
    string uuid;
    //int turn;
    int playerNo;
    bool ready = false;
    int balance = 200;
    int put=0;
    int mod=0;
    int skipStatus = false;
    string actionTaken;
};

typedef std::shared_ptr<player> player_ptr;

//----------------------------------------------------------------------
int turn=0;
int playerNumber=0;
int start;
string action;
int busted = 0;
int pot = 0;
int bid = 0;
int raise_by = 0;
int totalPlayers;
int foldPlayers = 0;
bool skip1=false;
bool skip2=false;
bool skip3=false;
bool skip4=false;
bool skip5=false;
int allIn=0;

int proceedPlayer = 0;
int winner;
int skipPlayers=0;
string phase = "b1phase";
class poker_table
{
public:
	poker_table()
	{
	Dealer.createDeck();
	Deck = Dealer.getDeck();
	}
    void join(player_ptr participant)
    {
	playerNumber++;
        cout<<"New Player has joined the room"<<endl;
        
        /*participant->playerNo=playerNum;
	participant->turn=player_turn;                          //gives turn # to each player
        player_turn++;
        playerNum++;*/
	participant->playerNo = playerNumber;
	cout<< "***************" << participant->playerNo << "***************" << endl;
/*
	    card1 = Deck.back();
            Deck.pop_back();
	    card2 = Deck.back();
	    Deck.pop_back();
            card3 = Deck.back();
	    Deck.pop_back();
            card4 = Deck.back();
	    Deck.pop_back();
            card5 = Deck.back();
	    Deck.pop_back();
	    card6 = Deck.back();
            Deck.pop_back();
	    card7 = Deck.back();
	    Deck.pop_back();
            card8 = Deck.back();
	    Deck.pop_back();
            card9 = Deck.back();
	    Deck.pop_back();
            card10 = Deck.back();
	    Deck.pop_back();

		*/
	    /*
		hand1[0] =card1;
		hand1[1] =card2;
		hand1[2] =card3;
		hand1[3] =card4;
		hand1[4] =card5;
		*/
		
		
		
	    boost::uuids::basic_random_generator<boost::mt19937> g;
            boost::uuids::uuid u=g();
	    //Dealer.organizeAndRank(hand1,rankHand1);
	    
            string playerID;
            stringstream ss;
            ss<<u;
            playerID=ss.str();
            participant->uuid = playerID;
//            participant->turn = player_turn;
		cout << "Player number " << participant->playerNo << "uuid is " << participant->uuid << endl;
		cout << "cards are ";
		for(int x = 0; x<5; x++)
		{
			hand[participant->playerNo][x] =Deck.back(); cout<<to_string(Deck.back())+" ";
            Deck.pop_back();
			ehand[participant->playerNo][x] =Deck.back();
            Deck.pop_back();
		}	
		cout<<endl;
		
		Dealer.organizeAndRank(hand[participant->playerNo],rankHand[participant->playerNo]);
	/*
            cout << "Player number " << participant->playerNo << "uuid is " << participant->uuid << endl;
	    cout << "cards are "<< card1 <<", " << card2 <<", " << card3 <<", " << card4 << ", "<<card5<<endl;
	*/
            participants_.insert(participant);
            
	    //send client their uuid and turn
            nlohmann::json to_player;
                    to_player["playerNo"] = participant->playerNo;
                    to_player["uuid"] = participant->uuid;
		    to_player["hand["+ to_string(participant->playerNo) +"][0]"]=hand[participant->playerNo][0];
                    to_player["hand["+ to_string(participant->playerNo) +"][1]"]=hand[participant->playerNo][1];
                    to_player["hand["+ to_string(participant->playerNo) +"][2]"]=hand[participant->playerNo][2];
                    to_player["hand["+ to_string(participant->playerNo) +"][3]"]=hand[participant->playerNo][3];
                    to_player["hand["+ to_string(participant->playerNo) +"][4]"]=hand[participant->playerNo][4];

		    to_player["ehand["+ to_string(participant->playerNo) +"][0]"]=ehand[participant->playerNo][0];
                    to_player["ehand["+ to_string(participant->playerNo) +"][1]"]=ehand[participant->playerNo][1];
                    to_player["ehand["+ to_string(participant->playerNo) +"][2]"]=ehand[participant->playerNo][2];
                    to_player["ehand["+ to_string(participant->playerNo) +"][3]"]=ehand[participant->playerNo][3];
                    to_player["ehand["+ to_string(participant->playerNo) +"][4]"]=ehand[participant->playerNo][4];
                    string t = to_player.dump();
                    chat_message sending;
                    if (t.size() < chat_message::max_body_length)
                    {
                        memcpy( sending.body(), t.c_str(), t.size() );
                                sending.body_length(t.size());
                                sending.encode_header();
                        participant->deliver(sending);
                    }

        

        participants_.insert(participant);
        for (auto msg: recent_msgs_)                    //CSE3310 (server)  previous chat messages are sent to a client
        {
            participant->deliver(msg);
        }
    }
    
    void leave(player_ptr participant)
    {
        participants_.erase(participant);
    }
    
    void deliver(const chat_message& msg)
    { 
        
        recent_msgs_.push_back(msg);
        while (recent_msgs_.size() > max_recent_msgs)
            recent_msgs_.pop_front();
        
        for (auto participant: participants_)       //CSE3310 (server)  messages are sent to all connected clients
        {
            participant->deliver(msg);
            //cout<<"sending to all the clients"<<endl;
        }
    }
    void deliver_to(const chat_message& msg,int deli_to)
    {
        recent_msgs_.push_back(msg);
        while (recent_msgs_.size() > max_recent_msgs)
            recent_msgs_.pop_front();
        
        for (auto participant: participants_)       //CSE3310 (server)  messages are sent to all connected clients
        {

            if(participant->playerNo==deli_to)
                participant->deliver(msg);
        }
    }
    void set_ready(bool status,int deli_to)
    {
        for (auto participant: participants_)       //CSE3310 (server)  messages are sent to all connected clients
        {

            if(participant->playerNo==deli_to)
            {
		
		nlohmann::json to_player;
		//to_player["p"+to_string(participant->playerNo)+" ready"];
                participant->ready=status;
                cout<<"This is the set_ready function\nplayer: "<<deli_to<<" status: "<<status<<endl;
            }
        }
    }
    bool allReady()                                 //checks if all the player in the servers are ready for the game
    {
        bool all_ready = true;
        for (auto participant: participants_)       //set all_ready to false if all players aren't ready
        {
            
            if(participant->ready==false)
            {
                all_ready=false;
            }
        }
        return all_ready;
    }
    int getSize()
    {
	    return participants_.size();
    }
    
private:
	vector<int>Deck;
    std::set<player_ptr> participants_;
    enum { max_recent_msgs = 100 };           //CSE3310 (server) maximum number of messages are stored
    chat_message_queue recent_msgs_;
};

//----------------------------------------------------------------------

class poker_player
: public player,
public std::enable_shared_from_this<poker_player>
{
public:
    poker_player(tcp::socket socket, poker_table& room)
    : socket_(std::move(socket)),
    room_(room)
    {
        //cout<<"poker_player constructor"<<endl;
        
    }
    
    void start()
    {
        //cout<<"poker_player start()"<<endl;
        
        room_.join(shared_from_this());
        do_read_header();
    }
    
    void deliver(const chat_message& msg)
    {
        //cout<<"poker_player deliver()"<<endl;
        
        bool write_in_progress = !write_msgs_.empty();
        write_msgs_.push_back(msg);
        if (!write_in_progress)
        {
            do_write();
            
        }
    }
    
private:
    void do_read_header()
    {
      auto self(shared_from_this());
      asio::async_read(socket_,
          asio::buffer(read_msg_.data(), chat_message::header_length),
          [this, self](std::error_code ec, std::size_t /*length*/)
          {
            if (!ec && read_msg_.decode_header())
            {
            // clear out the old buffer from the last read
            // a '\0' is a good value to make sure a string
            // is terminated
            for (unsigned int i=0;i<chat_message::max_body_length;i++)
            {
                read_msg_.body() [i] = '\0';
            }
	    	do_read_body();
            }
            else
            {
              room_.leave(shared_from_this());
            }
          });
    }

    void do_read_body()
    {
      auto self(shared_from_this());
      asio::async_read(socket_,
          asio::buffer(read_msg_.body(), read_msg_.body_length()),
          [this, self](std::error_code ec, std::size_t /*length*/)
          {
            if (!ec)
            {
                /*
                 json Davis example stuff was here
                 */
		if (turn==0)
		{	
		   nlohmann::json to_dealer = nlohmann::json::parse(std::string(read_msg_.body()));
		   nlohmann::json from_player = nlohmann::json::parse(std::string(read_msg_.body()));
		   room_.set_ready(to_dealer["ready"],shared_from_this()->playerNo);
		   totalPlayers=room_.getSize();
	           //send out ready status to all players
		   if (shared_from_this()->ready==true)
		   {
			nlohmann::json to_player;
	     		to_player["participant"]=shared_from_this()->playerNo;
			to_player["ready"]=true;
			to_player["pot"]=pot;
			if(shared_from_this()->balance >= 5)
			{
			shared_from_this()->balance-=5;
			pot+=5;
			to_player["balance"]=shared_from_this()->balance;
			}
			to_player["size"] = totalPlayers;
                        string t=to_player.dump();
                        chat_message sending;
                        if (t.size() < chat_message::max_body_length)
                        {
                           memcpy( sending.body(), t.c_str(), t.size() );
                           sending.body_length(t.size());
                           sending.encode_header();
                           room_.deliver(sending);
                        }
		   }	

		   if (room_.allReady()==false)
		   {
			cout<<"cant start game"<<endl;
			nlohmann::json to_player1;
			to_player1["start"]=false;
			//shared_from_this()->balance-=5;
			//pot=playerNumber+=5;
			to_player1["pot"]=pot;
			to_player1["balance"]=shared_from_this()->balance;
                        string t=to_player1.dump();
                        chat_message sending;
                        if (t.size() < chat_message::max_body_length)
                        {
                           memcpy( sending.body(), t.c_str(), t.size() );
                           sending.body_length(t.size());
                           sending.encode_header();
                           room_.deliver(sending);
                        }
		   }
                   else
                   {
			turn++;
			nlohmann::json to_player2;
			to_player2["start"]=true;
			to_player2["participant"]=shared_from_this()->playerNo;
			to_player2["turn"]=turn;
			to_player2["bid"]=bid;
			to_player2["pot"]=pot;
			to_player2["balance"]=shared_from_this()->balance;
                        string t=to_player2.dump();
                        chat_message sending;
                        if (t.size() < chat_message::max_body_length)
                        {
                            memcpy( sending.body(), t.c_str(), t.size() );
                            sending.body_length(t.size());
                            sending.encode_header();
                            room_.deliver(sending);
                        }
		   }
		
		}
	  
		//if turn not 0
		else
		{
			nlohmann::json to_player;
			nlohmann::json from_player = nlohmann::json::parse(std::string(read_msg_.body()));
			if (from_player["autoTurn"].empty()==false)
			{
				proceedPlayer = 0;
				skipPlayers = 0;
				foldPlayers=0;
				shared_from_this()->balance+=pot;
				action="win";
				pot = 0;
				bid = 0;
				busted=0;
				raise_by = 0;
				turn=0;
				Dealer.createDeck();
				phase = "b1phase";
				to_player["getAni"] = " ";
				vector<int> Deck = Dealer.getDeck();
				for(int x = 0; x < playerNumber; x++)
				{
					for(int y = 0; y<5; y++)
					{
						hand[x][y] =Deck.back();
				    		Deck.pop_back();
						ehand[x][y] =Deck.back();
				    		Deck.pop_back();
					}
					Dealer.organizeAndRank(hand[x],rankHand[x]);
				}
				
				for(int x = 0; x < playerNumber; x++)
				{
					for(int y = 0; y<5; y++)
					{
					to_player["hand["+ to_string(x) +"]["+to_string(y)+"]"]=hand[x][y];
					to_player["ehand["+ to_string(x) +"]["+to_string(y)+"]"]=ehand[x][y];
					}
				}
				string t=to_player.dump();
		                chat_message sending;
		                if (t.size() < chat_message::max_body_length)
		                {
		                    memcpy( sending.body(), t.c_str(), t.size() );
		                    sending.body_length(t.size());
		                    sending.encode_header();
		                    room_.deliver(sending);
		                }
					
			}
			if (from_player["ani"].empty()==false)
			{
				cout<<"ANI FROM "+ to_string(shared_from_this()->playerNo)<<endl;
				shared_from_this()->put=0;
				int tempAni=from_player["ani"];
				if(tempAni <= 0)
				{
	
				}
				else
				{
				shared_from_this()->balance-=tempAni;
				pot+=tempAni;
				cout<<"POT: "+ to_string(pot)<<endl;
				if(shared_from_this()->playerNo==1)
				{skip1=false;}
				if(shared_from_this()->playerNo==2)
				{skip2=false;}
				if(shared_from_this()->playerNo==3)
				{skip3=false;}
				if(shared_from_this()->playerNo==4)
				{skip4=false;}
				if(shared_from_this()->playerNo==5)
				{skip5=false;}turn = 0;
				}
				turn = 0;
			}
			if(from_player["action"].empty()==false && from_player["action"]=="bustedOut")
			{
			shared_from_this()->put=0;
			busted+=1;
			cout<<"BUSTED: "+to_string(busted)<<endl;
			if(shared_from_this()->playerNo==1)
				{skip1=true;cout<<"SKIP1 SET"<<endl;}
			if(shared_from_this()->playerNo==2)
				{skip2=true;cout<<"SKIP2 SET"<<endl;}
			if(shared_from_this()->playerNo==3)
				{skip3=true;cout<<"SKIP3 SET"<<endl;}
			if(shared_from_this()->playerNo==4)
				{skip4=true;cout<<"SKIP4 SET"<<endl;}
			if(shared_from_this()->playerNo==5)
				{skip5=true;cout<<"SKIP5 SET"<<endl;}
			turn = 0;
			skipPlayers = busted;
			}
			turn++;
			if (turn>=(playerNumber+1))turn=1;
			if (skip1==true && turn==1)
			       {proceedPlayer+=1;turn++;cout<<"SKIP1 USED"<<endl;}
			if (skip2==true && turn==2)                
                               {proceedPlayer+=1;turn++;cout<<"SKIP2 USED"<<endl;}
			if (skip3==true && turn==3)                
                               {proceedPlayer+=1;turn++;cout<<"SKIP3 USED"<<endl;}
			if (skip4==true && turn==4)                
                               {proceedPlayer+=1;turn++;cout<<"SKIP4 USED"<<endl;}
			if (skip5==true && turn==5)                
                               {proceedPlayer+=1;turn++;cout<<"SKIP5 USED"<<endl;}
			if (turn>=(playerNumber+1))turn=1;
		if(phase != "ephase")
		{
			
			//if(from_player["action"].empty()==false && from_player["action"] == "win")
				
		        if (from_player["action"].empty()==false)
			{
				action = from_player["action"]; 
				if(action == "bet")
				{
				proceedPlayer=1+skipPlayers;
				bid = from_player["bid"];
				shared_from_this()->put=bid;
				shared_from_this()->mod=bid;
				pot+=shared_from_this()->put;
				cout << "pot = " <<pot<<endl;
				shared_from_this()->balance-=shared_from_this()->put;
				if(shared_from_this()->balance==0)allIn+=1;
				}
				if(action == "raised")
				{
				proceedPlayer=1+skipPlayers;
				raise_by = from_player["raise_by"];
				cout << "pot = " <<pot<<endl;
				bid+=raise_by;
				if(shared_from_this()->put == 0)
				{
				shared_from_this()->put+=bid;
				shared_from_this()->balance-=bid;
				pot+=bid;
				}
				else
				{
				shared_from_this()->balance-=bid-shared_from_this()->put;
				pot+=bid-shared_from_this()->put;
				shared_from_this()->put+=bid-shared_from_this()->put;
				}
				if(shared_from_this()->balance==0){allIn+=1;proceedPlayer=1+skipPlayers;
					skipPlayers+=1;}
				}
				if(action == "called")
				{
					proceedPlayer+=1;
					if(shared_from_this()->put == 0)
					{
					shared_from_this()->put+=bid;
					shared_from_this()->balance-=bid;
					shared_from_this()->mod=bid;
					pot+=bid;
					}
					else
					{
					shared_from_this()->mod=bid-shared_from_this()->put;
					shared_from_this()->balance-=bid-shared_from_this()->put;
					pot+=bid-shared_from_this()->put;
					shared_from_this()->put+=bid-shared_from_this()->put;
					}
					if(shared_from_this()->balance == 0){allIn+=1;proceedPlayer=1+skipPlayers;
					skipPlayers+=1;}
				cout << "bid = " <<bid<<endl;
				cout << "pot = " <<pot<<endl;
				}
				if(action == "allin")
				{
					allIn+=1;
					proceedPlayer=1+skipPlayers;
					skipPlayers+=1;
					if(from_player["raise_by"].empty() == false)
					{
						raise_by = from_player["raise_by"];
						cout << "pot = " <<pot<<endl;
						bid+=raise_by;
						shared_from_this()->put+=bid;
						pot+=bid;
						shared_from_this()->balance-=bid;
					}
					if(from_player["bid"].empty() == false)
					{
						bid = from_player["bid"];
						shared_from_this()->put=bid;
						pot+=shared_from_this()->put;
						cout << "pot = " <<pot<<endl;
						shared_from_this()->balance-=shared_from_this()->put;
					}
					
				}
				if(action == "folded")
				{
					skipPlayers+=1;
					proceedPlayer+=1;
					foldPlayers+=1;
					if (shared_from_this()->playerNo==1)
						{rankHand[1][0]=0;skip1=true;}
					if (shared_from_this()->playerNo==2)
		                                {rankHand[2][0]=0;skip2=true;}
					if (shared_from_this()->playerNo==3)
		                                {rankHand[3][0]=0;skip3=true;}
					if (shared_from_this()->playerNo==4)
		                                {rankHand[4][0]=0;skip4=true;}
					if (shared_from_this()->playerNo==5)
		                                {rankHand[5][0]=0;skip5=true;}
					shared_from_this()->skipStatus=true;
				}
				if(action == "checked") proceedPlayer+=1;
				if (turn>=(playerNumber+1))turn=1;
				
			}
		}
		else{
			if (from_player["hand["+ to_string(shared_from_this()->playerNo) +"][0]"].empty() == false)
				hand[shared_from_this()->playerNo][0] = from_player["hand["+ to_string(playerNo) +"][0]"];
			if (from_player["hand["+ to_string(shared_from_this()->playerNo) +"][1]"].empty() == false)
			        hand[shared_from_this()->playerNo][1] = from_player["hand["+ to_string(playerNo) +"][1]"];
			if (from_player["hand["+ to_string(shared_from_this()->playerNo) +"][2]"].empty() == false)
			        hand[shared_from_this()->playerNo][2] = from_player["hand["+ to_string(playerNo) +"][2]"];
			if (from_player["hand["+ to_string(shared_from_this()->playerNo) +"][3]"].empty() == false)
			        hand[shared_from_this()->playerNo][3] = from_player["hand["+ to_string(playerNo) +"][3]"];
			if (from_player["hand["+ to_string(shared_from_this()->playerNo) +"][4]"].empty() == false)
			        hand[shared_from_this()->playerNo][4] = from_player["hand["+ to_string(playerNo) +"][4]"];
			if (from_player["gimmieCards"].empty() == false)
			{
				cout<<"Sending cards to player..."<<endl;
				
				Dealer.organizeAndRank(hand[shared_from_this()->playerNo],rankHand[shared_from_this()->playerNo]);
			    	if(skip1 == true) rankHand[1][0]=0;
				if(skip2 == true) rankHand[2][0]=0;
				if(skip3 == true) rankHand[3][0]=0;
				if(skip4 == true) rankHand[4][0]=0;
				if(skip5 == true) rankHand[5][0]=0;

				to_player["turn"]=turn;
				to_player["participant"]=shared_from_this()->playerNo;
				to_player["balance"]=shared_from_this()->balance;
				to_player["pot"]=pot;
				to_player["hand["+ to_string(shared_from_this()->playerNo) +"][0]"]=hand[shared_from_this()->playerNo][0];
				to_player["hand["+ to_string(shared_from_this()->playerNo) +"][1]"]=hand[shared_from_this()->playerNo][1];
				to_player["hand["+ to_string(shared_from_this()->playerNo) +"][2]"]=hand[shared_from_this()->playerNo][2];
			 	to_player["hand["+ to_string(shared_from_this()->playerNo) +"][3]"]=hand[shared_from_this()->playerNo][3];
				to_player["hand["+ to_string(shared_from_this()->playerNo) +"][4]"]=hand[shared_from_this()->playerNo][4];
				string t=to_player.dump();
			        chat_message sending;
	                        if (t.size() < chat_message::max_body_length)
	                        {
	                                memcpy( sending.body(), t.c_str(), t.size() );
	                                sending.body_length(t.size());
	                                sending.encode_header();
	                                room_.deliver(sending);
	                        }
			}
		        if (from_player["action"].empty()==false)
			{
			
				to_player["turn"]=turn;
				to_player["participant"]=shared_from_this()->playerNo;
				to_player["balance"]=shared_from_this()->balance;
				
				shared_from_this()->put=0;
				action = from_player["action"];
				cout<<"RECEIVED EXCHANGE: " + action<<endl;
				if(action == "exchanged")
				{
				proceedPlayer+=1;
				}
			}
		}
		nlohmann::json to_player2;
		
		cout<<"PROCEED: " + to_string(proceedPlayer)<<endl;
		//send turn to all players
		//if(phase=="ephase"){cout<<"IN EPHASE"<<endl;}
		/*
		if (shared_from_this()->balance==0 && turn==1)
		       {proceedPlayer+=1;turn++;}
		else if (shared_from_this()->balance==0 && turn==2)                
                       {proceedPlayer+=1;turn++;}
		else if (shared_from_this()->balance==0 && turn==3)                
                       {proceedPlayer+=1;turn++;}
		else if (shared_from_this()->balance==0 && turn==4)                
                       {proceedPlayer+=1;turn++;}
		else if (shared_from_this()->balance==0 && turn==5)                
                       {proceedPlayer+=1;turn++;}
		*/
		to_player2["action"]=action;
                to_player2["participant"]=shared_from_this()->playerNo;
		to_player2["raise_by"]=raise_by;
		to_player2["pot"]=pot;	
		to_player2["bid"]=bid;
		to_player2["balance"]=shared_from_this()->balance;
		
		if(proceedPlayer >= playerNumber && phase == "ephase")
		{
		cout<<"EPHASE OVER"<<endl;
		phase = "b2phase";
		to_player2["start"]=true;
		proceedPlayer = 0;
		/*
		if (shared_from_this()->balance==0 && turn==1)
		       {proceedPlayer+=1;turn++;}
		if (shared_from_this()->balance==0 && turn==2)                
                       {proceedPlayer+=1;turn++;}
		if (shared_from_this()->balance==0 && turn==3)                
                       {proceedPlayer+=1;turn++;}
		if (shared_from_this()->balance==0 && turn==4)                
                       {proceedPlayer+=1;turn++;}
		if (shared_from_this()->balance==0 && turn==5)                
                       {proceedPlayer+=1;turn++;}
		*/
		}
		if((proceedPlayer >= playerNumber && phase == "b2phase") || foldPlayers >= playerNumber-1 || (allIn >= playerNumber-foldPlayers && phase == "b2phase"))
		{
			cout<<"B2PHASE OVER"<<endl;
			cout << "FOLD PLAYERS: "+to_string(foldPlayers)<<endl;
			int winner = Dealer.compareHands(rankHand[1], rankHand[2], rankHand[3], rankHand[4], rankHand[5]);
			cout << "WINNER:  Player "+to_string(winner)<<endl;
			//busted = playerNumber;
			to_player2["winner"]=winner;
			skipPlayers = 0;
			foldPlayers=0;
			proceedPlayer=0;
			skip1 = false;
			skip2 = false;
			skip3 = false;
			skip4 = false;
			skip5 = false;
		}
		if(proceedPlayer >= playerNumber && phase == "b1phase")
		{	
			cout<<"POT SETTLED"<<endl;
			bid=0;
			raise_by=0;
			phase = "ephase";
			to_player2["start"]=true;
			proceedPlayer = 0;
			turn =1;
			if (skip1==true && turn==1)
			       {proceedPlayer+=1;turn++;}
			if (skip2==true && turn==2)                
		               {proceedPlayer+=1;turn++;}
			if (skip3==true && turn==3)                
		               {proceedPlayer+=1;turn++;}
			if (skip4==true && turn==4)                
		               {proceedPlayer+=1;turn++;}
			if (skip5==true && turn==5)                
		               {proceedPlayer+=1;turn++;}
		}
		if(busted>= playerNumber-1)
		{
		to_player2["theWinner"]=" ";
		}
		to_player2["mod"]=shared_from_this()->mod;
		to_player2["turn"]=turn;
		to_player2["phase"]=phase;
		//if(phase == "ephase") turn=0;
                string t=to_player2.dump();
                chat_message sending;
                if (t.size() < chat_message::max_body_length)
                {
                        memcpy( sending.body(), t.c_str(), t.size() );
                        sending.body_length(t.size());
                        sending.encode_header();
                        room_.deliver(sending);
                }
			
	}	
	    do_read_header();
            }
            else
            {
                room_.leave(shared_from_this());
            }
        });
    }
    
    void do_write()
    {
      auto self(shared_from_this());
      asio::async_write(socket_,
          asio::buffer(write_msgs_.front().data(),
            write_msgs_.front().length()),
          [this, self](std::error_code ec, std::size_t /*length*/)
          {
            if (!ec)
            {
              write_msgs_.pop_front();
              if (!write_msgs_.empty())
              {
                do_write();
              }
            }
            else
            {
              room_.leave(shared_from_this());
            }
          });
    }
    
    tcp::socket socket_;
    poker_table& room_;
    chat_message read_msg_;
    chat_message_queue write_msgs_;
};

//----------------------------------------------------------------------

class poker_game
{
public:
    poker_game(asio::io_context& io_context,
               const tcp::endpoint& endpoint)
    : acceptor_(io_context, endpoint)
    {
        //cout<<"Poker game started"<<endl;
        
        do_accept();
    }
    
private:
    void do_accept()
    {
        
        
        acceptor_.async_accept(
                               [this](std::error_code ec, tcp::socket socket)
                               {
            if (!ec)
            {
                std::make_shared<poker_player>(std::move(socket), room_)->start();
            }
            
            do_accept();
        });
    }
    
    tcp::acceptor acceptor_;
    poker_table room_;
};

//----------------------------------------------------------------------

int main(int argc, char* argv[])
{
    try
    {
        if (argc < 2)
        {
            std::cerr << "Usage: dealer <port> [<port> ...]\n";
            return 1;
        }
        
        asio::io_context io_context;
        
        std::list<poker_game> servers;
        for (int i = 1; i < argc; ++i)
        {
            tcp::endpoint endpoint(tcp::v4(), std::atoi(argv[i]));
            servers.emplace_back(io_context, endpoint);
        }
        
        io_context.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    
    return 0;
}

