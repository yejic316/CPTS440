// Agent.cc

#include <iostream>
#include <iomanip>
#include "Agent.h"

#include "Action.h"
#include "Location.h"
#include "Orientation.h"

#include <math.h>
#include <algorithm>

using namespace std;

Agent::Agent ()
{
	firstTry = true;
}

Agent::~Agent ()
{

}

void Agent::Initialize ()
{
	worldState.agentLocation = Location(1,1);
	worldState.agentOrientation = RIGHT;
	worldState.agentHasArrow = true;
	worldState.agentHasGold = false;
	actionList.clear();
	previousAction = CLIMB; // dummy action to start

	for(int i=0;i<5;i++)
	{
		for(int j=0;j<5;j++)
		{
			pitProb[i][j].ptrue = 0.2;
			pitProb[i][j].pfalse = 0.8;
		}
	}
	pitProb[0][0].ptrue = 0.0;
	pitProb[0][0].pfalse = 1.0;
	AddNewLocation(known, Location(1,1));


	if (firstTry)
	{
		worldState.goldLocation = Location(0,0); // unknown
		worldState.worldSize = 0; // unknown
		pathToGold.push_back(Location(1,1)); // path starts in (1,1)
	}
	else
	{
		if (worldState.goldLocation == Location(0,0))
		{
			// Didn't find gold on first try (should never happen, but just in case)
			pathToGold.clear();
			pathToGold.push_back(Location(1,1));
		}
		else
		{
			AddActionsFromPath(true); // forward through path from (1,1) to gold location
		}
	}
}

Action Agent::Process (Percept& percept)
{
	UpdateState(percept);
	UpdatePitProb(percept);
	FilterSafeLocations();
	if (actionList.empty())
	{
		if (percept.Glitter)
		{
			actionList.push_back(GRAB);
			AddActionsFromPath(false); // reverse path from (1,1) to gold location
		}
		else if (worldState.agentHasGold && (worldState.agentLocation == Location(1,1)))
		{
			actionList.push_back(CLIMB);
		}
		else
		{
			actionList.push_back(ChooseAction(percept));
		}
	}
	Action action = actionList.front();
	actionList.pop_front();
	previousAction = action;
	return action;
}

void Agent::GameOver (int score)
{
	firstTry = false;
}

void Agent::UpdateState (Percept& percept)
{
	int orientationInt = (int) worldState.agentOrientation;
	switch (previousAction)
	{
	case GOFORWARD:
		if (percept.Bump)
		{
			// Check if we can determine world size
			if (worldState.agentOrientation == RIGHT)
				worldState.worldSize = worldState.agentLocation.X;
			if (worldState.agentOrientation == UP)
				worldState.worldSize = worldState.agentLocation.Y;
			if (worldState.worldSize > 0)
				FilterSafeLocations();
		}
		else
		{
			Location forwardLocation;
			SetGoForward(forwardLocation);
			worldState.agentLocation = forwardLocation;

			// If haven't found gold yet, add this location to the pathToGold
			if (worldState.goldLocation == Location(0,0))
				AddToPath(worldState.agentLocation);
		}
		break;

	case TURNLEFT:
		worldState.agentOrientation = (Orientation) ((orientationInt + 1) % 4);
		break;

	case TURNRIGHT:
		orientationInt--;
		if (orientationInt < 0)
			orientationInt = 3;
		worldState.agentOrientation = (Orientation) orientationInt;
		break;

	case GRAB:
		worldState.agentHasGold = true; // Only GRAB when there's gold
		worldState.goldLocation = worldState.agentLocation; // HW5
		break;

	case SHOOT:
		worldState.agentHasArrow = false;

		// The only situation where we shoot is if we start out in (1,1)
		// facing RIGHT, perceive a stench, and don't know the wumpus location.
		// In this case, a SCREAM percept means the wumpus is in (2,1) (and dead),
		// or if no SCREAM, then the wumpus is in (1,2) (and still alive).
		possibleWumpusLocations.clear();
		if (percept.Scream)
			possibleWumpusLocations.push_back(Location(2,1));
		else
			possibleWumpusLocations.push_back(Location(1,2));

	case CLIMB:
		break;
	}

	// Update visited locations, safe locations, stench locations, clear locations,
	// and possible wumpus locations.
	AddNewLocation(visitedLocations, worldState.agentLocation);
	AddNewLocation(safeLocations, worldState.agentLocation);
	if (percept.Stench)
	{
		AddNewLocation(stenchLocations, worldState.agentLocation);
	}
	else
	{
		AddNewLocation(clearLocations, worldState.agentLocation);
	}

	if(percept.Breeze)
	{
		AddNewLocation(breeze, worldState.agentLocation);
	}
	if( !percept.Breeze && !percept.Stench)
	{
		AddAdjacentLocations(safeLocations, worldState.agentLocation);
	}
	if (possibleWumpusLocations.size() != 1) //if we don't know wumpus location yet
		UpdatePossibleWumpusLocations();
	FilterSafeLocations();
	Output();
}


