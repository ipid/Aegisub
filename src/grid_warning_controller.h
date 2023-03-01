#include <vector>
#include <libaegisub/signal.h>

namespace agi { struct Context; }

class AssDialogue;

class GridWarningController {
public:
    // Bool argument indicates that whether the warning should be shown
    agi::signal::Signal<int> AnnounceGridWarningChanged;

    GridWarningController(agi::Context *context);
    void OnSubtitlesCommit(int type);

    int GetWarningNum();
    std::vector<AssDialogue*> GetOverlappedEvents();

	DEFINE_SIGNAL_ADDERS(AnnounceGridWarningChanged, AddGridWarningChangeListener)

private:
    agi::Context *context;
    agi::signal::Connection subtitle_commit_connection;

    int warning_num = 0;
};
