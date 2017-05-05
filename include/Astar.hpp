#ifndef ASTAR
#define ASTAR
#include <rw/models/WorkCell.hpp>
#include <rw/kinematics/State.hpp>
#include <rw/models/Device.hpp>
#include <rw/rw.hpp>
#include <rw/math/Q.hpp>
//#include <rw/math/QMetric.hpp>
#include <rwlibs/proximitystrategies/ProximityStrategyPQP.hpp>
#include <rw/pathplanning/PlannerConstraint.hpp>
#include <rw/pathplanning/QConstraint.hpp>
#include <rw/pathplanning/QEdgeConstraint.hpp>
#include <rw/proximity/CollisionDetector.hpp>
#include <rw/proximity/CollisionStrategy.hpp>
#include <rw/common.hpp>




class Node;


struct sort_func;


class Astar
{
public:
	Astar(double size, std::vector<Node*> *graph, 	rw::math::QMetric::Ptr metric, rw::common::Ptr<rw::pathplanning::QConstraint> constraint, 	rw::common::Ptr<rw::pathplanning::QEdgeConstraint> edgeConstraint);

	void find_path(int startNodeId, int goalNodeId, std::vector<int> &path);

	~Astar();


private:
	double _size;
	int _numRun;
	std::vector<Node*> *_graph;
	rw::math::QMetric::Ptr _metric;
	rw::common::Ptr<rw::pathplanning::QConstraint> _constraint;
	rw::common::Ptr<rw::pathplanning::QEdgeConstraint> _edgeConstraint;
   	std::priority_queue<Node*, std::vector<Node*>, sort_func> *_openList;
	double calc_h(int cId, int gId);


};









#endif