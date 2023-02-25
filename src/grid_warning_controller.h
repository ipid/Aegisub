#include <libaegisub/signal.h>
#include "include/aegisub/context.h"

class GridWarningController {
public:
    GridWarningController(agi::Context *context);
    void OnSubtitlesCommit(int type);

private:
    agi::signal::Connection subtitle_commit_connection;
};
