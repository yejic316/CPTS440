// Agent.h

#ifndef AGENT_H
#define AGENT_H

#include "Action.h"
#include "Percept.h"
#include "WorldState.h"
#include <list>


#include "Location.h"

typedef struct Pit{
	double ptrue;
	double pfalse;
}ProbPit;

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

	void UpdatePitProb(Percept& percept);
	void AddFrontier(Percept& percept);
	void AddKnownNPitLocation(Location location);
	//bool CheckBreeze(list<Location> ftemp, Location location);
	list<Location> pathToGold;
	list<Location> stenchLocations;
	list<Location> clearLocations;
	list<Location> possibleWumpusLocations;
	list<Location> safeLocations;
	list<Location> visitedLocations;
	bool firstTry;

	ProbPit pitProb[5][5];
	list<Location> known;
	list<Location> breeze;
	list<Location> frontier;
	list<Location> pitLocation;

};

#endif // AGENT_H
