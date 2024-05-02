#pragma once
#include <string>
#include "chess.hpp"

namespace ChessSimulator {
	/**
	 * @brief Move a piece on the board
	 *
	 * @param fen The board as FEN
	 * @return std::string The move as UCI
	 */
	std::string Move(std::string fen);

	/*
	* MCTS Notes
	* 
	* - Start with root node, initial game state. Selection phase by default.
	* - Expand the tree with all possible moves that can be made from its state. Expansion phase.
	* - Simulate each unvisited node by randomly making moves from their respective states until an end state is reached. Simulation phase.
	*		- A node is only fully expanded once all child nodes have been visted, or simulated as a start state.
	*		- Simulation is done by making random moves until someone wins or theres a draw (1 for win, -1 for loss, 0 for draw)
	*		- A node passed through during simulation is not considered visited. These nodes also are not added to the statistics tree.
	* - Backpropagate the result of the simulation up the tree to the root node from the leaf/simulated node, updating the values of each node moved through in the
		process. Update phase.
	*		- The values updated are:
	*			- Simulation reward, a sum of all simulation results backpropagated through it.
	*			- Total visits, the amount of times this node has been backpropagated through overall.
	*		- The values determine how promising a certain simulation path is. High simulation reward and high visits indicate a high and reliable success rate.
	* - These steps (Selection -> Expansion -> Simulation -> Update) marks one cycle of MCTS.
	*		- This process is repeated however many times specified by the algorithm
	*		- We can start this from any board state and choose a move to make once complete. Usually the node with the most visits is the best one.
	*		- More cycles produce a more reliable and accurate decision but has the trade off using more time and memory
	* - Selection beyond the first cycle involves using the UCT formula, using it to work down to an unvisited node from the root node, weighing which are selected
	*	based on their values.
	*		- UCT formula: (sim reward / visits) + C * sqrt( log(parent visits) / visits )
	*		- C in the UCT formula is a constant used as a tradeoff between exploration of new nodes and exploitation of existing ones
	*		- We repeat selection through UCT until we select a node that hasn't been expanded in full yet, switching to expansion from it.
	* 
	* - Given that we know our starting board state, we only need to store the moves made to reach each node. We can move up and down the tree
	*	by applying and reversing the moves made at each node in the sequential order by which they were made from the root board state. The
	*	board state of the evaluation can probably just be stored in a MCTS eval class.
	*		- Where does the opposition play into this though? I know where it does in simulation, but how do we know what moves are legal and
	*		  what moves arent if we dont have a way to store/generate the opponents moves? Do we just randomly pick and store it in the expansion
	*		  phase? Or does each layer of the tree alternative between black and white moves like Minimax does?
	* 
	* The MCTS class needs to keep track of:
	*	- Starting board state
	*	- Simulated board state
	*	- Full node tree (stack allocated array of nodes)
	*	- First open node index
	* 
	* A MCTS tree node needs to keep track of:
	*	- The move made to generate the node
	*	- Parent node current move was made from
	*	- Child node of possible new moves that can be made
	*	- Simulation reward
	*	- Total visits
	*/

	struct MCTS_Node
	{
		chess::Move move;
		int parentIndex = -1;
		std::vector<int> childIndices;

		int visits = 0;
		float simReward = 0;
	};

	class MCTS_Evaluator
	{
	public:
		MCTS_Evaluator(chess::Board root, int depth);
		~MCTS_Evaluator();

		chess::Move genMove();

	private:
		void cycle();
		int selection(int nodeIndex);
		int expansion(int nodeIndex);
		void rollout(int leafIndex);
		float simulation(int leafIndex);
		void update(int nodeIndex, float simResult);
		float genUCT(MCTS_Node node);
		float genStateVal(chess::Board board);

		chess::Board m_RootBoard;
		chess::Board m_SimBoard;
		int m_Cycles = 0;

		MCTS_Node m_StatTree[10000];
		int m_FreeIndex = 0;
	};
}