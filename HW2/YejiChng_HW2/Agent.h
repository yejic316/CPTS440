// Agent.h

#ifndef AGENT_H
#define AGENT_H

#include "Action.h"
#include "Percept.h"
#include "Location.h"
#include "Orientation.h"

class Agent
{
public:
	Agent ();
	~Agent ();
	void Initialize ();
	Action Process (Percept& percept);
	void GameOver (int score);

	Location agentLocation;
	Orientation agentOrientation;
	bool agentHasArrow;
	bool agentHasGold;
};

#endif // AGENT_H
