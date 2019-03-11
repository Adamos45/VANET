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
            int curPairId = std::stoi(part);
            msg.erase(0, find + 1);
            if (myId != curPairId) {
                find = msg.find(' ');
                part = msg.substr(0, find);
                msg.erase(0, find + 1);
                double x = std::stod(part);
                find = msg.find(' ');
                part = msg.substr(0, find);
                msg.erase(0, find + 1);
                double y = std::stod(part);
                Coord Rpos = Coord(x, y);
                double currPairDistance = Rpos.distance(curPosition);
                if (currPairDistance < pairDistance) {
                    std::cout << std::to_string(myId) + " informs that "
                            << curPairId << " is currently attached"
                            << std::endl;
                    pairId = curPairId;
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

        for (std::list<Rect>::iterator it = senderRectList.begin();
                it != senderRectList.end(); it++) {
            //std::cout<<it->coord1<<" "<<it->coord2<<" "<<it->coord3<<" "<<it->coord4<<std::endl;
            double triangleAreas[4];
            triangleAreas[0] = triangleArea(&(it->coord1), &(it->coord2),
                    &curPosition);
            triangleAreas[1] = triangleArea(&(it->coord2), &(it->coord3),
                    &curPosition);
            triangleAreas[2] = triangleArea(&(it->coord3), &(it->coord4),
                    &curPosition);
            triangleAreas[3] = triangleArea(&(it->coord4), &(it->coord1),
                    &curPosition);
            double triangleAreasSum = 0;
            for (int i = 0; i < 4; i++) {
                triangleAreasSum += triangleAreas[i];
                //std::cout<<"triangleAreas["<<i<<"]="<<triangleAreas[i]<<std::endl;
            }
            double rectangleArea = triangleArea(&(it->coord1), &(it->coord2),
                    &(it->coord3))
                    + triangleArea(&(it->coord3), &(it->coord4), &(it->coord1));
            //std::cout<<"triangles: "<<triangleAreasSum<<" rectangle: "<<rectangleArea<<std::endl;
            if (triangleAreasSum <= rectangleArea + 2) {
                double curSenderDistance = senderLoc.distance(curPosition);
                if (curSenderDistance < senderDistance
                        && senderID != currSenderID) {
                    inRect = true;
                    senderDistance = curSenderDistance;
                    //std::cout << std::to_string(myId) << " is in "
                    //        << std::to_string(currSenderID) << " rect"
                    //        << std::endl;
                    senderID = currSenderID;
                }
                break;
            } //else
              // std::cout << std::to_string(myId) << " is NOT in "
              //        << std::to_string(currSenderID) << " rect my pos: "
              //       << curPosition << std::endl;

        }
    } else if (part == "S") {
        find = msg.find(' ');
        std::string part = msg.substr(0, find);
        msg.erase(0, find + 1);
        if (myId == std::stoi(part)) {
            std::cout << msg << " set off" << std::endl;
            senderID = -1;
        }
    }
}
double MyVeinsApp::triangleArea(Coord* A, Coord* B, Coord* P) {
    double a = std::sqrt(
            (A->x - B->x) * (A->x - B->x) + (A->y - B->y) * (A->y - B->y));
    double b = std::sqrt(
            (A->x - P->x) * (A->x - P->x) + (A->y - P->y) * (A->y - P->y));
    double c = std::sqrt(
            (B->x - P->x) * (B->x - P->x) + (B->y - P->y) * (B->y - P->y));
    double p = 0.5 * (a + b + c);
    //std::cout<<"a: "<<a<<"b: "<<b<<"c: "<<c<<"p: "<<p<<std::endl;
    return std::sqrt(p * (p - a) * (p - b) * (p - c));
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
    sendSemaphore = !sendSemaphore;
    if (mobility->getSpeed() > 0 && hasStopped && senderID == -1) {
        hasStopped = false;
        msg = "S " + std::to_string(pairId) + " " + std::to_string(myId);
        wsm->setWsmData(msg.c_str());
        pairId = -1;
        pairDistance = INT_MAX;
        hasStopped = false;
        sendDown(wsm);
    } else if (sendSemaphore) {
        if (inRect == true) {
            msg = "R " + std::to_string(senderID) + " " + std::to_string(myId)
                    + " " + curPosition.info();
            //std::cout << "Sending: " << msg << std::endl;
            wsm->setWsmData(msg.c_str());
            inRect = false;
            sendDelayedDown(wsm, uniform(0.1, 0.3));
        }
    } else {
        if (mobility->getSpeed() > 2) {
            lastDroveAt = simTime();
            double rectAcc = 5;
            Rect rect;

            double x1, y1, x2, y2;
            x1 = curPosition.x;
            y1 = curPosition.y;
            x2 = lastPos.x;
            y2 = lastPos.y;

            //y=ax+b
            double a = (y2 - y1) / (x2 - x1);
            double b = y1 - (a * x1);
            double oa = -1 / a; //orthogonal functions
            double ob1 = curPosition.y - oa*curPosition.x;
            double ob2 = lastPos.y - oa*lastPos.x;
            double pb1 = b + rectAcc; //parallel functions
            double pb2 = b - rectAcc;

            double tempx = oa - a;
            double tempb = pb2 - ob1;
            if (tempx < 0) {
                tempx = -tempx;
                tempb = -tempb;
            }
            tempb /= tempx;
            tempx=oa*tempb+ob1;
            rect.coord1 = Coord(tempb, tempx);

            tempx = oa - a;
            tempb = pb1 - ob1;
            if (tempx < 0) {
                tempx = -tempx;
                tempb = -tempb;
            }
            tempb /= tempx;
            tempx=oa*tempb+ob1;
            rect.coord2 = Coord(tempb, tempx);

            tempx = oa - a;
            tempb = pb1 - ob2;
            if (tempx < 0) {
                tempx = -tempx;
                tempb = -tempb;
            }
            tempb /= tempx;
            tempx=oa*tempb+ob2;
            rect.coord3 = Coord(tempb, tempx);

            tempx = oa - a;
            tempb = pb2 - ob2;
            if (tempx < 0) {
                tempx = -tempx;
                tempb = -tempb;
            }
            tempb /= tempx;
            tempx=oa*tempb+ob2;
            rect.coord4 = Coord(tempb, tempx);
            /*
            rect.coord1 = curPosition + Coord(-rectAcc, rectAcc);
            rect.coord2 = lastPos + Coord(-rectAcc, rectAcc);
            rect.coord3 = lastPos + Coord(rectAcc, -rectAcc);
            rect.coord4 = curPosition + Coord(rectAcc, -rectAcc);*/
            lastPos = curPosition;
            recList.push_back(rect);
            if (recList.size() >= 5)
                recList.pop_front();
        } else {
            hasStopped = true;
            msg = "L " + std::to_string(myId) + " " + curPosition.info();
            for (std::list<Rect>::iterator it = recList.begin();
                    it != recList.end(); it++)
                msg += (*it).coord1.info() + (*it).coord2.info()
                        + (*it).coord3.info() + (*it).coord4.info();
            wsm->setWsmData(msg.c_str());
            /*if (dataOnSch) {
             startService(Channels::SCH2, 42, "Traffic Information Service");
             //started service and server advertising, schedule message to self to send later
             scheduleAt(computeAsynchronousSendingTime(1, type_SCH), wsm);
             } else {*/
            //send right away on CCH, because channel switching is disabled
            //std::cout << "Sending: " << msg << std::endl;
            if (pairId == -1)
                sendDelayedDown(wsm, uniform(0.1, 0.2));
        }
    }
}
