#include "chess-simulator.h"
// disservin's lib. drop a star on his hard work!
// https://github.com/Disservin/chess-library
#include "chess.hpp"
#include <random>
using namespace ChessSimulator;

std::string ChessSimulator::Move(std::string fen)
{
	std::string moveStr;

	chess::Board iniBoard(fen);

	MCTS_Evaluator boardEval(iniBoard, 10);
	chess::Move move = boardEval.genMove();
	moveStr = chess::uci::moveToUci(move);

	return moveStr;
}

MCTS_Evaluator::MCTS_Evaluator(chess::Board root, int depth)
{
	m_RootBoard = root;
	m_Cycles = depth;
}

MCTS_Evaluator::~MCTS_Evaluator()
{

}

chess::Move MCTS_Evaluator::genMove()
{
	// Init root tree node
	MCTS_Node& rootNode = m_StatTree[0];
	rootNode.parentIndex = -1;
	rootNode.visits = 0;
	rootNode.simReward = 0;
	m_FreeIndex++;
	m_SimBoard = m_RootBoard;
	rollout(0);
	for (int i = 0; i < m_StatTree[0].childIndices.size(); i++)
	{
		float simResult = simulation(m_StatTree[0].childIndices[i]);
		update(m_StatTree[0].childIndices[i], simResult);
	}

	// Do MCTS cycles based on the tree resolution
	// specified by the class.
	int currentCycle = m_Cycles;
	while (currentCycle >= 0)
	{
		cycle();
		currentCycle--;
	}

	// Check each child node of the root node
	// to find the one with the most visits
	int bestIndex = 1;
	for (auto const index : rootNode.childIndices)
	{
		if (m_StatTree[index].simReward > m_StatTree[bestIndex].simReward)
		{
			bestIndex = index;
		}
	}

	// Return the move used to reach the node
	// with the most visits
	return m_StatTree[bestIndex].move;
}

void MCTS_Evaluator::cycle()
{
	// Reset the sim board to the root board state
	m_SimBoard = m_RootBoard;

	// Get the index of the node we're going to expand this cycle
	int expandedNodeIndex = selection(0);

	// Get a leaf node to rollout and simulate from the expanded node
	int leafNodeIndex = expansion(expandedNodeIndex);

	// Generate all possible moves for the given leaf node
	// This makes the node no longer a leaf
	rollout(leafNodeIndex);

	// For each newly generated leaf node, simulate a random game
	// and backpropagate the results up to the root.
	for (int i = 0; i < m_StatTree[leafNodeIndex].childIndices.size(); i++)
	{
		float simResult = simulation(m_StatTree[leafNodeIndex].childIndices[i]);
		update(m_StatTree[leafNodeIndex].childIndices[i], simResult);
	}
}

// Select the index of the highest UCT
int MCTS_Evaluator::selection(int nodeIndex)
{
	// Select the child node with the highest
	// UCT from the root game state to expand from
	MCTS_Node& node = m_StatTree[nodeIndex];
	int bestIndex = node.childIndices[0];
	float bestVal = genUCT(m_StatTree[bestIndex]);
	
	for (int i = 1; i < node.childIndices.size(); i++)
	{
		int currentIndex = node.childIndices[i];
		float currentVal = genUCT(m_StatTree[currentIndex]);

		if (currentVal > bestVal)
		{
			bestIndex = currentIndex;
			bestVal = currentVal;
		}

	}

	// Update the SimBoard to reflect the move
	// made by the given node
	m_SimBoard.makeMove(m_StatTree[bestIndex].move);
	return bestIndex;
}

// Find the leaf node with the best UCT from a given node
int MCTS_Evaluator::expansion(int nodeIndex)
{
	// Traverse through each of this node's
	// child nodes until a leaf node is found
	int currentIndex = nodeIndex;
	while (m_StatTree[currentIndex].childIndices.size() != 0)
	{
		// Pick the child node with the highest UCT
		currentIndex = selection(currentIndex);
	}

	return currentIndex;
}

