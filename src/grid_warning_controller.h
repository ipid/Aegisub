#include <vector>
#include <unordered_map>
#include <libaegisub/signal.h>

namespace agi { struct Context; }
class AssDialogue;

struct DialogueProperties
{
    const AssDialogue *original = nullptr;

    bool considered = true;
    int align = -1;
    bool hasMove = false;
    float posX = -1.0, posY = -1.0;
};

struct DialogueStartEndCache {
    int start = -1, end = -1;

    DialogueStartEndCache(int start, int end) : start(start), end(end) {}
};

class GridWarningController {
public:
    GridWarningController(agi::Context *context);
    void OnSubtitlesCommit(int type, const AssDialogue *line);

    int GetWarningNum();
    std::vector<const AssDialogue*> GetOverlappedEvents();

	DEFINE_SIGNAL_ADDERS(AnnounceGridWarningChanged, AddGridWarningChangeListener)

private:
    // Bool argument indicates that whether the warning should be shown
    agi::signal::Signal<int> AnnounceGridWarningChanged;

    // Caches
    std::vector<DialogueProperties> eventProps;
    std::unordered_map<const AssDialogue*, size_t> dialogueIndexCache;

    agi::Context *context;
    agi::signal::Connection subtitle_commit_connection;

    int warning_num = 0;
};
