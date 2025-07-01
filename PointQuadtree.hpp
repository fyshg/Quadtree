#ifndef POINTQUADTREE_HPP
#define POINTQUADTREE_HPP

#include <memory>
#include <cassert>
#include <stack>

template<typename CoordType, typename ValueType>   //Coordtype a number / Value type : some random value that is assignable
class PointQuadtree {
    private:
        struct Node;
        class Iterator; 

        using TreePtr = std::shared_ptr<Node>;
        using WeakTreePtr = std::weak_ptr<Node>;
        using Coord = std::pair<CoordType, CoordType>;
        using Direction = enum{NORTH_EAST, SOUTH_EAST, SOUTH_WEST, NORTH_WEST};

    public:

        PointQuadtree(){
        }

        PointQuadtree(PointQuadtree&& other): PointQuadtree(){
                swap(*this, other);
        }

        template<typename Iterator> //Random acces
        PointQuadtree(Iterator it_begin, Iterator it_end) : dim(it_end - it_begin){
                root_node = std::make_shared<Node>(*it_begin++);
                for(; it_begin != it_end; ++it_begin ){
                        insert(it_begin->first.first, it_begin->first.second, it_begin->second);
                }
        }

        template<typename Iterator>   //forward it
        PointQuadtree(Iterator it_begin, Iterator it_end, std::size_t dim) : dim(dim){
                root_node = std::make_shared<Node>(*it_begin++);
                for(; it_begin != it_end; ++it_begin ){
                        insert(it_begin->first.first, it_begin->first.second, it_begin->second);
                }
        }

        PointQuadtree(const PointQuadtree& other) : dim(other.dim), root_node(std::make_shared<Node>(*(other.root_node))){
        }

        std::pair<bool, Iterator> insert(CoordType x, CoordType y, ValueType  o_value){
        auto path = std::stack<std::pair<WeakTreePtr, Direction>>(); 
        std::pair<bool, Iterator> n; 
        if(root_node){
                path.push(std::make_pair(root_node, NORTH_WEST));
                n = root_node->insert(x,y,o_value, path);
                } else {
                        root_node = std::make_shared<Node>(x,y,o_value);
                        n = std::make_pair(true, begin()); 
                }
                if(n.first) dim++;
                
                return n; 
        }

        Iterator find(CoordType x, CoordType y, ValueType o_value){
                if(!root_node) return end(); 
                auto path = std::stack<std::pair<WeakTreePtr, Direction>>(); 
                path.push(std::make_pair(root_node, NORTH_WEST));
                return root_node->find(x,y, o_value, path);
        }

        std::vector<Iterator> find_in_rectangle(Coord bottom_left, Coord top_right) {   //all points in a region .... 
                if(!root_node) return std::vector<Iterator>();  
                assert(bottom_left.first < top_right.first && bottom_left.second < top_right.second);
                auto path = std::stack<std::pair<WeakTreePtr, Direction>>(); 
                path.push(std::make_pair(root_node, NORTH_WEST));
                return root_node->find_in_rectangle(bottom_left, top_right, path);
        }                     //Coordinates are of the bottom left point of the rectangle and the top right one 


        friend void swap(PointQuadtree& pq1, PointQuadtree& pq2){
                using std::swap; 
                swap(pq1.dim, pq2.dim); 
                swap(pq1.root_node, pq2.root_node);
        }
        PointQuadtree& operator=(PointQuadtree other){
                swap(*this, other);
                return *this; 
        }
        
        

        Iterator begin(){
                return Iterator(*this);
        }
        Iterator end(){
                return Iterator();
        }
        std::size_t size(){
                return dim; 
        }
    private:
                
        TreePtr root_node;
        std::size_t dim;

        struct Node{
            public: 

                ValueType value;
                Coord coord;

                Node(){
                }

                Node(std::pair<Coord, ValueType> object) : coord(object.first), value(object.second){
                }
                Node(const Node& other) : coord(other.coord),
                            value(other.value) {
                        
                        if(other.north_east)   north_east =                   std::make_shared<Node>(*(other.north_east));
                        if(other.south_east)   south_east =                    std::make_shared<Node>(*(other.south_east));
                        if(other.south_west)   south_west =                    std::make_shared<Node>(*(other.south_west));
                        if(other.north_west)   north_west =                    std::make_shared<Node>(*(other.north_west));
                }

                Node(CoordType x, CoordType y, ValueType value) : 
                            coord(std::make_pair(x,y)), value(value){
                }

