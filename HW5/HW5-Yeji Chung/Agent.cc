// Agent.cc

/* 
*  Note- Yeji Chung
*  multiple trial for each world hasn't implemented yet.
*  So, this agent only works for search and return mode.
*  For the second and more trial, it will do the same thing as the fist trial
*
*/


#include <iostream>
#include "Agent.h"

using namespace std;
using std::ostream;
Agent::Agent ()
{
	goldLocation = Location(0,0);
	wumpusLocation = Location(0,0);
	worldsize =1000;
	agentmode =0;
	pathlength =-1;
	push(agentLocation);


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
	grabgoldflag =0;
	stenchflag =0;
	copypathlength=0;

	for(int i=0;i<1000;i++)
	{
		for(int j=0;j<1000;j++)
		{
			world[i][j].stench = false;
			world[i][j].unsearched = true;
			world[i][j].wumpus = true;
		}
	}
	world[1][1].wumpus = false;
	world[1][1].unsearched = false;
	push(Location(1,1));
}
void Agent::printpath()
{
	for(int i=0; i<pathlength ; i++)
		cout<<path[i];
	for(int i=0; i<copypathlength ; i++)
		cout<<copypath[i];
}

Action Agent::Process (Percept& percept)
{
	cout<<"------------------------------------------------------"<<endl;
	//printpath();
	Action action;
	if(agentmode ==0)//search mode
	{
		if(percept.Glitter)
		{
			action = GRAB;
			agentHasGold = true; // updates that agent got gold
			goldLocation = agentLocation;
			agentmode =1; //return mode
		}
		else if(percept.Stench && (wumpusLocation == Location(0,0)))
		{// if stench but not sure wumpus' location
			world[agentLocation.Y][agentLocation.X].stench =true;
			if (agentLocation == Location(1,1)) // if stench at(1,1), go forward 
			{//because risks of wumpus at (2,1) and (1,2) are same.
				action = GOFORWARD;
				updataftergoforward();
			}
			else// if agent is not at (1,1)
			{
				if(!world[agentLocation.Y][agentLocation.X-1].wumpus && world[agentLocation.Y][agentLocation.X-1].unsearched)
				{//left location has no wumpus and unsearched
					if(agentOrientation == LEFT)
					{
						action = GOFORWARD;
						updataftergoforward();
					}
					else
					{
						action = TURNRIGHT;
						updateorientation(action);
					}
				}
				else if(!world[agentLocation.Y+1][agentLocation.X].wumpus && world[agentLocation.Y+1][agentLocation.X].unsearched)
				{//up location has no wumpus and unsearched
					if(agentOrientation == UP)
					{
						action = GOFORWARD;
						updataftergoforward();
					}
					else
					{
						action = TURNRIGHT;
						updateorientation(action);
					}
				}

				else if(!world[agentLocation.Y][agentLocation.X+1].wumpus && world[agentLocation.Y][agentLocation.X+1].unsearched)
				{//right location has no wumpus and unsearched
					if(agentOrientation == RIGHT)
					{
						action = GOFORWARD;
						updataftergoforward();
					}
					else
					{
						action = TURNRIGHT;
						updateorientation(action);
					}
				}
				else if(!world[agentLocation.Y-1][agentLocation.X].wumpus && world[agentLocation.Y-1][agentLocation.X].unsearched)
				{//down location has no wumpus and unsearched
					if(agentOrientation == DOWN)
					{
						action = GOFORWARD;
						updataftergoforward();
					}
					else
					{
						action = TURNRIGHT;
						updateorientation(action);
					}
				}
				//there is no info for wumpus on adjacent locations
				else
				{	cout<<"go back to previous position"<<endl;
					if(stenchflag ==0)
					{
						stenchflag = 1;
						action = TURNLEFT;
						updateorientation(action);
					}
					else if(stenchflag ==1)
					{
						stenchflag = 2;
						action = TURNLEFT;
						updateorientation(action);
					}
					else if(stenchflag ==2)
					{
						stenchflag = 0;
						action = GOFORWARD; //go back to previous position
						updataftergoforward();
					}
				}
			}
		}
		else if(percept.Bump)// if agent bumps to the wall
		{
			action = TURNLEFT;
			if(agentOrientation == RIGHT)
			{
				agentLocation.X -=1;// to stay in x-boundary
				worldsize = agentLocation.X;
				agentOrientation = UP;
			}
			else if(agentOrientation == UP)
			{
				agentLocation.Y -=1;// to stay in y-boundary
				worldsize = agentLocation.Y;
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
			pop();

		}
		else
		{
			if((agentLocation.X < worldsize) && !world[agentLocation.Y][agentLocation.X+1].wumpus && world[agentLocation.Y][agentLocation.X+1].unsearched)
			{// if Location (X+1, Y) unserached
				 //if there is possibility of wumpus at Location(X+1,Y) 
				if(agentOrientation == RIGHT)
				{
					action = GOFORWARD;
					updataftergoforward();
				}
				else// turn until heading LEFT
				{
					action = TURNRIGHT;
					updateorientation(action);
				}
			}
			else if((agentLocation.Y < worldsize) &&!world[agentLocation.Y+1][agentLocation.X].wumpus && world[agentLocation.Y+1][agentLocation.X].unsearched)
			{ //if Location(X,Y+1) was not searched yet and no wumpus, go there
				if(agentOrientation == UP)
				{
					action = GOFORWARD;
					updataftergoforward();
				}
				else
				{
					action = TURNRIGHT;
					updateorientation(action);
				}
			}
			else if((agentLocation.X > 1) &&!world[agentLocation.Y][agentLocation.X-1].wumpus && world[agentLocation.Y][agentLocation.X-1].unsearched)
			{ //if Location(X-1,Y) was not searched yet and no wumpus, go there
				if(agentOrientation == LEFT)
				{
					action = GOFORWARD;
					updataftergoforward();
				}
				else
				{
					action = TURNRIGHT;
					updateorientation(action);
				}
			}
			else if((agentLocation.Y > 1)  &&!world[agentLocation.Y-1][agentLocation.X].wumpus && world[agentLocation.Y-1][agentLocation.X].unsearched)
			{ //if Location(X,Y-1) was not searched yet and no wumpus, go there
				if(agentOrientation == DOWN)
				{
					action = GOFORWARD;
					updataftergoforward();
				}
				else
				{
					action = TURNRIGHT;
					updateorientation(action);
				}
			}
			else // all 4 adjacent location has been searched 
			{
				if( (agentOrientation ==  RIGHT) && (wumpusLocation.X == agentLocation.X+1)&& (wumpusLocation.Y == agentLocation.Y))
				{
					action = TURNLEFT;
					updateorientation(action);
				}
				else if( (agentOrientation ==  UP) && (wumpusLocation.X == agentLocation.X)&& (wumpusLocation.Y == agentLocation.Y+1))
				{
					action = TURNLEFT;
					updateorientation(action);
				}
				else if( (agentOrientation ==  LEFT) && (wumpusLocation.X == agentLocation.X-1)&& (wumpusLocation.Y == agentLocation.Y))
				{
					action = TURNRIGHT;
					updateorientation(action);
				}
				else if( (agentOrientation ==  DOWN) && (wumpusLocation.X == agentLocation.X)&& (wumpusLocation.Y == agentLocation.Y-1))
				{
					action = TURNRIGHT;
					updateorientation(action);
				}
				else
				{
					action = GOFORWARD;
					updataftergoforward();
				}
			}
		}
		printpath();
	}
	else if(agentmode ==1)// returning mode
	{
		// At first, turn  180 degree
		if(grabgoldflag ==0)
		{
			copyPath();
			nextloc = cpop();
			action = TURNRIGHT;
			updateorientation(action);
			grabgoldflag =1;
		}
		else if(grabgoldflag ==1)
		{
			action = TURNRIGHT;
			updateorientation(action);
			grabgoldflag =0;// this must be 2 for multiple trial mode. but it has not implemented yet.
		}//copleted turn 180 degree
		else
		{
			if (agentHasGold && (agentLocation == Location(1,1)))
			{
				agentmode =2;
				action = CLIMB;
			}
			//cout<<agentLocation<<"/"<<nextloc<<endl;
			if((agentLocation.X == nextloc.X) &&(agentLocation.Y+1 == nextloc.Y))
			{// have to go up
				if(agentOrientation == UP)
				{
					action = GOFORWARD;
					updataftergoforward();
					if(copypathlength !=0)
						nextloc = cpop();
				}
				else
				{
					action = TURNRIGHT;
					updateorientation(action);
				}
			}
			else if((agentLocation.X+1 == nextloc.X) &&(agentLocation.Y == nextloc.Y))
			{// have to go right
				if(agentOrientation == RIGHT)
				{
					action = GOFORWARD;
					updataftergoforward();
					if(copypathlength !=0)
						nextloc = cpop();
				}
				else
				{
					action = TURNRIGHT;
					updateorientation(action);
				}
			}
			else if((agentLocation.X == nextloc.X) &&(agentLocation.Y-1 == nextloc.Y))
			{// have to go down
				if(agentOrientation == DOWN)
				{
					action = GOFORWARD;
					updataftergoforward();
					if(copypathlength !=0)
						nextloc = cpop();
				}
				else
				{
					action = TURNRIGHT;
					updateorientation(action);
				}
			}
			else if((agentLocation.X-1 == nextloc.X) &&(agentLocation.Y == nextloc.Y))
			{// have to go left
				if(agentOrientation == LEFT)
				{
					action = GOFORWARD;
					updataftergoforward();
					if(copypathlength !=0)
						nextloc = cpop();
				}
				else
				{
					action = TURNRIGHT;
					updateorientation(action);
				}
			}
		}

		//successfully return to Locattion(1,1)
		//Next the agent will have path info and will run in mode 2
	}
	else if(agentmode ==2)
	{// in agent mode 2, the agent does not search the gold, but follow the recorded path.

		if(percept.Glitter)
		{
			action = GRAB;
			agentHasGold = true; // updates that agent got gold
			goldLocation = agentLocation;
			agentmode =1; //return mode
		}
	}

	return action;
}

void Agent::updatewumpus()
{
	if(!world[agentLocation.Y][agentLocation.X].stench)
	{// if "no stench" on an agentLocation, update "no wumpus" on four adjacent locations.
		world[agentLocation.Y+1][agentLocation.X].wumpus = false;
		world[agentLocation.Y-1][agentLocation.X].wumpus = false;
		world[agentLocation.Y][agentLocation.X+1].wumpus = false;
		world[agentLocation.Y][agentLocation.X-1].wumpus = false;
	}
	else  // if there is stench, check wumpusLocaton
	{
		if((agentLocation.X >0) && (agentLocation.Y <worldsize))
		{
			if(world[agentLocation.Y+1][agentLocation.X-1].stench )
			{
				if(!world[agentLocation.Y][agentLocation.X-1].wumpus)
					wumpusLocation = Location(agentLocation.X, agentLocation.Y+1);
				else if(!world[agentLocation.Y+1][agentLocation.X].wumpus)
					wumpusLocation = Location(agentLocation.X-1, agentLocation.Y);
			}
		}
		if((agentLocation.X <worldsize) && (agentLocation.Y <worldsize))
		{
			if(world[agentLocation.Y+1][agentLocation.X+1].stench )
			{
				if(!world[agentLocation.Y][agentLocation.X+1].wumpus)
					wumpusLocation = Location(agentLocation.X, agentLocation.Y+1);
				else if(!world[agentLocation.Y+1][agentLocation.X].wumpus)
					wumpusLocation = Location(agentLocation.X+1, agentLocation.Y);
			}
		}
		if((agentLocation.X <worldsize) && (agentLocation.Y >0))
		{
			if(world[agentLocation.Y-1][agentLocation.X+1].stench )
			{
				if(!world[agentLocation.Y][agentLocation.X+1].wumpus)
					wumpusLocation = Location(agentLocation.X, agentLocation.Y-1);
				else if(!world[agentLocation.Y-1][agentLocation.X].wumpus)
					wumpusLocation = Location(agentLocation.X+1, agentLocation.Y);
			}
		}
		if((agentLocation.X >0) && (agentLocation.Y >0))
		{
			if(world[agentLocation.Y-1][agentLocation.X-1].stench )
			{
				if(!world[agentLocation.Y][agentLocation.X-1].wumpus)
					wumpusLocation = Location(agentLocation.X, agentLocation.Y-1);
				else if(!world[agentLocation.Y-1][agentLocation.X].wumpus)
					wumpusLocation = Location(agentLocation.X-1, agentLocation.Y);
			}
		}
	}

}

void Agent::updataftergoforward()
{
	//cout<<"updategoforward"<<endl;
	updatewumpus();
	world[agentLocation.Y][agentLocation.X].unsearched = false;
	if(agentOrientation == RIGHT)
	{
		agentLocation.X +=1;
	}
	else if(agentOrientation == UP)
	{
		agentLocation.Y +=1;
	}
	else if(agentOrientation == LEFT)
	{
		agentLocation.X -=1;
	}
	else if(agentOrientation == DOWN)
	{
		agentLocation.Y -=1;
	}
	push(agentLocation);
}
void Agent::updateorientation(Action action)
{
	if(action == TURNLEFT)
	{
		if(agentOrientation ==UP)
			agentOrientation = LEFT;
		else if(agentOrientation ==LEFT)
			agentOrientation = DOWN;
		else if(agentOrientation ==DOWN)
			agentOrientation = RIGHT;
		else if(agentOrientation ==RIGHT)
			agentOrientation = UP;
	}
	if(action == TURNRIGHT)
	{
		if(agentOrientation ==UP)
			agentOrientation = RIGHT;
		else if(agentOrientation ==RIGHT)
			agentOrientation = DOWN;
		else if(agentOrientation ==DOWN)
			agentOrientation = LEFT;
		else if(agentOrientation ==LEFT)
			agentOrientation = UP;
	}
}
void Agent::copyPath()
{
	copypathlength = pathlength-1;
	for(int i=0;i<=pathlength;i++)
	{
		copypath[i] = path[i];
	}

	cout << "path copied!"<<endl;

}


void Agent::push(Location loc)
{
	path[pathlength] = loc;
	pathlength+=1;
}
Location Agent::pop()
{
	Location loc;
	loc = path[pathlength];
	path[pathlength] = Location(0,0);
	pathlength-=1;
	return loc;
}
Location Agent::cpop()
{
	Location loc;
	loc = copypath[copypathlength-1];
	copypath[copypathlength] = Location(0,0);
	copypathlength-=1;
	return loc;
}
void Agent::GameOver (int score)
{

}

ostream &operator<< (ostream &lhs, Location &rhs)
{
	lhs<<"("<< rhs.X <<"," << rhs.Y <<") ";
	return lhs;
}
