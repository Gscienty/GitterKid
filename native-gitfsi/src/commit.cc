#include "commit.h"
#include <algorithm>

namespace gitterKid {
    namespace fsi {
        commit::commit(commitBody &body, std::vector<byte>::iterator spliter, std::vector<byte>::iterator end)
            : body(body) {
            for (std::vector<byte>::iterator ch = spliter; ch != end;) {
                std::vector<byte>::iterator endLineIter = std::find(ch, end, (byte) '\n');

                if (endLineIter == ch) {
                    std::string message;
                    message.insert(message.begin(), ch + 1, end);
                    this->body.setMessage(message);
                    break;
                }

                std::vector<byte>::iterator spaceIter = std::find(ch, end, (byte) ' ');
                std::string tag;
                tag.insert(tag.begin(), ch, spaceIter);

                if (tag.compare("tree") == 0) {
                    std::string treeSign;
                    treeSign.insert(treeSign.begin(), spaceIter + 1, endLineIter);
                    this->body.setTreeSignture(treeSign);
                }
                else if (tag.compare("author") == 0) {
                    this->body.setAuthor(this->parsePersonLog(spaceIter + 1, endLineIter));
                }
                else if (tag.compare("committer") == 0) {
                    this->body.setCommitter(this->parsePersonLog(spaceIter + 1, endLineIter));
                }
                else if (tag.compare("parent") == 0) {
                    std::string parent;
                    parent.insert(parent.begin(), spaceIter + 1, endLineIter);
                    this->body.addParent(parent);
                }

                ch = endLineIter + 1;
            }
        }

        commit::commit(const content &origin) : body((*((commit *) &origin)).body) { }
    }
}