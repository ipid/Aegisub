#include "grid_warning_controller.h"
#include "ass_file.h"
#include <wx/event.h>

wxDEFINE_EVENT(EVT_COLLECTION_DONE, wxThreadEvent);

GridWarningController::GridWarningController(agi::Context *context)
: subtitle_commit_connection(context->ass->AddCommitListener(&GridWarningController::OnSubtitlesCommit, this)) {
    // TODO: 看看能做啥
}

void GridWarningController::OnSubtitlesCommit(int type) {
    // TODO: 看看能做啥
}
