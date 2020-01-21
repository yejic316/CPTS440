// Agent.cc

#include <iostream>
#include "Agent.h"

using namespace std;

Agent::Agent ()
{
	agentHasArrow = true;
	agentHasGold = false;
	agentLocation = Location(1,1);
	agentOrientation = RIGHT;
}

Agent::~Agent ()
{

}

void Agent::Initialize ()
{
	agentHasArrow = true;
	agentHasGold = false;
	agentLocation = Location(1,1);
	agentOrientation = RIGHT;
}

Action Agent::Process (Percept& percept)
{
	Action action;
	if(percept.Glitter)
	{
		action = GRAB;
		agentHasGold = true; // updates that agent got gold
	}
	else if (agentHasGold && (agentLocation == Location(1,1)))
	{
		action = CLIMB;
	}
	else if(percept.Stench && agentHasArrow)
	{
		action = SHOOT;
		agentHasArrow = false; // updates that agent used arrow
	}
	else if(percept.Bump)// if agent bumps to the wall
	{
		int r = rand()%2; //randomly generates 0 or 1 to decide TURNLEFT or TURNRIGHT
		if(r==0)
		{
			action = TURNLEFT;
			if(agentOrientation == RIGHT) //if bumps right wall
			{
				agentLocation.X -=1;// to stay in x-boundary 
				agentOrientation = UP;
			}
			else if(agentOrientation == UP)
			{
				agentLocation.Y -=1;// to stay in y-boundary
				agentOrientation = LEFT;
			}
			else if(agentOrientation == LEFT)
			{
				agentLocation.X +=1;// to stay in x-boundary
				agentOrientation = DOWN;
			}
			else if(agentOrientation == DOWN)
			{
				agentLocation.Y +=1;// to stay in y-boundary
				agentOrientation = RIGHT;
			}
		}
		else if(r==1)
		{
			action = TURNRIGHT;
			if(agentOrientation == RIGHT)
			{
				agentLocation.X -=1;// to stay in x-boundary
				agentOrientation = DOWN;
			}
			else if(agentOrientation == DOWN)
			{
				agentLocation.Y +=1;// to stay in y-boundary
				agentOrientation = LEFT;
			}
			else if(agentOrientation == LEFT)
			{
				agentLocation.X +=1;// to stay in x-boundary
				agentOrientation = UP;
			}
			else if(agentOrientation == UP)
			{
				agentLocation.Y -=1;// to stay in y-boundary
				agentOrientation = RIGHT;
			}
		}
	}
	else // otherwise, go forward
	{
		action = GOFORWARD;
		//cout<<"\nLocation :["<< agentLocation.X << ","<< agentLocation.Y<<"]\n";
		if(agentOrientation == RIGHT)
			agentLocation.X +=1; // update its x-location
		else if(agentOrientation == UP)
			agentLocation.Y +=1; // update its y-location
		else if(agentOrientation == LEFT)
			agentLocation.X -=1; // update its x-location
		else if(agentOrientation == DOWN)
			agentLocation.Y -=1; // update its y-location
	}

	return action;
}

void Agent::GameOver (int score)
{

}

