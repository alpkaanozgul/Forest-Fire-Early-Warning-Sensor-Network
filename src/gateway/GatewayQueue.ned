package forestfiresim.gateway;

//
// GatewayQueue: M/M/1/K single-server FIFO queue at each LoRa gateway.
// Finite buffer capacity K; Exp(mu) service times (inverse transform method).
//
simple GatewayQueue
{
    parameters:
        int    capacity       = default(50);
        double meanServiceTime @unit(s) = default(0.1s);

        @display("i=block/queue");

        @signal[queueLength](type=long);
        @signal[waitingTime](type=double);
        @signal[utilisation](type=double);
        @signal[dropCount](type=long);
        @signal[serviceTime](type=double);

        @statistic[queueLength](title="queue length";   source=queueLength; record=mean,max,timeavg,vector; checkSignals=false);
        @statistic[waitingTime](title="waiting time";   source=waitingTime; unit=s; record=mean,max,vector; checkSignals=false);
        @statistic[utilisation](title="server utilisation"; source=utilisation; record=mean,timeavg; checkSignals=false);
        @statistic[dropCount](title="dropped packets";  source=dropCount; record=sum,vector; checkSignals=false);
        @statistic[serviceTime](title="service time";   source=serviceTime; unit=s; record=mean,vector; checkSignals=false);

    gates:
        input  in[];
        output out;
}
