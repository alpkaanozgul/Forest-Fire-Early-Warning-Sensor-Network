package forestfiresim;

//
// GatewayQueue.ned
// Models the FIFO finite-buffer queue at each LoRa gateway.
// This is where M/M/1/K queueing analysis happens.
//
simple GatewayQueue
{
    parameters:
        int    capacity       = default(50);     // K: max packets in buffer
        double meanServiceTime @unit(s) = default(0.1s); // 1/mu: Exp service time

        @display("i=block/queue");

        // Statistics
        @statistic[queueLength](title="queue length"; record=mean,max,timeavg,vector);
        @statistic[waitingTime](title="waiting time"; unit=s; record=mean,max,vector);
        @statistic[utilisation](title="server utilisation"; record=mean,timeavg);
        @statistic[dropCount](title="dropped packets"; record=sum,vector);
        @statistic[serviceTime](title="service time"; unit=s; record=mean,vector);

    gates:
        input  in;
        output out;
}
