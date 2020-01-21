// Agent.h

#ifndef AGENT_H
#define AGENT_H

#include "Action.h"
#include "Percept.h"
#include "Location.h"
#include "Orientation.h"
#include <ostream>
//#include <vector>
ostream &operator<< (ostream &lhs, Location &rhs);
typedef struct AgentPercept{
	bool stench;
	bool unsearched;
	bool wumpus;
}APercept;

class Agent
{
public:
	Agent ();
	~Agent ();
	void Initialize ();
	Action Process (Percept& percept);
	void GameOver (int score);

	void push(Location loc);
	Location pop();
	Location cpop();
	void printpath();
	bool unsearchedloc(Location loc);
	void updataftergoforward();
	void updateorientation(Action action);
	void updatewumpus();
	void copyPath();
	//SearchedWorld** sworld;
	//std::vector <std::vector<Percept>> sworld;

	Location nextloc;
	APercept world[1000][1000];
	int pathlength;
	int copypathlength;

	int stenchflag;
	int grabgoldflag;

	int worldsize;
	int agentmode;
	//Location stenchloc[4];
	Location agentLocation;
	Location goldLocation;
	Location wumpusLocation;
	Location path[1000];
	Location copypath[1000];
	Orientation agentOrientation;
	bool agentHasArrow;
	bool agentHasGold;
};

#endif // AGENT_H
