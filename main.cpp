#include <SFML/Graphics.hpp>
#include <vector>
#include <list>
#include <algorithm>
#include <iostream>
#include <cmath>

using namespace std;
using namespace sf;

const int nMapWidth = 20;
const int nMapHeight = 20;
const int nNodeSize = 30;
const int nNodeBorder = 10;



struct sNode{
	
	bool bObstacle = false;
	bool bVisited = false;
	float fGlobalGoal, fLocalGoal;
	int x, y;
	vector<sNode*> vNeighbours;
	sNode* parent;
	RectangleShape rectNode;
};

sNode* initNodes(){
	
	sNode *nodes = new sNode[nMapWidth * nMapHeight];
	
	for(int x = 0; x < nMapWidth; x++){
		for(int y = 0; y < nMapHeight; y++){
			nodes[y * nMapWidth + x].x = x;
			nodes[y * nMapWidth + x].y = y;
			nodes[y * nMapWidth + x].bObstacle = false;
			nodes[y * nMapWidth + x].parent = nullptr;
			nodes[y * nMapWidth + x].bVisited = false;
			nodes[y * nMapWidth + x].rectNode.setSize(Vector2f(nNodeSize - nNodeBorder, nNodeSize - nNodeBorder));
			nodes[y * nMapWidth + x].rectNode.setPosition(x * nNodeSize, y * nNodeSize);
		}
	}
	
	for(int x = 0; x < nMapWidth; x++){
		for(int y = 0; y < nMapHeight; y++){
			
			if(y > 0)
				nodes[y * nMapWidth + x].vNeighbours.push_back(&nodes[(y - 1) * nMapWidth + x]);//top neighbour
			if(y < nMapHeight - 1)
				nodes[y * nMapWidth + x].vNeighbours.push_back(&nodes[(y + 1) * nMapWidth + x]);//bottom one
			if(x > 0)
				nodes[y * nMapWidth + x].vNeighbours.push_back(&nodes[(y) * nMapWidth + x - 1]);//left
			if(x < nMapWidth - 1)
				nodes[y * nMapWidth + x].vNeighbours.push_back(&nodes[(y) * nMapWidth + x + 1]);//right
		}
	}
	
	return nodes;
}

void drawNodes(sNode *nodes, sNode *nodeStart, sNode *nodeEnd, RenderWindow &window){
	
	VertexArray line(Lines, 2);
	
	for(int x = 0; x < nMapWidth; x++){
		for(int y = 0; y < nMapHeight; y++){
			for(auto n : nodes[y * nMapWidth + x].vNeighbours){
				
				line[0].position = Vector2f(x * nNodeSize + (nNodeSize - nNodeBorder) / 2, y * nNodeSize + (nNodeSize - nNodeBorder) / 2);
				line[0].color = Color(0, 0, 180);
				line[1].position = Vector2f(n->x * nNodeSize + (nNodeSize - nNodeBorder) / 2, n->y * nNodeSize + (nNodeSize - nNodeBorder) / 2);
				line[1].color = Color(0, 0, 180);
				window.draw(line);
			}
		}
	}
	
	for(int x = 0; x < nMapWidth; x++){
		for(int y = 0; y < nMapHeight; y++){
			nodes[y * nMapWidth + x].rectNode.setFillColor((nodes[y * nMapWidth + x].bObstacle)? Color::White: Color(0, 0, 180));
			nodes[y * nMapWidth + x].rectNode.setOutlineColor(Color::Black);
			if(nodes[y * nMapWidth + x].bVisited) nodes[y * nMapWidth + x].rectNode.setFillColor(Color(0, 0, 255));
			if(&nodes[y * nMapWidth + x] == nodeStart) nodes[y * nMapWidth + x].rectNode.setFillColor(Color::Green);
			if(&nodes[y * nMapWidth + x] == nodeEnd) nodes[y * nMapWidth + x].rectNode.setFillColor(Color::Red);
			window.draw(nodes[y * nMapWidth + x].rectNode);
		}
	}
	
	if(nodeEnd != nullptr){		
		
		sNode *p = nodeEnd;
		while(p->parent != nullptr){
			
			line[0].position = Vector2f(p->x * nNodeSize + (nNodeSize - nNodeBorder) / 2, p->y * nNodeSize + (nNodeSize - nNodeBorder) / 2);
			line[0].color = Color(Color::Yellow);
			line[1].position = Vector2f(p->parent->x * nNodeSize + (nNodeSize - nNodeBorder) / 2, p->parent->y * nNodeSize + (nNodeSize - nNodeBorder) / 2);
			line[1].color = Color(Color::Yellow);
			window.draw(line);
			
			p = p->parent;
		}
	}
	
	
}

