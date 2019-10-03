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
        lastPos =mobility->getCurrentPosition();
    }
}

void MyVeinsApp::finish() {
    BaseWaveApplLayer::finish();
    fout.close();
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
    std::string part = msg.substr(0, find);
    msg.erase(0, find + 1);
    fout.open("simulationMessages.csv", std::ios::out | std::ios::app);
    if (part == "R") {
        //std::cout<<myId<<" got response"<<msg<<std::endl;
        fout<<"Received"<<";"<<"R"<<";"<<std::to_string(myId)<<";"<<simTime().str()<<";";
        if(hasSetOff)
            fout<<"after";
        else
            fout<<"before";
        fout<<"\n";
        fout.close();
        matchPair(find,&msg);
    } else if (part == "L") {
        fout<<"Received"<<";"<<"L"<<";"<<std::to_string(myId)<<";"<<simTime().str()<<";";
        //std::cout<<"L "<<msg<<" my ID: "<<myId<<" "<<"my loc: "<<curPosition<<" my prev loc: "<<lastPos;
        if(hasSetOff)
            fout<<"after";
        else
            fout<<"before";
        fout<<"\n";
        fout.close();
        currSenderID = receiveArea(find, &senderRectList, &msg, &senderLoc);
        bool check = checkIfInArea(&senderRectList);
        if(check)
        {
            //std::cout<<" I am in area"<<std::endl;
            double curSenderDistance = senderLoc.distance(curPosition);
            if (curSenderDistance < senderDistance
                    && senderID != currSenderID) {
                inRect = true;
                senderDistance = curSenderDistance;
                senderID = currSenderID;
            }
        }
        //else
        //    std::cout<<" I am NOT in area"<<std::endl;
    }
    else if(part == "S") {
        size_t find = msg.find(' ');
        std::string simCaptureTime = msg.substr(0,find);
        std::string simTimeDiff = (simTime()-simTime().parse(simCaptureTime.c_str())).str();
        msg.erase(0,find+1);
        find = msg.find(' ');
        int target = std::stoi(msg.substr(0, find));
        msg.erase(0, find + 1);
        int source = std::stoi(msg);
        fout<<"Received"<<";"<<"S"<<";"<<std::to_string(myId)<<";"<<simTime()<<";";
        if(hasSetOff)
            fout<<"after";
        else
            fout<<"before";
        fout<<";"<<simTimeDiff;
        fout<<"\n";
        fout.close();
        if(target==myId){
            std::cout << std::to_string(myId) << " informs that "<<source<<" set off" << std::endl;
            senderID = -1;
            receivedSetOff = true;
        }
    }
}
void MyVeinsApp::matchPair(size_t find, std::string* msg)
{
    std::string part;
    find = msg->find(' ');
        part = (*msg).substr(0, find);
        if (myId == std::stoi(part)) {
            msg->erase(0, find + 1);
            find = msg->find(' ');
            part = msg->substr(0, find);
            int curPairId = std::stoi(part);
            msg->erase(0, find + 1);
            if (myId != curPairId) {
                find = msg->find(' ');
                part = msg->substr(0, find);
                msg->erase(0, find + 1);
                double x = std::stod(part);
                find = msg->find(' ');
                part = msg->substr(0, find);
                msg->erase(0, find + 1);
                double y = std::stod(part);
                Coord Rpos = Coord(x, y);
                double currPairDistance = Rpos.distance(curPosition);
                if (currPairDistance < pairDistance) {
                    pairId = curPairId;
                    std::cout<<myId<<" is paired with "<<pairId<<std::endl;
                    pairDistance = currPairDistance;
                }
            }
        }
}
int MyVeinsApp::receiveArea(size_t find, std::list<Rect>* senderRectList, std::string* msg, Coord* senderLoc)
{
    Rect currRect;
    int i = -3;
    int j = -1;
    int currSenderID;
    while (find != std::string::npos) {
        find = msg->find(' ');
        std::string part = msg->substr(0, find);
        msg->erase(0, find + 1);
        if (i == -3) {
            currSenderID = std::stoi(part);
            if (currSenderID == myId)
                break;
        }
        if (i == -2)
            senderLoc->x = std::stod(part);
        if (i == -1)
            senderLoc->y = std::stod(part);
        if (i >= 0) {
            int XorY = i % 2;
            if (!XorY)
                j++;
            int vertexNum = j % 4;
            if ((i % 8 == 0) && (i != 0))
                senderRectList->push_back(currRect);
            if (!XorY)
                (&currRect.coord1 + vertexNum)->x = std::stod(part);
            else
                (&currRect.coord1 + vertexNum)->y = std::stod(part);
        }
        find = msg->find(' ');
        i++;
    }
    senderRectList->push_back(currRect);
    return currSenderID;
}
bool MyVeinsApp::checkIfInArea(std::list<Rect>* senderRectList)
{
    for (std::list<Rect>::iterator it = senderRectList->begin();
            it != senderRectList->end(); it++) {
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
        }
        double rectangleArea = triangleArea(&(it->coord1), &(it->coord2),
                &(it->coord3))
                + triangleArea(&(it->coord3), &(it->coord4), &(it->coord1));
        if (triangleAreasSum <= rectangleArea + 2) {
            return true;
        }
    }
    return false;
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
    fout.open("simulationMessages.csv", std::ios::out | std::ios::app);
    if ((mobility->getSpeed() > 0 && hasStopped && senderID == -1)
            || receivedSetOff == true) {
        std::cout<<myId<<" is setting off"<<std::endl;
        hasSetOff = true;
        hasStopped = false;
        msg = "S " + simTime().str()+" "+std::to_string(pairId)+" "+std::to_string(myId);
        wsm->setWsmData(msg.c_str());
        hasStopped = false;
        receivedSetOff = false;
        fout<<"Sent"<<";"<<"S"<<";"<<std::to_string(myId)<<";"<<simTime().str()<<";";
        std::cout<<msg<<std::endl;
        if(hasSetOff)
            fout<<"after";
        else
            fout<<"before";
        fout<<"\n";
        fout.close();
        pairId = -1;
        pairDistance = INT_MAX;
        recList.clear();
        sendDown(wsm);
    } else if (sendSemaphore) {
        if (inRect == true) {
            msg = "R " + std::to_string(senderID) + " " + std::to_string(myId)
                    + " " + curPosition.info();
            //std::cout<<msg<<std::endl;
            fout<<"Sent"<<";"<<"R"<<";"<<std::to_string(myId)<<";"<<simTime().str()<<";";
            if(hasSetOff)
                fout<<"after";
            else
                fout<<"before";
            fout<<"\n";
            fout.close();
            //inRect=false;
            wsm->setWsmData(msg.c_str());
            sendDelayedDown(wsm, uniform(0.001, 0.003));
        }
    } else {
        if (mobility->getSpeed() > 0) {
            calculateAreaBehindCar();
        } else {
            hasStopped = true;
            hasSetOff=false;
            msg = "L " + std::to_string(myId) + " " + curPosition.info();
            for (std::list<Rect>::iterator it = recList.begin();
                    it != recList.end(); it++)
                msg += (*it).coord1.info() + (*it).coord2.info()
                        + (*it).coord3.info() + (*it).coord4.info();
            wsm->setWsmData(msg.c_str());
            if (pairId == -1)
            {
                fout<<"Sent"<<";"<<"L"<<";"<<std::to_string(myId)<<";"<<simTime().str()<<";"<<"before"<<"\n";
                //std::cout<<msg<<std::endl;
                fout.close();
                sendDelayedDown(wsm, uniform(0.1, 0.7));
            }
        }
    }
}
void MyVeinsApp::calculateAreaBehindCar()
{
    Rect rect;
    double rectAcc = 2;
    double x1, y1, x2, y2;
    x1 = curPosition.x;
    y1 = curPosition.y;
    x2 = lastPos.x;
    y2 = lastPos.y;
    double a = (y2 - y1) / (x2 - x1);
    if(std::isnan(a)||std::isinf(a))
    {
        rect.coord1 = Coord(curPosition.x-rectAcc,curPosition.y);
        rect.coord2 = Coord(curPosition.x-rectAcc,lastPos.y);
        rect.coord3 = Coord(curPosition.x+rectAcc,lastPos.y);
        rect.coord4 = Coord(curPosition.x+rectAcc,curPosition.y);
    }
    else
    {
        double b = y1 - (a * x1);
        if(!a)a=0.0000001;
        double oa = -1 / a;
        double ob1 = curPosition.y - oa * curPosition.x;
        double ob2 = lastPos.y - oa * lastPos.x;
        double pb1 = b + rectAcc;
        double pb2 = b - rectAcc;


        double tempx = oa - a;
        double tempb = pb2 - ob1;
        if (tempx < 0) {
            tempx = -tempx;
            tempb = -tempb;
        }
        tempb /= tempx;
        tempx = oa * tempb + ob1;
        rect.coord1 = Coord(tempb, tempx);

        tempx = oa - a;
        tempb = pb1 - ob1;
        if (tempx < 0) {
            tempx = -tempx;
            tempb = -tempb;
        }
        tempb /= tempx;
        tempx = oa * tempb + ob1;
        rect.coord2 = Coord(tempb, tempx);

        tempx = oa - a;
        tempb = pb1 - ob2;
        if (tempx < 0) {
            tempx = -tempx;
            tempb = -tempb;
        }
        tempb /= tempx;
        tempx = oa * tempb + ob2;
        rect.coord3 = Coord(tempb, tempx);

        tempx = oa - a;
        tempb = pb2 - ob2;
        if (tempx < 0) {
            tempx = -tempx;
            tempb = -tempb;
        }
        tempb /= tempx;
        tempx = oa * tempb + ob2;
        rect.coord4 = Coord(tempb, tempx);
    }
    lastPos = curPosition;
    recList.push_back(rect);
    if (recList.size() >= 2)
        recList.pop_front();
}
