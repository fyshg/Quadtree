#ifndef REGIONQUADTREE_HPP
#define REGIONQUADTREE_HPP

#include <memory>
#include <math.h>
#include <cassert>
#include <iostream>
#include <vector>
//maybe support initialization by 1D array .. if there is time left at the end ... 
// concetps, to Array , tests/exceptions...
//methods rotate ??

class RegionQuadtree {
        private:
                struct Node;
                class Iterator;
        public: 
                
                using Direction = enum{NORTH_EAST, SOUTH_EAST, SOUTH_WEST, NORTH_WEST};
                using Colour = enum{WHITE, BLACK, GREY};
                using TreePtr = std::shared_ptr<Node>;

                TreePtr root_node;
                std::size_t dim;



                RegionQuadtree(){
                }
                RegionQuadtree(RegionQuadtree&& other) : RegionQuadtree(){
                        swap(*this, other);
                }
                RegionQuadtree(const RegionQuadtree& other) : dim(other.dim), root_node(std::make_shared<Node>(*(other.root_node))) {
                }
                
        
                RegionQuadtree& operator=(RegionQuadtree other) {
                         swap(*this, other);
                         return *this;
                }
                friend void swap(RegionQuadtree& qt1, RegionQuadtree& qt2) {
                        using std::swap;
                        swap(qt1.dim, qt2.dim);
                        swap(qt1.root_node, qt2.root_node);
                }

                template<typename Iterator>  //only random access iterators please and only iterators on iterators or random Access data structurs like arrays
                RegionQuadtree(const Iterator& it_begin, const Iterator& it_end) :   //change this copy constructing is expensive
                        root_node(std::make_shared<Node>(it_begin, it_end, it_end - it_begin, 0, 0)), dim(it_end - it_begin) {  //N must be set for the root node ....
                                assert( dim > 0 &&  ceil(log2(dim) == floor(log2(dim))));

                }
                
                Iterator begin(){
                        return Iterator(*this);
                }
                Iterator end(){
                        return Iterator();
                }
                
                template<typename T>
                RegionQuadtree(T (&arr)[1][1]) : root_node(arr[1][1] ? BLACK : WHITE) {
                }

                // Methods for static Array
                template<std::size_t N, typename T>
                RegionQuadtree(T (&arr)[N][N]) : RegionQuadtree(std::begin(arr), std::end(arr)){
                }
                
                std::size_t number_of_leaves(){
                        return root_node->number_of_leaves();
                }

                std::vector<std::vector<bool>> get_array(){
                        std::vector<std::vector<bool>> arr(dim);
                        for(std::size_t i = 0; i < dim; ++i ){
                                arr[i] = std::vector<bool>(dim);
                        }
                        root_node->get_array(dim, 0, 0, arr);
                        return arr;
        }
        private:
                struct Node{
                        public:
                                
                                Node() {
                                }
                
                                Node(Colour colour) : colour(colour){
                                }
                                
                                Node(const Node& other) : colour(other.colour) {
                                        if(other.colour == GREY){
                                              north_east = std::make_shared<Node>(*(other.north_east));
                                              south_east = std::make_shared<Node>(*(other.south_east));
                                              south_west = std::make_shared<Node>(*(other.south_west));
                                              north_west = std::make_shared<Node>(*(other.north_west));
                                       }   
                                }


                                template<typename Iterator>  //only random access iterators please
                                Node(const Iterator& it_begin, const Iterator& it_end, std::size_t N, std::size_t x, std::size_t y) {     //gets N as region side length and x and y as offset for the region
                                        if (N == 1 ){
                                                colour = it_begin[x][y] ? BLACK : WHITE;
                                                //std::cout << "node at " << x << " " << y <<  "with N==1 " << "has the colour: " << (colour == 1 ? "BLACK" : ( colour == 0 ? "WHITE" : "GREY")) << std::endl;
                                        } else {
                                                N = N/2;  //sidelength of subtrees...
                                                TreePtr n_e = std::make_shared<Node>(it_begin, it_end, N, N + x, 0 + y );   //maybe hier schon make_shared kp bin kein teiler
                                                TreePtr s_e = std::make_shared<Node>(it_begin, it_end, N, N + x, N + y ); //das hier called den Copy constructor. Das ist vermutlich unnötig. 
                                                TreePtr s_w = std::make_shared<Node>(it_begin, it_end, N, 0 + x, N + y );
                                                TreePtr n_w = std::make_shared<Node>(it_begin, it_end, N, 0 + x, 0 + y );

                                                if(n_e->colour == s_e->colour && s_e->colour == s_w->colour && s_w->colour == n_w->colour && n_w->colour == n_e->colour && (n_e->colour != GREY)){
                                                        colour = n_e->colour;
                                                } else {
                                                        north_east = std::move(n_e);
                                                        south_east = std::move(s_e);
                                                        south_west = std::move(s_w);
                                                        north_west = std::move(n_w);
                                                        colour = GREY;
                                                }
                                        }
                                }
                                