// Generate all possible moves for a leaf node and add them as children
void MCTS_Evaluator::rollout(int leafIndex)
{
	// Generate all moves for the current leaf
	// Won't work if SimBoard wasn't properly updated
	// by the selection function's process.
	MCTS_Node& expandedNode = m_StatTree[leafIndex];
	expandedNode.childIndices.clear();

	chess::Movelist moves;
	chess::movegen::legalmoves(moves, m_SimBoard);

	// For each possible move
	for (auto const& move : moves)
	{
		// Gen new node using unused node from stat tree
		int newNodeIndex = m_FreeIndex;
		MCTS_Node& newNode = m_StatTree[newNodeIndex];
		m_FreeIndex++;

		newNode.parentIndex = leafIndex;
		newNode.childIndices.clear();
		newNode.move = move;
		newNode.visits = 0;
		newNode.simReward = 0;

		expandedNode.childIndices.push_back(newNodeIndex);
	}
}

// Simulate from the leaf's state and return the endgame result
float MCTS_Evaluator::simulation(int leafIndex)
{
	chess::Board leafBoard = m_SimBoard;
	leafBoard.makeMove(m_StatTree[leafIndex].move);

	std::random_device rand;
	std::mt19937 gen(rand());

	// Simulate a random game until an end state is hit
	bool endState = false;
	while (!endState)
	{
		chess::Movelist moves;
		chess::movegen::legalmoves(moves, leafBoard);
		if (leafBoard.isGameOver().first != chess::GameResultReason::NONE)
		{
			endState = true; continue;
		}

		std::uniform_int_distribution<> moveRange(0, moves.size() - 1);
		int moveID = moveRange(gen);
		leafBoard.makeMove(moves[moveID]);
	}

	// Get the value of the simulation's end state
	float simResult = 0;


	if (leafBoard.isGameOver().second == chess::GameResult::LOSE)
	{
		if (leafBoard.sideToMove() == m_RootBoard.sideToMove())
		{
			simResult = -1;
		}

		else if (leafBoard.sideToMove() != m_RootBoard.sideToMove())
		{
			simResult = 1;
		}
	}

	else if (leafBoard.isGameOver().second == chess::GameResult::DRAW)
	{
		//simResult = genStateVal(leafBoard);
		
		if (m_RootBoard.sideToMove() == chess::Color::BLACK)
		{
			//simResult *= -1;
		}
	}

	return simResult;
}

// Backpropagate up the tree from a node until the root is hit
void MCTS_Evaluator::update(int nodeIndex, float simResult)
{
	int currentIndex = nodeIndex;
	//m_StatTree[nodeIndex].visits++;
	
	// Backtrack up the tree until the root is hit
	// (ie. a node w/o a parent)
	while (currentIndex != -1)
	{
		// Update node visit count and sim result
		m_StatTree[currentIndex].simReward += simResult;

		// Update current node to parent node
		int parentIndex = m_StatTree[currentIndex].parentIndex;
		currentIndex = parentIndex;
		m_StatTree[currentIndex].visits++; // is this wrong?
	}
}

float MCTS_Evaluator::genUCT(MCTS_Node node)
{
	float value = 0;

	float visits = node.visits + 0.0001;
	float parentVisits = m_StatTree[node.parentIndex].visits + 0.0001;

	float exploit = (node.simReward) / visits;
	float expansion = sqrt(log(parentVisits) / visits);
	value = exploit + (sqrt(2) * expansion);

	return value;
}

float MCTS_Evaluator::genStateVal(chess::Board board)
{
	std::string boardFen = board.getFen();
	float val = 0;

	for (int i = 0; i < boardFen.size(); i++)
	{
		switch (boardFen[i])
		{
			case 'k':
				val -= 2500;
				break;

			case 'K':
				val += 2500;
				break;

			case 'q':
				val -= 1000;
				break;

			case 'Q':
				val += 1000;
				break;

			case 'b':
				val -= 350;
				break;

			case 'B':
				val += 350;
				break;

			case 'n':
				val -= 350;
				break;

			case 'N':
				val += 350;
				break;

			case 'r':
				val -= 500;
				break;

			case 'R':
				val += 500;
				break;

			case 'p':
				val -= 100;
				break;

			case 'P':
				val += 100;
				break;

			default:
				break;
		}
	}
	val /= (6700 / 2);
	return val;
}