/*
 * Update possible wumpus locations based on the current set of stench locations
 * and clear locations.
 */
void Agent::UpdatePossibleWumpusLocations()
{
	list<Location> tmpLocations;
	list<Location> adjacentLocations;
	possibleWumpusLocations.clear();
	Location location1, location2;
	list<Location>::iterator itr1, itr2;
	// Determine possible wumpus locations consistent with available stench information
	for (itr1 = stenchLocations.begin(); itr1 != stenchLocations.end(); ++itr1)
	{
		location1 = *itr1;
		// Build list of adjacent locations to this stench location
		adjacentLocations.clear();
		AddAdjacentLocations(adjacentLocations, location1);
		if (possibleWumpusLocations.empty())
		{
			// Must be first stench location in list, so add all adjacent locations
			possibleWumpusLocations = adjacentLocations;
		}
		else
		{
			// Eliminate possible wumpus locations not adjacent to this stench location
			tmpLocations = possibleWumpusLocations;
			possibleWumpusLocations.clear();
			for (itr2 = tmpLocations.begin(); itr2 != tmpLocations.end(); ++itr2)
			{
				location2 = *itr2;
				if (LocationInList(adjacentLocations, location2))
					possibleWumpusLocations.push_back(location2);
			}
		}
	}
	// Eliminate possible wumpus locations adjacent to a clear location
	for (itr1 = clearLocations.begin(); itr1 != clearLocations.end(); ++itr1)
	{
		location1 = *itr1;
		// Build list of adjacent locations to this clear location
		adjacentLocations.clear();
		AddAdjacentLocations(adjacentLocations, location1);
		tmpLocations = possibleWumpusLocations;
		possibleWumpusLocations.clear();
		for (itr2 = tmpLocations.begin(); itr2 != tmpLocations.end(); ++itr2)
		{
			location2 = *itr2;
			if (!LocationInList(adjacentLocations, location2))
				possibleWumpusLocations.push_back(location2);
		}
	}
}


/*
 * Sets the given location to the location resulting in a successful GOFORWARD.
 * Replaced Move() method from HW2.
 */
void Agent::SetGoForward(Location& location)
{
	location = worldState.agentLocation;
	switch (worldState.agentOrientation)
	{
	case RIGHT:
		location.X++;
		break;
	case UP:
		location.Y++;
		break;
	case LEFT:
		location.X--;
		break;
	case DOWN:
		location.Y--;
		break;
	}
}


/*
 * Add given location to agent's pathToGold. But if this location is already on the
 * pathToGold, then remove everything after the first occurrence of this location.
 */
void Agent::AddToPath(Location& location) {
	list<Location>::iterator itr = find(pathToGold.begin(), pathToGold.end(), location);
	if (itr != pathToGold.end())
	{
		// Location already on path (i.e., a loop), so remove everything after this element
		++itr;
		pathToGold.erase(itr, pathToGold.end());
	}
	else
	{
		pathToGold.push_back(location);
	}
}


/*
 * Choose and return an action when we haven't found the gold yet.
 * Handle special case where we start out in (1,1), perceive a stench, and
 * don't know the wumpus location. In this case, SHOOT. Orientation will be
 * RIGHT, so if SCREAM, then wumpus in (2,1); otherwise in (1,2). This is
 * handled in the UpdateState method.
 */
