// Opengl
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/vector_angle.hpp>

// memmove
#include <string.h>

#include "defines.h"
#include "Settings.h"
#include "View.h"
#include "Source.h"
#include "Primitives.h"
#include "PickingVisitor.h"
#include "Mesh.h"
#include "Mixer.h"
#include "FrameBuffer.h"
#include "UserInterfaceManager.h"
#include "Log.h"

#define CIRCLE_PIXELS 64
#define CIRCLE_PIXEL_RADIUS 1024.0

bool View::need_deep_update_ = true;

View::View(Mode m) : mode_(m)
{
}

void View::restoreSettings()
{
    scene.root()->scale_ = Settings::application.views[mode_].default_scale;
    scene.root()->translation_ = Settings::application.views[mode_].default_translation;
}

void View::saveSettings()
{
    Settings::application.views[mode_].default_scale = scene.root()->scale_;
    Settings::application.views[mode_].default_translation = scene.root()->translation_;
}

void View::draw()
{
    // draw scene of this view
    scene.root()->draw(glm::identity<glm::mat4>(), Rendering::manager().Projection());
}

void View::update(float dt)
{
    // recursive update from root of scene
    scene.update( dt );

    // a more complete update is requested
    if (View::need_deep_update_) {
        // reorder sources
        scene.ws()->sort();
    }
}

View::Cursor View::drag (glm::vec2 from, glm::vec2 to)
{
    static glm::vec3 start_translation = glm::vec3(0.f);
    static glm::vec2 start_position = glm::vec2(0.f);

    if ( start_position != from ) {
        start_position = from;
        start_translation = scene.root()->translation_;
    }

    // unproject
    glm::vec3 gl_Position_from = Rendering::manager().unProject(from);
    glm::vec3 gl_Position_to = Rendering::manager().unProject(to);

    // compute delta translation
    scene.root()->translation_ = start_translation + gl_Position_to - gl_Position_from;

    return Cursor_ResizeAll;
}

MixingView::MixingView() : View(MIXING)
{
    // read default settings
    if ( Settings::application.views[mode_].name.empty() ) {
        // no settings found: store application default
        Settings::application.views[mode_].name = "Mixing";
        scene.root()->scale_ = glm::vec3(2.0f, 2.0f, 1.0f);
        saveSettings();
    }

    // Mixing scene background
    Mesh *disk = new Mesh("mesh/disk.ply");
    disk->setTexture(textureMixingQuadratic());
    scene.bg()->attach(disk);

    glm::vec4 pink( 0.8f, 0.f, 0.8f, 1.f );
    Mesh *circle = new Mesh("mesh/circle.ply");
    circle->shader()->color = pink;
    scene.bg()->attach(circle);
}


void MixingView::zoom( float factor )
{
    float z = scene.root()->scale_.x;
    z = CLAMP( z + 0.1f * factor, 0.2f, 10.f);
    scene.root()->scale_.x = z;
    scene.root()->scale_.y = z;
}

View::Cursor MixingView::grab (glm::vec2 from, glm::vec2 to, Source *s, std::pair<Node *, glm::vec2>)
{
    if (!s)
        return Cursor_Arrow;

    Group *sourceNode = s->group(mode_);

    static glm::vec3 start_translation = glm::vec3(0.f);
    static glm::vec2 start_position = glm::vec2(0.f);

    if ( start_position != from ) {
        start_position = from;
        start_translation = sourceNode->translation_;
    }

    // unproject
    glm::vec3 gl_Position_from = Rendering::manager().unProject(from, scene.root()->transform_);
    glm::vec3 gl_Position_to   = Rendering::manager().unProject(to, scene.root()->transform_);

    // compute delta translation
    sourceNode->translation_ = start_translation + gl_Position_to - gl_Position_from;

    // request update
    s->touch();

    return Cursor_ResizeAll;
}

