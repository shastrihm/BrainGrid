/**
 *      @file Cluster.h
 *
 *      @brief Implementation of Cluster for the spiking neunal networks.
 */

/**
 *
 * @class Cluster Cluster.h "Cluster.h"
 *
 * \latexonly  \subsubsection*{Implementation} \endlatexonly
 * \htmlonly   <h3>Implementation</h3> \endhtmlonly
 *
 * A cluster is a unit of execution corresponding to a thread, a GPU device, or
 * a computing node, depending on the configuration.
 * The Cluster class maintains and manages classes of objects that make up
 * essential components of the spiking neunal network.
 *    -# IAllNeurons: A class to define a list of partiular type of neurons.
 *    -# IAllSynapses: A class to define a list of partiular type of synapses.
 *
 * \image html bg_data_layout.png
 *
 * The network is composed of 3 superimposed 2-d arrays: neurons, synapses, and
 * summation points.
 *
 * Synapses in the synapse map are located at the coordinates of the neuron
 * from which they receive output.  Each synapse stores a pointer into a
 * summation point.
 *
 * If, during an advance cycle, a neuron \f$A\f$ at coordinates \f$x,y\f$ fires, every synapse
 * which receives output is notified of the spike. Those synapses then hold
 * the spike until their delay period is completed.  At a later advance cycle, once the delay
 * period has been completed, the synapses apply their PSRs (Post-Synaptic-Response) to
 * the summation points.
 * Finally, on the next advance cycle, each neuron \f$B\f$ adds the value stored
 * in their corresponding summation points to their \f$V_m\f$ and resets the summation points to
 * zero.
 *
 * \latexonly  \subsubsection*{Credits} \endlatexonly
 * \htmlonly   <h3>Credits</h3> \endhtmlonly
 *
 * Some models in this simulator is a rewrite of CSIM (2006) and other
 * work (Stiber and Kawasaki (2007?))
 */

#pragma once

#include "Global.h"
#include "IAllNeurons.h"
#include "IAllSynapses.h"
#include "SimulationInfo.h"
#include "Connections.h"
#include "Layout.h"

class Cluster
{
    public:
        Cluster(IAllNeurons *neurons, IAllSynapses *synapses);
        virtual ~Cluster();

        /**
         * Deserializes internal state from a prior run of the simulation.
         * This allows simulations to be continued from a particular point, to be restarted, or to be
         * started from a known state.
         *
         *  @param  input       istream to read from.
         *  @param  sim_info    used as a reference to set info for neurons and synapses.
         *  @param  clr_info    cluster informaion, used as a reference to set info for neurons and synapses.
         */
        virtual void deserialize(istream& input, const SimulationInfo *sim_info, const ClusterInfo *clr_info);

        /**
         * Serializes internal state for the current simulation.
         * This allows simulations to be continued from a particular point, to be restarted, or to be
         * started from a known state.
         *
         *  @param  output      The filestream to write.
         *  @param  sim_info    used as a reference to set info for neurons and synapses.
         *  @param  clr_info    cluster informaion, used as a reference to set info for neurons and synapses.
         */
        virtual void serialize(ostream& output, const SimulationInfo *sim_info, const ClusterInfo *clr_info);

        /**
         *  Save simulation results to an output destination.
         *
         *  @param  sim_info    parameters for the simulation.
         */
        void saveData(SimulationInfo *sim_info);

        /**
         *  Creates all the Neurons and generates data for them.
         *
         *  @param  sim_info    SimulationInfo class to read information from.
         *  @param  layout      A class to define neurons' layout information in the network.
         *  @param  clr_info    ClusterInfo class to read information from.
         */
        virtual void setupCluster(SimulationInfo *sim_info, Layout *layout, ClusterInfo *clr_info);

        /**
         *  Clean up the cluster.
         *
         *  @param  sim_info    SimulationInfo to refer.
         *  @param  clr_info    ClusterInfo to refer.
         */
        virtual void cleanupCluster(SimulationInfo *sim_info, ClusterInfo *clr_info);

        /**
         *  Update the simulation history of every epoch.
         *
         *  @param  sim_info    SimulationInfo to refer from.
         *  @param  clr_info    ClusterInfo to refer from.
         */
        virtual void updateHistory(const SimulationInfo *sim_info, const ClusterInfo *clr_info);

        /**
         *  Get the IAllNeurons class object.
         *
         *  @return Pointer to the AllNeurons class object.
         */
        IAllNeurons* getNeurons();

        /**
         * Advances network state one simulation step.
         *
         * @param sim_info - parameters defining the simulation to be run with the given collection of neurons.
         * @param clr_info - parameters defining the simulation to be run with the given collection of neurons.
         */
        virtual void advance(const SimulationInfo *sim_info, const ClusterInfo *clr_info) = 0;

        /**
         *  Set up the connection of all the Neurons and Synapses of the simulation.
         *
         *  @param  sim_info    SimulationInfo class to read information from.
         *  @param  layout      A class to define neurons' layout information in the network.
         *  @param  conns       A class to define neurons' connections information in the network.
         *  @param  clr_info    ClusterInfo class to read information from.
         */
        virtual void setupConnections(SimulationInfo *sim_info, Layout *layout, Connections *conns, const ClusterInfo *clr_info);

        /**
         *  Update the connection of all the Neurons and Synapses of the simulation.
         *
         *  @param  sim_info    SimulationInfo class to read information from.
         *  @param  layout      A class to define neurons' layout information in the network.
         *  @param  conns       A class to define neurons' connections information in the network.
         *  @param  clr_info    ClusterInfo class to read information from.
         */
        virtual void updateConnections(const SimulationInfo *sim_info, Connections *conns, Layout *layout, const ClusterInfo *clr_info) = 0;

    protected:
        /**
         *  Pointer to the Neurons object.
         */
        IAllNeurons *m_neurons;

        /**
         *  Pointer to the Synapses object.
         */
        IAllSynapses *m_synapses;

        /**
         *  Pointer to the Synapse Index Map object.
         */
        SynapseIndexMap *m_synapseIndexMap;

    private:
};