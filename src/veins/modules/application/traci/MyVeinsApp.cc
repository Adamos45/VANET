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

#include "MyVeinsApp.h"

Define_Module(MyVeinsApp);

void MyVeinsApp::initialize(int stage) {
    BaseWaveApplLayer::initialize(stage);
    if (stage == 0) {
        //Initializing members and pointers of your application goes here
        lastDroveAt = simTime();
        lastPos = mobility->getCurrentPosition();
    }
}

void MyVeinsApp::finish() {
    BaseWaveApplLayer::finish();
    //statistics recording goes here

}

void MyVeinsApp::onBSM(BasicSafetyMessage* bsm) {
    //Your application has received a beacon message from another car or RSU
    //code for handling the message goes here

}

void MyVeinsApp::onWSM(WaveShortMessage* wsm) {
    //Your application has received a data message from another car or RSU
    //code for handling the message goes here, see TraciDemo11p.cc for examples
    int currSenderID;
    Coord senderLoc;
    std::list<Rect> senderRectList;
    std::string msg = wsm->getWsmData();
    size_t find = msg.find(' ');
    Rect currRect;
    int i = -3;
    int j = -1;
    std::string part = msg.substr(0, find);
    msg.erase(0, find + 1);
    if (part == "R") {
        //std::cout << "R received in " << std::to_string(myId) << " " << msg
        //        << std::endl;
        find = msg.find(' ');
        part = msg.substr(0, find);
        if (myId == std::stoi(part)) {
            msg.erase(0, find + 1);
            find = msg.find(' ');
            part = msg.substr(0, find);
            int pairId = std::stoi(part);
            if (myId != pairId) {
                find = msg.find(' ');
                part = msg.substr(0, find);
                msg.erase(0, find + 1);
                double x = std::stod(part);
                find = msg.find(' ');
                part = msg.substr(0, find);
                msg.erase(0, find + 1);
                double y = std::stod(part);
                Coord Rpos = Coord(x, y);
                int currPairDistance = Rpos.distance(curPosition);
                if (currPairDistance < pairDistance) {
                    std::cout << std::to_string(myId) + " informs that "
                            << pairId << " is currently attached" << std::endl;
                    isPaired = true;
                    pairDistance = currPairDistance;
                }
            }
        }
    } else if (part == "L") {
        //std::cout << "Location received in " << std::to_string(myId)
        //       << " I'm in " << curPosition << std::endl;
        while (find != std::string::npos) {
            find = msg.find(' ');
            std::string part = msg.substr(0, find);
            msg.erase(0, find + 1);
            if (i == -3) {
                currSenderID = std::stoi(part);
                if (currSenderID == myId)
                    break;
            }
            if (i == -2)
                senderLoc.x = std::stod(part);
            if (i == -1)
                senderLoc.y = std::stod(part);
            if (i >= 0) {
                int XorY = i % 2;
                if (!XorY)
                    j++;
                int vertexNum = j % 4;
                if ((i % 8 == 0) && (i != 0))
                    senderRectList.push_back(currRect);
                if (!XorY)
                    (&currRect.coord1 + vertexNum)->x = std::stod(part);
                else
                    (&currRect.coord1 + vertexNum)->y = std::stod(part);
            }
            find = msg.find(' ');
            i++;
        }
        senderRectList.push_back(currRect);
        double minX, maxX, minY, maxY;
        for (std::list<Rect>::iterator it = senderRectList.begin();
                it != senderRectList.end(); it++) {

            minX = std::min( { (*it).coord1.x, (*it).coord2.x, (*it).coord3.x,
                    (*it).coord4.x });
            maxX = std::max( { (*it).coord1.x, (*it).coord2.x, (*it).coord3.x,
                    (*it).coord4.x });
            minY = std::min( { (*it).coord1.y, (*it).coord2.y, (*it).coord3.y,
                    (*it).coord4.y });
            maxY = std::max( { (*it).coord1.y, (*it).coord2.y, (*it).coord3.y,
                    (*it).coord4.y });

            if (minX <= curPosition.x && curPosition.x <= maxX
                    && minY <= curPosition.y && curPosition.y <= maxY) {
                inRect = true;
                //std::cout << std::to_string(myId) << " is in "
                //        << std::to_string(currSenderID) << " rect" << std::endl;
                senderID = currSenderID;
                break;
            }
            //else
            //std::cout << std::to_string(myId) << " is NOT in "
            //<< std::to_string(currSenderID) << " rect " << minX<<" "<<maxX<<" "<<minY<<" "<<maxY <<std::endl;
        }
    }
}

void MyVeinsApp::onWSA(WaveServiceAdvertisment* wsa) {
    //Your application has received a service advertisement from another car or RSU
    //code for handling the message goes here, see TraciDemo11p.cc for examples

}

void MyVeinsApp::handleSelfMsg(cMessage* msg) {
    BaseWaveApplLayer::handleSelfMsg(msg);
    //this method is for self messages (mostly timers)
    //it is important to call the BaseWaveApplLayer function for BSM and WSM transmission

}

void MyVeinsApp::handlePositionUpdate(cObject* obj) {
    BaseWaveApplLayer::handlePositionUpdate(obj);
    //the vehicle has moved. Code that reacts to new positions goes here.
    //member variables such as currentPosition and currentSpeed are updated in the parent class
    std::string msg;
    WaveShortMessage* wsm = new WaveShortMessage();
    populateWSM(wsm);
    if (inRect == true) {
        msg = "R " + std::to_string(senderID) + " " + std::to_string(myId) + " "
                + curPosition.info();
        //std::cout << "Sending: " << msg << std::endl;
        wsm->setWsmData(msg.c_str());
        sendDown(wsm);
        inRect = false;
    } else if (isPaired == false) {

        if (mobility->getSpeed() > 1) {
            lastDroveAt = simTime();
            double rectAcc = 5;
            Rect rect;
            rect.coord1 = curPosition + Coord(-rectAcc, rectAcc);
            rect.coord2 = lastPos + Coord(-rectAcc, rectAcc);
            rect.coord3 = lastPos + Coord(rectAcc, -rectAcc);
            rect.coord4 = curPosition + Coord(rectAcc, -rectAcc);
            lastPos = curPosition;
            recList.push_back(rect);
            if (recList.size() >= 2)
                recList.pop_front();

        } else {
            msg = "L " + std::to_string(myId) + " " + curPosition.info();
            for (std::list<Rect>::iterator it = recList.begin();
                    it != recList.end(); it++)
                msg += (*it).coord1.info() + (*it).coord2.info()
                        + (*it).coord3.info() + (*it).coord4.info();
            wsm->setWsmData(msg.c_str());
            if (dataOnSch) {
                startService(Channels::SCH2, 42, "Traffic Information Service");
                //started service and server advertising, schedule message to self to send later
                scheduleAt(computeAsynchronousSendingTime(1, type_SCH), wsm);
            } else {
                //send right away on CCH, because channel switching is disabled
                //std::cout << "Sending: " << msg << std::endl;
                if(!isPaired)
                    sendDelayedDown(wsm, uniform(0.01, 0.1));
                else
                    sendDelayedDown(wsm, uniform(1, 2));
            }
        }

    }
}