uint MixingView::textureMixingQuadratic()
{
    static GLuint texid = 0;
    if (texid == 0) {
        // generate the texture with alpha exactly as computed for sources
        glGenTextures(1, &texid);
        glBindTexture(GL_TEXTURE_2D, texid);
        GLubyte matrix[CIRCLE_PIXELS*CIRCLE_PIXELS * 4];
        GLubyte color[4] = {0,0,0,0};
        GLfloat luminance = 1.f;
        GLfloat alpha = 0.f;
        GLfloat distance = 0.f;
        int l = -CIRCLE_PIXELS / 2 + 1, c = 0;

        for (int i = 0; i < CIRCLE_PIXELS / 2; ++i) {
            c = -CIRCLE_PIXELS / 2 + 1;
            for (int j=0; j < CIRCLE_PIXELS / 2; ++j) {
                // distance to the center
                distance = (GLfloat) ((c * c) + (l * l)) / CIRCLE_PIXEL_RADIUS;
//                distance = (GLfloat) sqrt( (GLfloat) ((c * c) + (l * l))) / (GLfloat) sqrt(CIRCLE_PIXEL_RADIUS); // linear
                // luminance
                luminance = 255.f * CLAMP( 0.95f - 0.8f * distance, 0.f, 1.f);
                color[0] = color[1] = color[2] = static_cast<GLubyte>(luminance);
                // alpha
                alpha = 255.f * CLAMP( 1.f - distance , 0.f, 1.f);
                color[3] = static_cast<GLubyte>(alpha);

                // 1st quadrant
                memmove(&matrix[ j * 4 + i * CIRCLE_PIXELS * 4 ], color, 4);
                // 4nd quadrant
                memmove(&matrix[ (CIRCLE_PIXELS -j -1)* 4 + i * CIRCLE_PIXELS * 4 ], color, 4);
                // 3rd quadrant
                memmove(&matrix[ j * 4 + (CIRCLE_PIXELS -i -1) * CIRCLE_PIXELS * 4 ], color, 4);
                // 4th quadrant
                memmove(&matrix[ (CIRCLE_PIXELS -j -1) * 4 + (CIRCLE_PIXELS -i -1) * CIRCLE_PIXELS * 4 ], color, 4);

                ++c;
            }
            ++l;
        }
        // two components texture : luminance and alpha
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, CIRCLE_PIXELS, CIRCLE_PIXELS, 0, GL_RGBA, GL_UNSIGNED_BYTE, (float *) matrix);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    }
    return texid;
}

RenderView::RenderView() : View(RENDERING), frame_buffer_(nullptr)
{
    // set resolution to settings or default
    setResolution();
}

RenderView::~RenderView()
{
    if (frame_buffer_)
        delete frame_buffer_;
}

void RenderView::setResolution(glm::vec3 resolution)
{
    if (resolution.x < 128.f || resolution.y < 128.f)
        resolution = FrameBuffer::getResolutionFromParameters(Settings::application.framebuffer_ar, Settings::application.framebuffer_h);

    if (frame_buffer_)
        delete frame_buffer_;

    frame_buffer_ = new FrameBuffer(resolution);
}

void RenderView::draw()
{
    static glm::mat4 projection = glm::ortho(-1.f, 1.f, 1.f, -1.f, -SCENE_DEPTH, 1.f);
    glm::mat4 P  = glm::scale( projection, glm::vec3(1.f / frame_buffer_->aspectRatio(), 1.f, 1.f));
    frame_buffer_->begin();
    scene.root()->draw(glm::identity<glm::mat4>(), P);
    frame_buffer_->end();
}


GeometryView::GeometryView() : View(GEOMETRY)
{
    // read default settings
    if ( Settings::application.views[mode_].name.empty() ) {
        // no settings found: store application default
        Settings::application.views[mode_].name = "Geometry";
        scene.root()->scale_ = glm::vec3(1.2f, 1.2f, 1.0f);
        saveSettings();
    }

    // Geometry Scene background
    Surface *rect = new Surface;
    scene.bg()->attach(rect);

    Frame *border = new Frame(Frame::SHARP_THIN);
    border->color = glm::vec4( 0.8f, 0.f, 0.8f, 1.f );
    scene.fg()->attach(border);

}

void GeometryView::update(float dt)
{
    View::update(dt);

    // reorder depth if needed
    if (View::need_deep_update_) {

        // update rendering of render frame
        FrameBuffer *output = Mixer::manager().session()->frame();
        if (output){
            for (NodeSet::iterator node = scene.bg()->begin(); node != scene.bg()->end(); node++) {
                (*node)->scale_.x = output->aspectRatio();
            }
            for (NodeSet::iterator node = scene.fg()->begin(); node != scene.fg()->end(); node++) {
                (*node)->scale_.x = output->aspectRatio();
            }
        }
    }
}