Action Agent::ChooseAction(Percept& percept)
{
	Action action;
	Location forwardLocation;
	SetGoForward(forwardLocation);
	//FilterSafeLocations();
	if (percept.Stench && (worldState.agentLocation == Location(1,1)) &&
			(possibleWumpusLocations.size() != 1))
	{
		action = SHOOT;
	}

	else if (LocationInList(safeLocations, forwardLocation)
		&& (! LocationInList(visitedLocations, forwardLocation)) 
		&& !LocationInList(pitLocation, forwardLocation))
	{
		// If happen to be facing safe unvisited location, then move there
		action = GOFORWARD;
	}
	//else if(LocationInList(safeLocations, forwardLocation))
	else
	{
		// Choose randomly from GOFORWARD, TURNLEFT, and TURNRIGHT, but don't
		// GOFORWARD into a possible wumpus location or a wall
		if (LocationInList(possibleWumpusLocations, forwardLocation) ||
			OutsideWorld(forwardLocation) ||
			LocationInList(pitLocation, forwardLocation))
		{
			action = (Action)  ((rand() % 2) + 1); // TURNLEFT, TURNRIGHT
		}
		else
		{
			if(LocationInList(known,forwardLocation)
			|| pitProb[forwardLocation.Y-1][forwardLocation.X-1].ptrue <0.5)
				action = GOFORWARD;
			else if(!LocationInList(safeLocations, forwardLocation))
				action = (Action)  ((rand() % 2) + 1); // TURNLEFT, TURNRIGHT
			else
				action = (Action) (rand() % 3); // GOFORWARD, TURNLEFT, TURNRIGHT
		}
	}
	return action;
}


/*
 * Add a sequence of actions to actionList that moves the agent along the
 * pathToGold if forward=true, or the reverse if forward=false. Assumes at
 * least one location is on pathToGold.
 */
void Agent::AddActionsFromPath(bool forward)
{
	list<Location> path = pathToGold;
	if (!forward)
		path.reverse();
	Location currentLocation = worldState.agentLocation;
	Orientation currentOrientation = worldState.agentOrientation;
	list<Location>::iterator itr_loc = path.begin();
	++itr_loc;
	while (itr_loc != path.end())
	{
		Location nextLocation = *itr_loc;
		Orientation nextOrientation;
		if (nextLocation.X > currentLocation.X)
			nextOrientation = RIGHT;
		if (nextLocation.X < currentLocation.X)
			nextOrientation = LEFT;
		if (nextLocation.Y > currentLocation.Y)
			nextOrientation = UP;
		if (nextLocation.Y < currentLocation.Y)
			nextOrientation = DOWN;

		// Find shortest turn sequence (assuming RIGHT=0, UP=1, LEFT=2, DOWN=3)
		int diff = ((int) currentOrientation) - ((int) nextOrientation);
		if ((diff == 1) || (diff == -3))
		{
			actionList.push_back(TURNRIGHT);
		}
		else
		{
			if (diff != 0)
				actionList.push_back(TURNLEFT);
			if ((diff == 2) || (diff == -2))
				actionList.push_back(TURNLEFT);
		}
		actionList.push_back(GOFORWARD);
		currentLocation = nextLocation;
		currentOrientation = nextOrientation;
		++itr_loc;
	}
}


/*
 * Return true if given location is in given location list; otherwise, return false.
 */
bool Agent::LocationInList(list<Location>& locationList, const Location& location)
{
	if (find(locationList.begin(), locationList.end(), location) != locationList.end())
	{
		return true;
	}
	return false;
}


/*
 * Add location to given list, if not already present.
 */
void Agent::AddNewLocation(list<Location>& locationList, const Location& location)
{
	if (!LocationInList(locationList, location))
	{
		locationList.push_back(location);
	}
}


/*
 * Add locations that are adjacent to the given location to the given location list.
 * Doesn't add locations outside the left and bottom borders of the world, but might
 * add locations outside the top and right borders of the world, if we don't know the
 * world size.
 */
void Agent::AddAdjacentLocations(list<Location>& locationList, const Location& location)
{
	int worldSize = worldState.worldSize;
	if ((worldSize == 0) || (location.Y < worldSize))
		AddNewLocation(locationList, Location(location.X, location.Y + 1)); // up
	if ((worldSize == 0) || (location.X < worldSize))
		AddNewLocation(locationList, Location(location.X + 1, location.Y)); // right
	if (location.X > 1)
		AddNewLocation(locationList, Location(location.X - 1, location.Y)); // left
	if (location.Y > 1)
		AddNewLocation(locationList, Location(location.X, location.Y - 1)); // down
}


/*
 * Return true if given location is outside known world.
 */
bool Agent::OutsideWorld(Location& location)
{
	int worldSize = worldState.worldSize;
	if ((location.X < 1) || (location.Y < 1))
		return true;
	if ((worldSize > 0) && ((location.X > worldSize) || (location.Y > worldSize)))
		return true;
	return false;
}

