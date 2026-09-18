//////////////////////////////////////////////////////////////////////////////
// Filename    : MonsterSummonInfo.cpp
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "MonsterSummonInfo.h"

#include <list>
#include <vector>

#include "MonsterInfo.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
//
// MonsterCollectionInfo :: parseString( infoString )
//
//////////////////////////////////////////////////////////////////////////////
//	SpriteType_t	SpriteType;
//	int             Num;
//
//        a        b   c
// ex1>  "(Deadbody, 20)"        // 20 Deadbody
//
//        0 2   6
//        a b   c
// ex2>  "(5, 10)"               // SpriteType 5 (Deadbody), 10 of them
//
// ex3>  "({34}, 20)"    // Here {} means a MonsterType is used.
//////////////////////////////////////////////////////////////////////////////
void MonsterCollectionInfo::parseString(const string& text) {
    SpriteType = 0;
    MonsterType = 0;
    Num = 0;

    if (text.size() < 5)
        return;

    size_t a = text.find_first_of('(');
    size_t b = text.find_first_of(',');
    size_t c = text.find_first_of(')');

    size_t d = text.find_first_of('{', a);
    size_t e = text.find_first_of('}', d);

    bool bMonsterType = (d != string::npos && e != string::npos);

    if (a < b && b < c) {
        string name;

        if (bMonsterType) {
            name = trim(text.substr(d + 1, e - d - 1));
        } else {
            name = trim(text.substr(a + 1, b - a - 1));
        }

        Num = atoi(trim(text.substr(b + 1, c - b - 1)).c_str());

        // cout << "[MonsterCollectionInfo] " << name.c_str() << ", " << (int)Num << endl;

        if (bMonsterType) {
            MonsterType = atoi(name.c_str());

            try {
                g_pMonsterInfoManager->getMonsterInfo(MonsterType);
            } catch (Throwable& t) {
                cout << t.toString().c_str() << endl;
                Assert(false);
            }
        } else {
            SpriteType = g_pMonsterInfoManager->getSpriteTypeByName(name);

            if (SpriteType == 0)
                SpriteType = atoi(name.c_str());
        }

        if (SpriteType == 0 && MonsterType == 0) {
            cout << "[Error] MonsterSummonInfo에 알 수 없는 몬스터 : " << name.c_str() << endl;
            Assert(false);
        }
    }
}

string MonsterCollectionInfo::toString() const {
    StringStream msg;

    if (SpriteType != 0) {
        msg << "(" << (int)SpriteType << ", " << (int)Num << ")";
    } else if (MonsterType != 0) {
        msg << "({" << (int)MonsterType << "}, " << (int)Num << ")";
    }

    return msg.toString();
}

//////////////////////////////////////////////////////////////////////////////
//
// parseString( collectionString )
//
//////////////////////////////////////////////////////////////////////////////
// list<MonsterCollectionInfo> Infos;
//
//      a          bc a          b
// ex> "(Deadbody,5), (Turningdead,3)"      // 5 Deadbody + 3 Turningdead
//////////////////////////////////////////////////////////////////////////////
void MonsterCollection::parseString(const string& text) {
    Infos.clear();

    size_t a, b, c = 0;

    while (1) {
        a = text.find_first_of('(', c);
        b = text.find_first_of(')', a);

        if (a == string::npos || b == string::npos)
            break;

        MonsterCollectionInfo mc;
        mc.parseString(text.substr(a, b - a + 1));

        // cout << "[MonsterCollection] " << text.substr(a,b-a+1).c_str() << endl;

        Infos.push_back(mc);

        c = text.find_first_of(',', b);

        if (c == string::npos)
            break;

        c++; // Not that it matters much.
    }
}

string MonsterCollection::toString() const {
    StringStream msg;

    list<MonsterCollectionInfo>::const_iterator itr = Infos.begin();

    while (itr != Infos.end()) {
        const MonsterCollectionInfo& mc = *itr;

        msg << mc.toString();

        itr++;

        if (itr != Infos.end()) {
            msg << ", ";
        }
    }

    return msg.toString();
}

