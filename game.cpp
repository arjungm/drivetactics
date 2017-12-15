#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <limits>

using namespace std;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
enum CardType
{
  Gas,
  Steer,
  Brake,
  Gun,
  Mine
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
string to_string(CardType t)
{
  switch(t) {
    case Gas: { return "Gas"; }
    case Steer: { return "Steer"; }
    case Brake: { return "Brake"; }
    case Gun: { return "Gun"; }
    case Mine: { return "Mine"; }
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
struct PlayerStats
{
  int speed;
  int maneuvering;
  int traction;

  char diamond;

  int points;
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// helper function for player prompts
template<typename T>
T BlockingInput(const vector<T>& valid)
{
  T input;
  cin >> input;
  while(find(valid.begin(), valid.end(), input)==valid.end() || !cin) {
    cout << endl;
    if(!cin) {
      cout << "Unexpected Input: ";
    }
    cout << "Try again...";
    cin.clear();
    cin.ignore(100,'\n');
    cin >> input;
  }
  return input;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// helper class for diamond-repositioning randomness
template<typename T>
class PseudoRandomHelper
{
public:
  PseudoRandomHelper(const vector<T>& items)
  : _items(items)
  , _head(0)
  {
  }

  T Next()
  {
    if(_head==0) {
      random_shuffle(_items.begin(), _items.end());
    }

    const T& ret = _items[_head];
    _head = (_head+1)%_items.size();
    return ret;
  }

private:
  
  vector<T> _items;

  size_t _head;
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
int main(int argc, char* argv[])
{
  size_t zoneIndex = 0;

  // input the encounter list (aka the track)
  vector<int> zones;
  int zoneInput = 0;
  const vector<int> kValidZones{0,1,2,3,4};
  printf("Enter zone list:\n");
  while(zoneInput >= 0) {
    zoneInput = BlockingInput<int>(kValidZones);
    if(zoneInput == 0) {
      zoneInput = -1;
      printf("...done\n");
    } else {
      zones.push_back(zoneInput);
    }
  }

  printf("**** INIT ****\n");
  vector<size_t> kValid5({1,2,3,4,5});
  vector<char> kValidDiamond({'F','R','L','B'});
  PlayerStats stateA { 1, 0, 3, 'R', 0 };
  PlayerStats stateB { 1, 0, 3, 'L', 0 };

  vector<pair<char,char> > permutations;
  for(int i=0; i<4; ++i) {
    for(int j=0; j<4; ++j) {
      if(i!=j) {
        permutations.push_back({kValidDiamond[i], kValidDiamond[j]});
      }
    }
  }
  PseudoRandomHelper<pair<char,char> > rngD(permutations);

  printf("...initialized\n");

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // helper function: shows current state of player positions
  const auto showDiamond = [&]() {
      printf("___* Player Position *___\n");
      //use stateA.diamond & use stateB.diamond
      char worldR = ' ';
      char worldF = ' ';
      char worldL = ' ';
      char worldB = ' ';

      switch(stateA.diamond){
        case 'R': worldR = 'A'; break;
        case 'F': worldF = 'A'; break;
        case 'L': worldL = 'A'; break;
        case 'B': worldB = 'A'; break;
      }

      switch(stateB.diamond){
        case 'R': worldR = 'B'; break;
        case 'F': worldF = 'B'; break;
        case 'L': worldL = 'B'; break;
        case 'B': worldB = 'B'; break;
      }

      printf(" _ r:%c _ \n",worldR);
      printf("b:%c _ f:%c \n",worldB,worldF);
      printf(" _ l:%c _ \n",worldL);
  };

  // helper function: prints the state of the game (scores and stats)
  const auto showState = [&]() {
    printf("*\n*\n*\n");
    printf("Player %s has S=%d, M=%d, T=%d, D=%c (Points=%2d)\n", "A", stateA.speed, stateA.maneuvering, stateA.traction, stateA.diamond, stateA.points);
    printf("Player %s has S=%d, M=%d, T=%d, D=%c (Points=%2d)\n", "B", stateB.speed, stateB.maneuvering, stateB.traction, stateB.diamond, stateB.points);

    showDiamond();

    cout << endl;
    cout << endl;
    for(size_t i=0; i<zones.size(); ++i) {
      size_t p = (i + zoneIndex)%zones.size();
      switch(zones[p]) {
        case 1: cout << "SS"; break;
        case 2: cout << "ST"; break;
        case 3: cout << "TT"; break;
        case 4: cout << "TS"; break;
      }
      cout << " - ";
    }
    cout << endl;
    cout << endl;
  };

  const auto clearScreen = []() {
    for(int i=0; i<50; ++i) {
      printf("\n");
    }
  };

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // helper function: prompts the player for an action
  const auto handlePlayerInput = [kValid5=kValid5,&showState,&clearScreen]()->CardType
  {
    stringstream ss;
    ss << "Pick an action from below" << endl;
    for(size_t i=0; i<5; ++i) {
      ss << (i+1) << ") " << to_string(CardType(i)) << endl;
    }
    ss << "SELECTION:" << endl;
    cout << ss.str();

    // prompt for result
    size_t selection = BlockingInput(kValid5);
    selection = selection - 1;
    return CardType(selection);
  };

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // helper function: resolve the inputs of the player actions
  // STEPS = { Stat update, Diamond reposition, Damage calculation, Scoring }
  const auto resolveRound = [kValidDiamond=kValidDiamond,&rngD,&showState,&clearScreen]
                              (PlayerStats& s1, CardType c1,
                               PlayerStats& s2, CardType c2,
                               int zoneVal)
  {
    // RESOLVE STAT UPDATES
    // helper function
    const auto statUpdates = [](PlayerStats& s, CardType c)
    {
      switch(c) {
        case Gas:
          {
            s.speed = min(s.speed+1, 4);
            s.traction = max(0, s.traction-2);
            break;
          }
        case Steer:
          {
            s.maneuvering = min(s.maneuvering+1, 4);
            break;
          }
        case Brake:
          {
            s.speed = max(s.speed-1, 0);
            break;
          }
        case Gun:
        case Mine:
          break;
      }
    };
    statUpdates(s1,c1);
    statUpdates(s2,c2);

    // helper function: determine where you can steer/brake towards
    const auto getValidDiamondTargets = [&](CardType c, char diamond)->vector<char>
    {
      if(c==Steer) {
        switch(diamond) {
          case 'F': return {'F'};
          case 'R': return {'R','L','F'};
          case 'L': return {'R','L','F'};
          case 'B': return kValidDiamond;
        }
      }

      if(c==Brake) {
        switch(diamond) {
          case 'B': return {'B'};
          case 'R': return {'R','L','B'};
          case 'L': return {'R','L','B'};
          case 'F': return kValidDiamond;
        }
      }

      return kValidDiamond;
    };
    
    // RESOLVE STEERING/BRAKING INPUTS
    if(c1==Steer || c2==Steer || c1==Brake || c2==Brake) {
      char d1 = s1.diamond;
      char d2 = s2.diamond;

      // prompt if the player requested a change
      if(c1==Brake || c1==Steer) {
        clearScreen();
        showState();
        printf("**** RESOLVE DIAMOND ****\n\n");
        printf("Vehicle A diamond (current=%c)? ", s1.diamond);
        const auto& validDiamond1 = getValidDiamondTargets(c1,s1.diamond);
        cout << "Choose from { ";
        for(auto& ch : validDiamond1) {
          cout << ch << " ";
        }
        cout << "}" << endl;
        d1 = BlockingInput<char>(validDiamond1);
      }

      // prompt if the player requested a change
      if(c2==Brake || c2==Steer) {
        clearScreen();
        showState();
        printf("**** RESOLVE DIAMOND ****\n\n");
        printf("Vehicle B diamond (current=%c)? ", s2.diamond);
        const auto& validDiamond2 = getValidDiamondTargets(c2,s2.diamond);
        cout << "Choose from { ";
        for(auto& ch : validDiamond2) {
          cout << ch << " ";
        }
        cout << "}" << endl;
        d2 = BlockingInput<char>(validDiamond2);
      }

      // resolve by honoring swaps, and if there's a collide it's random
      if(d1==d2) {
        printf("...randomizing\n");
        auto randomPos = rngD.Next();
        while(d1!=randomPos.first && d2!=randomPos.second) {
          randomPos = rngD.Next();
        }
        s1.diamond = randomPos.first;
        s2.diamond = randomPos.second;
      } else {
        s1.diamond = d1;
        s2.diamond = d2;
      }
    }

    // APPLY DIAMOND BUFF
    // helper function
    const auto applyDiamondBuff = [](PlayerStats& s) {
      switch(s.diamond) {
        case 'F': s.speed = min(s.speed+1, 4); break;
        case 'R':
        case 'L':
                  s.traction = min(s.traction+1, 4);
                  break;
        case 'B':
                  s.traction = min(s.traction+2, 4);
                  break;
      }
    };
    applyDiamondBuff(s1);
    applyDiamondBuff(s2);

    // RESOLVE ATTACK
    // helper function: check for attack and critical hit
    const auto resolveAttack = [](CardType action1, char diamond1, char diamond2)->int
    {
      const auto char2index = [](char ch)->int {
        switch(ch) {
          case 'B': return 0;
          case 'R': return 1;
          case 'L': return 2;
          case 'F': return 3;
        }
        return 0;
      };

      int gunDamage[4][4] = {{0,1,1,2},
                             {0,0,0,1},
                             {0,0,0,1},
                             {0,0,0,0}};

      int mineDamage[4][4] = {{0,0,0,0},
                              {1,0,0,0},
                              {1,0,0,0},
                              {1,1,1,0}};

      int from = char2index(diamond1);
      int to   = char2index(diamond2);

      if(action1==Gun) {
        return gunDamage[from][to];
      }

      if(action1==Mine) {
        return mineDamage[from][to];
      }

      return 0;
    };

    if(c1==Gun || c1==Mine || c2==Gun || c2==Mine) {
      printf("**** RESOLVE DAMAGE ****\n\n");
      int damageA = resolveAttack(c2, s2.diamond, s1.diamond);
      if(damageA != 0) {
        s1.speed = max(0, s1.speed - max(0, damageA - s1.maneuvering));
        s1.maneuvering = (s1.maneuvering < damageA) ? 0 : (s1.maneuvering-damageA);
      }

      int damageB = resolveAttack(c1, s1.diamond, s2.diamond);
      if(damageB != 0) {
        s2.speed = max(0, s2.speed - max(0, damageB - s2.maneuvering));
        s2.maneuvering = (s2.maneuvering < damageB) ? 0 : (s2.maneuvering-damageB);
      }
    }

    // PER-ROUND SCORING
    int ws,wm,wt;
    switch(zoneVal) {
      case 1: /*SS*/ ws= 2; wm= 0; wt= 0; break;
      case 2: /*ST*/ ws=-1; wm= 1; wt= 2; break;
      case 3: /*TS*/ ws= 1; wm= 0; wt= 1; break;
      case 4: /*TT*/ ws=-2; wm= 2; wt= 0; break;
    }

    s1.points += ws*s1.speed + wt*s1.traction + wm*s1.maneuvering;
    s2.points += ws*s2.speed + wt*s2.traction + wm*s2.maneuvering;
  };

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // MAIN
  printf("**** BEGIN GAME ****\n");
  size_t lapCount = 0;
  while(lapCount < 5) {
    clearScreen();
    showState();
    printf("**** Player %s's turn ****\n", "A");
    auto cA = handlePlayerInput();

    clearScreen();
    showState();
    printf("**** Player %s's turn ****\n", "B");
    auto cB = handlePlayerInput();

    resolveRound(stateA, cA, stateB, cB, zones[zoneIndex]);
    zoneIndex = (zoneIndex+1);
    if(zoneIndex == zones.size()) {
      lapCount++;
      zoneIndex=0;
    }
  }
}
