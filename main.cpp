#include "region_quadtree.hpp"
#include <iostream>
#include <vector>
#include "point_quadtree.hpp"
#include <string>

int main(){
	

	int  a[8][8] = {{0,0,0,0,0,0,1,1}, 
			{0,0,0,0,0,1,1,1}, 
			{0,0,0,1,1,1,0,0}, 
			{0,0,1,1,1,0,0,0}, 
			{0,0,1,0,0,0,0,0}, 
			{0,0,1,0,0,0,0,0},
			{0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0}};


	RegionQuadtree rq1(a);
	

	std::cout << "Quadtree c has " <<  rq1.number_of_leaves() << " leaves " << std::endl;


	RegionQuadtree rq2(rq1);   


	for(auto col: rq2){
		std::cout << "Colour is: " << col << std::endl;

	}
	

	std::cout << "The underlying Array is: " << std::endl; 

	auto u = rq2.get_array();
	for(auto col : u){
		for(auto el: col){
			std::cout << el << " " ;
		}
		std::cout << std::endl;
	}



	std::array<std::pair<std::pair<long  long, long long> , int>, 10> points; 

	int j = 0; 
	for(auto& el : points ){  
		el = std::make_pair(std::make_pair(j,10 - j), 1);
		++j;
	}
	points[0] = std::make_pair(std::make_pair(5,4), 10);  //get a more balanced tree

	PointQuadtree<int, int> pq(points.begin(), points.end());


	std::cout << " Point (5,5,10) is in the tree: " <<  (( pq.find(0,1,10) != pq.end()) ? "true" : "false")   <<  std::endl;

	
	auto in = pq.insert(1,2,3);
	//std::cout << *(in.second).value << std::endl;
	

	auto region = pq.find_in_rectangle(std::make_pair(1,1), std::make_pair(5,5));

	std::cout << "The following values are in the rectangle " << std::endl; 
	for(auto n : region){
		std::cout << "x: " << n->coord.first << " y: " << n->coord.second << " value: " << n->value << std::endl; 
	}

	auto qp(pq);
	std::cout << " the copied Object stores the points:  " << std::endl; 
	for(auto n : qp){
		std::cout << "x: " << n.coord.first << " y: " << n.coord.second << " value: " << n.value << std::endl; 
	}

}
