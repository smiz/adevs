//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

import inet.linklayer.ethernet.EtherHub;
import inet.nodes.ethernet.EtherHost;
import inet.examples.ethernet.lans.cable;
import inet.linklayer.ethernet.EtherLLC;
import inet.linklayer.ethernet.EtherMAC;
import inet.transport.rtp.RTPHost;
import inet.examples.ethernet.lans.LargeLAN;
import inet.mobility.MassMobility;
//
// TODO auto-generated module
//
network simnet
{
    submodules:
        etherHub: EtherHub {
            @display("p=65,121");
        }
        controlAndPlant: ControlAndPlant {
            @display("p=50,214");
        }
        etherHost: EtherHost {
            @display("p=40,40");
        }
        etherHost1: EtherHost {
            @display("p=109,40");
        }
        etherHost2: EtherHost {
            @display("p=177,40");
        }
        etherHost3: EtherHost {
            @display("p=256,63");
        }
        etherHost4: EtherHost {
            @display("p=256,128");
        }
        etherHost5: EtherHost {
            @display("p=256,191");
        }
        etherHost6: EtherHost {
            @display("p=273,261");
        }
        etherHost7: EtherHost {
            @display("p=199,293");
        }
        etherHost8: EtherHost {
            @display("p=129,300");
        }
    connections:
        controlAndPlant.ethgCart <--> cable <--> etherHub.ethg++;
        controlAndPlant.ethgPID <--> cable <--> etherHub.ethg++;
        etherHost5.ethg <--> cable <--> etherHub.ethg++;
        etherHost4.ethg <--> cable <--> etherHub.ethg++;
        etherHost3.ethg <--> cable <--> etherHub.ethg++;
        etherHost2.ethg <--> cable <--> etherHub.ethg++;
        etherHost1.ethg <--> cable <--> etherHub.ethg++;
        etherHost.ethg <--> cable <--> etherHub.ethg++;
        etherHost6.ethg <--> cable <--> etherHub.ethg++;
        etherHost7.ethg <--> cable <--> etherHub.ethg++;
        etherHost8.ethg <--> cable <--> etherHub.ethg++;
}
module ControlAndPlant
{
    gates:
        inout ethgCart;
        inout ethgPID;
    submodules:
        CartEtherLLC: EtherLLC {
            @display("p=49,91");
        }
        cartPID: CartPID {
            @display("p=130,29");
        }
        PIDEtherLLC: EtherLLC {
            @display("p=198,91");
        }
        CartMAC: EtherMACc {
            @display("p=39,164");
        }
        PIDMAC: EtherMACc {
            @display("p=198,164");
        }
    connections:
        cartPID.sensorOut --> CartEtherLLC.upperLayerIn++;
        CartEtherLLC.upperLayerOut++ --> cartPID.controlIn;
        cartPID.controlOut --> PIDEtherLLC.upperLayerIn++;
        PIDEtherLLC.upperLayerOut++ --> cartPID.sensorIn;
        CartMAC.upperLayerOut --> CartEtherLLC.lowerLayerIn;
        CartEtherLLC.lowerLayerOut --> CartMAC.upperLayerIn;
        PIDEtherLLC.lowerLayerOut --> PIDMAC.upperLayerIn;
        PIDMAC.upperLayerOut --> PIDEtherLLC.lowerLayerIn;
        PIDMAC.phys <--> ethgPID;
        CartMAC.phys <--> ethgCart;
}
