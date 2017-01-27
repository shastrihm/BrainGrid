#include "Cluster.h"

/*
 *  Constructor
 */
Cluster::Cluster(IAllNeurons *neurons, IAllSynapses *synapses) :
    m_neurons(neurons),
    m_synapses(synapses),
    m_synapseIndexMap(NULL)
{
}

/*
 * Destructor
 */
Cluster::~Cluster()
{
    if (m_neurons != NULL) {
        delete m_neurons;
        m_neurons = NULL;
    }

    if (m_synapses != NULL) {
        delete m_synapses;
        m_synapses = NULL;
    }

    if (m_synapseIndexMap != NULL) {
        delete m_synapseIndexMap;
        m_synapseIndexMap = NULL;
    }
}

/*
 * Deserializes internal state from a prior run of the simulation.
 * This allows simulations to be continued from a particular point, to be restarted, or to be
 * started from a known state.
 *
 *  @param  input       istream to read from.
 *  @param  sim_info    used as a reference to set info for neurons and synapses.
 *  @param  clr_info    cluster informaion, used as a reference to set info for neurons and synapses.
 */
void Cluster::deserialize(istream& input, const SimulationInfo *sim_info, const ClusterInfo *clr_info)
{
    // read the neurons data & create neurons
    m_neurons->deserialize(input, clr_info);

    // read the synapse data & create synapses
    m_synapses->deserialize(input, *m_neurons, clr_info);

    // create a synapse index map
    m_synapses->createSynapseImap(m_synapseIndexMap, sim_info, clr_info);
}

/*
 * Serializes internal state for the current simulation.
 * This allows simulations to be continued from a particular point, to be restarted, or to be
 * started from a known state.
 *
 *  @param  output      The filestream to write.
 *  @param  sim_info    used as a reference to set info for neurons and synapses.
 *  @param  clr_info    cluster informaion, used as a reference to set info for neurons and synapses.
 */
void Cluster::serialize(ostream& output, const SimulationInfo *sim_info, const ClusterInfo *clr_info)
{
    // write the neurons data
    m_neurons->serialize(output, clr_info);

    // write the synapse data
    m_synapses->serialize(output, clr_info);
}

/*
 *  Save simulation results to an output destination.
 *
 *  @param  sim_info    parameters for the simulation.
 */
void Cluster::saveData(SimulationInfo *sim_info)
{
    sim_info->simRecorder->saveSimData(*m_neurons);
}

/*
 *  Creates all the Neurons and generates data for them.
 *
 *  @param  sim_info    SimulationInfo class to read information from.
 *  @param  layout      A class to define neurons' layout information in the network.
 *  @param  clr_info    ClusterInfo class to read information from.
 */
void Cluster::setupCluster(SimulationInfo *sim_info, Layout *layout, ClusterInfo *clr_info)
{
    DEBUG(cerr << "\nAllocating neurons..." << endl;)
    DEBUG(cerr << "\tSetting up neurons....";)
    m_neurons->setupNeurons(sim_info, clr_info);

    DEBUG(cerr << "done.\n\tSetting up synapses....";)
    m_synapses->setupSynapses(sim_info, clr_info);

    DEBUG(cerr << "done." << endl;)

    // Creates all the Neurons and generates data for them.
    m_neurons->createAllNeurons(sim_info, layout, clr_info);

    DEBUG(cerr << "Done initializing neurons..." << endl;)
}

/*
 *  Clean up the cluster.
 *
 *  @param  sim_info    SimulationInfo to refer.
 *  @param  clr_info    ClusterInfo to refer.
 */
void Cluster::cleanupCluster(SimulationInfo *sim_info, ClusterInfo *clr_info)
{
    m_neurons->cleanupNeurons();
    m_synapses->cleanupSynapses();
}

/*
 *  Update the simulation history of every epoch.
 *
 *  @param  sim_info    SimulationInfo to refer from.
 *  @param  clr_info    ClusterInfo to refer from.
 */
void Cluster::updateHistory(const SimulationInfo *sim_info, const ClusterInfo *clr_info)
{
    sim_info->simRecorder->compileHistories(*m_neurons, clr_info);
}

/*
 *  Get the IAllNeurons class object.
 *
 *  @return Pointer to the AllNeurons class object.
 */
IAllNeurons* Cluster::getNeurons()
{
    return m_neurons;
}

/*
 *  Set up the connection of all the Neurons and Synapses of the simulation.
 *
 *  @param  sim_info    SimulationInfo class to read information from.
 *  @param  layout      A class to define neurons' layout information in the network.
 *  @param  conns       A class to define neurons' connections information in the network.
 *  @param  clr_info    ClusterInfo class to read information from.
 */
void Cluster::setupConnections(SimulationInfo *sim_info, Layout *layout, Connections *conns, const ClusterInfo *clr_info)
{
    conns->setupConnections(sim_info, layout, m_neurons, m_synapses);
    
    m_synapses->createSynapseImap(m_synapseIndexMap, sim_info, clr_info);
}
