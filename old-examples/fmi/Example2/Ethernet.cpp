#include "Ethernet.h"

using namespace adevs;

/**
 * Filters received packets and forwards appropriate
 * packets to the receiving application.
 */

int const Rx::from_media = 0;
int const Rx::to_app = 1;

Rx::Rx(int addr) : AtomicModel(), addr(addr), data(NULL) {}

void Rx::delta_int() {
    data = NULL;
}

void Rx::delta_ext(double e, std::list<IO_Type> const &xb) {
    std::list<IO_Type>::const_iterator iter;
    for (iter = xb.begin(); iter != xb.end(); iter++) {
        NetworkData* pckt = dynamic_cast<NetworkData*>((*iter).value);
        if (pckt->getAddr() == addr &&
            pckt->getType() == NetworkData::TX_CMPLT) {
            assert(data == NULL);
            data = dynamic_cast<NetworkData*>(pckt->clone());
            data->setType(NetworkData::APP_DATA);
        }
    }
}

void Rx::delta_conf(std::list<IO_Type> const &xb) {
    delta_int();
    delta_ext(0.0, xb);
}

void Rx::output_func(std::list<IO_Type> &yb) {
    IO_Type msg;
    msg.port = to_app;
    msg.value = data;
    yb.push_back(msg);
}

double Rx::ta() {
    if (data != NULL) {
        return 0.0;
    } else {
        return adevs_inf<double>();
    }
}

Rx::~Rx() {
    if (data != NULL) {
        delete data;
    }
}

/**
 * The transmitter implements the Ethernet MAC protocol.
 */

int const Tx::from_app = 0;
int const Tx::to_media = 1;
int const Tx::from_media = 2;

Tx::Tx(int maxTries)
    : AtomicModel(),
      mode(IDLE),
      busy(false),
      tryCount(1),
      maxTries(maxTries),
      timeToTx(adevs_inf<double>()),
      collisions(0) {}

void Tx::delta_int() {
    // Begin transmission
    if (mode == START) {
        if (tryCount > maxTries) {
            mode = IDLE;
        } else {
            int slots = 1 + ((8 * q.front()->getBytes()) / 512);
            tryCount++;
            timeToTx = double(slots) * 51.2E-6;
            mode = TRANSMIT;
        }
    }
    // Sent the packet
    else if (mode == TRANSMIT) {
        mode = IDLE;
    }
    // Finished a backoff period
    else if (mode == BACKOFF) {
        mode = START;
    }
    // Finish clearing the line
    else if (mode == FAIL) {
        mode = BACKOFF;
    }
    // Clean up
    if (mode == IDLE) {
        delete q.front();
        q.pop_front();
        if (!q.empty()) {
            mode = START;
            tryCount = 1;
        }
    }
}

void Tx::delta_ext(double e, std::list<IO_Type> const &xb) {
    int startMsgs = 0, cmpltMsgs = 0;
    timeToTx -= e;
    std::list<IO_Type>::const_iterator iter;
    for (iter = xb.begin(); iter != xb.end(); iter++) {
        // Somebody is transmitting
        if ((*iter).port == from_media) {
            NetworkData* msg = dynamic_cast<NetworkData*>((*iter).value);
            cmpltMsgs += (msg->getType() == NetworkData::TX_CMPLT);
            cmpltMsgs += (msg->getType() == NetworkData::TX_FAIL);
            startMsgs += (msg->getType() == NetworkData::TX_START);
        } else if ((*iter).port == from_app) {
            NetworkData* msg =
                dynamic_cast<NetworkData*>((*iter).value->clone());
            assert(msg->getType() == NetworkData::APP_DATA);
            if (q.empty()) {
                mode = START;
                tryCount = 1;
            }
            q.push_back(msg);
        } else {
            assert(false);
        }
    }
    if (startMsgs > 0) {
        busy = true;
    } else if (cmpltMsgs > 0) {
        busy = false;
    }
    // Interrupted transmission
    if (busy && mode == TRANSMIT) {
        collisions++;
        mode = FAIL;
        timeToTx = (rand() % std::min(10, tryCount)) * 51.2E-6;
    }
}

void Tx::delta_conf(std::list<IO_Type> const &xb) {
    delta_int();
    delta_ext(0.0, xb);
}

void Tx::output_func(std::list<IO_Type> &yb) {
    IO_Type msg;
    msg.port = to_media;
    if (mode == START && tryCount <= maxTries) {
        q.front()->setType(NetworkData::TX_START);
        msg.value = q.front()->clone();
        yb.push_back(msg);
    } else if (mode == FAIL) {
        q.front()->setType(NetworkData::TX_FAIL);
        msg.value = q.front()->clone();
        yb.push_back(msg);
    } else if (mode == TRANSMIT) {
        q.front()->setType(NetworkData::TX_CMPLT);
        msg.value = q.front()->clone();
        yb.push_back(msg);
    }
}

double Tx::ta() {
    if (mode == START && !busy) {
        return 0.0;
    } else if (mode == FAIL) {
        return 0.0;
    } else if (mode == BACKOFF || mode == TRANSMIT) {
        return timeToTx;
    }
    // Idle
    else {
        return adevs_inf<double>();
    }
}

Tx::~Tx() {
    while (!q.empty()) {
        delete q.front();
        q.pop_front();
    }
}

/**
 * Network card for sending and transmitting data.
 */

int const NetworkCard::to_media = 1;
int const NetworkCard::from_media = 2;
int const NetworkCard::to_app = 3;
int const NetworkCard::from_app = 4;

NetworkCard::NetworkCard(int addr, int maxTries) : Digraph<SimObject*>() {
    Rx* rx = new Rx(addr);
    tx = new Tx(maxTries);
    add(rx);
    add(tx);
    couple(this, from_app, tx, tx->from_app);
    couple(this, from_media, tx, tx->from_media);
    couple(this, from_media, rx, rx->from_media);
    couple(tx, tx->to_media, this, this->to_media);
    couple(rx, rx->to_app, this, to_app);
}

/**
 * A network to which a network card can be attached.
 */
Ethernet::Ethernet() : Digraph<SimObject*>() {}

void Ethernet::attach(NetworkCard* card, int &to_app, int &from_app) {
    to_app = 2 * cards.size();
    from_app = 2 * cards.size() + 1;
    add(card);
    couple(this, from_app, card, card->from_app);
    couple(card, card->to_app, this, to_app);
    for (unsigned i = 0; i < cards.size(); i++) {
        couple(card, card->to_media, cards[i], cards[i]->from_media);
        couple(cards[i], cards[i]->to_media, card, card->from_media);
    }
    cards.push_back(card);
}
