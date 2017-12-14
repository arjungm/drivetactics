#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <limits>

using namespace std;

enum CardType
{
  Gas,
  Steer,
  Brake,
  Gun,
  Mine
};

template <typename T>
class Deck
{
public:

  Deck(const vector<pair<T,size_t> >& config)
  : _cards()
  {
    for(const auto& item : config) {
      for(size_t i=0; i<item.second; ++i) {
        _cards.push_back(item.first);
      }
    }

    random_shuffle(_cards.begin(), _cards.end());
  }

  T Draw()
  {
    T ret = _cards.back();
    _cards.pop_back();
    return ret;
  }

  const T& Peek(size_t index = 0)
  {
    index = _cards.size() - index;
    return _cards[index];
  }

  bool empty() const
  {
    return _cards.empty();
  }
  
private:

  vector<T> _cards;
};

/*

  switch(t) {
    case Gas:
    case Steer:
    case Brake:
    case Gun:
    case Mine:
  }

*/

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

struct PlayerStats
{
  int speed;
  int maneuvering;
  int traction;

  char diamond;

  int points;
};

const vector<pair<CardType, size_t> > kMainConfig =
{
 {Gas,10},
 {Steer,5},
 {Brake,5},
 {Gun,6},
 {Mine,4}
};

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

typedef Deck<CardType> MyDeck;
typedef vector<CardType> Hand;

int main(int argc, char* argv[])
{
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

  size_t zoneIndex = 0;
  
  /*
  MyDeck deckA(kMainConfig);
  MyDeck deckB(kMainConfig);

  Hand handA;
  Hand handB;
  for(int i=0; i<5; i++) {
    handA.push_back(deckA.Draw());
    handB.push_back(deckB.Draw());
  }
  */
  
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
  const auto showState = [&]() {
    printf("*\n*\n*\n");
    printf("Player %s has S=%d, M=%d, T=%d, D=%c (Points=%2d)\n", "A", stateA.speed, stateA.maneuvering, stateA.traction, stateA.diamond, stateA.points);
    printf("Player %s has S=%d, M=%d, T=%d, D=%c (Points=%2d)\n", "B", stateB.speed, stateB.maneuvering, stateB.traction, stateB.diamond, stateB.points);

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

  const auto resolveRound = [kValidDiamond=kValidDiamond,&rngD,&showState,&clearScreen]
                              (PlayerStats& s1, CardType c1,
                               PlayerStats& s2, CardType c2,
                               int zoneVal)
  {
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
    
    if(c1==Steer || c2==Steer || c1==Brake || c2==Brake) {
      clearScreen();
      showState();
      printf("**** RESOLVE DIAMOND ****\n\n");
      printf("Vehicle A diamond (current=%c)? ", s1.diamond);
      char d1 = BlockingInput<char>(kValidDiamond);

      clearScreen();
      showState();
      printf("**** RESOLVE DIAMOND ****\n\n");
      printf("Vehicle B diamond (current=%c)? ", s2.diamond);
      char d2 = BlockingInput<char>(kValidDiamond);

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

    if(c1==Gun || c1==Mine || c2==Gun || c2==Mine) {
      printf("**** RESOLVE DAMAGE ****\n\n");
      printf("Vehicle A diamond=%c\n", s1.diamond);
      printf("Vehicle B diamond=%c\n", s2.diamond);

      printf("Vehicle A damage? ");
      int damageA;
      cin >> damageA;
      if(damageA != 0) {
        s1.speed = max(0, s1.speed - max(0, damageA - s1.maneuvering));
        s1.maneuvering = (s1.maneuvering < damageA) ? 0 : (s1.maneuvering-damageA);
      }

      printf("Vehicle B damage? ");
      int damageB;
      cin >> damageB;
      if(damageB != 0) {
        s2.speed = max(0, s2.speed - max(0, damageB - s2.maneuvering));
        s2.maneuvering = (s2.maneuvering < damageB) ? 0 : (s2.maneuvering-damageB);
      }
    }

    // scoring
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
