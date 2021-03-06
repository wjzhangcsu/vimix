#include <glm/gtc/matrix_transform.hpp>

#include "MediaSource.h"

#include "defines.h"
#include "ImageShader.h"
#include "ImageProcessingShader.h"
#include "Resource.h"
#include "Primitives.h"
#include "MediaPlayer.h"
#include "Visitor.h"
#include "Log.h"

MediaSource::MediaSource() : Source(), path_("")
{
    // create media player
    mediaplayer_ = new MediaPlayer;

    // create media surface:
    // - textured with original texture from media player
    // - crop & repeat UV can be managed here
    // - additional custom shader can be associated
    mediasurface_ = new Surface(rendershader_);

}

MediaSource::~MediaSource()
{
    // delete media surface & player
    delete mediasurface_;
    delete mediaplayer_;
}

void MediaSource::setPath(const std::string &p)
{
    path_ = p;
    mediaplayer_->open(path_);
    mediaplayer_->play(true);

    Log::Notify("Opening %s", p.c_str());
}

std::string MediaSource::path() const
{
    return path_;
}

MediaPlayer *MediaSource::mediaplayer() const
{
    return mediaplayer_;
}

bool MediaSource::failed() const
{
    return mediaplayer_->failed();
}

uint MediaSource::texture() const
{
    return mediaplayer_->texture();
}

void MediaSource::init()
{
    if ( mediaplayer_->isOpen() ) {

        // update video
        mediaplayer_->update();

        // once the texture of media player is created
        if (mediaplayer_->texture() != Resource::getTextureBlack()) {

            // get the texture index from media player, apply it to the media surface
            mediasurface_->setTextureIndex( mediaplayer_->texture() );

            // create Frame buffer matching size of media player
            float height = float(mediaplayer()->width()) / mediaplayer()->aspectRatio();
            FrameBuffer *renderbuffer = new FrameBuffer(mediaplayer()->width(), (uint)height, true);

            // set the renderbuffer of the source and attach rendering nodes
            attach(renderbuffer);

            // icon in mixing view
            if (mediaplayer_->duration() == GST_CLOCK_TIME_NONE)
                overlays_[View::MIXING]->attach( new Mesh("mesh/icon_image.ply") );
            else
                overlays_[View::MIXING]->attach( new Mesh("mesh/icon_video.ply") );

            // done init
            initialized_ = true;
        }
    }

}

void MediaSource::render()
{
    if (!initialized_)
        init();
    else {
        // update video
        mediaplayer_->update();

        // render the media player into frame buffer
        static glm::mat4 projection = glm::ortho(-1.f, 1.f, 1.f, -1.f, -1.f, 1.f);
        renderbuffer_->begin();
        mediasurface_->draw(glm::identity<glm::mat4>(), projection);
        renderbuffer_->end();
    }
}

void MediaSource::accept(Visitor& v)
{
    Source::accept(v);
    v.visit(*this);
}
