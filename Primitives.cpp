#include "Primitives.h"
#include "ImageShader.h"
#include "Resource.h"
#include "FrameBuffer.h"
#include "MediaPlayer.h"
#include "Visitor.h"
#include "Log.h"

#include <glad/glad.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>


static const std::vector<glm::vec3> square_points {
            glm::vec3( -0.99f, -1.f, 0.f ), glm::vec3( -1.f, -0.99f, 0.f ),
            glm::vec3( -1.f, 0.99f, 0.f ), glm::vec3( -0.99f, 1.f, 0.f ),
            glm::vec3( 0.99f, 1.f, 0.f ), glm::vec3( 1.f, 0.99f, 0.f ),
            glm::vec3( 1.f, -0.99f, 0.f ), glm::vec3( 0.99f, -1.f, 0.f ),
            glm::vec3( -0.99f, -1.f, 0.f )
};


Surface::Surface(Shader *s) : Primitive(s)
{
    // geometry
    points_ = std::vector<glm::vec3> { glm::vec3( -1.f, -1.f, 0.f ), glm::vec3( -1.f, 1.f, 0.f ),
            glm::vec3( 1.f, -1.f, 0.f ), glm::vec3( 1.f, 1.f, 0.f ) };
    colors_ = std::vector<glm::vec4> { glm::vec4( 1.f, 1.f, 1.f , 1.f ), glm::vec4(  1.f, 1.f, 1.f, 1.f  ),
            glm::vec4( 1.f, 1.f, 1.f, 1.f ), glm::vec4( 1.f, 1.f, 1.f, 1.f ) };
    texCoords_ = std::vector<glm::vec2> { glm::vec2( 0.f, 1.f ), glm::vec2( 0.f, 0.f ),
            glm::vec2( 1.f, 1.f ), glm::vec2( 1.f, 0.f ) };
    indices_ = std::vector<uint> { 0, 1, 2, 3 };
    drawMode_ = GL_TRIANGLE_STRIP;
}


void Surface::init()
{
    // use static unique vertex array object
    static uint unique_vao_ = 0;
    static uint unique_drawCount = 0;
    if (unique_vao_) {
        // 1. only init Node (not the primitive vao)
        Node::init();
        // 2. use the global vertex array object
        vao_ = unique_vao_;
        drawCount_ = unique_drawCount;
    }
    else {
        // 1. init the Primitive (only once)
        Primitive::init();
        // 2. remember global vertex array object
        unique_vao_ = vao_;
        unique_drawCount = drawCount_;
        // 3. unique_vao_ will NOT be deleted because ImageSurface::deleteGLBuffers_() is empty
    }
}

void Surface::accept(Visitor& v)
{
    Primitive::accept(v);
    v.visit(*this);
}

ImageSurface::ImageSurface(const std::string& path, Shader *s) : Surface(s), resource_(path), textureindex_(0)
{

}

void ImageSurface::init()
{
    Surface::init();

    // load image if specified (should always be the case)
    if ( resource_.empty())
        textureindex_ = Resource::getTextureBlack();
    else {
        float ar = 1.0;
        textureindex_ = Resource::getTextureImage(resource_, &ar);
        scale_.x = ar;
    }

    // a new shader for a new image
    shader_ = new ImageShader();
}

