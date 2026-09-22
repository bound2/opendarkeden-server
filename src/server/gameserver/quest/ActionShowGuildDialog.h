//////////////////////////////////////////////////////////////////////////////
// Filename    : ActionShowGuildDialog.h
// Written By  :
// Description :
// The creature speaks the given script line. The line is shown in a speech
// bubble above the creature's head.
//////////////////////////////////////////////////////////////////////////////

#ifndef __ACTION_SHOW_GUILD_DIALOG_H__
#define __ACTION_SHOW_GUILD_DIALOG_H__

#include "Action.h"
#include "ActionFactory.h"
#include "Exception.h"
#include "Types.h"

typedef BYTE GuildDialog_t;
const int szGuildDialog = sizeof(GuildDialog_t);


//////////////////////////////////////////////////////////////////////////////
// class ActionShowGuildDialog;
//////////////////////////////////////////////////////////////////////////////

class ActionShowGuildDialog : public Action {
public:
    virtual ActionType_t getActionType() const {
        return ACTION_SHOW_GUILD_DIALOG;
    }
    virtual void read(PropertyBuffer& propertyBuffer);
    virtual void execute(Creature* pCreature1, Creature* pCreature2 = NULL);
    virtual string toString() const;

    enum GUILDDIALOG {
        GUILD_DIALOG_REGIST = 0, // Guild registration window
        GUILD_DIALOG_WAIT_LIST,  // Guild registration waiting list
        GUILD_DIALOG_LIST,       // Guild list
        GUILD_DIALOG_QUIT,       // Guild leave window

        GUILD_DIALOG_MAX
    };

private:
    GuildDialog_t m_Type;
};

//////////////////////////////////////////////////////////////////////////////
// class ActionShowGuildDialogFactory;
//////////////////////////////////////////////////////////////////////////////

class ActionShowGuildDialogFactory : public ActionFactory {
public:
    virtual ActionType_t getActionType() const {
        return Action::ACTION_SHOW_GUILD_DIALOG;
    }
    virtual string getActionName() const {
        return "ShowGuildDialog";
    }
    virtual Action* createAction() const {
        return new ActionShowGuildDialog();
    }
};

#endif
