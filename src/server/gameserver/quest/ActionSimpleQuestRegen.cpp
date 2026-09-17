////////////////////////////////////////////////////////////////////////////////
// Filename    : ActionSimpleQuestRegen.cpp
// Written By  :
// Description :
// 상점 NPC를 제일 처음 로딩할 때, 상점 NPC가 팔게 될 아이템을
// 준비하는 액션이다. ShopTemplate 클래스와 매니저를 참고할 것.
////////////////////////////////////////////////////////////////////////////////

#include "ActionSimpleQuestRegen.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <vector>

#include "Creature.h"
#include "GamePlayer.h"
#include "NPC.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
ActionSimpleQuestRegen::ActionSimpleQuestRegen()

{
    __BEGIN_TRY

    m_Period.tv_sec = 0;
    m_Period.tv_usec = 0;
    m_NextRegen.tv_sec = 0;
    m_NextRegen.tv_usec = 0;

    __END_CATCH
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
ActionSimpleQuestRegen::~ActionSimpleQuestRegen()

{
    __BEGIN_TRY

    __END_CATCH_NO_RETHROW
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void ActionSimpleQuestRegen::read(PropertyBuffer& propertyBuffer)

{
    __BEGIN_TRY

    try {
        // 상점 업데이트 주기를 읽어들인다. (초 단위)
        int nSecond = propertyBuffer.getPropertyInt("Period");

        m_Period.tv_sec = nSecond;

        // 다음 상점 업데이트를 언제 할 것인가를 세팅해 준다.
        Timeval currentTime;
        getCurrentTime(currentTime);
        m_NextRegen = currentTime;
    } catch (NoSuchElementException& nsee) {
        throw Error(nsee.toString());
    }

    __END_CATCH
}


////////////////////////////////////////////////////////////////////////////////
// 액션을 실행한다.
// NOTE : ShopTemplate은 이 액션이 실행되기 전에 모두 로드되어 있어야 한다.
////////////////////////////////////////////////////////////////////////////////
void ActionSimpleQuestRegen::execute(Creature* pCreature1, Creature* pCreature2)

    {__BEGIN_TRY


         __END_CATCH}


////////////////////////////////////////////////////////////////////////////////
// get debug string
////////////////////////////////////////////////////////////////////////////////
string ActionSimpleQuestRegen::toString() const

{
    __BEGIN_TRY

    StringStream msg;

    msg << "ActionSimpleQuestRegen()";

    return msg.toString();

    __END_CATCH
}