                std::pair<bool, Iterator> insert(CoordType x, 
                         CoordType y,   ValueType& o_value, 
                         std::stack<std::pair<WeakTreePtr, Direction>>& path){
                        

                        if(x == coord.first &&  y ==coord.second) return std::make_pair(false, Iterator()); //object is already in the tree ... 

                        if(x >= coord.first && y >= coord.second){
                                if(north_east){ 
                                        path.push(std::make_pair(north_east, NORTH_EAST));
                                        return  north_east->insert(x,y,o_value, path);
                                }else { 
                                        north_east = std::make_shared<Node>(x,y,o_value);
                                        path.push(std::make_pair(north_east, NORTH_EAST));
                                        return std::make_pair(true, Iterator(path));
                                }
                        } else if(x > coord.first && y < coord.second){
                                if(south_east){ 
                                        path.push(std::make_pair(south_east, SOUTH_EAST));
                                        return  south_east->insert(x,y,o_value, path);
                                }else { 
                                        south_east = std::make_shared<Node>(x,y,o_value);
                                        path.push(std::make_pair(south_east, SOUTH_EAST));
                                        return std::make_pair(true, Iterator(path));
                                }
                        }  else if(x <= coord.first && y <= coord.second){                  //ne and sw are open ...  doesnt handle collisions
                                if(south_west){ 
                                        path.push(std::make_pair(south_west, SOUTH_WEST));
                                        return  south_west->insert(x,y,o_value, path);
                                }else { 
                                        south_west = std::make_shared<Node>(x,y,o_value);
                                        path.push(std::make_pair(south_west, SOUTH_WEST));
                                        return std::make_pair(true, Iterator(path));
                                }
                        } else if(x < coord.first && y > coord.second){
                                if(north_west){ 
                                        path.push(std::make_pair(north_west, NORTH_WEST));
                                        return  north_west->insert(x,y,o_value, path);
                                }else { 
                                        north_west = std::make_shared<Node>(x,y,o_value);
                                        path.push(std::make_pair(north_west, NORTH_WEST));
                                        return std::make_pair(true, Iterator(path));
                                }
                        }
                        return std::make_pair(false, Iterator()); 
                }

                Iterator find(CoordType x, CoordType y, ValueType& o_value, std::stack<std::pair<WeakTreePtr, Direction>>& path) {
                        
                        if ( coord.first == x && coord.second == y && value == o_value ) return Iterator(path);         
                        if(x >= coord.first && y >= coord.second && north_east) {
                                path.push(std::make_pair(north_east, NORTH_EAST));
                           return north_east->find(x,y,o_value, path);
                        } else if( x > coord.first && y < coord.second && south_east) {
                                path.push(std::make_pair(south_east, SOUTH_EAST)); return south_east->find(x,y,o_value, path);
                        } else if( x <= coord.first && y <= coord.second && south_west){
                                path.push(std::make_pair(south_west, SOUTH_WEST)); return south_west->find(x,y,o_value, path);
                        } else if( x < coord.first && y > coord.second && north_west){
                                path.push(std::make_pair(north_west, NORTH_WEST)); return north_west->find(x,y,o_value, path);
                        }   
                        return Iterator();
                }

                std::vector<Iterator> find_in_rectangle(Coord bottom_left, Coord top_right, std::stack<std::pair<WeakTreePtr, Direction>>& path ){
                        std::vector<Iterator> points; 


                        if(coord.first <= top_right.first && coord.second <= top_right.second && north_east){
                                path.push(std::make_pair(north_east, NORTH_EAST));
                                auto n = north_east->find_in_rectangle(bottom_left, top_right, path );
                                points.insert(points.end(), n.begin(), n.end() );
                                path.pop();
                        }
                        if(coord.first > top_right.first && coord.second < bottom_left.second && south_east){  //bottom right has to be in so topright first bottom right sec
                                path.push(std::make_pair(south_east, SOUTH_EAST));
                                auto n = south_east->find_in_rectangle(bottom_left, top_right, path );
                                points.insert(points.end(), n.begin(), n.end() );
                                path.pop();
                        }
                        if(coord.first >= bottom_left.first && coord.second >= bottom_left.second && south_west){
                                path.push(std::make_pair(south_west, SOUTH_WEST));
                                auto n = south_west->find_in_rectangle(bottom_left, top_right, path );
                                points.insert(points.end(), n.begin(), n.end() );
                                path.pop();
                        }
                        if(coord.first > bottom_left.first && coord.second < top_right.second && north_west){
                                path.push(std::make_pair(north_west, NORTH_WEST));
                                auto n = north_west->find_in_rectangle(bottom_left, top_right, path );
                                points.insert(points.end(), n.begin(), n.end() );
                                path.pop();
                        }

                        if(coord.first >= bottom_left.first && coord.second >= bottom_left.second && coord.first <= top_right.first && coord.second <= top_right.second){
                                points.push_back(Iterator(path)); 
                        }
                        return points; 
                }
                

