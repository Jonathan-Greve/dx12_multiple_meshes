#ifndef MESHGENERATOR_H_ 
#define MESHGENERATOR_H_
#include "Mesh.h"

class MeshGenerator
{
public:
	static Mesh GenerateTriangle(XMFLOAT3 v1, XMFLOAT3 v2, XMFLOAT3 v3, std::string name);
	static Mesh GenerateUnitCircle(std::string name);
	static Mesh GenerateUnitBox(std::string name);

private:
};

#endif