void ImageSurface::draw(glm::mat4 modelview, glm::mat4 projection)
{
    if ( !initialized() )
        init();

    glBindTexture(GL_TEXTURE_2D, textureindex_);

    Primitive::draw(modelview, projection);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void ImageSurface::accept(Visitor& v)
{
    Primitive::accept(v);
    v.visit(*this);
}

MediaSurface::MediaSurface(const std::string& uri, Shader *s) : Surface(s)
{
    uri_ = uri;
    mediaplayer_ = new MediaPlayer;
}

MediaSurface::~MediaSurface()
{
    delete mediaplayer_;
}

void MediaSurface::init()
{
    Surface::init();

    mediaplayer_->open(uri_);
    mediaplayer_->play(true);
}

void MediaSurface::draw(glm::mat4 modelview, glm::mat4 projection)
{
    if ( !initialized() )
        init();

    if ( mediaplayer_->isOpen() )
        glBindTexture(GL_TEXTURE_2D, mediaplayer_->texture());
    else
        glBindTexture(GL_TEXTURE_2D, Resource::getTextureBlack());

    Primitive::draw(modelview, projection);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void MediaSurface::update( float dt )
{
    if ( mediaplayer_->isOpen() ) {
        mediaplayer_->update();
        scale_.x = mediaplayer_->aspectRatio();
    }

    Primitive::update( dt );
}

void MediaSurface::accept(Visitor& v)
{
    Primitive::accept(v);
    v.visit(*this);
}


FrameBufferSurface::FrameBufferSurface(FrameBuffer *fb, Shader *s) : Surface(s), frame_buffer_(fb)
{

}

void FrameBufferSurface::init()
{
    Surface::init();

    // set aspect ratio
    scale_.x = frame_buffer_->aspectRatio();

    // a new shader for a new image
    shader_ = new ImageShader();
}

void FrameBufferSurface::draw(glm::mat4 modelview, glm::mat4 projection)
{
    if ( !initialized() )
        init();

    glBindTexture(GL_TEXTURE_2D, frame_buffer_->texture());

    Primitive::draw(modelview, projection);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void FrameBufferSurface::accept(Visitor& v)
{
    Primitive::accept(v);
    v.visit(*this);
}


Points::Points(std::vector<glm::vec3> points, glm::vec4 color, uint pointsize) : Primitive(new Shader)
{
    for(size_t i = 0; i < points.size(); ++i)
    {
        points_.push_back( points[i] );
        colors_.push_back( color );
        indices_.push_back ( i );
    }

    drawMode_ = GL_POINTS;
    pointsize_ = pointsize;
}



void Points::draw(glm::mat4 modelview, glm::mat4 projection)
{
    if ( !initialized() )
        init();

    glPointSize(pointsize_);

    Primitive::draw(modelview, projection);
}

void Points::accept(Visitor& v)
{
    Primitive::accept(v);
    v.visit(*this);
}

LineStrip::LineStrip(std::vector<glm::vec3> points, glm::vec4 color, uint linewidth) : Primitive(new Shader), linewidth_(linewidth)
{
    for(size_t i = 0; i < points.size(); ++i)
    {
        points_.push_back( points[i] );
        colors_.push_back( color );
        indices_.push_back ( i );
    }

    drawMode_ = GL_LINE_STRIP;
}

void LineStrip::draw(glm::mat4 modelview, glm::mat4 projection)
{
    if ( !initialized() )
        init();

    glLineWidth(linewidth_);

    Primitive::draw(modelview, projection);
}

void LineStrip::accept(Visitor& v)
{
    Primitive::accept(v);
    v.visit(*this);
}


LineSquare::LineSquare(glm::vec4 color, uint linewidth) : LineStrip(square_points, color, linewidth)
{
}

void LineSquare::init()
{
    // use static unique vertex array object
    static uint unique_vao_ = 0;
    static uint unique_drawCount = 0;
    if (unique_vao_) {
        // 1. only init Node (not the primitive vao)
        Node::init();
        // 2. use the global vertex array object
        vao_ = unique_vao_;
        drawCount_ = unique_drawCount;
    }
    else {
        // 1. init the Primitive (only once)
        Primitive::init();
        // 2. remember global vertex array object
        unique_vao_ = vao_;
        unique_drawCount = drawCount_;
        // 3. unique_vao_ will NOT be deleted because LineCircle::deleteGLBuffers_() is empty
    }
}

void LineSquare::accept(Visitor& v)
{
    Primitive::accept(v);
    v.visit(*this);
}


LineCircle::LineCircle(glm::vec4 color, uint linewidth) : LineStrip(std::vector<glm::vec3>(), color, linewidth)
{
    static int N = 72;
    static float a =  glm::two_pi<float>() / static_cast<float>(N);
    glm::vec3 P(1.f, 0.f, 0.f);
    for (int i = 0; i < N ; i++ ){
        points_.push_back( glm::vec3(P) );
        colors_.push_back( color );
        indices_.push_back ( i );

        P = glm::rotateZ(P, a);
    }
    // loop
    points_.push_back( glm::vec3(1.f, 0.f, 0.f) );
    colors_.push_back( color );
    indices_.push_back ( N );
}

void LineCircle::init()
{
    // use static unique vertex array object
    static uint unique_vao_ = 0;
    static uint unique_drawCount = 0;
    if (unique_vao_) {
        // 1. only init Node (not the primitive vao)
        Node::init();
        // 2. use the global vertex array object
        vao_ = unique_vao_;
        drawCount_ = unique_drawCount;
    }
    else {
        // 1. init the Primitive (only once)
        Primitive::init();
        // 2. remember global vertex array object
        unique_vao_ = vao_;
        unique_drawCount = drawCount_;
        // 3. unique_vao_ will NOT be deleted because LineCircle::deleteGLBuffers_() is empty
    }
}

void LineCircle::accept(Visitor& v)
{
    Primitive::accept(v);
    v.visit(*this);
}

