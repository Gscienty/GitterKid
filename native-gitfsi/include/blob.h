#ifndef _GIT_FSI_BLOB_
#define _GIT_FSI_BLOB_

#include "define.h"
#include "content.h"
#include <string>

namespace gitter_kid {
namespace fsi {

class blob : public content {
private:
    std::basic_string<byte> _body;
public:
    virtual gitter_kid::fsi::obj_type type() const override;
    blob(std::basic_string<byte> &body);
    std::basic_string<byte> &body();
};

}
}

#endif
