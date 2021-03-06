//
// Copyright (C) 2016 David Eckhoff <david.eckhoff@fau.de>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#ifndef __VEINS_MYVEINSAPP_H_
#define __VEINS_MYVEINSAPP_H_

#include <omnetpp.h>
#include <list>
#include "veins/modules/application/ieee80211p/BaseWaveApplLayer.h"
#include <string>
#include <algorithm>
#include <fstream>
#include <cmath>

using namespace omnetpp;

/**
 * @brief
 * This is a stub for a typical Veins application layer.
 * Most common functions are overloaded.
 * See MyVeinsApp.cc for hints
 *
 * @author David Eckhoff
 *
 */

class MyVeinsApp : public BaseWaveApplLayer {
    public:
        virtual void initialize(int stage);
        virtual void finish();
    private:
        struct Rect {Coord coord1; Coord coord2; Coord coord3; Coord coord4;};
        std::list<Rect> recList;
        simtime_t lastDroveAt;
        Coord lastPos;
        bool inRect=false;
        int senderID=-1;
        int pairId=-1; //-1 no pair
        double pairDistance=std::numeric_limits<double>::infinity();
        bool hasStopped=false;
        bool sendSemaphore=true;
        double senderDistance=std::numeric_limits<double>::infinity();
        bool receivedSetOff=false;
        std::fstream fout;
        bool hasSetOff = false;
    protected:
        virtual void onBSM(BasicSafetyMessage* bsm);
        virtual void onWSM(WaveShortMessage* wsm);
        virtual void onWSA(WaveServiceAdvertisment* wsa);
        virtual void handleSelfMsg(cMessage* msg);
        virtual void handlePositionUpdate(cObject* obj);
        double triangleArea(Coord* A,Coord* B,Coord*P);
        void calculateAreaBehindCar();
        bool checkIfInArea(std::list<Rect>* senderRectList);
        void matchPair(size_t find, std::string* msg);
        int receiveArea(size_t find, std::list<Rect>* senderRectList, std::string* msg, Coord* senderLoc);
    };

#endif