            private: 
                friend class Iterator;
                TreePtr north_east;
                TreePtr south_east;
                TreePtr south_west; 
                TreePtr north_west;
        };

        class Iterator{
            public: 
                Iterator(PointQuadtree& pq) : path(std::stack<std::pair<WeakTreePtr,Direction>>()) {
                        path.push(std::make_pair(pq.root_node, NORTH_WEST)); // consider root node as sprecial case which is n w
                }
                
                Iterator(std::stack<std::pair<WeakTreePtr, Direction>> path) : path(path) {

                }
                
                Iterator() : path(std::stack<std::pair<WeakTreePtr, Direction>>()) {

                }
                Node& operator*() {
                        return *path.top().first.lock();
                }   
                const Node& operator*() const {
                        return *(path.top().first.lock());
                }
                Node* operator->() {
                        return &(*path.top().first.lock());
                }
                Iterator& operator++() {
                        advance();
                        return *this;
                }
                
                bool operator==(const Iterator& other){

                        bool end1 = path.empty();
                        bool end2 = other.path.empty();
                        if (end1 && end2) return true;
                
                        bool invalid1 = end1 || !path.top().first.lock();
                        bool invalid2 = end2 || !path.top().first.lock();
                        if (invalid1 && invalid2) return true;
                        if (invalid1 || invalid2) return false;

                        return path.top().first.lock() ==                      other.path.top().first.lock();
                }

                bool operator!=(const Iterator& other){
                        return !(*this == other);
                }
                

            private: 
                void advance() {

                        bool found = false; 
                        TreePtr current = path.top().first.lock(); 

                        auto search1 = find_next(current);  // first search for next in Children 

                        if (search1.first) {
                                path.push(search1.second);
                                found = true; 
                        }       

                        Direction current_dir = path.top().second;
                        //if that doesnt work in parents until something is found or the stack is empty

                        while(!path.empty() && !found){
                                
                                
                                path.pop(); 
                                if( path.empty()) break; 
                                TreePtr father =  path.top().first.lock(); 
                                auto search2 = find_next(father, current_dir);

                                if ( search2.first) {
                                        path.push(search2.second);
                                        found = true; 
                                }

                                current = father; 
                                current_dir = path.top().second; 
                        }

                        
                        //handling for end ??? 
                
                }

                std::pair<bool, std::pair<TreePtr, Direction>> find_next(TreePtr father, Direction dir){   // finds the next node given a father node and a direction to its son 
                        if(dir == NORTH_EAST && father->south_east){
                                 return std::make_pair(true, std::make_pair(father->south_east, SOUTH_EAST));
                        } 
                        if( (dir == NORTH_EAST || dir == SOUTH_EAST) && father->south_west){
                                 return std::make_pair(true, std::make_pair(father->south_west, SOUTH_WEST));
                        }
                        if(( dir == NORTH_EAST || dir == SOUTH_EAST || dir == SOUTH_WEST) && father->north_west) {
                                 return std::make_pair(true, std::make_pair(father->north_west, NORTH_WEST));
                        }
                         return std::make_pair(false, std::make_pair(TreePtr(), NORTH_WEST));
                }

                std::pair<bool, std::pair<TreePtr, Direction>> find_next(TreePtr node){   // just finds the next node ... 
                        if              ( node->north_east) return std::make_pair(true, std::make_pair(node->north_east, NORTH_EAST));
                        else if ( node->south_east) return std::make_pair(true, std::make_pair(node->south_east, SOUTH_EAST));
                        else if ( node->south_west) return std::make_pair(true, std::make_pair(node->south_west, SOUTH_WEST));
                        else if ( node->north_west) return std::make_pair(true, std::make_pair(node->north_west, NORTH_WEST));

                        return std::make_pair(false,std::make_pair( TreePtr(), NORTH_WEST));
                }

                std::stack<std::pair<WeakTreePtr, Direction>> path; 
        };
};
#endif