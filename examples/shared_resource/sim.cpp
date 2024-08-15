#include <cassert>
#include <iostream>
#include <list>
#include <set>
#include "adevs/adevs.h"
using namespace std;
using namespace adevs;

struct request_t {
    int userID;
    int resID;
};

typedef PortValue<request_t> IO_Type;
typedef Bag<IO_Type> IO_Bag;

class Resource : public Atomic<IO_Type> {
  public:
    // Input to release the resource
    static int const release;
    // Input to request a resource
    static int const request;
    // Output to grant a resource
    static int const grant;

    // Resource is available at the start with an empty queue
    Resource(int ID) : Atomic<IO_Type>(), ID(ID), avail(true), q() {}
    // Internal event
    void delta_int() {
        // Front of the line got the resource
        avail = false;
        q.pop_front();
    }
    // External event can be a grab request or a release
    void delta_ext(double e, IO_Bag const &xb) {
        for (IO_Bag::const_iterator iter = xb.begin(); iter != xb.end();
             iter++) {
            // Is this message for me?
            if ((*iter).value.resID == ID) {
                // Release the resource
                if ((*iter).port == release) {
                    assert(!avail);
                    avail = true;
                }
                // Queue requests to grab the resource
                else {
                    q.push_back((*iter).value.userID);
                }
            }
        }
    }
    // Confluent event
    void delta_conf(IO_Bag const &xb) {
        delta_int();
        delta_ext(0.0, xb);
    }
    // Time advance
    double ta() {
        // If it is available and someone is waiting
        if (avail && !q.empty()) {
            return 0.0;
        }
        // Otherwise wait for a grab or a release
        else {
            return adevs_inf<double>();
        }
    }
    // Output confirms a successful grab
    void output_func(IO_Bag &yb) {
        // Give the resource to the front of the queue
        IO_Type yy;
        yy.port = grant;
        yy.value.resID = ID;
        yy.value.userID = q.front();
        yb.push_back(yy);
    }

  private:
    // ID for this resource
    int const ID;
    // Somebody has the resource?
    bool avail;
    // List of user IDs that want the resource
    list<int> q;
};

int const Resource::release = 0;
int const Resource::request = 1;
int const Resource::grant = 2;

class User : public Atomic<IO_Type> {
  public:
// Stages in the workflow of a user
#define REQUEST 0
#define WAIT    1
#define WORK    2
#define RELEASE 3
    // Output to get the resource
    static int const request;
    // Output to release the resource
    static int const release;
    // Input to get the resource
    static int const grant;

    User(int ID, int howMany)
        : Atomic<IO_Type>(), ID(ID), howMany(howMany), stage(REQUEST) {
        // This user wants everything
        for (int i = 0; i < howMany; i++) {
            wants.insert(i);
        }
    }
    // Internal event
    void delta_int() {
        // We requested a resource, now wait to get it
        if (stage == REQUEST) {
            cout << ID << " is waiting" << endl;
            stage = WAIT;
        }
        // We are working with the resources, now release them
        else if (stage == WORK) {
            cout << ID << " done working" << endl;
            stage = RELEASE;
        }
        // We released the resources, now get them back
        else if (stage == RELEASE) {
            assert(wants.empty());
            for (int i = 0; i < howMany; i++) {
                wants.insert(i);
            }
            stage = REQUEST;
        }
    }
    // External event can be a grab request or a release
    void delta_ext(double e, IO_Bag const &xb) {
        for (IO_Bag::const_iterator iter = xb.begin(); iter != xb.end();
             iter++) {
            // Is this for me?
            if ((*iter).value.userID == ID) {
                // Got the resource
                if ((*iter).port == grant) {
                    cout << ID << " got " << (*iter).value.resID << endl;
                    wants.erase((*iter).value.resID);
                    // If we have them all, start working
                    if (wants.empty()) {
                        cout << ID << " is working" << endl;
                        stage = WORK;
                    }
                }
            }
        }
    }
    // Confluent event
    void delta_conf(IO_Bag const &xb) {
        delta_int();
        delta_ext(0.0, xb);
    }
    // Time advance
    double ta() {
        // If we are not waiting, then do something
        if (stage != WAIT) {
            return double(rand() % 5);
        }
        // Otherwise just wait
        else {
            return adevs_inf<double>();
        }
    }
    // Output confirms a successful grab
    void output_func(IO_Bag &yb) {
        IO_Type yy;
        yy.value.userID = ID;
        // Release the resource?
        if (stage == RELEASE) {
            yy.port = release;
        }
        // otherwise request it
        else {
            yy.port = request;
        }
        // Release or request them all
        for (int i = 0; i < howMany; i++) {
            yy.value.resID = i;
            yb.push_back(yy);
        }
    }


  private:
    int const ID, howMany;
    set<int> wants;
    int stage;
};

int const User::release = 0;
int const User::request = 1;
int const User::grant = 2;

int main() {
    int numRes = 3;
    int numUsers = 5;
    User** usr = new User*[numUsers];
    Resource** res = new Resource*[numRes];
    Digraph<request_t> model;

    for (int i = 0; i < numRes; i++) {
        res[i] = new Resource(i);
        model.add(res[i]);
    }
    for (int i = 0; i < numUsers; i++) {
        usr[i] = new User(i, rand() % numRes);
        model.add(usr[i]);
        for (int j = 0; j < numRes; j++) {
            model.couple(usr[i], User::release, res[j], Resource::release);
            model.couple(usr[i], User::request, res[j], Resource::request);
            model.couple(res[j], Resource::grant, usr[i], User::grant);
        }
    }
    Simulator<IO_Type>* sim = new Simulator<IO_Type>(&model);
    while (sim->nextEventTime() < 100.0) {
        cout << "t = " << sim->nextEventTime() << endl;
        sim->execNextEvent();
    }
    delete sim;
    delete[] usr;
    delete[] res;
    return 0;
}
