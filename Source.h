#ifndef SOURCE_H
#define SOURCE_H

#include <string>
#include <map>
#include <list>

#include "View.h"
#include "Mesh.h"

class ImageShader;
class ImageProcessingShader;
class FrameBuffer;
class FrameBufferSurface;
class MediaPlayer;
class Surface;
class Session;
class Frame;
class Source;
class CloneSource;

typedef std::list<Source *> SourceList;
typedef std::list<CloneSource *> CloneList;

class Source
{
public:
    // create a source and add it to the list
    // only subclasses of sources can actually be instanciated
    Source();
    virtual ~Source();

    // manipulate name of source
    void setName (const std::string &name);
    inline std::string name () const { return name_; }
    inline const char *initials() const { return initials_; }

    // cloning mechanism
    virtual CloneSource *clone();

    // make visible or hidden
    void setVisible(bool on);

    // an overlay can be displayed on top of the source
    void setOverlayVisible(bool on);

    // get handle on the node used to manipulate the source in a view
    inline Group *group(View::Mode m) const { return groups_.at(m); }
    inline Node *groupNode(View::Mode m) const { return static_cast<Node*>(groups_.at(m)); }
    Handles *handleNode(Handles::Type t) const;

    // a Source has a shader to control image processing effects
    inline ImageProcessingShader *processingShader() const { return rendershader_; }

    // a Source has a shader to control mixing effects
    inline ImageShader *blendingShader() const { return blendingshader_; }

    // every Source has a frame buffer from the renderbuffer
    virtual FrameBuffer *frame() const;

    // touch to request update
    inline void touch() { need_update_ = true; }

    // a Source shall be updated before displayed (Mixing, Geometry and Layer)
    void update (float dt);

    // accept all kind of visitors
    virtual void accept (Visitor& v);

    // a Source shall informs if the source failed (i.e. shall be deleted)
    virtual bool failed() const = 0;

    // a Source shall define a way to get a texture
    virtual uint texture() const = 0;

    // a Source shall define how to render into the frame buffer
    virtual void render() = 0;

protected:
    // name
    std::string name_;
    char initials_[3];

    // every Source shall be initialized on first draw
    bool initialized_;
    virtual void init() = 0;

    // nodes
    std::map<View::Mode, Group*> groups_;

    // render() fills in the renderbuffer at every frame
    // NB: rendershader_ is applied at render()
    FrameBuffer *renderbuffer_;
    void attach(FrameBuffer *renderbuffer);

    // the rendersurface draws the renderbuffer in the scene
    // It is associated to the rendershader for mixing effects
    FrameBufferSurface *rendersurface_;

    // rendershader performs image processing
    ImageProcessingShader *rendershader_;

    // blendingshader provides mixing controls
    ImageShader *blendingshader_;

    // overlay to be displayed on top of source
    std::map<View::Mode, Group*> overlays_;
    Handles *resize_handle_, *resize_H_handle_, *resize_V_handle_, *rotate_handle_;

    // update
    bool need_update_;
    float dt_;

    // clones
    CloneList clones_;
};

struct hasName: public std::unary_function<Source*, bool>
{
    inline bool operator()(const Source* elem) const {
       return (elem && elem->name() == _n);
    }
    hasName(std::string n) : _n(n) { }

private:
    std::string _n;
};

struct hasNode: public std::unary_function<Source*, bool>
{
    bool operator()(const Source* elem) const;
    hasNode(Node *n) : _n(n) { }

private:
    Node *_n;
};


class CloneSource : public Source
{
    friend class Source;

public:
    ~CloneSource();

    // implementation of source API
    void render() override;
    uint texture() const override { return origin_->texture(); }
    bool failed() const override  { return origin_ == nullptr; }
    void accept (Visitor& v) override;

    CloneSource *clone() override;
    inline Source *origin() const { return origin_; }

protected:
    // only Source class can create new CloneSource via clone();
    CloneSource(Source *origin);

    void init() override;
    Surface *clonesurface_;
    Source *origin_;
};


#endif // SOURCE_H