                                void get_array(std::size_t N, std::size_t x, std::size_t y, std::vector<std::vector<bool>>& arr){
                                        
                                        if( colour == GREY) {
                                                        N = N/2;
                                                        north_east->get_array(N, N + x, y, arr);
                                                        south_east->get_array(N, N + x, N + y, arr);
                                                        south_west->get_array(N, x, N + y, arr );
                                                        north_west->get_array(N, x, y, arr);
                                                } else {
                                                        for( std::size_t i = x; i < N+x; ++i){
                                                                for(std::size_t j = y; j < N+y; ++j){
                                                                        arr[i][j] = colour;
                                                        }
                                                }
                                        }
                                }
                                
                                std::size_t number_of_leaves(){
                                if(colour == BLACK || colour == WHITE){
                                        return 1;
                                } else if(colour == GREY){
                                        return north_east->number_of_leaves() + south_east->number_of_leaves() + south_west->number_of_leaves() + north_west->number_of_leaves();
                                }
                                }
                        private:
                                friend class Iterator; 
                                TreePtr north_east;
                                TreePtr south_east;
                                TreePtr south_west; 
                                TreePtr north_west;
                                Colour colour;
                };      
                class Iterator {  //has to be random acces ...                   
                        public: 
                                
                                Iterator(RegionQuadtree& quad_tree) : index(0), root_node(quad_tree.root_node), leaves(quad_tree.number_of_leaves()), path(std::make_unique<std::vector<Direction>>()){  
                                        advance();
                                }
                                Iterator() : index(0), leaves(0){
                                }
                                Colour operator*() {
                                        return current_node.lock()->colour;
                                }   
                                const Colour operator*() const {
                                       return current_node.lock()->colour;
                                }
                                Colour* operator->() {
                                       return &current_node.lock()->colour;
                                }
                                Iterator& operator++() {
                                       advance();
                                       return *this;
                                }
                                bool operator==(const Iterator& other){

                                        bool end1 = index >= leaves;
                                        bool end2 = other.index >= other.leaves;
                                        if (end1 && end2) return true;
                                          
                                        bool invalid1 = end1 || !current_node.lock();
                                        bool invalid2 = end2 || !other.current_node.lock();
                                        if (invalid1 && invalid2) return true;
                                        if (invalid1 || invalid2) return false;

                                        return current_node.lock() == other.current_node.lock() && index == other.index && root_node.lock() == other.root_node.lock() && leaves == other.leaves;
                                }
                                bool operator!=(const Iterator& other){
                                        return !(*this == other);
                                }
                        private: 
                                using WeakTreePtr = std::weak_ptr<Node>;
                                WeakTreePtr current_node;
                                WeakTreePtr root_node;
                                // Variable, die den Weg zur Node repräsentiert. Diese weiß dann auch über die größe des Subbaums Bescheid. Das ganze erfolgt nach enum Reihenfolge. 
                                std::unique_ptr<std::vector<Direction>> path;
                                std::size_t index;
                                std::size_t leaves;  //n is the sidelength of the original Array. It is needed to get the max depth of the RegionQuadtree  

                                void advance() {

                                        auto r = root_node.lock();
                                        auto c = current_node.lock();
                                        r->number_of_leaves();
                                        if((r == c || !c) && index != leaves) {         //Wenn current = root oder wenn keine current existiert dann pointe auf die nordöstlichste node, 
                                                c = r;
                                                while( c->north_east){
                                                        current_node = c->north_east;
                                                        c = current_node.lock();                //Was ist daran falsch= 
                                                        path->push_back(NORTH_EAST); // 0 corresponds to North east. 
                                                }
                                        } else if(c && path && path->size() != 0 && index != leaves) {  
                                                
                                                std::size_t size = path->size();
                                                for(std::size_t i = size ; i> 0; --i ) {
                                                        if( (*path)[i-1] != NORTH_WEST ){
                                                                //std::cout << "current path: " <<  (*path)[i] << "next path: " << static_cast<Direction>(static_cast<int>((*path)[i]) +1) << std::endl;
                                                                (*path)[i-1] =  static_cast<Direction>((static_cast<int>((*path)[i-1])) +1);  //sets path to the next element.
                                                                break;
                                                        } else if (  (*path)[i-1] == NORTH_WEST){  // no next element
                                                                if(i == 0){
                                                                index = leaves; current_node = std::weak_ptr<Node>();  //end iterator
                                                                } 
                                                                path->erase(path->begin() + i-1);  // make sure the path is shorter again. 
                                                                
                                                        } 
                                                }
                                                //walk the path
                                                if(index < leaves ){  //mistake here somewhere
                                                        c = r; 
                                                        for( auto dir : *path) {
                                                                
                                                                switch (dir) {
                                                                        case NORTH_EAST : current_node = c->north_east; break;                                                                                            
                                                                        case SOUTH_EAST : current_node = c->south_east; break;
                                                                        case SOUTH_WEST : current_node = c->south_west; break;
                                                                        case NORTH_WEST : current_node = c->north_west; break;                                           
                                                                }

                                                                c = current_node.lock();

                                                        }
                                                        if(c->colour == GREY){ // c isch mal wieder em ty; 
                                                                while(c->north_east){
                                                                        current_node = c->north_east;
                                                                        c = current_node.lock();

                                                                        path->push_back(NORTH_EAST);
                                                                }
                                                        }            
                                                        index++;
                                                }
                                        }
                                }
                                
                };
                
                        

                
                
                        

};

#endif