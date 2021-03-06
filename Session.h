#ifndef SESSION_H
#define SESSION_H


#include "View.h"
#include "Source.h"

class Session
{
public:
    Session();
    ~Session();

    // add given source into the session
    SourceList::iterator addSource (Source *s);

    // delete the source s from the session
    SourceList::iterator deleteSource (Source *s);

    // get ptr to front most source and remove it from the session
    // Does not delete the source
    Source *popSource();

    // management of list of sources
    bool empty() const;
    uint numSource() const;
    SourceList::iterator begin ();
    SourceList::iterator end ();
    SourceList::iterator find (int index);
    SourceList::iterator find (Source *s);
    SourceList::iterator find (std::string name);
    SourceList::iterator find (Node *node);
    int index (SourceList::iterator it) const;

    // update all sources and mark sources which failed
    void update (float dt);

    // return the last source which failed
    Source *failedSource() { return failedSource_; }

    // get frame result of render
    inline FrameBuffer *frame () const { return render_.frame(); }

    // configure rendering resolution
    void setResolution(glm::vec3 resolution);

    // configuration for group nodes of views
    inline Group *config (View::Mode m) const { return config_.at(m); }

    // name of file containing this session (for transfer)
    void setFilename(const std::string &filename) { filename_ = filename; }
    std::string filename() const { return filename_; }

protected:
    RenderView render_;
    Source *failedSource_;
    SourceList sources_;
    std::string filename_;
    std::map<View::Mode, Group*> config_;
};

#endif // SESSION_H
