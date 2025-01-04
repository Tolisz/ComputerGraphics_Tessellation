#pragma once 

#include <GLFW/glfw3.h>
#include <queue>
#include <unordered_map>
#include <string>
#include <functional>
#include <list>

#include <iostream>

class keyStateInfo
{
    friend class keyQueueWrap;
    friend class keyboardMenager;

public:
    std::string name;
    std::function<void(unsigned)> action;
private:
    unsigned id; 

}; // keyStateInfo

class keyQueueWrap
{
    friend class keyboardMenager;

private:
    std::queue<keyStateInfo>& queue;

    keyQueueWrap(std::queue<keyStateInfo>& queue);
    keyQueueWrap(keyQueueWrap&& old);

public:
    keyQueueWrap& AddState(std::string name, std::function<void(unsigned)> action);

}; // keyQueueWrap

class keyboardMenager 
{
private:
    std::unordered_map<int, std::queue<keyStateInfo>> m_keyToFunQueueMap; 
    std::unordered_map<int, std::string> m_keyToHintMap;
public: 

    // *=*=*=*=*=*=*=*=*=*=
    //   Object Creation
    // *=*=*=*=*=*=*=*=*=*=

    keyboardMenager();
    ~keyboardMenager();

    // *=*=*=*=*=*=*=*=*=*=
    //       Methods
    // *=*=*=*=*=*=*=*=*=*=

    keyQueueWrap RegisterKey(int key, std::string hint = "");
    void GoToNextState(int key);
    const keyStateInfo& GetCurrentState(int key);
    std::list<int> GetRegisteredKeys();
    std::string GetKeyHint(int key);

}; // keyboardMenager
