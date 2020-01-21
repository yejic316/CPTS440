// Agent.h

#ifndef AGENT_H
#define AGENT_H

#include "Action.h"
#include "Percept.h"
#include "WorldState.h"
#include <list>

// HW5
#include "Location.h"

class Agent
{
public:
	Agent ();
	~Agent ();
	void Initialize ();
	Action Process (Percept& percept);
	void GameOver (int score);
	
	void UpdateState (Percept& percept);

	WorldState worldState;
	list<Action> actionList;
	Action previousAction;
	
	// HW5
	void UpdatePossibleWumpusLocations();
	void SetGoForward(Location& location);
	void AddToPath(Location& location);
	Action ChooseAction(Percept& percept);
	void AddActionsFromPath(bool forward);
	bool LocationInList(list<Location>& locationList, const Location& location);
	void AddNewLocation(list<Location>& locationList, const Location& location);
	void AddAdjacentLocations(list<Location>& locationList, const Location& location);
	bool OutsideWorld(Location& location);
	void FilterSafeLocations();
	void Output();
		
	list<Location> pathToGold;
	list<Location> stenchLocations;
	list<Location> clearLocations;
	list<Location> possibleWumpusLocations;
	list<Location> safeLocations;
	list<Location> visitedLocations;
	bool firstTry;
};

#endif // AGENT_H
