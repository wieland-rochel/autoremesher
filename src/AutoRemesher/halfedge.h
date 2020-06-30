#ifndef AUTO_REMESHER_HALF_EDGE_H
#define AUTO_REMESHER_HALF_EDGE_H
#include <vector>
#include <cstddef>
#include <queue>
#include <limits>
#include <AutoRemesher/Vector3>

namespace AutoRemesher
{
    
namespace HalfEdge
{
    
struct Vertex;
struct HalfEdge;
struct Face;

struct Vertex
{
    size_t index;
    Vector3 position;
    Vertex *_previous = nullptr;
    Vertex *_next = nullptr;
    HalfEdge *anyHalfEdge = nullptr;
    double curvature = std::numeric_limits<double>::max();
};

struct HalfEdge
{
    HalfEdge *_previous = nullptr;
    HalfEdge *_next = nullptr;
    Vertex *startVertex = nullptr;
    Face *leftFace = nullptr;
    HalfEdge *previousHalfEdge = nullptr;
    HalfEdge *nextHalfEdge = nullptr;
    HalfEdge *oppositeHalfEdge = nullptr;
    double length2 = 0.0;
};

struct Face
{
    Face *_previous = nullptr;
    Face *_next = nullptr;
    HalfEdge *anyHalfEdge = nullptr;
};

class Mesh
{
public:
    Mesh(const std::vector<Vector3> &vertices,
        std::vector<std::vector<size_t>> &triangles);
    ~Mesh();
    Vertex *allocVertex();
    Face *allocFace();
    HalfEdge *allocHalfEdge();
    void freeVertex(Vertex *vertex);
    void freeFace(Face *face);
    void freeHalfEdge(HalfEdge *halfEdge);
    
    inline HalfEdge *findShortestHalfEdgeAroundVertex(Vertex *vertex) const
    {
        HalfEdge *halfEdge = vertex->anyHalfEdge;
        double shortestLength2 = std::numeric_limits<double>::max();
        HalfEdge *shortest = halfEdge;
        do {
            if (halfEdge->length2 < shortestLength2) {
                shortest = halfEdge;
                shortestLength2 = halfEdge->length2;
            }
            halfEdge = halfEdge->oppositeHalfEdge->nextHalfEdge;
        } while (halfEdge != vertex->anyHalfEdge);
        return shortest;
    }
    
    void exportPly(const char *filename);
    
private:
    inline void calculateVertexCurvature(Vertex *vertex)
    {
        double sumOfAngle = 0.0;
        HalfEdge *halfEdge = vertex->anyHalfEdge;
        do {
            sumOfAngle += Vector3::angle(halfEdge->nextHalfEdge->startVertex->position - vertex->position,
                halfEdge->previousHalfEdge->startVertex->position - vertex->position);
            halfEdge = halfEdge->oppositeHalfEdge->nextHalfEdge;
        } while (halfEdge != vertex->anyHalfEdge);
        vertex->curvature = std::abs(2.0 * M_PI - sumOfAngle);
    }

    Vertex *m_firstVertex = nullptr;
    Vertex *m_lastVertex = nullptr;
    Face *m_firstFace = nullptr;
    Face *m_lastFace = nullptr;
    HalfEdge *m_firstHalfEdge = nullptr;
    HalfEdge *m_lastHalfEdge = nullptr;
    size_t m_repeatedHalfEdges = 0;
    size_t m_aloneHalfEdges = 0;
    size_t m_vertexCount = 0;
    size_t m_faceCount = 0;
    
    struct vertexCurvatureComparer
    {
        bool operator()(const Vertex *lhs, const Vertex *rhs) const
        {
            return lhs->curvature < rhs->curvature;
        }
    };
    vertexCurvatureComparer m_vertexCurvatureComparer;
    
    std::priority_queue<Vertex *, std::vector<Vertex *>, vertexCurvatureComparer> m_flatVertexPointers;
};

}
    
}

#endif