/*
 * Filters from safeLocations any locations that are outside the upper or right borders of the world.
 */
void Agent::FilterSafeLocations()
{
	list<Location>::iterator itr;
	Location location;
	int worldSize = worldState.worldSize;
	list<Location> tmpLocations = safeLocations;

	safeLocations.clear();

	for (list<Location>::iterator itr = tmpLocations.begin();itr != tmpLocations.end(); ++itr)
	{
		Location location = *itr;
		if ((location.X < 1) || (location.Y < 1))
			continue;
		if ((worldSize > 0) && ((location.X > worldSize) || (location.Y > worldSize)))
			continue;
		if(LocationInList(possibleWumpusLocations, location))
			continue;
		if(pitProb[location.Y-1][location.X-1].ptrue >=0.5)
			continue;
		safeLocations.push_back(location);
	}
}

void Agent::Output()
{
	list<Location>::iterator itr;
	Location location;
	cout << "World Size: " << worldState.worldSize << endl;
	cout << "Visited Locations:";
	for (itr = visitedLocations.begin(); itr != visitedLocations.end(); ++itr)
	{
		location = *itr;
		cout << " (" << location.X << "," << location.Y << ")";
	}
	cout << endl;
	cout << "Safe Locations:";
	for (itr = safeLocations.begin(); itr != safeLocations.end(); ++itr)
	{
		location = *itr;
		cout << " (" << location.X << "," << location.Y << ")";
	}
	cout << endl;
	cout << "Possible Wumpus Locations:";
	for (itr = possibleWumpusLocations.begin(); itr != possibleWumpusLocations.end(); ++itr)
	{
		location = *itr;
		cout << " (" << location.X << "," << location.Y << ")";
	}
	cout << endl;
	cout << "Gold Location: (" << worldState.goldLocation.X << "," << worldState.goldLocation.Y << ")\n";
	cout << "Path To Gold:";
	for (itr = pathToGold.begin(); itr != pathToGold.end(); ++itr)
	{
		location = *itr;
		cout << " (" << location.X << "," << location.Y << ")";
	}
	cout << endl;
	cout << "Action List:";
	for (list<Action>::iterator itr_a = actionList.begin(); itr_a != actionList.end(); ++itr_a)
	{
		cout << " ";
		PrintAction(*itr_a);
	}
	cout << endl;
	cout << endl;
}

void Agent::AddFrontier(Percept& percept)
{
	Location location = worldState.agentLocation;
	int worldSize = worldState.worldSize;

	frontier.clear();
	//add currentlocation to known, if safe
	if(!LocationInList(known, Location(location.X, location.Y)))
		AddNewLocation(known, Location(location.X,location.Y));
	//remove currentlocation if it is in frontier
	if(LocationInList(frontier, Location(location.X, location.Y)))
		frontier.remove( Location(location.X, location.Y));

	//add adjacent location to known,if no breeze at current location
	//add adjacent location	to frontier, if breeze at current location
	if ((worldSize == 0) || (location.Y < worldSize))
	{//up
		if(!LocationInList(known, Location(location.X, location.Y + 1)))
			AddNewLocation(frontier, Location(location.X, location.Y + 1)); 
	}
	if ((worldSize == 0) || (location.X < worldSize))
	{//right
		if(!LocationInList(known, Location(location.X+1, location.Y)))
			AddNewLocation(frontier, Location(location.X + 1, location.Y));

	}
	if (location.X > 1)
	{//left
		if(!LocationInList(known, Location(location.X-1, location.Y)))
			AddNewLocation(frontier, Location(location.X-1, location.Y));

	}
	if (location.Y > 1)
	{//down
		if(!LocationInList(known, Location(location.X, location.Y - 1)))
			AddNewLocation(frontier, Location(location.X, location.Y - 1));
	}

}
void Agent::AddKnownNPitLocation(Location location)
{
	int worldSize = worldState.worldSize;

	if(pitProb[location.Y-1][location.X-1].ptrue == 1.0)
	{//add pit, known
		if( !LocationInList(pitLocation, Location(location.X, location.Y)))
			AddNewLocation(pitLocation, Location(location.X, location.Y));
		if( !LocationInList(known, Location(location.X, location.Y)))
			AddNewLocation(known, Location(location.X, location.Y));
	}
	if(pitProb[location.Y-1][location.X-1].ptrue == 0.0)
	{  //not pit, known
		if( !LocationInList(known, Location(location.X, location.Y)))
			AddNewLocation(known, Location(location.X, location.Y));
	}

}

