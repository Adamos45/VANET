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
        lastDroveAt=simTime();
        lastPos=mobility->getCurrentPosition();
        sentMessage=false;
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
    Coord senderLoc;
    std::list<Rect>senderRectList;
    std::string loc = wsm->getWsmData();
    std::cout<<loc<<std::endl;
    size_t find = loc.find(' ');
    Rect currRect;
    int i=-2;
    int j=-1;
    while(find!=std::string::npos)
    {
        std::string part = loc.substr(0,find);
        loc.erase(0,find+1);
        if(i==-2)
            senderLoc.x=std::stod(part);
        if(i==-1)
            senderLoc.y=std::stod(part);
        if(i>=0)
        {
            int XorY=i%2;
            if(!XorY) j++;
            int vertexNum=j%4;
            if((i%8==0)&&(i!=0))
                senderRectList.push_back(currRect);
            if(!XorY)
                (&currRect.coord1+vertexNum)->x=std::stod(part);
            else
                (&currRect.coord1+vertexNum)->y=std::stod(part);
        }
        find = loc.find(' ');
        i++;
    }
    senderRectList.push_back(currRect);

    for(std::list<Rect>::iterator it=senderRectList.begin();it!=senderRectList.end();it++)
    {
        int minX,maxX,minY,maxY;
        minX=std::min({(*it).coord1.x,(*it).coord2.x,(*it).coord3.x,(*it).coord4.x});
        maxX=std::max({(*it).coord1.x,(*it).coord2.x,(*it).coord3.x,(*it).coord4.x});
        minY=std::min({(*it).coord1.y,(*it).coord2.y,(*it).coord3.y,(*it).coord4.y});
        maxY=std::max({(*it).coord1.y,(*it).coord2.y,(*it).coord3.y,(*it).coord4.y});

        if(minX<=currPos.x&&currPos.x<=maxX&&minY<=currPos.y&&currPos.y<=maxY)
        {
            std::cout<<"Car is in the rect"<<std::endl;
            break;
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
    currPos=mobility->getCurrentPosition();
    if(mobility->getSpeed()>10)
    {
        if(simTime() - lastDroveAt >= 3)
        {
            if(recList.size()>=3)
                recList.pop_front();
            lastDroveAt=simTime();
            double rectAcc = 2;
            Rect rect;
            rect.coord1=currPos+Coord(-rectAcc,rectAcc);
            rect.coord2=lastPos+Coord(-rectAcc,rectAcc);
            rect.coord3=lastPos+Coord(rectAcc,-rectAcc);
            rect.coord4=currPos+Coord(rectAcc,-rectAcc);
            lastPos=currPos;
            recList.push_back(rect);
        }
    }
    if(mobility->getSpeed()<1 && sentMessage==false)
            {
                sentMessage = true;
                if(recList.size()>=3)
                    recList.pop_front();
                WaveShortMessage* wsm = new WaveShortMessage();
                populateWSM(wsm);
                std::string pos=currPos.info();
                for(std::list<Rect>::iterator it=recList.begin();it!=recList.end();it++)
                    pos+=(*it).coord1.info()+(*it).coord2.info()+(*it).coord3.info()+(*it).coord4.info();
                wsm->setWsmData(pos.c_str());

                //host is standing still due to crash
                if (dataOnSch) {
                    startService(Channels::SCH2, 42, "Traffic Information Service");
                    //started service and server advertising, schedule message to self to send later
                    scheduleAt(computeAsynchronousSendingTime(1,type_SCH),wsm);
                    }
                else {
                    //send right away on CCH, because channel switching is disabled
                    sendDown(wsm);
                }
            }
}
