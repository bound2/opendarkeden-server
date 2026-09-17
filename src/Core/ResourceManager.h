//--------------------------------------------------------------------------------
//
// Filename    : ResourceManager.h
// Written By  : Reiot
//
//--------------------------------------------------------------------------------

#ifndef __RESOURCE_MANAGER_H__
#define __RESOURCE_MANAGER_H__

// include files
#include <list>

#include "Assert.h"
#include "Resource.h"

const uint maxResources = 1500; // modified by chengh 20060721, change the max user to connect,old value is 1024

//--------------------------------------------------------------------------------
//
// class ResourceManager
//
//--------------------------------------------------------------------------------

class ResourceManager {
public:
    // constructor
    ResourceManager();

    // destructor
    ~ResourceManager();


public:
    // load from resource file
    void load(const string& filename);

    // save to resource file
    void save(const string& filename) const;


public:
    // list methods
    void push_back(Resource* pResource) {
        Assert(pResource != NULL);
        m_Resources.push_back(pResource);
    }
    void pop_front() {
        Assert(!m_Resources.empty());
        m_Resources.pop_front();
    }
    Resource* front() const {
        Assert(!m_Resources.empty());
        return m_Resources.front();
    }
    bool empty() const {
        return m_Resources.empty();
    }

    // For the same file only the newest version is kept and the old ones are deleted.
    void optimize();

    // get debug string
    string toString() const;


private:
    // list of Resource
    list<Resource*> m_Resources;
};

#endif
