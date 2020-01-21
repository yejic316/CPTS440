# PyAgent.py

from random import randint
import Action
import Orientation

    
class WorldState:
    def __init__(self):
        self.agentLocation = [1,1]
        self.agentOrientation = Orientation.RIGHT
        self.agentHasArrow = True
        self.agentHasGold = False
    
class Agent:
    def __init__(self):
        self.worldState = WorldState()
        self.previousAction = Action.CLIMB
        self.actionList = []
    
    def Initialize(self):
        self.worldState.agentLocation = [1,1]
        self.worldState.agentOrientation = Orientation.RIGHT
        self.worldState.agentHasArrow = True
        self.worldState.agentHasGold = False
        self.previousAction = Action.CLIMB
        self.actionList = []
    
    def Process(self, percept):
        self.UpdateState(percept)
        if (not self.actionList):
            if (percept['Glitter']): # Rule 3a
                self.actionList.append(Action.GRAB)
            elif (self.worldState.agentHasGold and (self.worldState.agentLocation == [1,1])): # Rule 3b
                self.actionList.append(Action.CLIMB)
            elif (percept['Stench'] and self.worldState.agentHasArrow): # Rule 3c
                self.actionList.append(Action.SHOOT)
            elif (percept['Bump']): # Rule 3d
                randomAction = randint(1,2) # 1=TURNLEFT, 2=TURNRIGHT
                self.actionList.append(randomAction)
            else: # Rule 3e
                self.actionList.append(Action.GOFORWARD)
        action = self.actionList.pop(0)
        self.previousAction = action
        return action
    
    def UpdateState(self, percept):
        currentOrientation = self.worldState.agentOrientation
        if (self.previousAction == Action.GOFORWARD):
            if (not percept['Bump']):
                self.Move()
        if (self.previousAction == Action.TURNLEFT):
            self.worldState.agentOrientation = (currentOrientation + 1) % 4
        if (self.previousAction == Action.TURNRIGHT):
            currentOrientation -= 1
            if (currentOrientation < 0):
                currentOrientation = 3
            self.worldState.agentOrientation = currentOrientation
        if (self.previousAction == Action.GRAB):
            self.worldState.agentHasGold = True # Only GRAB when there's gold
        if (self.previousAction == Action.SHOOT):
            self.worldState.agentHasArrow = False
        # Nothing to do for CLIMB
        
    def Move(self):
        X = self.worldState.agentLocation[0]
        Y = self.worldState.agentLocation[1]
        if (self.worldState.agentOrientation == Orientation.RIGHT):
            X = X + 1
        if (self.worldState.agentOrientation == Orientation.UP):
            Y = Y + 1
        if (self.worldState.agentOrientation == Orientation.LEFT):
            X  = X - 1
        if (self.worldState.agentOrientation == Orientation.DOWN):
            Y = Y - 1
        self.worldState.agentLocation = [X,Y]
        
# Global agent
myAgent = 0

def PyAgent_Constructor ():
    print "PyAgent_Constructor"
    global myAgent
    myAgent = Agent()

def PyAgent_Destructor ():
    print "PyAgent_Destructor"

def PyAgent_Initialize ():
    print "PyAgent_Initialize"
    global myAgent
    myAgent.Initialize()

def PyAgent_Process (stench,breeze,glitter,bump,scream):
    global myAgent
    percept = {'Stench': bool(stench), 'Breeze': bool(breeze), 'Glitter': bool(glitter), 'Bump': bool(bump), 'Scream': bool(scream)}
    #print "PyAgent_Process: percept = " + str(percept)
    return myAgent.Process(percept)

def PyAgent_GameOver (score):
    print "PyAgent_GameOver: score = " + str(score)