void GeometryView::zoom( float factor )
{
    float z = scene.root()->scale_.x;
    z = CLAMP( z + 0.1f * factor, 0.2f, 10.f);
    scene.root()->scale_.x = z;
    scene.root()->scale_.y = z;
}

View::Cursor GeometryView::grab (glm::vec2 from, glm::vec2 to, Source *s, std::pair<Node *, glm::vec2> pick)
{
    View::Cursor ret = Cursor_Arrow;

    // work on the given source
    if (!s)
        return ret;
    Group *sourceNode = s->group(mode_);

    // remember source transform at moment of clic at position 'from'
    static glm::vec2 start_clic_position = glm::vec2(0.f);
    static glm::vec3 start_translation = glm::vec3(0.f);
    static glm::vec3 start_scale = glm::vec3(1.f);
    static glm::vec3 start_rotation = glm::vec3(0.f);
    if ( start_clic_position != from ) {
        start_clic_position = from;
        start_translation = sourceNode->translation_;
        start_scale = sourceNode->scale_;
        start_rotation = sourceNode->rotation_;
    }

    // grab coordinates in scene-View reference frame
    glm::vec3 gl_Position_from = Rendering::manager().unProject(from, scene.root()->transform_);
    glm::vec3 gl_Position_to = Rendering::manager().unProject(to, scene.root()->transform_);

    // grab coordinates in source-root reference frame
    glm::vec4 S_from = glm::inverse(sourceNode->transform_) * glm::vec4( gl_Position_from,  1.f );
    glm::vec4 S_to = glm::inverse(sourceNode->transform_) * glm::vec4( gl_Position_to,  1.f );
    glm::vec3 S_resize = glm::vec3(S_to) / glm::vec3(S_from);

//    Log::Info(" screen coordinates ( %.1f, %.1f ) ", to.x, to.y);
//    Log::Info(" scene  coordinates ( %.1f, %.1f ) ", gl_Position_to.x, gl_Position_to.y);
//    Log::Info(" source coordinates ( %.1f, %.1f, %.1f ) ", S_from.x, S_from.y, S_from.z);
//    Log::Info("                    ( %.1f, %.1f, %.1f ) ", S_to.x, S_to.y, S_to.z);

    // which manipulation to perform?
    if (pick.first)  {
        // picking on the resizing handles in the corners
        if ( pick.first == s->handleNode(Handles::RESIZE) ) {
            if (UserInterface::manager().keyboardModifier())
                S_resize.y = S_resize.x;
            sourceNode->scale_ = start_scale * S_resize;

//            Log::Info(" resize            ( %.1f, %.1f ) ", S_resize.x, S_resize.y);
//            glm::vec3 factor = S_resize * glm::vec3(0.5f, 0.5f, 1.f);
////            glm::vec3 factor = S_resize * glm::vec3(1.f, 1.f, 1.f);
////            factor *= glm::sign( glm::vec3(pick.second, 1.f) );
//            sourceNode->scale_ = start_scale + factor;

//            sourceNode->translation_ = start_translation + factor;
////            sourceNode->translation_ = start_translation + S_resize * factor;

            // select cursor depending on diagonal
            glm::vec2 axis = glm::sign(pick.second);
            ret = axis.x * axis.y > 0.f ? Cursor_ResizeNESW : Cursor_ResizeNWSE;
        }
        // picking on the resizing handles left or right
        else if ( pick.first == s->handleNode(Handles::RESIZE_H) ) {
            sourceNode->scale_ = start_scale * glm::vec3(S_resize.x, 1.f, 1.f);
            ret = Cursor_ResizeEW;
        }
        // picking on the resizing handles top or bottom
        else if ( pick.first == s->handleNode(Handles::RESIZE_V) ) {
            sourceNode->scale_ = start_scale * glm::vec3(1.f, S_resize.y, 1.f);
            ret = Cursor_ResizeNS;
        }
        // picking on the rotating handle
        else if ( pick.first == s->handleNode(Handles::ROTATE) ) {
            float angle = glm::orientedAngle( glm::normalize(glm::vec2(S_from)), glm::normalize(glm::vec2(S_to)));

            if (UserInterface::manager().keyboardModifier())
                angle = float ( int(angle * 30.f) ) / 30.f;
            sourceNode->rotation_ = start_rotation + glm::vec3(0.f, 0.f, angle);
            ret = Cursor_Hand;
        }
        // picking anywhere but on a handle: user wants to move the source
        else {
            sourceNode->translation_ = start_translation + gl_Position_to - gl_Position_from;
            ret = Cursor_ResizeAll;
        }
    }
    // don't have a handle, we can only move the source
    else {
        sourceNode->translation_ = start_translation + gl_Position_to - gl_Position_from;
        ret = Cursor_ResizeAll;
    }

    // request update
    s->touch();

    return ret;
}