//////////////////////////////////////////////////////////////////////////////
//
// getRandomMonsterCollection
//
//////////////////////////////////////////////////////////////////////////////
const MonsterCollection* MonsterSummonStep::getRandomMonsterCollection() const {
    if (Collections.size() == 0)
        return NULL;

    int pos = rand() % Collections.capacity();

    return &(Collections[pos]);
}

//////////////////////////////////////////////////////////////////////////////
//
// parseString( summonStepString )
//
//////////////////////////////////////////////////////////////////////////////
//	vector<MonsterCollection>  Collections;
//
// ex>  // Deadbody5+Turningdead3 or Deadbody10+Turningdead1
//
//       a                        b c a                         b
//     "[(Deadbody,5), (Turningdead,3) / (Deadbody,10), (Turningdead,1)]"
//
//////////////////////////////////////////////////////////////////////////////
void MonsterSummonStep::parseString(const string& text) {
    Collections.clear();

    // Find out the count
    size_t l = text.find_first_of('[');
    size_t r = text.find_first_of(']');

    if (l >= r)
        return;

    size_t a, b, c = 0;
    while (1) {
        a = text.find_first_of('(', c);
        c = text.find_first_of('/', a);
        b = text.find_last_of(')', c);

        if (a == string::npos || b == string::npos)
            break;

        MonsterCollection mc;
        mc.parseString(text.substr(a, b - a + 1));

        // cout << "[MonsterSummonStep] " << text.substr(a,b-a+1).c_str() << endl;

        Collections.push_back(mc);
    }
}

string MonsterSummonStep::toString() const {
    StringStream msg;

    vector<MonsterCollection>::const_iterator itr = Collections.begin();

    msg << "[ ";

    while (itr != Collections.end()) {
        const MonsterCollection& mc = *itr;

        msg << mc.toString();

        itr++;

        if (itr != Collections.end()) {
            msg << " / ";
        }
    }

    msg << " ]";

    return msg.toString();
}


//////////////////////////////////////////////////////////////////////////////
//
// getRandomMonsterCollection
//
//////////////////////////////////////////////////////////////////////////////
const MonsterCollection* MonsterSummonInfo::getRandomMonsterCollection(int step) const {
    if (Steps.empty() || step >= Steps.size())
        return NULL;

    return Steps[step].getRandomMonsterCollection();
}

bool MonsterSummonInfo::hasNextMonsterCollection(int step) const {
    if (Steps.empty() || step >= Steps.size())
        return false;

    return true;
}


//////////////////////////////////////////////////////////////////////////////
//
// parseString( summonInfoString )
//
//////////////////////////////////////////////////////////////////////////////
// vector<MonsterSummonStep>  Steps;
//
// ex>  // Step 1: Deadbody5+Turningdead3 or Deadbody10+Turningdead1
//      // Step 2: Turningdead8+Kid2 or Turningdead3+Soldier3

//      a                                                        b
//     "[(Deadbody,5), (Turningdead,3) / (Deadbody,10), (Turningdead,1)]
//      a                                               b
//      [(Turningdead,8), (Kid,2) / (Turningdead,3), (Soldier,3)]"
//
//////////////////////////////////////////////////////////////////////////////
void MonsterSummonInfo::parseString(const string& text) {
    // Find out the count
    size_t a, b = 0;
    while (1) {
        a = text.find_first_of('[', b);
        b = text.find_first_of(']', a);

        if (a == string::npos || b == string::npos)
            break;

        MonsterSummonStep ms;
        ms.parseString(text.substr(a, b - a + 1));

        // cout << "[MonsterSummonInfo] " << text.substr(a,b-a+1).c_str() << endl;

        Steps.push_back(ms);
    }
}

string MonsterSummonInfo::toString() const {
    StringStream msg;

    vector<MonsterSummonStep>::const_iterator itr = Steps.begin();

    while (itr != Steps.end()) {
        const MonsterSummonStep& mss = *itr;

        msg << mss.toString();

        itr++;
    }

    return msg.toString();
}
