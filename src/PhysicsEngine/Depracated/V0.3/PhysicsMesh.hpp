#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <vector>

class PhysicsMesh{
public:
    std::vector<int> indices; //an index of every triangle in the mesh. Every three indices is a triangle
    std::vector<glm::vec3> vertices; //an array storing all vertices of the mesh

    //get a vertex with a specific index.
    /*glm::vec3 getVertex(int indexLocation) {
        return vertices[indexLocation];
    }*/

    //get a specific tri in the mesh
    std::vector<glm::vec3> getTri(int tri) {

        std::vector<glm::vec3> output; //create a variable to store the function's output
        
        //ensure the triangle exists
        if(indices.size() >= 3 * tri + 2) {

            //add the vertices to the output
            for(int i = 0; i < 3; i++) {
                output.push_back(vertices[indices[3 * tri + i]]);
            }
        }
        return output;
    }
};