LayerView::LayerView() : View(LAYER), aspect_ratio(1.f)
{
    // read default settings
    if ( Settings::application.views[mode_].name.empty() ) {
        // no settings found: store application default
        Settings::application.views[mode_].name = "Layer";
        scene.root()->scale_ = glm::vec3(0.8f, 0.8f, 1.0f);
        scene.root()->translation_ = glm::vec3(1.3f, 0.5f, 0.0f);
        saveSettings();
    }

    // Geometry Scene background
    Surface *rect = new Surface;
    rect->shader()->color.a = 0.3f;
    scene.bg()->attach(rect);

    Mesh *persp = new Mesh("mesh/perspective_layer.ply");
    persp->translation_.z = -0.1f;
    scene.bg()->attach(persp);

    Frame *border = new Frame(Frame::ROUND_SHADOW);
    border->color = glm::vec4( 0.8f, 0.f, 0.8f, 0.7f );
    scene.bg()->attach(border);
}

void LayerView::update(float dt)
{
    View::update(dt);

    // reorder depth if needed
    if (View::need_deep_update_) {

        // update rendering of render frame
        FrameBuffer *output = Mixer::manager().session()->frame();
        if (output){
            aspect_ratio = output->aspectRatio();
            for (NodeSet::iterator node = scene.bg()->begin(); node != scene.bg()->end(); node++) {
                (*node)->scale_.x = aspect_ratio;
            }
            for (NodeSet::iterator node = scene.ws()->begin(); node != scene.ws()->end(); node++) {
                (*node)->translation_.y = (*node)->translation_.x / aspect_ratio;
            }
        }
    }
}

void LayerView::zoom (float factor)
{
    float z = scene.root()->scale_.x;
    z = CLAMP( z + 0.1f * factor, 0.2f, 10.f);
    scene.root()->scale_.x = z;
    scene.root()->scale_.y = z;
}


void LayerView::setDepth (Source *s, float d)
{
    if (!s)
        return;

    float depth = d;

    // negative depth given; find the front most depth
    if ( depth < 0.f ) {
        Node *front = scene.ws()->front();
        if (front)
            depth = front->translation_.z + 0.5f;
        else
            depth = 0.5f;
    }

    // move the layer node of the source
    Group *sourceNode = s->group(mode_);

    // diagonal movement only
    sourceNode->translation_.x = CLAMP( -depth, -(SCENE_DEPTH - 2.f), 0.f);
    sourceNode->translation_.y = sourceNode->translation_.x / aspect_ratio;

    // change depth
    sourceNode->translation_.z = -sourceNode->translation_.x;

    // request update
    s->touch();

    // request reordering
    View::need_deep_update_ = true;
}

View::Cursor LayerView::grab (glm::vec2 from, glm::vec2 to, Source *s, std::pair<Node *, glm::vec2> pick)
{
    if (!s)
        return Cursor_Arrow;

    static glm::vec3 start_translation = glm::vec3(0.f);
    static glm::vec2 start_position = glm::vec2(0.f);

    if ( start_position != from ) {
        start_position = from;
        start_translation = s->group(mode_)->translation_;
    }

    // unproject
    glm::vec3 gl_Position_from = Rendering::manager().unProject(from, scene.root()->transform_);
    glm::vec3 gl_Position_to   = Rendering::manager().unProject(to, scene.root()->transform_);

    // compute delta translation
    glm::vec3 dest_translation = start_translation + gl_Position_to - gl_Position_from;

    // apply change
    setDepth( s,  MAX( -dest_translation.x, 0.f) );

    return Cursor_ResizeAll;
}