void Agent::UpdatePitProb(Percept& percept)
{
	list<Location>::iterator itr;
	list<Location> ftemp;
	double ftempProb = 0.0;
	int i=0, nTrue =0, nFalse =0;
	Location location;
	Location curloc = worldState.agentLocation;
	AddFrontier(percept);

	// calculate probability of pit
	//1. set P(currentLocation)
	pitProb[curloc.Y-1][curloc.X-1].ptrue = 0.0;
	pitProb[curloc.Y-1][curloc.X-1].pfalse = 1.0;


	//2. set P(Frontier)
	for(itr = frontier.begin();itr!=frontier.end();itr++)
	{
		location = *itr;
		if(!percept.Breeze)
		{
			pitProb[location.Y-1][location.X-1].ptrue = 0.0;
			pitProb[location.Y-1][location.X-1].pfalse = 1.0;
		}
		else if(frontier.size() ==1)
		{
			pitProb[location.Y-1][location.X-1].ptrue = 1.0;
			pitProb[location.Y-1][location.X-1].pfalse = 0.0;
		}
		else if(pitProb[curloc.Y][curloc.X-1].ptrue==1.0 ||
			pitProb[curloc.Y-1][curloc.X].ptrue==1.0 ||
			pitProb[curloc.Y-1][curloc.X-2].ptrue==1.0 ||
			pitProb[curloc.Y-2][curloc.X-1].ptrue==1.0 )
		{
			pitProb[location.Y-1][location.X-1].ptrue = 0.2;
			pitProb[location.Y-1][location.X-1].pfalse = 0.8;
		}
		else
		{// if there is no known adjacent location
			ftemp = frontier;
			ftemp.remove(*itr);
			ftempProb =0.0;

			pitProb[location.Y-1][location.X-1].ptrue = 0.0;
			pitProb[location.Y-1][location.X-1].pfalse = 0.0;

			//suppose Pit x,y = true
			for(int n=ftemp.size();n>=0; n--)
			{
				ftempProb = pow(0.2, n) * pow(0.8, ftemp.size()-n);
				pitProb[location.Y-1][location.X-1].ptrue += ftempProb;
				pitProb[location.Y-1][location.X-1].pfalse += ftempProb;

			}
			//eliminate the case of breeze but no pit in adjacent location
			pitProb[location.Y-1][location.X-1].pfalse -= pow (0.8, ftemp.size());

			pitProb[location.Y-1][location.X-1].ptrue *=0.2;
			pitProb[location.Y-1][location.X-1].pfalse *=0.8;
			pitProb[location.Y-1][location.X-1].ptrue =pitProb[location.Y-1][location.X-1].ptrue / (pitProb[location.Y-1][location.X-1].ptrue + pitProb[location.Y-1][location.X-1].pfalse);
		}
	}

	AddKnownNPitLocation(location);

//print
	cout<< endl<< "pitLocation: ";
	for (itr = pitLocation.begin(); itr != pitLocation.end(); ++itr)
	{
		location = *itr;
		cout << " (" << location.X << "," << location.Y << ")";
	}

	cout<<endl<< "known: ";
	for (itr = known.begin(); itr != known.end(); ++itr)
	{
		location = *itr;
		cout << " (" << location.X << "," << location.Y << ")";
	}

	cout<<endl<< "frontier: ";
	for (itr = frontier.begin(); itr != frontier.end(); ++itr)
	{
		location = *itr;
		cout << " (" << location.X << "," << location.Y << ")";
	}
	cout<<endl<< "Breeze: ";
	for (itr = breeze.begin(); itr != breeze.end(); ++itr)
	{
		location = *itr;
		cout << " (" << location.X << "," << location.Y << ")";
	}
	cout << endl;

	for (itr = frontier.begin(); itr != frontier.end(); ++itr)
	{
		location = *itr;
		cout<< "Computing proPit: ";
		cout << " (" << location.X << "," << location.Y << ")";
		cout << pitProb[location.Y][location.X].ptrue <<endl;
	}

	cout << "P(Pit) :" << endl;
	for(int i=4;i>=0;i--)
	{
		for(int j=0;j<5;j++)
		{
			cout << setprecision(2) << fixed << pitProb[i][j].ptrue <<" ";
		}
		cout<<endl;
	}
	cout << endl;
}