void A_Star(sNode *nodes, sNode *nodeStart, sNode *nodeEnd){
	//reset everything
	for(int x = 0; x < nMapWidth; x++)
		for(int y = 0; y < nMapHeight; y++){
			
			nodes[y * nMapWidth + x].bVisited = false;
			nodes[y * nMapWidth + x].fGlobalGoal = INFINITY;
			nodes[y * nMapWidth + x].fLocalGoal = INFINITY;
			nodes[y * nMapWidth + x].parent = nullptr;
		}
		
	auto distance = [](sNode *a, sNode *b){
		
		return sqrtf((a->x - b->x) * (a->x - b->x) + (a->y - b->y) * (a->y - b->y));
	};
	
	auto heuristic = [distance](sNode *a, sNode *b){
		
		return distance(a, b);
	};
	
	sNode *nodeCurrent = nodeStart;
	nodeStart->fLocalGoal = 0.f;
	nodeStart->fGlobalGoal = heuristic(nodeStart, nodeEnd);
	
	list<sNode*> listNotTestedNodes;
	listNotTestedNodes.push_back(nodeStart);
	
	while(!listNotTestedNodes.empty() && nodeCurrent != nodeEnd){//second term is for immediate breaking after reaching an End;
															
		listNotTestedNodes.sort([](const sNode *lhs, const sNode *rhs) { return lhs->fGlobalGoal < rhs->fGlobalGoal; });
		
		while(!listNotTestedNodes.empty() && listNotTestedNodes.front()->bVisited){
			
			listNotTestedNodes.pop_front();
		}
		
		if(listNotTestedNodes.empty()) break;
		
		nodeCurrent = listNotTestedNodes.front();
		nodeCurrent->bVisited = true;
		
		for(auto nodeNeighbour : nodeCurrent->vNeighbours){
			
			if(!nodeNeighbour->bVisited && !nodeNeighbour->bObstacle){
				
				listNotTestedNodes.push_back(nodeNeighbour);
			}
			
			float fPossiblyLowerGoal = nodeCurrent->fLocalGoal + distance(nodeCurrent, nodeNeighbour);
			
			if(fPossiblyLowerGoal < nodeNeighbour->fLocalGoal){
				
				nodeNeighbour->parent = nodeCurrent;
				nodeNeighbour->fLocalGoal = fPossiblyLowerGoal;
				
				nodeNeighbour->fGlobalGoal = nodeNeighbour->fLocalGoal + heuristic(nodeNeighbour, nodeEnd);
			}
		}
	}
	
	
}

int main()
{
    RenderWindow window(VideoMode(nMapWidth * nNodeSize, nMapHeight * nNodeSize), "A-Star");
	
	sNode *nodes = nullptr;
	sNode *nodeStart = nullptr, *nodeEnd = nullptr;

	nodes = initNodes();
	
	nodeStart = &nodes[(nMapHeight / 2) * nMapWidth + 1];
	nodeEnd = &nodes[(nMapHeight / 2) * nMapWidth + nMapWidth - 2];
	
	Vector2i mousePos;
	
	A_Star(nodes, nodeStart, nodeEnd);
	
    while (window.isOpen()){
		
		mousePos = Mouse::getPosition(window);
		
		int nSelectedNodeX = mousePos.x / nNodeSize;
		int nSelectedNodeY = mousePos.y / nNodeSize;
		
        Event event;
        
        while (window.pollEvent(event)){
			
            if (event.type == Event::Closed || Keyboard::isKeyPressed(Keyboard::Escape)){
				
                window.close();
            }
			
			if(event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left){
				
				if(nSelectedNodeX >= 0 && nSelectedNodeX < nMapWidth){
					if(nSelectedNodeY >= 0 && nSelectedNodeY < nMapHeight){
						
						if(Keyboard::isKeyPressed(Keyboard::LShift))							
							nodeStart = &nodes[nSelectedNodeY * nMapWidth + nSelectedNodeX];
						else if(Keyboard::isKeyPressed(Keyboard::LControl))
							nodeEnd = &nodes[nSelectedNodeY * nMapWidth + nSelectedNodeX];
						else
							nodes[nSelectedNodeY * nMapWidth + nSelectedNodeX].bObstacle = !nodes[nSelectedNodeY * nMapWidth + nSelectedNodeX].bObstacle;
							
						A_Star(nodes, nodeStart, nodeEnd);
					}
				}
			}
			
        }
        
		window.clear(Color::Black);
		drawNodes(nodes, nodeStart, nodeEnd, window);
        window.display();
    }
}