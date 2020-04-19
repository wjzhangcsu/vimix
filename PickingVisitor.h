#ifndef PICKINGVISITOR_H
#define PICKINGVISITOR_H

#include <glm/glm.hpp>
#include <vector>

#include "Visitor.h"

class PickingVisitor: public Visitor
{
    glm::vec2 point_;
    glm::mat4 modelview_;
    std::vector<Node *> nodes_;

public:
    PickingVisitor(glm::vec2 coordinates);
    std::vector<Node *> picked() { return nodes_; }

    // Elements of Scene
    void visit(Scene& n);
    void visit(Node& n);
    void visit(Group& n);
    void visit(Switch& n);
    void visit(Animation& n);
    void visit(Primitive& n);
    void visit(Surface& n);
    void visit(ImageSurface&){}
    void visit(MediaSurface&){}
    void visit(FrameBufferSurface&){}
    void visit(LineStrip&){}
    void visit(LineSquare&);
    void visit(LineCircle& n);
    void visit(Mesh&){}

    // not picking other Elements with attributes
    void visit(MediaPlayer&) {}
    void visit(Shader&) {}
    void visit(ImageShader&) {}
};

#endif // PICKINGVISITOR_H
